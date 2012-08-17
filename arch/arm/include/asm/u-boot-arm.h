/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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

#ifndef _U_BOOT_ARM_H_
#define _U_BOOT_ARM_H_	1

/* for the following variables, see start.S */
extern ulong _bss_start_ofs;	/* BSS start relative to _start */
extern ulong _bss_end_ofs;		/* BSS end relative to _start */
extern ulong _end_ofs;		/* end of image relative to _start */
extern ulong IRQ_STACK_START;	/* top of IRQ stack */
extern ulong FIQ_STACK_START;	/* top of FIQ stack */
extern ulong _TEXT_BASE;	/* code start */
extern ulong _datarel_start_ofs;
extern ulong _datarelrolocal_start_ofs;
extern ulong _datarellocal_start_ofs;
extern ulong _datarelro_start_ofs;
extern ulong IRQ_STACK_START_IN;	/* 8 bytes in IRQ stack */

/* cpu/.../cpu.c */
int	cpu_init(void);
int	cleanup_before_linux(void);

/* Set up ARMv7 MMU, caches and TLBs */
void	cpu_init_cp15(void);

/* cpu/.../arch/cpu.c */
int	arch_cpu_init(void);
int	arch_misc_init(void);

/* board/.../... */
int	board_init(void);
int	dram_init (void);
void	dram_init_banksize (void);
void	setup_serial_tag (struct tag **params);
void	setup_revision_tag (struct tag **params);

/* cpu/.../interrupt.c */
int	arch_interrupt_init	(void);
void	reset_timer_masked	(void);
ulong	get_timer_masked	(void);
void	udelay_masked		(unsigned long usec);

#endif	/* _U_BOOT_ARM_H_ */
