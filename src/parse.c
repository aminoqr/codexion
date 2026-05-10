/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aminoqr <aminoqr@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aminoqr           #+#    #+#             */
/*   Updated: 2026/05/09 17:37:15 by aminoqr          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* [12] Strict positive-long parser. Rejects empty input, signs, non-digits. */
static int	parse_long_positive(const char *s, long *out)
{
	long	value;
	int		i;

	if (!s || !s[0])
		return (1);
	value = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (1);
		value = value * 10 + (s[i] - '0');
		if (value < 0 || value > 2147483647L)
			return (1);
		i++;
	}
	*out = value;
	return (0);
}

/* [13] Recognise the scheduler keyword (must be exactly "fifo" or "edf"). */
static int	parse_scheduler(const char *s, t_scheduler *out)
{
	if (!s)
		return (1);
	if (strcmp(s, "fifo") == 0)
		return (*out = SCHED_FIFO_POLICY, 0);
	if (strcmp(s, "edf") == 0)
		return (*out = SCHED_EDF_POLICY, 0);
	return (1);
}

/* [14] Reject configurations the simulation cannot honour. */
static int	validate_config(t_config *cfg)
{
	if (cfg->num_coders < 1 || cfg->compiles_required < 1)
		return (1);
	if (cfg->time_to_burnout < 1 || cfg->time_to_compile < 1)
		return (1);
	if (cfg->time_to_debug < 0 || cfg->time_to_refactor < 0)
		return (1);
	if (cfg->dongle_cooldown < 0)
		return (1);
	return (0);
}

/* [15] Parse all 8 mandatory arguments into the config struct. */
int	parse_args(int argc, char **argv, t_config *cfg)
{
	long	v;

	if (argc != 9)
		return (1);
	if (parse_long_positive(argv[1], &v))
		return (1);
	cfg->num_coders = (int)v;
	if (parse_long_positive(argv[2], &cfg->time_to_burnout))
		return (1);
	if (parse_long_positive(argv[3], &cfg->time_to_compile))
		return (1);
	if (parse_long_positive(argv[4], &cfg->time_to_debug))
		return (1);
	if (parse_long_positive(argv[5], &cfg->time_to_refactor))
		return (1);
	if (parse_long_positive(argv[6], &v))
		return (1);
	cfg->compiles_required = (int)v;
	if (parse_long_positive(argv[7], &cfg->dongle_cooldown))
		return (1);
	if (parse_scheduler(argv[8], &cfg->scheduler))
		return (1);
	return (validate_config(cfg));
}

/* [16] User-facing usage string printed on bad input. */
void	print_usage(void)
{
	fprintf(stderr,
		"usage: codexion <number_of_coders> <time_to_burnout> "
		"<time_to_compile> <time_to_debug> <time_to_refactor> "
		"<number_of_compiles_required> <dongle_cooldown> <fifo|edf>\n"
		"  all numbers must be positive integers (>=1, durations in ms)\n");
}
