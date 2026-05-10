/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aminoqr <aminoqr@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aminoqr           #+#    #+#             */
/*   Updated: 2026/05/09 17:37:15 by aminoqr          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* [46] Return the id of the first burned-out coder, or 0 if none. A coder */
/*      is burned out iff (now - last_compile_start) >= time_to_burnout.   */
static int	burnout_check(t_sim *sim)
{
	int		i;
	long	now;
	long	last;
	long	ttb;

	now = now_ms();
	ttb = sim->cfg.time_to_burnout;
	i = 0;
	while (i < sim->cfg.num_coders)
	{
		pthread_mutex_lock(&sim->coders[i].state_lock);
		last = sim->coders[i].last_compile_start_ms;
		pthread_mutex_unlock(&sim->coders[i].state_lock);
		if (now - last >= ttb)
			return (sim->coders[i].id);
		i++;
	}
	return (0);
}

/* [47] True iff every coder has reached the required compile count. */
static int	all_finished(t_sim *sim)
{
	int	v;

	pthread_mutex_lock(&sim->finished_lock);
	v = (sim->finished_count >= sim->cfg.num_coders);
	pthread_mutex_unlock(&sim->finished_lock);
	return (v);
}

/* [48] Set stop=1 and wake every dongle so threads exit cond_timedwait. */
static void	broadcast_stop(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->stop_lock);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_lock);
	i = 0;
	while (i < sim->cfg.num_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].lock);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].lock);
		i++;
	}
}

/* [49] Monitor loop. 2 ms cadence keeps burnout detection comfortably    */
/*      under the 10 ms log-latency requirement.                          */
void	*monitor_thread(void *arg)
{
	t_sim	*sim;
	int		victim;

	sim = (t_sim *)arg;
	while (!sim_should_stop(sim))
	{
		victim = burnout_check(sim);
		if (victim != 0)
		{
			log_burnout(sim, victim);
			broadcast_stop(sim);
			return (NULL);
		}
		if (all_finished(sim))
		{
			broadcast_stop(sim);
			return (NULL);
		}
		usleep(2000);
	}
	return (NULL);
}
