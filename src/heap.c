/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aasylbye <aasylbye@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 17:37:15 by aasylbye          #+#    #+#             */
/*   Updated: 2026/06/12 18:25:14 by aasylbye         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

int	heap_peek(t_heap *h, t_waiter *out)
{
	if (h->size == 0)
		return (0);
	*out = h->items[0];
	return (1);
}

/* Safe to call on a zero-initialised heap (cleanup after a failed init). */
void	heap_destroy(t_heap *h)
{
	if (h->items)
		free(h->items);
	h->items = NULL;
	h->size = 0;
	h->capacity = 0;
}
