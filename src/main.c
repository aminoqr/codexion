/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:24:35 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	spawn_threads(t_sim *sim)
{
	int	i;

	if (pthread_create(&sim->monitor_thread, NULL, monitor_thread, sim) != 0)
		return (1);
	i = 0;
	while (i < sim->cfg.num_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_thread, &sim->coders[i]) != 0)
			return (1);
		i++;
	}
	return (0);
}

static void	join_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->cfg.num_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(sim->monitor_thread, NULL);
}

/* Seed every coder's last_compile_start BEFORE launching the monitor,      */
/* otherwise it could see a 0 field and mis-fire a burnout at startup.      */
static int	run_simulation(t_sim *sim)
{
	int	i;

	sim->start_ms = now_ms();
	i = 0;
	while (i < sim->cfg.num_coders)
	{
		sim->coders[i].last_compile_start_ms = sim->start_ms;
		i++;
	}
	if (spawn_threads(sim) != 0)
		return (1);
	join_threads(sim);
	return (0);
}

int	main(int argc, char **argv)
{
	t_sim	sim;
	int		rc;

	memset(&sim, 0, sizeof(t_sim));
	if (parse_args(argc, argv, &sim.cfg) != 0)
	{
		print_usage();
		return (1);
	}
	if (sim_init(&sim) != 0)
	{
		fprintf(stderr, "codexion: failed to initialise simulation\n");
		sim_cleanup(&sim);
		return (1);
	}
	rc = run_simulation(&sim);
	sim_cleanup(&sim);
	return (rc);
}
