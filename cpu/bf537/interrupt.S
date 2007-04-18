/*
 * U-boot - interrupt.S Processing of interrupts and exception handling
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This file is based on interrupt.S
 *
 * Copyright (C) 2003  Metrowerks, Inc. <mwaddel@metrowerks.com>
 * Copyright (C) 2002  Arcturus Networks Ltd. Ted Ma <mated@sympatico.ca>
 * Copyright (C) 1998  D. Jeff Dionne <jeff@ryeham.ee.ryerson.ca>,
 *                     Kenneth Albanowski <kjahds@kjahds.com>,
 *                     The Silver Hammer Group, Ltd.
 *
 * (c) 1995, Dionne & Associates
 * (c) 1995, DKG Display Tech.
 *
 * This file is also based on exception.asm
 * (C) Copyright 2001-2005 - Analog Devices, Inc.  All rights reserved.
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

#define ASSEMBLY
#include <config.h>
#include <asm/blackfin.h>
#include <asm/hw_irq.h>
#include <asm/entry.h>
#include <asm/blackfin_defs.h>

.global _blackfin_irq_panic;

.text
.align 2

#ifndef CONFIG_KGDB
.global _evt_emulation
_evt_emulation:
	SAVE_CONTEXT
	r0 = IRQ_EMU;
	r1 = seqstat;
	sp += -12;
	call _blackfin_irq_panic;
	sp += 12;
	rte;
#endif

.global _evt_nmi
_evt_nmi:
	SAVE_CONTEXT
	r0 = IRQ_NMI;
	r1 = RETN;
	sp += -12;
	call _blackfin_irq_panic;
	sp += 12;

_evt_nmi_exit:
	rtn;

.global _trap
_trap:
	SAVE_ALL_SYS
	r0 = sp;	/* stack frame pt_regs pointer argument ==> r0 */
	sp += -12;
	call _trap_c
	sp += 12;
	RESTORE_ALL_SYS
	rtx;

.global _evt_rst
_evt_rst:
	SAVE_CONTEXT
	r0 = IRQ_RST;
	r1 = RETN;
	sp += -12;
	call _do_reset;
	sp += 12;

_evt_rst_exit:
	rtn;

irq_panic:
	r0 = IRQ_EVX;
	r1 =  sp;
	sp += -12;
	call _blackfin_irq_panic;
	sp += 12;

.global _evt_ivhw
_evt_ivhw:
	SAVE_CONTEXT
	RAISE 14;

_evt_ivhw_exit:
	 rti;

.global _evt_timer
_evt_timer:
	SAVE_CONTEXT
	r0 = IRQ_CORETMR;
	sp += -12;
	/* Polling method used now. */
	/* call timer_int; */
	sp += 12;
	RESTORE_CONTEXT
	rti;
	nop;

.global _evt_evt7
_evt_evt7:
	SAVE_CONTEXT
	r0 = 7;
	sp += -12;
	call _process_int;
	sp += 12;

evt_evt7_exit:
	RESTORE_CONTEXT
	rti;

.global _evt_evt8
_evt_evt8:
	SAVE_CONTEXT
	r0 = 8;
	sp += -12;
	call _process_int;
	sp += 12;

evt_evt8_exit:
	RESTORE_CONTEXT
	rti;

.global _evt_evt9
_evt_evt9:
	SAVE_CONTEXT
	r0 = 9;
	sp += -12;
	call _process_int;
	sp += 12;

evt_evt9_exit:
	RESTORE_CONTEXT
	rti;

.global _evt_evt10
_evt_evt10:
	SAVE_CONTEXT
	r0 = 10;
	sp += -12;
	call _process_int;
	sp += 12;

evt_evt10_exit:
	RESTORE_CONTEXT
	rti;

.global _evt_evt11
_evt_evt11:
	SAVE_CONTEXT
	r0 = 11;
	sp += -12;
	call _process_int;
	sp += 12;

evt_evt11_exit:
	RESTORE_CONTEXT
	rti;

.global _evt_evt12
_evt_evt12:
	SAVE_CONTEXT
	r0 = 12;
	sp += -12;
	call _process_int;
	sp += 12;
evt_evt12_exit:
	 RESTORE_CONTEXT
	 rti;

.global _evt_evt13
_evt_evt13:
	SAVE_CONTEXT
	r0 = 13;
	sp += -12;
	call _process_int;
	sp += 12;

evt_evt13_exit:
	 RESTORE_CONTEXT
	 rti;

.global _evt_system_call
_evt_system_call:
	[--sp] = r0;
	[--SP] = RETI;
	r0 = [sp++];
	r0 += 2;
	[--sp] = r0;
	RETI = [SP++];
	r0 = [SP++];
	SAVE_CONTEXT
	sp += -12;
	call _exception_handle;
	sp += 12;
	RESTORE_CONTEXT
	RTI;

evt_system_call_exit:
	rti;

.global _evt_soft_int1
_evt_soft_int1:
	[--sp] = r0;
	[--SP] = RETI;
	r0 = [sp++];
	r0 += 2;
	[--sp] = r0;
	RETI = [SP++];
	r0 = [SP++];
	SAVE_CONTEXT
	sp += -12;
	call _exception_handle;
	sp += 12;
	RESTORE_CONTEXT
	RTI;

evt_soft_int1_exit:
	rti;
