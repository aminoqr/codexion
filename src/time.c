/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:25:46 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000L + (long)tv.tv_usec / 1000L);
}

long	elapsed_ms(long start_ms)
{
	return (now_ms() - start_ms);
}

/* Sleep in 50 ms slices, re-checking the stop flag between slices so the   */
/* coder shuts down promptly instead of sleeping the whole duration.        */
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
