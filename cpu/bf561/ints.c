/*
 * U-boot - ints.c Interrupt related routines
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * This file is based on ints.c
 *
 * Apr18 2003, Changed by HuTao to support interrupt cascading for Blackfin
 *	drivers
 *
 * Copyright 1996 Roman Zippel
 * Copyright 1999 D. Jeff Dionne <jeff@uclinux.org>
 * Copyright 2000-2001 Lineo, Inc. D. Jefff Dionne <jeff@lineo.ca>
 * Copyright 2002 Arcturus Networks Inc. MaTed <mated@sympatico.ca>
 * Copyright 2003 Metrowerks/Motorola
 *
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <linux/stddef.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/traps.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/machdep.h>
#include <asm/setup.h>
#include <asm/blackfin.h>
#include "cpu.h"

void blackfin_irq_panic(int reason, struct pt_regs *regs)
{
	printf("\n\nException: IRQ 0x%x entered\n", reason);
	printf("code=[0x%x], ", (unsigned int)(regs->seqstat & 0x3f));
	printf("stack frame=0x%x, ", (unsigned int)regs);
	printf("bad PC=0x%04x\n", (unsigned int)regs->pc);
	dump(regs);
	printf("Unhandled IRQ or exceptions!\n");
	printf("Please reset the board \n");
}

void blackfin_init_IRQ(void)
{
	*(unsigned volatile long *)(SIC_IMASK) = SIC_UNMASK_ALL;
	cli();
#ifndef CONFIG_KGDB
	*(unsigned volatile long *)(EVT_EMULATION_ADDR) = 0x0;
#endif
	*(unsigned volatile long *)(EVT_NMI_ADDR) =
	    (unsigned volatile long)evt_nmi;
	*(unsigned volatile long *)(EVT_EXCEPTION_ADDR) =
	    (unsigned volatile long)trap;
	*(unsigned volatile long *)(EVT_HARDWARE_ERROR_ADDR) =
	    (unsigned volatile long)evt_ivhw;
	*(unsigned volatile long *)(EVT_RESET_ADDR) =
	    (unsigned volatile long)evt_rst;
	*(unsigned volatile long *)(EVT_TIMER_ADDR) =
	    (unsigned volatile long)evt_timer;
	*(unsigned volatile long *)(EVT_IVG7_ADDR) =
	    (unsigned volatile long)evt_evt7;
	*(unsigned volatile long *)(EVT_IVG8_ADDR) =
	    (unsigned volatile long)evt_evt8;
	*(unsigned volatile long *)(EVT_IVG9_ADDR) =
	    (unsigned volatile long)evt_evt9;
	*(unsigned volatile long *)(EVT_IVG10_ADDR) =
	    (unsigned volatile long)evt_evt10;
	*(unsigned volatile long *)(EVT_IVG11_ADDR) =
	    (unsigned volatile long)evt_evt11;
	*(unsigned volatile long *)(EVT_IVG12_ADDR) =
	    (unsigned volatile long)evt_evt12;
	*(unsigned volatile long *)(EVT_IVG13_ADDR) =
	    (unsigned volatile long)evt_evt13;
	*(unsigned volatile long *)(EVT_IVG14_ADDR) =
	    (unsigned volatile long)evt_system_call;
	*(unsigned volatile long *)(EVT_IVG15_ADDR) =
	    (unsigned volatile long)evt_soft_int1;
	*(volatile unsigned long *)ILAT = 0;
	asm("csync;");
	sti();
	*(volatile unsigned long *)IMASK = 0xffbf;
	asm("csync;");
}

void exception_handle(void)
{
#if defined (CONFIG_PANIC_HANG)
	display_excp();
#else
	udelay(100000);		/* allow messages to go out */
	do_reset(NULL, 0, 0, NULL);
#endif
}

void display_excp(void)
{
	printf("Exception!\n");
}
