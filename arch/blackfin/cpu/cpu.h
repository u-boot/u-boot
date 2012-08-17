/*
 *  U-boot - cpu.h
 *
 *  Copyright (c) 2005-2007 Analog Devices Inc.
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
