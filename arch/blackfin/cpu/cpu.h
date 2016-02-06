/*
 *  U-Boot - cpu.h
 *
 *  Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CPU_H_
#define _CPU_H_

#include <command.h>

void board_reset(void) __attribute__((__weak__));
void bfin_dump(struct pt_regs *reg);
void bfin_panic(struct pt_regs *reg);
void dump(struct pt_regs *regs);

asmlinkage void trap(void);
asmlinkage void evt_nmi(void);
asmlinkage void evt_default(void);

#endif
