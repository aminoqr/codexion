/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:25:30 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Returns the first coder where now - last_compile_start >= ttb, else 0.   */
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

static int	all_finished(t_sim *sim)
{
	int	v;

	pthread_mutex_lock(&sim->finished_lock);
	v = (sim->finished_count >= sim->cfg.num_coders);
	pthread_mutex_unlock(&sim->finished_lock);
	return (v);
}

/* Set stop, then wake every dongle so parked coders leave cond_timedwait.  */
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

/* 2 ms polling keeps burnout detection well under the 10 ms log budget.    */
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
