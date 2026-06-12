/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:25:19 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongle(t_dongle *d, int id, t_scheduler pol, int capacity)
{
	d->id = id;
	d->owner = 0;
	d->cooldown_until_ms = 0;
	d->next_seq = 0;
	if (pthread_mutex_init(&d->lock, NULL) != 0)
		return (1);
	if (pthread_cond_init(&d->cond, NULL) != 0)
		return (1);
	if (heap_init(&d->waiters, capacity, pol) != 0)
		return (1);
	return (0);
}

static int	init_coder(t_coder *c, int id, t_sim *sim)
{
	c->id = id;
	c->compile_count = 0;
	c->last_compile_start_ms = 0;
	c->sim = sim;
	if (pthread_mutex_init(&c->state_lock, NULL) != 0)
		return (1);
	return (0);
}

static int	init_arrays(t_sim *sim)
{
	int	i;

	sim->coders = (t_coder *)malloc(sizeof(t_coder) * sim->cfg.num_coders);
	sim->dongles = (t_dongle *)malloc(sizeof(t_dongle) * sim->cfg.num_coders);
	if (!sim->coders || !sim->dongles)
		return (1);
	memset(sim->coders, 0, sizeof(t_coder) * sim->cfg.num_coders);
	memset(sim->dongles, 0, sizeof(t_dongle) * sim->cfg.num_coders);
	i = 0;
	while (i < sim->cfg.num_coders)
	{
		if (init_dongle(&sim->dongles[i], i + 1, sim->cfg.scheduler,
				sim->cfg.num_coders) != 0)
			return (1);
		if (init_coder(&sim->coders[i], i + 1, sim) != 0)
			return (1);
		i++;
	}
	return (0);
}

static int	init_sync(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_lock, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->finished_lock, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->log_lock, NULL) != 0)
		return (1);
	sim->stop = 0;
	sim->finished_count = 0;
	return (0);
}

int	sim_init(t_sim *sim)
{
	if (init_sync(sim) != 0)
		return (1);
	if (init_arrays(sim) != 0)
		return (1);
	return (0);
}
