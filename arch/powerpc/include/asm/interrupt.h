/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 * This work has been supported by: QTechnology  http://qtec.com/
 * Based on interrupts.c Wolfgang Denk-DENX Software Engineering-wd@denx.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef INTERRUPT_H
#define INTERRUPT_H

#if defined(CONFIG_XILINX_440)
#include <asm/xilinx_irq.h>
#else
#include <asm/ppc4xx-uic.h>
#endif

void pic_enable(void);
void pic_irq_enable(unsigned int irq);
void pic_irq_disable(unsigned int irq);
void pic_irq_ack(unsigned int irq);
void external_interrupt(struct pt_regs *regs);
void interrupt_run_handler(int vec);

#endif
