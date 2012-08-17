/*
 * Freescale i.MX28 SPL functions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
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

#ifndef	__M28_INIT_H__
#define	__M28_INIT_H__

void early_delay(int delay);

void mx28_power_init(void);

#ifdef	CONFIG_SPL_MX28_PSWITCH_WAIT
void mx28_power_wait_pswitch(void);
#else
static inline void mx28_power_wait_pswitch(void) { }
#endif

void mx28_mem_init(void);

#endif	/* __M28_INIT_H__ */
