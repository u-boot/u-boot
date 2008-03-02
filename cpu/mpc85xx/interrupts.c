/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002 (440 port)
 * Scott McNutt, Artesyn Communication Producs, smcnutt@artsyncp.com
 *
 * (C) Copyright 2003 Motorola Inc. (MPC85xx port)
 * Xianghua Xiao (X.Xiao@motorola.com)
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
#include <watchdog.h>
#include <command.h>
#include <asm/processor.h>
#include <ppc_asm.tmpl>

unsigned decrementer_count;		/* count value for 1e6/HZ microseconds */

static __inline__ unsigned long get_msr(void)
{
	unsigned long msr;

	asm volatile("mfmsr %0" : "=r" (msr) :);
	return msr;
}

static __inline__ void set_msr(unsigned long msr)
{
	asm volatile("mtmsr %0" : : "r" (msr));
	asm volatile("isync");
}

static __inline__ unsigned long get_dec (void)
{
	unsigned long val;

	asm volatile ("mfdec %0":"=r" (val):);

	return val;
}


static __inline__ void set_dec (unsigned long val)
{
	if (val)
		asm volatile ("mtdec %0"::"r" (val));
}

void enable_interrupts (void)
{
	set_msr (get_msr() | MSR_EE);
}

/* returns flag if MSR_EE was set before */
int disable_interrupts (void)
{
	ulong msr = get_msr();
	set_msr (msr & ~MSR_EE);
	return ((msr & MSR_EE) != 0);
}

int interrupt_init (void)
{
	volatile ccsr_pic_t *pic = (void *)(CFG_MPC85xx_PIC_ADDR);

	pic->gcr = MPC85xx_PICGCR_RST;
	while (pic->gcr & MPC85xx_PICGCR_RST);
	pic->gcr = MPC85xx_PICGCR_M;
	decrementer_count = get_tbclk() / CFG_HZ;
	mtspr(SPRN_TCR, TCR_PIE);
	set_dec (decrementer_count);
	set_msr (get_msr () | MSR_EE);

#ifdef CONFIG_INTERRUPTS
	pic->iivpr1 = 0x810001;	/* 50220 enable ecm interrupts */
	debug("iivpr1@%x = %x\n",&pic->iivpr1, pic->iivpr1);

	pic->iivpr2 = 0x810002;	/* 50240 enable ddr interrupts */
	debug("iivpr2@%x = %x\n",&pic->iivpr2, pic->iivpr2);

	pic->iivpr3 = 0x810003;	/* 50260 enable lbc interrupts */
	debug("iivpr3@%x = %x\n",&pic->iivpr3, pic->iivpr3);

#ifdef CONFIG_PCI1
	pic->iivpr8 = 0x810008;	/* enable pci1 interrupts */
	debug("iivpr8@%x = %x\n",&pic->iivpr8, pic->iivpr8);
#endif
#if defined(CONFIG_PCI2) || defined(CONFIG_PCIE2)
	pic->iivpr9 = 0x810009;	/* enable pci1 interrupts */
	debug("iivpr9@%x = %x\n",&pic->iivpr9, pic->iivpr9);
#endif
#ifdef CONFIG_PCIE1
	pic->iivpr10 = 0x81000a;	/* enable pcie1 interrupts */
	debug("iivpr10@%x = %x\n",&pic->iivpr10, pic->iivpr10);
#endif
#ifdef CONFIG_PCIE3
	pic->iivpr11 = 0x81000b;	/* enable pcie3 interrupts */
	debug("iivpr11@%x = %x\n",&pic->iivpr11, pic->iivpr11);
#endif

	pic->ctpr=0;		/* 40080 clear current task priority register */
#endif

	return (0);
}

/*
 * Install and free a interrupt handler. Not implemented yet.
 */

void
irq_install_handler(int vec, interrupt_handler_t *handler, void *arg)
{
	return;
}

void
irq_free_handler(int vec)
{
	return;
}

/****************************************************************************/


volatile ulong timestamp = 0;

/*
 * timer_interrupt - gets called when the decrementer overflows,
 * with interrupts disabled.
 * Trivial implementation - no need to be really accurate.
 */
void timer_interrupt(struct pt_regs *regs)
{
	timestamp++;
	set_dec (decrementer_count);
	mtspr(SPRN_TSR, TSR_PIS);
#if defined(CONFIG_WATCHDOG)
	if ((timestamp % 1000) == 0)
		reset_85xx_watchdog();
#endif /* CONFIG_WATCHDOG */
}

void reset_timer (void)
{
	timestamp = 0;
}

ulong get_timer (ulong base)
{
	return (timestamp - base);
}

void set_timer (ulong t)
{
	timestamp = t;
}

#if defined(CONFIG_CMD_IRQ)

/*******************************************************************************
 *
 * irqinfo - print information about PCI devices,not implemented.
 *
 */
int
do_irqinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf ("\nInterrupt-unsupported:\n");

	return 0;
}

#endif
