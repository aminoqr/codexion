/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:24:50 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>

typedef enum e_scheduler
{
	SCHED_FIFO_POLICY = 0,
	SCHED_EDF_POLICY = 1
}	t_scheduler;

/* Read-only after parsing; shared freely without a lock. */
typedef struct s_config
{
	int			num_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	int			compiles_required;
	long		dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

typedef struct s_waiter
{
	int		coder_id;
	long	arrival_seq;
	long	deadline_ms;
}	t_waiter;

typedef struct s_heap
{
	t_waiter	*items;
	int			size;
	int			capacity;
	t_scheduler	policy;
}	t_heap;

/* Forward declaration: t_coder needs to back-point at the simulation state. */
typedef struct s_sim	t_sim;

/* State guarded by `lock`; coders block on `cond` until granted. */
typedef struct s_dongle
{
	int				id;
	int				owner;
	long			cooldown_until_ms;
	long			next_seq;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	t_heap			waiters;
}	t_dongle;

/* last_compile_start_ms is the burnout reference point. */
typedef struct s_coder
{
	int				id;
	int				compile_count;
	long			last_compile_start_ms;
	pthread_mutex_t	state_lock;
	pthread_t		thread;
	t_sim			*sim;
}	t_coder;

struct s_sim
{
	t_config		cfg;
	long			start_ms;
	int				stop;
	int				finished_count;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	finished_lock;
	pthread_mutex_t	log_lock;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor_thread;
};

/* parse.c */
int		parse_args(int argc, char **argv, t_config *cfg);
void	print_usage(void);

/* time.c */
long	now_ms(void);
long	elapsed_ms(long start_ms);
void	precise_sleep_ms(t_sim *sim, long duration_ms);

/* log.c */
void	log_state(t_sim *sim, int coder_id, const char *msg);
void	log_burnout(t_sim *sim, int coder_id);

/* heap.c + heap_ops.c */
int		heap_init(t_heap *h, int capacity, t_scheduler policy);
void	heap_destroy(t_heap *h);
int		heap_push(t_heap *h, int coder_id, long seq, long deadline);
int		heap_pop(t_heap *h);
int		heap_peek(t_heap *h, t_waiter *out);

/* init.c */
int		sim_init(t_sim *sim);

/* dongle.c */
int		dongle_acquire(t_sim *sim, t_dongle *d, int coder_id);
void	dongle_release(t_sim *sim, t_dongle *d, int coder_id);

/* coder.c */
int		sim_should_stop(t_sim *sim);
void	*coder_thread(void *arg);

/* monitor.c */
void	*monitor_thread(void *arg);

/* cleanup.c */
void	sim_cleanup(t_sim *sim);

#endif
