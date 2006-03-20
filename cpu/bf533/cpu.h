/*
 *  U-boot - cpu.h
 *
 *  Copyright (c) 2005 blackfin.uclinux.org
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

#ifndef _CPU_H_
#define _CPU_H_

#include <command.h>

#define INTERNAL_IRQS (32)
#define NUM_IRQ_NODES 16
#define DEF_INTERRUPT_FLAGS 1
#define MAX_TIM_LOAD	0xFFFFFFFF

void blackfin_irq_panic(int reason, struct pt_regs * reg);
extern void dump(struct pt_regs * regs);
void display_excp(void);
asmlinkage void evt_nmi(void);
asmlinkage void evt_exception(void);
asmlinkage void trap(void);
asmlinkage void evt_ivhw(void);
asmlinkage void evt_rst(void);
asmlinkage void evt_timer(void);
asmlinkage void evt_evt7(void);
asmlinkage void evt_evt8(void);
asmlinkage void evt_evt9(void);
asmlinkage void evt_evt10(void);
asmlinkage void evt_evt11(void);
asmlinkage void evt_evt12(void);
asmlinkage void evt_evt13(void);
asmlinkage void evt_soft_int1(void);
asmlinkage void evt_system_call(void);
void blackfin_irq_panic(int reason, struct pt_regs * regs);
void blackfin_free_irq(unsigned int irq, void *dev_id);
void call_isr(int irq, struct pt_regs * fp);
void blackfin_do_irq(int vec, struct pt_regs *fp);
void blackfin_init_IRQ(void);
void blackfin_enable_irq(unsigned int irq);
void blackfin_disable_irq(unsigned int irq);
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int blackfin_request_irq(unsigned int irq,
		     void (*handler)(int, void *, struct pt_regs *),
		     unsigned long flags,const char *devname,void *dev_id);
void timer_init(void);
#endif
