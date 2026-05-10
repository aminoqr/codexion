/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aminoqr <aminoqr@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aminoqr           #+#    #+#             */
/*   Updated: 2026/05/09 17:37:15 by aminoqr          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* [20] Print one timestamped line under the log mutex. fflush guarantees   */
/*      the burnout line escapes stdio block-buffering well within 10 ms.   */
static void	log_line(t_sim *sim, int coder_id, const char *msg)
{
	long	ts;

	pthread_mutex_lock(&sim->log_lock);
	ts = elapsed_ms(sim->start_ms);
	printf("%ld %d %s\n", ts, coder_id, msg);
	fflush(stdout);
	pthread_mutex_unlock(&sim->log_lock);
}

/* [21] Standard state log. Suppressed once the simulation has stopped so   */
/*      we never print "is compiling" after the "burned out" line.          */
void	log_state(t_sim *sim, int coder_id, const char *msg)
{
	if (sim_should_stop(sim))
		return ;
	log_line(sim, coder_id, msg);
}

/* [22] Burnout log. Always printed, even while `stop` is being set, so     */
/*      the line escapes within the 10 ms latency budget.                   */
void	log_burnout(t_sim *sim, int coder_id)
{
	log_line(sim, coder_id, "burned out");
}
