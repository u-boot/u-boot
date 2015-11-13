/*
 * Added to U-boot,
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
 * Copyright (C) 2007
 *
 * LEON2/3 LIBIO low-level routines
 * Written by Jiri Gaisler.
 * Copyright (C) 2004  Gaisler Research AB
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SPARC_WINMACRO_H__
#define __SPARC_WINMACRO_H__

#include <asm/asmmacro.h>
#include <asm/stack.h>

/* Store the register window onto the 8-byte aligned area starting
 * at %reg.  It might be %sp, it might not, we don't care.
 */
#define RW_STORE(reg) \
	std	%l0, [%reg + RW_L0]; \
	std	%l2, [%reg + RW_L2]; \
	std	%l4, [%reg + RW_L4]; \
	std	%l6, [%reg + RW_L6]; \
	std	%i0, [%reg + RW_I0]; \
	std	%i2, [%reg + RW_I2]; \
	std	%i4, [%reg + RW_I4]; \
	std	%i6, [%reg + RW_I6];

/* Load a register window from the area beginning at %reg. */
#define RW_LOAD(reg) \
	ldd	[%reg + RW_L0], %l0; \
	ldd	[%reg + RW_L2], %l2; \
	ldd	[%reg + RW_L4], %l4; \
	ldd	[%reg + RW_L6], %l6; \
	ldd	[%reg + RW_I0], %i0; \
	ldd	[%reg + RW_I2], %i2; \
	ldd	[%reg + RW_I4], %i4; \
	ldd	[%reg + RW_I6], %i6;

/* Loading and storing struct pt_reg trap frames. */
#define PT_LOAD_INS(base_reg) \
	ldd	[%base_reg + SF_REGS_SZ + PT_I0], %i0; \
	ldd	[%base_reg + SF_REGS_SZ + PT_I2], %i2; \
	ldd	[%base_reg + SF_REGS_SZ + PT_I4], %i4; \
	ldd	[%base_reg + SF_REGS_SZ + PT_I6], %i6;

#define PT_LOAD_GLOBALS(base_reg) \
	ld	[%base_reg + SF_REGS_SZ + PT_G1], %g1; \
	ldd	[%base_reg + SF_REGS_SZ + PT_G2], %g2; \
	ldd	[%base_reg + SF_REGS_SZ + PT_G4], %g4; \
	ldd	[%base_reg + SF_REGS_SZ + PT_G6], %g6;

#define PT_LOAD_YREG(base_reg, scratch) \
	ld	[%base_reg + SF_REGS_SZ + PT_Y], %scratch; \
	wr	%scratch, 0x0, %y;

#define PT_LOAD_PRIV(base_reg, pt_psr, pt_pc, pt_npc) \
	ld	[%base_reg + SF_REGS_SZ + PT_PSR], %pt_psr; \
	ld	[%base_reg + SF_REGS_SZ + PT_PC], %pt_pc; \
	ld	[%base_reg + SF_REGS_SZ + PT_NPC], %pt_npc;

#define PT_LOAD_ALL(base_reg, pt_psr, pt_pc, pt_npc, scratch) \
	PT_LOAD_YREG(base_reg, scratch) \
	PT_LOAD_INS(base_reg) \
	PT_LOAD_GLOBALS(base_reg) \
	PT_LOAD_PRIV(base_reg, pt_psr, pt_pc, pt_npc)

#define PT_STORE_INS(base_reg) \
	std	%i0, [%base_reg + SF_REGS_SZ + PT_I0]; \
	std	%i2, [%base_reg + SF_REGS_SZ + PT_I2]; \
	std	%i4, [%base_reg + SF_REGS_SZ + PT_I4]; \
	std	%i6, [%base_reg + SF_REGS_SZ + PT_I6];

#define PT_STORE_GLOBALS(base_reg) \
	st	%g1, [%base_reg + SF_REGS_SZ + PT_G1]; \
	std	%g2, [%base_reg + SF_REGS_SZ + PT_G2]; \
	std	%g4, [%base_reg + SF_REGS_SZ + PT_G4]; \
	std	%g6, [%base_reg + SF_REGS_SZ + PT_G6];

#define PT_STORE_YREG(base_reg, scratch) \
	rd	%y, %scratch; \
	st	%scratch, [%base_reg + SF_REGS_SZ + PT_Y];

#define PT_STORE_PRIV(base_reg, pt_psr, pt_pc, pt_npc) \
	st	%pt_psr, [%base_reg + SF_REGS_SZ + PT_PSR]; \
	st	%pt_pc,  [%base_reg + SF_REGS_SZ + PT_PC]; \
	st	%pt_npc, [%base_reg + SF_REGS_SZ + PT_NPC];

#define PT_STORE_ALL(base_reg, reg_psr, reg_pc, reg_npc, g_scratch) \
	PT_STORE_PRIV(base_reg, reg_psr, reg_pc, reg_npc) \
	PT_STORE_GLOBALS(base_reg) \
	PT_STORE_YREG(base_reg, g_scratch) \
	PT_STORE_INS(base_reg)

/* Store the fpu register window*/
#define FW_STORE(reg) \
	std	%f0, [reg + FW_F0]; \
	std	%f2, [reg + FW_F2]; \
	std	%f4, [reg + FW_F4]; \
	std	%f6, [reg + FW_F6]; \
	std	%f8, [reg + FW_F8]; \
	std	%f10, [reg + FW_F10]; \
	std	%f12, [reg + FW_F12]; \
	std	%f14, [reg + FW_F14]; \
	std	%f16, [reg + FW_F16]; \
	std	%f18, [reg + FW_F18]; \
	std	%f20, [reg + FW_F20]; \
	std	%f22, [reg + FW_F22]; \
	std	%f24, [reg + FW_F24]; \
	std	%f26, [reg + FW_F26]; \
	std	%f28, [reg + FW_F28]; \
	std	%f30, [reg + FW_F30]; \
	st	%fsr, [reg + FW_FSR];

/* Load a fpu register window from the area beginning at reg. */
#define FW_LOAD(reg) \
	ldd	[reg + FW_F0], %f0; \
	ldd	[reg + FW_F2], %f2; \
	ldd	[reg + FW_F4], %f4; \
	ldd	[reg + FW_F6], %f6; \
	ldd	[reg + FW_F8], %f8; \
	ldd	[reg + FW_F10], %f10; \
	ldd	[reg + FW_F12], %f12; \
	ldd	[reg + FW_F14], %f14; \
	ldd	[reg + FW_F16], %f16; \
	ldd	[reg + FW_F18], %f18; \
	ldd	[reg + FW_F20], %f20; \
	ldd	[reg + FW_F22], %f22; \
	ldd	[reg + FW_F24], %f24; \
	ldd	[reg + FW_F26], %f26; \
	ldd	[reg + FW_F28], %f28; \
	ldd	[reg + FW_F30], %f30; \
	ld	[reg + FW_FSR], %fsr;

#endif
