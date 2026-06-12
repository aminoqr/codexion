/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:25:23 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* fflush under the lock pushes the line out of stdio buffering so the      */
/* burnout line meets the 10 ms latency budget.                            */
static void	log_line(t_sim *sim, int coder_id, const char *msg)
{
	long	ts;

	pthread_mutex_lock(&sim->log_lock);
	ts = elapsed_ms(sim->start_ms);
	printf("%ld %d %s\n", ts, coder_id, msg);
	fflush(stdout);
	pthread_mutex_unlock(&sim->log_lock);
}

/* Suppressed after stop so no state line ever follows the burnout line.    */
void	log_state(t_sim *sim, int coder_id, const char *msg)
{
	if (sim_should_stop(sim))
		return ;
	log_line(sim, coder_id, msg);
}

/* Always printed (never suppressed), even as stop is being set.            */
void	log_burnout(t_sim *sim, int coder_id)
{
	log_line(sim, coder_id, "burned out");
}
