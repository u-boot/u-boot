/*
 * (C) Copyright 2000-2004
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 *
 * Atapted for ppc4XX by Denis Peter
 */

#include <common.h>
#include <commproc.h>
#include <asm/io.h>

#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)

#if defined(CONFIG_SYS_POST_WORD_ADDR)
# define _POST_ADDR	((CONFIG_SYS_OCM_DATA_ADDR) + (CONFIG_SYS_POST_WORD_ADDR))
#elif defined(CONFIG_SYS_POST_ALT_WORD_ADDR)
# define _POST_ADDR	(CONFIG_SYS_POST_ALT_WORD_ADDR)
#endif

void post_word_store (ulong a)
{
	volatile void *save_addr = (volatile void *)(_POST_ADDR);

	out_be32(save_addr, a);
}

ulong post_word_load (void)
{
	volatile void *save_addr = (volatile void *)(_POST_ADDR);

	return in_be32(save_addr);
}

#endif	/* CONFIG_POST || CONFIG_LOGBUFFER*/

#ifdef CONFIG_BOOTCOUNT_LIMIT

void bootcount_store (ulong a)
{
	volatile ulong *save_addr =
		(volatile ulong *)(CONFIG_SYS_OCM_DATA_ADDR + CONFIG_SYS_BOOTCOUNT_ADDR);

	save_addr[0] = a;
	save_addr[1] = BOOTCOUNT_MAGIC;
}

ulong bootcount_load (void)
{
	volatile ulong *save_addr =
		(volatile ulong *)(CONFIG_SYS_OCM_DATA_ADDR + CONFIG_SYS_BOOTCOUNT_ADDR);

	if (save_addr[1] != BOOTCOUNT_MAGIC)
		return 0;
	else
		return save_addr[0];
}

#endif /* CONFIG_BOOTCOUNT_LIMIT */
