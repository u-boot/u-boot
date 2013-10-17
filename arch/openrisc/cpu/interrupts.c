/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/types.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/openrisc_exc.h>

struct irq_action {
	interrupt_handler_t *handler;
	void *arg;
	int count;
};

static struct irq_action handlers[32];

void interrupt_handler(void)
{
	int irq;

	while ((irq = ffs(mfspr(SPR_PICSR)))) {
		if (handlers[--irq].handler) {
			handlers[irq].handler(handlers[irq].arg);
			handlers[irq].count++;
		} else {
			/* disable the interrupt */
			mtspr(SPR_PICMR, mfspr(SPR_PICMR) & ~(1 << irq));
			printf("Unhandled interrupt: %d\n", irq);
		}
		/* clear the interrupt */
		mtspr(SPR_PICSR, mfspr(SPR_PICSR) & ~(1 << irq));
	}
}

int interrupt_init(void)
{
	/* install handler for external interrupt exception */
	exception_install_handler(EXC_EXT_IRQ, interrupt_handler);
	/* Enable interrupts in supervisor register */
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_IEE);

	return 0;
}

void enable_interrupts(void)
{
	/* Set interrupt enable bit in supervisor register */
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_IEE);
	/* Enable timer exception */
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);
}

int disable_interrupts(void)
{
	/* Clear interrupt enable bit in supervisor register */
	mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_IEE);
	/* Disable timer exception */
	mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);

	return 0;
}

void irq_install_handler(int irq, interrupt_handler_t *handler, void *arg)
{
	if (irq < 0 || irq > 31)
		return;

	handlers[irq].handler = handler;
	handlers[irq].arg = arg;
}

void irq_free_handler(int irq)
{
	if (irq < 0 || irq > 31)
		return;

	handlers[irq].handler = 0;
	handlers[irq].arg = 0;
}

#if defined(CONFIG_CMD_IRQ)
int do_irqinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;

	printf("\nInterrupt-Information:\n\n");
	printf("Nr  Routine   Arg       Count\n");
	printf("-----------------------------\n");

	for (i = 0; i < 32; i++) {
		if (handlers[i].handler) {
			printf("%02d  %08lx  %08lx  %d\n",
				i,
				(ulong)handlers[i].handler,
				(ulong)handlers[i].arg,
				handlers[i].count);
		}
	}
	printf("\n");

	return 0;
}
#endif
