/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:25:03 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Read under state_lock to avoid a torn 64-bit read of the timestamp.      */
static long	compute_deadline(t_sim *sim, int coder_id)
{
	t_coder	*c;
	long	value;

	c = &sim->coders[coder_id - 1];
	pthread_mutex_lock(&c->state_lock);
	value = c->last_compile_start_ms + sim->cfg.time_to_burnout;
	pthread_mutex_unlock(&c->state_lock);
	return (value);
}

static int	dongle_can_grant(t_dongle *d, int coder_id)
{
	t_waiter	top;

	if (d->owner != 0)
		return (0);
	if (now_ms() < d->cooldown_until_ms)
		return (0);
	if (!heap_peek(&d->waiters, &top))
		return (0);
	return (top.coder_id == coder_id);
}

/* Wake on a release broadcast or after a 10 ms backstop, so a delayed or   */
/* missed broadcast can never block a waiter indefinitely.                  */
static void	dongle_wait_step(t_dongle *d)
{
	struct timespec	ts;
	long			now;
	long			target_ms;

	now = now_ms();
	target_ms = d->cooldown_until_ms;
	if (target_ms <= now + 10)
		target_ms = now + 10;
	ts.tv_sec = target_ms / 1000;
	ts.tv_nsec = (target_ms % 1000) * 1000000L;
	pthread_cond_timedwait(&d->cond, &d->lock, &ts);
}

/* Returns 1 when the dongle was taken, 0 if cancelled by stop. */
int	dongle_acquire(t_sim *sim, t_dongle *d, int coder_id)
{
	long	seq;
	long	deadline;

	pthread_mutex_lock(&d->lock);
	seq = d->next_seq++;
	deadline = compute_deadline(sim, coder_id);
	heap_push(&d->waiters, coder_id, seq, deadline);
	while (!sim_should_stop(sim) && !dongle_can_grant(d, coder_id))
		dongle_wait_step(d);
	if (sim_should_stop(sim))
	{
		pthread_mutex_unlock(&d->lock);
		return (0);
	}
	heap_pop(&d->waiters);
	d->owner = coder_id;
	pthread_mutex_unlock(&d->lock);
	log_state(sim, coder_id, "has taken a dongle");
	return (1);
}

/* Broadcast (not signal) so every waiter re-checks: only the heap top can  */
/* actually be granted, the rest go back to waiting.                        */
void	dongle_release(t_sim *sim, t_dongle *d, int coder_id)
{
	(void)coder_id;
	pthread_mutex_lock(&d->lock);
	d->owner = 0;
	d->cooldown_until_ms = now_ms() + sim->cfg.dongle_cooldown;
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->lock);
}
