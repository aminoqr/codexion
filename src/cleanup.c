/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:24:59 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

static void	cleanup_sync(t_sim *sim)
{
	pthread_mutex_destroy(&sim->stop_lock);
	pthread_mutex_destroy(&sim->finished_lock);
	pthread_mutex_destroy(&sim->log_lock);
}

/* Called from main() only after every thread has been joined. */
void	sim_cleanup(t_sim *sim)
{
	cleanup_dongles(sim);
	cleanup_coders(sim);
	cleanup_sync(sim);
}
