/*
 * (C) Copyright 2004, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
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
 */

#include <common.h>

#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)

#if !defined(CONFIG_SYS_NIOS_POST_WORD_ADDR)
#error "*** CONFIG_SYS_NIOS_POST_WORD_ADDR not defined ***"
#endif

void post_word_store (ulong a)
{
	volatile void *save_addr = (void *)(CONFIG_SYS_NIOS_POST_WORD_ADDR);
	*(volatile ulong *) save_addr = a;
}

ulong post_word_load (void)
{
	volatile void *save_addr = (void *)(CONFIG_SYS_NIOS_POST_WORD_ADDR);
	return *(volatile ulong *) save_addr;
}

#endif	/* CONFIG_POST || CONFIG_LOGBUFFER*/
