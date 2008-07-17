/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 * This work has been supported by: QTechnology  http://qtec.com/
 * Based on interrupts.c Wolfgang Denk-DENX Software Engineering-wd@denx.de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
