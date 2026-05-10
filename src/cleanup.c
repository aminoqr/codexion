/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aminoqr <aminoqr@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aminoqr           #+#    #+#             */
/*   Updated: 2026/05/09 17:37:15 by aminoqr          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* [50] Destroy every dongle's mutex / condvar / heap. */
static void	cleanup_dongles(t_sim *sim)
{
	int	i;

	if (!sim->dongles)
		return ;
	i = 0;
	while (i < sim->cfg.num_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].lock);
		pthread_cond_destroy(&sim->dongles[i].cond);
		heap_destroy(&sim->dongles[i].waiters);
		i++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
}

/* [51] Destroy every coder's state lock. */
static void	cleanup_coders(t_sim *sim)
{
	int	i;

	if (!sim->coders)
		return ;
	i = 0;
	while (i < sim->cfg.num_coders)
	{
		pthread_mutex_destroy(&sim->coders[i].state_lock);
		i++;
	}
	free(sim->coders);
	sim->coders = NULL;
}

/* [52] Destroy global locks created by init_sync. */
static void	cleanup_sync(t_sim *sim)
{
	pthread_mutex_destroy(&sim->stop_lock);
	pthread_mutex_destroy(&sim->finished_lock);
	pthread_mutex_destroy(&sim->log_lock);
}

/* [53] Top-level cleanup invoked from main() after threads have joined. */
void	sim_cleanup(t_sim *sim)
{
	cleanup_dongles(sim);
	cleanup_coders(sim);
	cleanup_sync(sim);
}
