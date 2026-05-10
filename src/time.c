/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aminoqr <aminoqr@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aminoqr           #+#    #+#             */
/*   Updated: 2026/05/09 17:37:15 by aminoqr          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* [17] Wall-clock millisecond timestamp built from gettimeofday(). */
long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000L + (long)tv.tv_usec / 1000L);
}

/* [18] Milliseconds elapsed since `start_ms`. */
long	elapsed_ms(long start_ms)
{
	return (now_ms() - start_ms);
}

/* [19] Sleep `duration_ms` in 50 ms slices, re-checking sim_should_stop    */
/*      between slices so the simulation can shut down promptly.            */
void	precise_sleep_ms(t_sim *sim, long duration_ms)
{
	long	deadline;
	long	remaining;

	if (duration_ms <= 0)
		return ;
	deadline = now_ms() + duration_ms;
	while (!sim_should_stop(sim))
	{
		remaining = deadline - now_ms();
		if (remaining <= 0)
			return ;
		if (remaining > 50)
			usleep(50000);
		else
			usleep((useconds_t)(remaining * 1000));
	}
}
