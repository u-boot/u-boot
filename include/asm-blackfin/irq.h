/*
 * U-boot - irq.h Interrupt related header file
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * This file was based on
 * linux/arch/$(ARCH)/platform/$(PLATFORM)/irq.c
 *
 * Changed by HuTao Apr18, 2003
 *
 * Copyright was missing when I got the code so took from MIPS arch ...MaTed---
 * Copyright (C) 1994 by Waldorf GMBH, written by Ralf Baechle
 * Copyright (C) 1995, 96, 97, 98, 99, 2000, 2001 by Ralf Baechle
 *
 * Adapted for BlackFin (ADI) by Ted Ma <mated@sympatico.ca>
 * Copyright (c) 2002 Arcturus Networks Inc. (www.arcturusnetworks.com)
 * Copyright (c) 2002 Lineo, Inc. <mattw@lineo.com>
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

#ifndef _BLACKFIN_IRQ_H_
#define _BLACKFIN_IRQ_H_

#include <linux/config.h>
#include <asm/hw_irq.h>

/*
 *   On the Blackfin, the interrupt structure allows remmapping of the hardware
 *   levels.
 * - I'm going to assume that the H/W level is going to stay at the default
 *   settings. If someone wants to go through and abstart this out, feel free
 *   to mod the interrupt numbering scheme.
 * - I'm abstracting the interrupts so that uClinux does not know anything
 *   about the H/W levels. If you want to change the H/W AND keep the abstracted
 *   levels that uClinux sees, you should be able to do most of it here.
 * - I've left the "abstract" numbering sparce in case someone wants to pull the
 *   interrupts apart (just the TX/RX for the various devices)
 */

#define	NR_IRQS		SYS_IRQS
/*
 * "Generic" interrupt sources
 */
#define IRQ_SCHED_TIMER	(8)	/* interrupt source for scheduling timer */

static __inline__ int irq_cannonicalize(int irq)
{
	return irq;
}

/*
 * Machine specific interrupt sources.
 *
 * Adding an interrupt service routine for a source with this bit
 * set indicates a special machine specific interrupt source.
 * The machine specific files define these sources.
 *
 * The IRQ_MACHSPEC bit is now gone - the only thing it did was to
 * introduce unnecessary overhead.
 *
 * All interrupt handling is actually machine specific so it is better
 * to use function pointers, as used by the Sparc port, and select the
 * interrupt handling functions when initializing the kernel. This way
 * we save some unnecessary overhead at run-time.
 * 01/11/97 - Jes
 */

extern void (*mach_enable_irq) (unsigned int);
extern void (*mach_disable_irq) (unsigned int);
extern int sys_request_irq(unsigned int,
			   void (*)(int, void *, struct pt_regs *),
			   unsigned long, const char *, void *);
extern void sys_free_irq(unsigned int, void *);

/*
 * various flags for request_irq() - the Amiga now uses the standard
 * mechanism like all other architectures - SA_INTERRUPT and SA_SHIRQ
 * are your friends.
 */
#define IRQ_FLG_LOCK	(0x0001)	/* handler is not replaceable   */
#define IRQ_FLG_REPLACE	(0x0002)	/* replace existing handler     */
#define IRQ_FLG_FAST	(0x0004)
#define IRQ_FLG_SLOW	(0x0008)
#define IRQ_FLG_STD	(0x8000)	/* internally used              */

/*
 * This structure is used to chain together the ISRs for a particular
 * interrupt source (if it supports chaining).
 */
typedef struct irq_node {
	void (*handler) (int, void *, struct pt_regs *);
	unsigned long flags;
	void *dev_id;
	const char *devname;
	struct irq_node *next;
} irq_node_t;

/*
 * This structure has only 4 elements for speed reasons
 */
typedef struct irq_handler {
	void (*handler) (int, void *, struct pt_regs *);
	unsigned long flags;
	void *dev_id;
	const char *devname;
} irq_handler_t;

/* count of spurious interrupts */
extern volatile unsigned int num_spurious;

/*
 * This function returns a new irq_node_t
 */
extern irq_node_t *new_irq_node(void);

/*
 * Some drivers want these entry points
 */
#define enable_irq(x)	(mach_enable_irq  ? (*mach_enable_irq)(x)  : 0)
#define disable_irq(x)	(mach_disable_irq ? (*mach_disable_irq)(x) : 0)

#define enable_irq_nosync(x)	enable_irq(x)
#define disable_irq_nosync(x)	disable_irq(x)

#endif
