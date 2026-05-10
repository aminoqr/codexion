/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aminoqr <aminoqr@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aminoqr           #+#    #+#             */
/*   Updated: 2026/05/09 17:37:15 by aminoqr          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* [09] Spawn the monitor first, then every coder. Returns 1 on failure.    */
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

/* [10] Join every coder, then the monitor (the monitor exits last).       */
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

/* [10] Boot the simulation. We must seed every coder's last_compile_start  */
/*      BEFORE launching the monitor, otherwise the monitor could observe a */
/*      0-valued field and mis-fire a "burned out" right at startup.        */
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

/* [11] Program entry point: parse, init, run, cleanup. */
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
