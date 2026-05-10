/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aminoqr <aminoqr@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aminoqr           #+#    #+#             */
/*   Updated: 2026/05/09 17:37:15 by aminoqr          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*                                                                            */
/*  Reading guide: numbered comments [NN] mark the order in which the         */
/*  codebase was authored. Read them in ascending order:                      */
/*                                                                            */
/*    [01]..[08]  this header (types + public API)                            */
/*    [09]..[11]  src/main.c                                                  */
/*    [12]..[16]  src/parse.c                                                 */
/*    [17]..[19]  src/time.c                                                  */
/*    [20]..[22]  src/log.c                                                   */
/*    [23]..[25]  src/heap.c                                                  */
/*    [26]..[30]  src/heap_ops.c                                              */
/*    [31]..[35]  src/init.c                                                  */
/*    [36]..[40]  src/dongle.c                                                */
/*    [41]..[45]  src/coder.c                                                 */
/*    [46]..[49]  src/monitor.c                                               */
/*    [50]..[53]  src/cleanup.c                                               */
/*                                                                            */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>

/* [01] Scheduler policy used by every dongle's waiting queue. */
typedef enum e_scheduler
{
	SCHED_FIFO_POLICY = 0,
	SCHED_EDF_POLICY = 1
}	t_scheduler;

/* [02] Read-only configuration parsed once from the command-line. */
typedef struct s_config
{
	int			num_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	int			compiles_required;
	long		dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

/* [03] One entry inside a dongle's priority queue (the "waiter"). */
typedef struct s_waiter
{
	int		coder_id;
	long	arrival_seq;
	long	deadline_ms;
}	t_waiter;

/* [04] Custom binary heap used as priority queue (FIFO or EDF comparator). */
typedef struct s_heap
{
	t_waiter	*items;
	int			size;
	int			capacity;
	t_scheduler	policy;
}	t_heap;

/* Forward declaration: t_coder needs to back-point at the simulation state. */
typedef struct s_sim	t_sim;

/* [05] Dongle resource state, protected by `lock`. Coders wait on `cond`. */
typedef struct s_dongle
{
	int				id;
	int				owner;
	long			cooldown_until_ms;
	long			next_seq;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	t_heap			waiters;
}	t_dongle;

/* [06] Coder state. last_compile_start_ms is the burnout reference point. */
typedef struct s_coder
{
	int				id;
	int				compile_count;
	long			last_compile_start_ms;
	pthread_mutex_t	state_lock;
	pthread_t		thread;
	t_sim			*sim;
}	t_coder;

/* [07] Top-level simulation state, allocated once on main()'s stack. */
struct s_sim
{
	t_config		cfg;
	long			start_ms;
	int				stop;
	int				finished_count;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	finished_lock;
	pthread_mutex_t	log_lock;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor_thread;
};

/* [08] Public API across translation units. */

/* parse.c */
int		parse_args(int argc, char **argv, t_config *cfg);
void	print_usage(void);

/* time.c */
long	now_ms(void);
long	elapsed_ms(long start_ms);
void	precise_sleep_ms(t_sim *sim, long duration_ms);

/* log.c */
void	log_state(t_sim *sim, int coder_id, const char *msg);
void	log_burnout(t_sim *sim, int coder_id);

/* heap.c + heap_ops.c */
int		heap_init(t_heap *h, int capacity, t_scheduler policy);
void	heap_destroy(t_heap *h);
int		heap_push(t_heap *h, int coder_id, long seq, long deadline);
int		heap_pop(t_heap *h);
int		heap_peek(t_heap *h, t_waiter *out);

/* init.c */
int		sim_init(t_sim *sim);

/* dongle.c */
int		dongle_acquire(t_sim *sim, t_dongle *d, int coder_id);
void	dongle_release(t_sim *sim, t_dongle *d, int coder_id);

/* coder.c */
int		sim_should_stop(t_sim *sim);
void	*coder_thread(void *arg);

/* monitor.c */
void	*monitor_thread(void *arg);

/* cleanup.c */
void	sim_cleanup(t_sim *sim);

#endif
