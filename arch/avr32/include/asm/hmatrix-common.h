/*
 * Copyright (C) 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_HMATRIX_COMMON_H__
#define __ASM_AVR32_HMATRIX_COMMON_H__

/* HMATRIX register offsets */
struct hmatrix_regs {
	u32	MCFG[16];
	u32	SCFG[16];
	struct {
		u32	A;
		u32	B;
	} PRS[16];
	u32	MRCR;
	u32	__reserved[3];
	u32	SFR[16];
};

/* Bitfields in MCFG */
#define HMATRIX_ULBT_OFFSET			0
#define HMATRIX_ULBT_SIZE			3

/* Bitfields in SCFG */
#define HMATRIX_SLOT_CYCLE_OFFSET		0
#define HMATRIX_SLOT_CYCLE_SIZE			8
#define HMATRIX_DEFMSTR_TYPE_OFFSET		16
#define HMATRIX_DEFMSTR_TYPE_SIZE		2
#define HMATRIX_FIXED_DEFMSTR_OFFSET		18
#define HMATRIX_FIXED_DEFMSTR_SIZE		4
#define HMATRIX_ARBT_OFFSET			24
#define HMATRIX_ARBT_SIZE			1

/* Bitfields in PRS.A */
#define HMATRIX_M0PR_OFFSET			0
#define HMATRIX_M0PR_SIZE			4
#define HMATRIX_M1PR_OFFSET			4
#define HMATRIX_M1PR_SIZE			4
#define HMATRIX_M2PR_OFFSET			8
#define HMATRIX_M2PR_SIZE			4
#define HMATRIX_M3PR_OFFSET			12
#define HMATRIX_M3PR_SIZE			4
#define HMATRIX_M4PR_OFFSET			16
#define HMATRIX_M4PR_SIZE			4
#define HMATRIX_M5PR_OFFSET			20
#define HMATRIX_M5PR_SIZE			4
#define HMATRIX_M6PR_OFFSET			24
#define HMATRIX_M6PR_SIZE			4
#define HMATRIX_M7PR_OFFSET			28
#define HMATRIX_M7PR_SIZE			4

/* Bitfields in PRS.B */
#define HMATRIX_M8PR_OFFSET			0
#define HMATRIX_M8PR_SIZE			4
#define HMATRIX_M9PR_OFFSET			4
#define HMATRIX_M9PR_SIZE			4
#define HMATRIX_M10PR_OFFSET			8
#define HMATRIX_M10PR_SIZE			4
#define HMATRIX_M11PR_OFFSET			12
#define HMATRIX_M11PR_SIZE			4
#define HMATRIX_M12PR_OFFSET			16
#define HMATRIX_M12PR_SIZE			4
#define HMATRIX_M13PR_OFFSET			20
#define HMATRIX_M13PR_SIZE			4
#define HMATRIX_M14PR_OFFSET			24
#define HMATRIX_M14PR_SIZE			4
#define HMATRIX_M15PR_OFFSET			28
#define HMATRIX_M15PR_SIZE			4

/* Constants for ULBT */
#define HMATRIX_ULBT_INFINITE			0
#define HMATRIX_ULBT_SINGLE			1
#define HMATRIX_ULBT_FOUR_BEAT			2
#define HMATRIX_ULBT_EIGHT_BEAT			3
#define HMATRIX_ULBT_SIXTEEN_BEAT		4

/* Constants for DEFMSTR_TYPE */
#define HMATRIX_DEFMSTR_TYPE_NO_DEFAULT		0
#define HMATRIX_DEFMSTR_TYPE_LAST_DEFAULT	1
#define HMATRIX_DEFMSTR_TYPE_FIXED_DEFAULT	2

/* Constants for ARBT */
#define HMATRIX_ARBT_ROUND_ROBIN		0
#define HMATRIX_ARBT_FIXED_PRIORITY		1

/* Bit manipulation macros */
#define HMATRIX_BIT(name)					\
	(1 << HMATRIX_##name##_OFFSET)
#define HMATRIX_BF(name,value)					\
	(((value) & ((1 << HMATRIX_##name##_SIZE) - 1))		\
	 << HMATRIX_##name##_OFFSET)
#define HMATRIX_BFEXT(name,value)				\
	(((value) >> HMATRIX_##name##_OFFSET)			\
	 & ((1 << HMATRIX_##name##_SIZE) - 1))
#define HMATRIX_BFINS(name,value,old)				\
	(((old) & ~(((1 << HMATRIX_##name##_SIZE) - 1)		\
		    << HMATRIX_##name##_OFFSET))		\
	 | HMATRIX_BF(name,value))

/* Register access macros */
#define __hmatrix_reg(reg)					\
	(((volatile struct hmatrix_regs *)ATMEL_BASE_HMATRIX)->reg)
#define hmatrix_read(reg)					\
	(__hmatrix_reg(reg))
#define hmatrix_write(reg, value)				\
	do { __hmatrix_reg(reg) = (value); } while (0)

#define hmatrix_slave_read(slave, reg)				\
	hmatrix_read(reg[HMATRIX_SLAVE_##slave])
#define hmatrix_slave_write(slave, reg, value)			\
	hmatrix_write(reg[HMATRIX_SLAVE_##slave], value)

#endif /* __ASM_AVR32_HMATRIX_COMMON_H__ */
