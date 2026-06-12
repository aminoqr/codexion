*This project has been created as part of the 42 curriculum by aasylbye.*

# Codexion

> Master the race for resources before the deadline masters you.

## Description

**Codexion** is a concurrency simulation in C, modelled on the classic Dining
Philosophers problem with the twist that "philosophers" are coders and the
shared utensils are USB dongles needed to compile quantum code. Every coder
must alternately **compile**, **debug**, and **refactor** without burning out:
if a coder hasn't *started* a new compile within `time_to_burnout`
milliseconds since their last compile (or since the start of the simulation),
they burn out and the run stops.

Compiling requires **two** dongles held simultaneously. Each pair of adjacent
coders shares one dongle; for `N` coders there are exactly `N` dongles
arranged around the table. Two arbitration policies are supported:


| Policy | Behaviour                                                                                                                |
| ------ | ------------------------------------------------------------------------------------------------------------------------ |
| `fifo` | First-In-First-Out: the dongle is granted to the coder whose request arrived first.                                      |
| `edf`  | Earliest-Deadline-First: the dongle is granted to the coder closest to burnout (`last_compile_start + time_to_burnout`). |


The simulation stops in one of two ways:

1. A coder burns out (a `burned out` line is printed within 10 ms of the actual
  burnout time).
2. Every coder has compiled at least `number_of_compiles_required` times.

## Instructions

### Build

```bash
make            # produces ./codexion
make clean      # remove obj/
make fclean     # remove obj/ and the binary
make re         # full rebuild
```

The Makefile compiles with `-Wall -Wextra -Werror -pthread`.

### Run

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
           <time_to_debug>   <time_to_refactor> \
           <number_of_compiles_required> <dongle_cooldown> <fifo|edf>
```

All eight arguments are mandatory. Numerical values must be positive integers
(in milliseconds for durations). Invalid input is rejected with a usage line on
`stderr` and a non-zero exit code.

### Examples

```bash
# 4 coders, FIFO, plenty of slack: should run to completion.
./codexion 4 1000 100 100 100 5 0 fifo

# 5 coders, EDF, with a 20 ms cooldown after each release.
./codexion 5 1500 200 200 200 10 20 edf

# 1 coder + 1 dongle = cannot ever compile, burns out at t=ttb.
./codexion 1 500 100 100 100 5 0 fifo
```

### Sample log

```
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
201 2 has taken a dongle
401 1 is refactoring
402 2 is debugging
...
1505 4 burned out
```

## Reading guide

The codebase is split into small single-responsibility translation units:

| File                 | Topic                                    |
| -------------------- | ---------------------------------------- |
| `include/codexion.h` | Types, enums, public API                 |
| `src/main.c`         | Entry point + thread orchestration       |
| `src/parse.c`        | Argument parsing & validation            |
| `src/time.c`         | Wall-clock helpers, cooperative sleep    |
| `src/log.c`          | Mutex-serialised logging                 |
| `src/heap.c`         | Heap lifecycle                           |
| `src/heap_ops.c`     | Heap push / pop with FIFO+EDF comparator |
| `src/init.c`         | Allocation & sync primitives             |
| `src/dongle.c`       | Dongle acquire / release / arbitration   |
| `src/coder.c`        | Coder thread loop and phases             |
| `src/monitor.c`      | Burnout watchdog thread                  |
| `src/cleanup.c`      | Symmetric tear-down                      |


## Blocking cases handled

Codexion deliberately addresses every classic concurrency hazard the subject
calls out. The defences are summarised below; each refers back to the part of
the codebase that implements it.

- **Mutual exclusion** (Coffman #1).
Each dongle's state is owned by `pthread_mutex_t lock` (`t_dongle.lock`).
The `owner` field, `cooldown_until_ms` timer and the waiter heap can only be
read or written while that mutex is held. No two coders can therefore
observe the same dongle as "free" at the same time.
- **Deadlock prevention via resource ordering** (breaks Coffman #4 — circular
wait). In `coder_compile` (`src/coder.c`) every coder always
acquires the **lower-id** dongle first and the higher-id one second. This
imposes a total ordering on the resources and makes a circular wait
impossible. With `N>=2` coders the classic dining-philosophers deadlock
cannot form.
- **Hold-and-wait stays bounded** (mitigates Coffman #2).
A coder that fails to acquire the second dongle releases the first one
immediately (`(void)dongle_release(...)` in `coder_compile`). The simulation
shutdown path uses the same release-on-failure pattern via
`sim_should_stop` checks inside `dongle_acquire`.
- **No-preemption side-effect** (Coffman #3) is *intentional*: a holder is
never forced to drop a dongle. Liveness is restored entirely by the
ordering above and by fair arbitration below.
- **Starvation prevention** (FIFO and EDF fairness).
Every dongle owns its own priority queue (`t_heap` waiters). On
`dongle_acquire` the requester is pushed with a monotonic
`arrival_seq` and a freshly-computed `deadline_ms`. Only the heap top may
be granted the dongle (`dongle_can_grant` in `src/dongle.c`). Under EDF, the queue
is sorted by deadline; under FIFO, by arrival sequence; both policies
break ties with `arrival_seq` and then `coder_id` for fully deterministic
behaviour.
- **Cooldown handling.**
After every release, `dongle_release` arms `cooldown_until_ms = now_ms() + dongle_cooldown` and broadcasts on the dongle's condition variable.
`dongle_can_grant` refuses to hand the dongle out until that
timestamp is reached, even if a waiter is at the heap top, satisfying the
mandatory cooldown rule.
- **Precise burnout detection.**
A dedicated **monitor thread** (`src/monitor.c`) polls every coder
every 2 ms. Each coder publishes its `last_compile_start_ms` under
`state_lock` whenever it actually starts compiling. As soon as
`now - last_compile_start >= time_to_burnout` the monitor logs the burnout
line and asserts the global stop flag. The 2 ms cadence is well below the
10 ms log-latency tolerance the subject demands.
- **Log serialisation.**
All log lines pass through `log_state` / `log_burnout` (`src/log.c`),
which take `sim->log_lock` before each `printf` and `fflush`
call. As a result no two messages can interleave on the same line.
- **Clean shutdown.**
`broadcast_stop` sets `sim->stop = 1` and broadcasts every
dongle's condvar so blocked coders return from `pthread_cond_timedwait`,
observe stop, and exit `dongle_acquire` without taking the dongle.
`precise_sleep_ms` re-checks `sim_should_stop` between sleep
slices so coders mid-phase also exit promptly.
- **No memory leaks.**
`sim_cleanup` symmetrically tears down every mutex, condition
variable, heap, and array allocated by `sim_init`. Tested under
`valgrind --leak-check=full --show-leak-kinds=all`: 0 bytes in use at
exit, 0 errors.

## Thread synchronization mechanisms

Codexion uses three POSIX primitives, each with a precise purpose:

### `pthread_mutex_t`


| Mutex                 | Defined in                  | Protects                                                        |
| --------------------- | --------------------------- | --------------------------------------------------------------- |
| `t_sim.stop_lock`     | `t_sim`                     | The `sim->stop` flag (read by everyone, written by the monitor) |
| `t_sim.finished_lock` | `t_sim`                     | `sim->finished_count`, the "all coders done" counter            |
| `t_sim.log_lock`      | `t_sim`                     | `printf` / `fflush` so log lines never interleave               |
| `t_dongle.lock`       | `t_dongle` (one per dongle) | `owner`, `cooldown_until_ms`, `next_seq`, `waiters`             |
| `t_coder.state_lock`  | `t_coder` (one per coder)   | `last_compile_start_ms`, `compile_count`                        |


A consistent **lock-acquisition order** (`t_dongle.lock` -> `t_coder.state_lock`)
is enforced everywhere: only `compute_deadline` takes
`state_lock` while already holding a dongle's lock, and nowhere else are the
two ever held together. Combined with the dongle resource ordering inside
`coder_compile`, this yields a global lock graph that contains no cycles ->
no deadlock.

### `pthread_cond_t`

Every dongle owns a condition variable `t_dongle.cond`. It is the mechanism by
which a releaser tells all waiters "something changed, please re-check whether
you can take the dongle":

```c
// dongle_acquire (simplified)
pthread_mutex_lock(&d->lock);
heap_push(&d->waiters, ...);
while (!sim_should_stop(sim) && !dongle_can_grant(d, coder_id))
    dongle_wait_step(d);            // pthread_cond_timedwait
heap_pop(&d->waiters);
d->owner = coder_id;
pthread_mutex_unlock(&d->lock);
```

```c
// dongle_release
pthread_mutex_lock(&d->lock);
d->owner = 0;
d->cooldown_until_ms = now_ms() + sim->cfg.dongle_cooldown;
pthread_cond_broadcast(&d->cond);   // wake every waiter
pthread_mutex_unlock(&d->lock);
```

Two key correctness ingredients:

1. The condition is **always re-checked under the mutex** (the `while` loop in
  `dongle_acquire`), so spurious wake-ups and lost broadcasts are harmless.
2. `pthread_cond_timedwait` uses an absolute deadline derived from
  `cooldown_until_ms`. This guarantees forward progress even if a broadcast
   is somehow missed: the timeout falls back to a periodic poll.

### Custom event-style stop signal

The "monitor -> everyone" notification is implemented as a `(stop_lock, stop)`
pair instead of a dedicated condition variable. Stop is **read**
(`sim_should_stop`) from every long-running spot — the coder main loop,
`coder_phase`, `precise_sleep_ms`, `dongle_acquire` — and **written** exactly
once by `broadcast_stop`. After flipping the flag, the monitor
broadcasts every dongle's condvar so threads currently parked in
`pthread_cond_timedwait` re-evaluate the loop condition immediately and exit.

### Race condition prevention examples

- `**last_compile_start_ms`**: written by a coder under `state_lock`, read by
the monitor under the same lock. Without the lock, the monitor could read a
torn 64-bit value on 32-bit architectures and mis-compute the deadline.
- `**finished_count`**: incremented by `compile_finalize` under
`finished_lock`; read by `all_finished` under the same lock.
Because the increment happens *exactly when* a coder reaches
`compiles_required`, the count is monotone and double-counting is
impossible.
- **Heap waiters**: only manipulated while the dongle's `lock` is held, so
`heap_push` and `heap_pop` are effectively single-threaded with respect to
each individual dongle.
- **Logging**: `log_lock` makes `printf`+`fflush` atomic from the perspective
of other threads. Since both stdout writes for one log entry occur inside
the same critical section, partial / interleaved lines are impossible.

## Resources

### Documentation & references

- POSIX Threads man pages (`man 7 pthreads`, `pthread_mutex_init(3)`,
`pthread_cond_wait(3)`, `pthread_cond_timedwait(3)`, etc.)
- Coffman, E. G., Elphick, M., Shoshani, A. (1971): *System Deadlocks*. The
four conditions for deadlock cited in the Blocking section.
- C. Hoare (1974): *Monitors: An Operating System Structuring Concept* — the
intellectual root of the condition-variable pattern used around dongles.
- The classic Dining Philosophers problem (Dijkstra, 1965) — Codexion is a
refinement of it with cooldowns and pluggable arbitration policies.
- Linux Programmer's Manual: `gettimeofday(2)`, `usleep(3)`, `write(2)`.
- Bryant & O'Hallaron, *Computer Systems: A Programmer's Perspective*,
chapters on concurrent programming.

### Use of AI tooling

AI assistance (Cursor + Claude) was used as a sparring partner for:

- Brainstorming an initial **file layout** that fits inside the 42 norm
(5 functions × 25 lines per file) before any code was written.
- Reviewing the **lock-acquisition order** to validate that the dongle-id
ordering plus the `dongle.lock -> state_lock` invariant indeed eliminate
every cycle in the lock graph.
- Drafting English prose for this README's *Blocking cases* and *Thread
synchronization* sections, which were then trimmed and corrected by hand.
- Sanity-checking the heap comparator's tie-breaker choices against the  
subject's "fully deterministic EDF" requirement.

