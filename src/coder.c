/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aminoqr <aminoqr@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aminoqr           #+#    #+#             */
/*   Updated: 2026/05/09 17:37:15 by aminoqr          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* [41] Read the global stop flag under its dedicated mutex. */
int	sim_should_stop(t_sim *sim)
{
	int	v;

	pthread_mutex_lock(&sim->stop_lock);
	v = sim->stop;
	pthread_mutex_unlock(&sim->stop_lock);
	return (v);
}

/* [42] Post-sleep bookkeeping: bump compile_count and the global          */
/*      "finished" tally if this coder just hit compiles_required.         */
static void	compile_finalize(t_sim *sim, t_coder *c)
{
	pthread_mutex_lock(&c->state_lock);
	c->compile_count++;
	pthread_mutex_unlock(&c->state_lock);
	pthread_mutex_lock(&sim->finished_lock);
	if (c->compile_count == sim->cfg.compiles_required)
		sim->finished_count++;
	pthread_mutex_unlock(&sim->finished_lock);
}

/* [43] Compile cycle. Always acquire the lower-id dongle first to break   */
/*      the dining-philosophers circular-wait condition.                   */
static void	coder_compile(t_sim *sim, t_coder *c)
{
	t_dongle	*first;
	t_dongle	*second;
	t_dongle	*tmp;

	first = &sim->dongles[c->id - 1];
	second = &sim->dongles[c->id % sim->cfg.num_coders];
	if (second->id < first->id)
	{
		tmp = first;
		first = second;
		second = tmp;
	}
	if (!dongle_acquire(sim, first, c->id))
		return ;
	if (!dongle_acquire(sim, second, c->id))
		return ((void)dongle_release(sim, first, c->id));
	pthread_mutex_lock(&c->state_lock);
	c->last_compile_start_ms = now_ms();
	pthread_mutex_unlock(&c->state_lock);
	log_state(sim, c->id, "is compiling");
	precise_sleep_ms(sim, sim->cfg.time_to_compile);
	compile_finalize(sim, c);
	dongle_release(sim, second, c->id);
	dongle_release(sim, first, c->id);
}

/* [44] Helper for the debug / refactor phases (log + cooperative sleep). */
static void	coder_phase(t_sim *sim, t_coder *c, const char *msg, long ms)
{
	if (sim_should_stop(sim))
		return ;
	log_state(sim, c->id, msg);
	precise_sleep_ms(sim, ms);
}

/* [45] Coder thread entry. The 1-coder degenerate branch is inlined: the  */
/*      lone coder grabs the only dongle and waits for stop.               */
void	*coder_thread(void *arg)
{
	t_coder	*c;
	t_sim	*sim;

	c = (t_coder *)arg;
	sim = c->sim;
	if (sim->cfg.num_coders == 1)
	{
		if (dongle_acquire(sim, &sim->dongles[0], c->id))
		{
			while (!sim_should_stop(sim))
				usleep(1000);
			dongle_release(sim, &sim->dongles[0], c->id);
		}
		return (NULL);
	}
	while (!sim_should_stop(sim))
	{
		coder_compile(sim, c);
		coder_phase(sim, c, "is debugging", sim->cfg.time_to_debug);
		coder_phase(sim, c, "is refactoring", sim->cfg.time_to_refactor);
	}
	return (NULL);
}
