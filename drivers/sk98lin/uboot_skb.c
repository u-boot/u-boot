/*
 * Definitions for the 'struct sk_buff' memory handlers in U-Boot.
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>

#ifdef CONFIG_SK98

#include <common.h>
#include "u-boot_compat.h"

#define MAX_SKB		50

static struct sk_buff *sk_table[MAX_SKB];


struct sk_buff * alloc_skb(u32 size, int dummy)
{
	int i;
	struct sk_buff * ret = NULL;

	for (i = 0; i < MAX_SKB; i++)
	{
		if (sk_table[i])
		{
				/* Already allocated.
				 */
			continue;
		}

		sk_table[i] = malloc(sizeof(struct sk_buff));
		if (! sk_table[i])
		{
			printf("alloc_skb: malloc failed\n");
			break;
		}

		memset(sk_table[i], 0, sizeof(struct sk_buff));
		sk_table[i]->data = sk_table[i]->data_unaligned =
		                                            malloc(size + 16);
		if (! sk_table[i]->data)
		{
			printf("alloc_skb: malloc failed\n");
			free(sk_table[i]);
			sk_table[i] = NULL;
			break;
		}

		sk_table[i]->data += 16 - ((u32)sk_table[i]->data & 15);
		sk_table[i]->len = size;

		break;
	}

	if (i < MAX_SKB)
	{
		ret = sk_table[i];
	}

	if (! ret)
	{
		printf("Unable to allocate skb!\n");
	}

	return ret;
}

void dev_kfree_skb_any(struct sk_buff *skb)
{
	int i;

	for (i = 0; i < MAX_SKB; i++)
	{
		if (sk_table[i] != skb)
		{
			continue;
		}

		free(skb->data_unaligned);
		free(skb);
		sk_table[i] = NULL;
		break;
	}

	if (i == MAX_SKB)
	{
		printf("SKB allocation error!\n");
	}
}

void skb_reserve(struct sk_buff *skb, unsigned int len)
{
	skb->data+=len;
}

void skb_put(struct sk_buff *skb, unsigned int len)
{
	skb->len+=len;
}

#endif /* CONFIG_SK98 */
