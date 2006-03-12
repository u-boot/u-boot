/*
 * U-boot - interrupt.S Processing of interrupts and exception handling
 *
 * Copyright (c) 2005 blackfin.uclinux.org
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define ASSEMBLY

#include <asm/hw_irq.h>
#include <asm/entry.h>
#include <asm/blackfin_defs.h>
#include <asm/cpu/bf533_irq.h>

.global blackfin_irq_panic;

.text
.align 2

#ifndef CONFIG_KGDB
.global evt_emulation
evt_emulation:
	SAVE_CONTEXT
	r0 = IRQ_EMU;
	r1 = seqstat;
	sp += -12;
	call blackfin_irq_panic;
	sp += 12;
	rte;
#endif

.global evt_nmi
evt_nmi:
	SAVE_CONTEXT
	r0 = IRQ_NMI;
	r1 = RETN;
	sp += -12;
	call blackfin_irq_panic;
	sp += 12;

_evt_nmi_exit:
	rtn;

.global trap
trap:
	[--sp] = r0;
	[--sp] = r1;
	[--sp] = p0;
	[--sp] = p1;
	[--sp] = astat;
	r0 = seqstat;
	R0 <<= 26;
	R0 >>= 26;
	p0 = r0;
	p1.l = EVTABLE;
	p1.h = EVTABLE;
	p0 = p1 + (p0 << 1);
	r1 = W[p0] (Z);
	p1 = r1;
	jump (pc + p1);

.global _EVENT1
_EVENT1:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT2
_EVENT2:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT3
_EVENT3:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT4
_EVENT4:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT5
_EVENT5:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT6
_EVENT6:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT7
_EVENT7:
	RAISE 15;
	JUMP.S _EXIT;

.global _EVENT8
_EVENT8:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT9
_EVENT9:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT10
_EVENT10:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT11
_EVENT11:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT12
_EVENT12:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT13
_EVENT13:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT14
_EVENT14:
/*	RAISE 14;	*/
	CALL	_cplb_hdr;
	JUMP.S _EXIT;

.global _EVENT19
_EVENT19:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT20
_EVENT20:
	RAISE 14;
	JUMP.S _EXIT;

.global _EVENT21
_EVENT21:
	RAISE 14;
	JUMP.S _EXIT;

.global _EXIT
_EXIT:
	ASTAT = [sp++];
	p1 = [sp++];
	p0 = [sp++];
	r1 = [sp++];
	r0 = [sp++];
	RTX;

EVTABLE:
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x0000;
	.byte2 0x003E;
	.byte2 0x0042;
	.byte4 0x0000;
	.byte4 0x0000;
	.byte4 0x0000;
	.byte4 0x0000;
	.byte4 0x0000;
	.byte4 0x0000;
	.byte4 0x0000;
	.byte2 0x0000;
	.byte2 0x001E;
	.byte2 0x0022;
	.byte2 0x0032;
	.byte2 0x002e;
	.byte2 0x0002;
	.byte2 0x0036;
	.byte2 0x002A;
	.byte2 0x001A;
	.byte2 0x0016;
	.byte2 0x000A;
	.byte2 0x000E;
	.byte2 0x0012;
	.byte2 0x0006;
	.byte2 0x0026;

.global evt_rst
evt_rst:
	SAVE_CONTEXT
	r0 = IRQ_RST;
	r1 = RETN;
	sp += -12;
	call do_reset;
	sp += 12;

_evt_rst_exit:
	rtn;

irq_panic:
	r0 = IRQ_EVX;
	r1 =  sp;
	sp += -12;
	call blackfin_irq_panic;
	sp += 12;

.global evt_ivhw
evt_ivhw:
	SAVE_CONTEXT
	RAISE 14;

_evt_ivhw_exit:
	 rti;

.global evt_timer
evt_timer:
	SAVE_CONTEXT
	r0 = IRQ_CORETMR;
	sp += -12;
	/* Polling method used now. */
	/* call timer_int; */
	sp += 12;
	RESTORE_CONTEXT
	rti;
	nop;

.global evt_evt7
evt_evt7:
	SAVE_CONTEXT
	r0 = 7;
	sp += -12;
	call process_int;
	sp += 12;

evt_evt7_exit:
	RESTORE_CONTEXT
	rti;

.global evt_evt8
evt_evt8:
	SAVE_CONTEXT
	r0 = 8;
	sp += -12;
	call process_int;
	sp += 12;

evt_evt8_exit:
	RESTORE_CONTEXT
	rti;

.global evt_evt9
evt_evt9:
	SAVE_CONTEXT
	r0 = 9;
	sp += -12;
	call process_int;
	sp += 12;

evt_evt9_exit:
	RESTORE_CONTEXT
	rti;

.global evt_evt10
evt_evt10:
	SAVE_CONTEXT
	r0 = 10;
	sp += -12;
	call process_int;
	sp += 12;

evt_evt10_exit:
	RESTORE_CONTEXT
	rti;

.global evt_evt11
evt_evt11:
	SAVE_CONTEXT
	r0 = 11;
	sp += -12;
	call process_int;
	sp += 12;

evt_evt11_exit:
	RESTORE_CONTEXT
	rti;

.global evt_evt12
evt_evt12:
	SAVE_CONTEXT
	r0 = 12;
	sp += -12;
	call process_int;
	sp += 12;
evt_evt12_exit:
	 RESTORE_CONTEXT
	 rti;

.global evt_evt13
evt_evt13:
	SAVE_CONTEXT
	r0 = 13;
	sp += -12;
	call process_int;
	sp += 12;

evt_evt13_exit:
	 RESTORE_CONTEXT
	 rti;

.global evt_system_call
evt_system_call:
	[--sp] = r0;
	[--SP] = RETI;
	r0 = [sp++];
	r0 += 2;
	[--sp] = r0;
	RETI = [SP++];
	r0 = [SP++];
	SAVE_CONTEXT
	sp += -12;
	call display_excp;
	sp += 12;
	RESTORE_CONTEXT
	RTI;

evt_system_call_exit:
	rti;

.global evt_soft_int1
evt_soft_int1:
	[--sp] = r0;
	[--SP] = RETI;
	r0 = [sp++];
	r0 += 2;
	[--sp] = r0;
	RETI = [SP++];
	r0 = [SP++];
	SAVE_CONTEXT
	sp += -12;
	call display_excp;
	sp += 12;
	RESTORE_CONTEXT
	RTI;

evt_soft_int1_exit:
	rti;
