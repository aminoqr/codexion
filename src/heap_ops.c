/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_ops.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:25:15 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* EDF compares deadlines first; FIFO never does. arrival_seq then coder_id */
/* are deterministic tie-breakers so ordering is never ambiguous.           */
static int	waiter_less(const t_waiter *a, const t_waiter *b, t_scheduler p)
{
	if (p == SCHED_EDF_POLICY && a->deadline_ms != b->deadline_ms)
		return (a->deadline_ms < b->deadline_ms);
	if (a->arrival_seq != b->arrival_seq)
		return (a->arrival_seq < b->arrival_seq);
	return (a->coder_id < b->coder_id);
}

static void	sift_up(t_heap *h, int i)
{
	int			parent;
	t_waiter	tmp;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!waiter_less(&h->items[i], &h->items[parent], h->policy))
			break ;
		tmp = h->items[i];
		h->items[i] = h->items[parent];
		h->items[parent] = tmp;
		i = parent;
	}
}

static void	sift_down(t_heap *h, int i)
{
	int			best;
	int			l;
	t_waiter	tmp;

	while (1)
	{
		l = 2 * i + 1;
		if (l >= h->size)
			break ;
		best = l;
		if (l + 1 < h->size
			&& waiter_less(&h->items[l + 1], &h->items[l], h->policy))
			best = l + 1;
		if (!waiter_less(&h->items[best], &h->items[i], h->policy))
			break ;
		tmp = h->items[i];
		h->items[i] = h->items[best];
		h->items[best] = tmp;
		i = best;
	}
}

int	heap_push(t_heap *h, int coder_id, long seq, long deadline)
{
	if (h->size >= h->capacity)
		return (1);
	h->items[h->size].coder_id = coder_id;
	h->items[h->size].arrival_seq = seq;
	h->items[h->size].deadline_ms = deadline;
	h->size++;
	sift_up(h, h->size - 1);
	return (0);
}

int	heap_pop(t_heap *h)
{
	if (h->size == 0)
		return (1);
	h->size--;
	if (h->size > 0)
	{
		h->items[0] = h->items[h->size];
		sift_down(h, 0);
	}
	return (0);
}
