/*
 * U-boot - blackfin_defs.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
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

#ifndef __BLACKFIN_DEFS_H__
#define __BLACKFIN_DEFS_H__

#define TS_MAGICKEY		0x5a5a5a5a
#define TASK_STATE		0
#define TASK_FLAGS		4
#define TASK_PTRACE		24
#define TASK_BLOCKED		636
#define TASK_COUNTER		32
#define TASK_SIGPENDING		8
#define TASK_NEEDRESCHED	20
#define TASK_THREAD		600
#define TASK_MM			44
#define TASK_ACTIVE_MM		80
#define THREAD_KSP		0
#define THREAD_USP		4
#define THREAD_SR		8
#define THREAD_ESP0		12
#define THREAD_PC		16
#define PT_ORIG_R0		208
#define PT_R0			204
#define PT_R1			200
#define PT_R2			196
#define PT_R3			192
#define PT_R4			188
#define PT_R5			184
#define PT_R6			180
#define PT_R7			176
#define PT_P0			172
#define PT_P1			168
#define PT_P2			164
#define PT_P3			160
#define PT_P4			156
#define PT_P5			152
#define PT_A0w			72
#define PT_A1w			64
#define PT_A0x			76
#define PT_A1x			68
#define PT_RETS			28
#define PT_RESERVED		32
#define PT_ASTAT		36
#define PT_SEQSTAT		8
#define PT_PC			24
#define PT_IPEND		0
#define PT_USP			144
#define PT_FP			148
#define PT_SYSCFG		4
#define IRQ_HANDLER		0
#define IRQ_DEVID		8
#define IRQ_NEXT		16
#define STAT_IRQ		5148
#define SIGSEGV			11
#define SEGV_MAPERR		196609
#define SIGTRAP			5
#define PT_PTRACED		1
#define PT_TRACESYS		2
#define PT_DTRACE		4

#endif
