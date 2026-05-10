/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aminoqr <aminoqr@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aminoqr           #+#    #+#             */
/*   Updated: 2026/05/09 17:37:15 by aminoqr          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* [23] Allocate the items array and remember the comparator policy. */
int	heap_init(t_heap *h, int capacity, t_scheduler policy)
{
	if (capacity < 1)
		capacity = 1;
	h->items = (t_waiter *)malloc(sizeof(t_waiter) * (size_t)capacity);
	if (!h->items)
		return (1);
	h->size = 0;
	h->capacity = capacity;
	h->policy = policy;
	return (0);
}

/* [24] Look at the highest-priority waiter without removing it. */
int	heap_peek(t_heap *h, t_waiter *out)
{
	if (h->size == 0)
		return (0);
	*out = h->items[0];
	return (1);
}

/* [25] Free the heap storage. Safe on a zero-initialised heap. */
void	heap_destroy(t_heap *h)
{
	if (h->items)
		free(h->items);
	h->items = NULL;
	h->size = 0;
	h->capacity = 0;
}
