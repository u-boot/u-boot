/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
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

#ifndef _U_BOOT_NDS32_H_
#define _U_BOOT_NDS32_H_	1

/* for the following variables, see start.S */
extern ulong __bss_start;	/* BSS start relative to _start */
extern ulong __bss_end__;	/* BSS end relative to _start */
extern ulong _end;		/* end of image relative to _start */
extern ulong _start;		/* start of image relative to _start */
extern ulong _TEXT_BASE;	/* code start */
extern ulong IRQ_STACK_START;	/* top of IRQ stack */
extern ulong FIQ_STACK_START;	/* top of FIQ stack */

/* cpu/.../cpu.c */
int	cleanup_before_linux(void);

/* board/.../... */
int	board_init(void);
int	dram_init(void);

/* cpu/.../interrupt.c */
void	reset_timer_masked(void);

#endif	/* _U_BOOT_NDS32_H_ */
