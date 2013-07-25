/*
 * (C) Copyright 2006
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de
 *
 * (C) Copyright -2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* this section was ripped out of arch/powerpc/syslib/mpc52xx_pic.c in the
 * Linux 2.6 source with the following copyright.
 *
 * Based on (well, mostly copied from) the code from the 2.4 kernel by
 * Dale Farnsworth <dfarnsworth@mvista.com> and Kent Borg.
 *
 * Copyright (C) 2004 Sylvain Munaut <tnt@246tNt.com>
 * Copyright (C) 2003 Montavista Software, Inc
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <command.h>

struct irq_action {
	interrupt_handler_t *handler;
	void *arg;
	ulong count;
};

static struct irq_action irq_handlers[NR_IRQS];

static struct mpc5xxx_intr *intr;
static struct mpc5xxx_sdma *sdma;

static void mpc5xxx_ic_disable(unsigned int irq)
{
	u32 val;

	if (irq == MPC5XXX_IRQ0) {
		val = in_be32(&intr->ctrl);
		val &= ~(1 << 11);
		out_be32(&intr->ctrl, val);
	} else if (irq < MPC5XXX_IRQ1) {
		BUG();
	} else if (irq <= MPC5XXX_IRQ3) {
		val = in_be32(&intr->ctrl);
		val &= ~(1 << (10 - (irq - MPC5XXX_IRQ1)));
		out_be32(&intr->ctrl, val);
	} else if (irq < MPC5XXX_SDMA_IRQ_BASE) {
		val = in_be32(&intr->main_mask);
		val |= 1 << (16 - (irq - MPC5XXX_MAIN_IRQ_BASE));
		out_be32(&intr->main_mask, val);
	} else if (irq < MPC5XXX_PERP_IRQ_BASE) {
		val = in_be32(&sdma->IntMask);
		val |= 1 << (irq - MPC5XXX_SDMA_IRQ_BASE);
		out_be32(&sdma->IntMask, val);
	} else {
		val = in_be32(&intr->per_mask);
		val |= 1 << (31 - (irq - MPC5XXX_PERP_IRQ_BASE));
		out_be32(&intr->per_mask, val);
	}
}

static void mpc5xxx_ic_enable(unsigned int irq)
{
	u32 val;

	if (irq == MPC5XXX_IRQ0) {
		val = in_be32(&intr->ctrl);
		val |= 1 << 11;
		out_be32(&intr->ctrl, val);
	} else if (irq < MPC5XXX_IRQ1) {
		BUG();
	} else if (irq <= MPC5XXX_IRQ3) {
		val = in_be32(&intr->ctrl);
		val |= 1 << (10 - (irq - MPC5XXX_IRQ1));
		out_be32(&intr->ctrl, val);
	} else if (irq < MPC5XXX_SDMA_IRQ_BASE) {
		val = in_be32(&intr->main_mask);
		val &= ~(1 << (16 - (irq - MPC5XXX_MAIN_IRQ_BASE)));
		out_be32(&intr->main_mask, val);
	} else if (irq < MPC5XXX_PERP_IRQ_BASE) {
		val = in_be32(&sdma->IntMask);
		val &= ~(1 << (irq - MPC5XXX_SDMA_IRQ_BASE));
		out_be32(&sdma->IntMask, val);
	} else {
		val = in_be32(&intr->per_mask);
		val &= ~(1 << (31 - (irq - MPC5XXX_PERP_IRQ_BASE)));
		out_be32(&intr->per_mask, val);
	}
}

static void mpc5xxx_ic_ack(unsigned int irq)
{
	u32 val;

	/*
	 * Only some irqs are reset here, others in interrupting hardware.
	 */

	switch (irq) {
	case MPC5XXX_IRQ0:
		val = in_be32(&intr->ctrl);
		val |= 0x08000000;
		out_be32(&intr->ctrl, val);
		break;
	case MPC5XXX_CCS_IRQ:
		val = in_be32(&intr->enc_status);
		val |= 0x00000400;
		out_be32(&intr->enc_status, val);
		break;
	case MPC5XXX_IRQ1:
		val = in_be32(&intr->ctrl);
		val |= 0x04000000;
		out_be32(&intr->ctrl, val);
		break;
	case MPC5XXX_IRQ2:
		val = in_be32(&intr->ctrl);
		val |= 0x02000000;
		out_be32(&intr->ctrl, val);
		break;
	case MPC5XXX_IRQ3:
		val = in_be32(&intr->ctrl);
		val |= 0x01000000;
		out_be32(&intr->ctrl, val);
		break;
	default:
		if (irq >= MPC5XXX_SDMA_IRQ_BASE
		    && irq < (MPC5XXX_SDMA_IRQ_BASE + MPC5XXX_SDMA_IRQ_NUM)) {
			out_be32(&sdma->IntPend,
				 1 << (irq - MPC5XXX_SDMA_IRQ_BASE));
		}
		break;
	}
}

static void mpc5xxx_ic_disable_and_ack(unsigned int irq)
{
	mpc5xxx_ic_disable(irq);
	mpc5xxx_ic_ack(irq);
}

static void mpc5xxx_ic_end(unsigned int irq)
{
	mpc5xxx_ic_enable(irq);
}

void mpc5xxx_init_irq(void)
{
	u32 intr_ctrl;

	/* Remap the necessary zones */
	intr = (struct mpc5xxx_intr *)(MPC5XXX_ICTL);
	sdma = (struct mpc5xxx_sdma *)(MPC5XXX_SDMA);

	/* Disable all interrupt sources. */
	out_be32(&sdma->IntPend, 0xffffffff);	/* 1 means clear pending */
	out_be32(&sdma->IntMask, 0xffffffff);	/* 1 means disabled */
	out_be32(&intr->per_mask, 0x7ffffc00);	/* 1 means disabled */
	out_be32(&intr->main_mask, 0x00010fff);	/* 1 means disabled */
	intr_ctrl = in_be32(&intr->ctrl);
	intr_ctrl |= 0x0f000000 |	/* clear IRQ 0-3 */
	    0x00ff0000 |	/* IRQ 0-3 level sensitive low active */
	    0x00001000 |	/* MEE master external enable */
	    0x00000000 |	/* 0 means disable IRQ 0-3 */
	    0x00000001;		/* CEb route critical normally */
	out_be32(&intr->ctrl, intr_ctrl);

	/* Zero a bunch of the priority settings.  */
	out_be32(&intr->per_pri1, 0);
	out_be32(&intr->per_pri2, 0);
	out_be32(&intr->per_pri3, 0);
	out_be32(&intr->main_pri1, 0);
	out_be32(&intr->main_pri2, 0);
}

int mpc5xxx_get_irq(struct pt_regs *regs)
{
	u32 status;
	int irq = -1;

	status = in_be32(&intr->enc_status);

	if (status & 0x00000400) {	/* critical */
		irq = (status >> 8) & 0x3;
		if (irq == 2)	/* high priority peripheral */
			goto peripheral;
		irq += MPC5XXX_CRIT_IRQ_BASE;
	} else if (status & 0x00200000) {	/* main */
		irq = (status >> 16) & 0x1f;
		if (irq == 4)	/* low priority peripheral */
			goto peripheral;
		irq += MPC5XXX_MAIN_IRQ_BASE;
	} else if (status & 0x20000000) {	/* peripheral */
	      peripheral:
		irq = (status >> 24) & 0x1f;
		if (irq == 0) {	/* bestcomm */
			status = in_be32(&sdma->IntPend);
			irq = ffs(status) + MPC5XXX_SDMA_IRQ_BASE - 1;
		} else
			irq += MPC5XXX_PERP_IRQ_BASE;
	}

	return irq;
}

/****************************************************************************/

int interrupt_init_cpu(ulong * decrementer_count)
{
	*decrementer_count = get_tbclk() / CONFIG_SYS_HZ;

	mpc5xxx_init_irq();

	return (0);
}

/****************************************************************************/

/*
 * Handle external interrupts
 */
void external_interrupt(struct pt_regs *regs)
{
	int irq, unmask = 1;

	irq = mpc5xxx_get_irq(regs);

	mpc5xxx_ic_disable_and_ack(irq);

	enable_interrupts();

	if (irq_handlers[irq].handler != NULL)
		(*irq_handlers[irq].handler) (irq_handlers[irq].arg);
	else {
		printf("\nBogus External Interrupt IRQ %d\n", irq);
		/*
		 * turn off the bogus interrupt, otherwise it
		 * might repeat forever
		 */
		unmask = 0;
	}

	if (unmask)
		mpc5xxx_ic_end(irq);
}

void timer_interrupt_cpu(struct pt_regs *regs)
{
	/* nothing to do here */
	return;
}

/****************************************************************************/

/*
 * Install and free a interrupt handler.
 */

void irq_install_handler(int irq, interrupt_handler_t * handler, void *arg)
{
	if (irq < 0 || irq >= NR_IRQS) {
		printf("irq_install_handler: bad irq number %d\n", irq);
		return;
	}

	if (irq_handlers[irq].handler != NULL)
		printf("irq_install_handler: 0x%08lx replacing 0x%08lx\n",
		       (ulong) handler, (ulong) irq_handlers[irq].handler);

	irq_handlers[irq].handler = handler;
	irq_handlers[irq].arg = arg;

	mpc5xxx_ic_enable(irq);
}

void irq_free_handler(int irq)
{
	if (irq < 0 || irq >= NR_IRQS) {
		printf("irq_free_handler: bad irq number %d\n", irq);
		return;
	}

	mpc5xxx_ic_disable(irq);

	irq_handlers[irq].handler = NULL;
	irq_handlers[irq].arg = NULL;
}

/****************************************************************************/

#if defined(CONFIG_CMD_IRQ)
void do_irqinfo(cmd_tbl_t * cmdtp, bd_t * bd, int flag, int argc, char * const argv[])
{
	int irq, re_enable;
	u32 intr_ctrl;
	char *irq_config[] = { "level sensitive, active high",
		"edge sensitive, rising active edge",
		"edge sensitive, falling active edge",
		"level sensitive, active low"
	};

	re_enable = disable_interrupts();

	intr_ctrl = in_be32(&intr->ctrl);
	printf("Interrupt configuration:\n");

	for (irq = 0; irq <= 3; irq++) {
		printf("IRQ%d: %s\n", irq,
		       irq_config[(intr_ctrl >> (22 - 2 * irq)) & 0x3]);
	}

	puts("\nInterrupt-Information:\n" "Nr  Routine   Arg       Count\n");

	for (irq = 0; irq < NR_IRQS; irq++)
		if (irq_handlers[irq].handler != NULL)
			printf("%02d  %08lx  %08lx  %ld\n", irq,
			       (ulong) irq_handlers[irq].handler,
			       (ulong) irq_handlers[irq].arg,
			       irq_handlers[irq].count);

	if (re_enable)
		enable_interrupts();
}
#endif
