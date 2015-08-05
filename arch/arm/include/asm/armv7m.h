/*
 * (C) Copyright 2010,2011
 * Vladimir Khusainov, Emcraft Systems, vlad@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARMV7M_H
#define ARMV7M_H

#if defined(__ASSEMBLY__)
.syntax unified
.thumb
#endif

#define V7M_SCB_BASE		0xE000ED00
#define V7M_MPU_BASE		0xE000ED90

#define V7M_SCB_VTOR		0x08

#if !defined(__ASSEMBLY__)
struct v7m_scb {
	uint32_t cpuid;		/* CPUID Base Register */
	uint32_t icsr;		/* Interrupt Control and State Register */
	uint32_t vtor;		/* Vector Table Offset Register */
	uint32_t aircr;		/* App Interrupt and Reset Control Register */
};
#define V7M_SCB				((struct v7m_scb *)V7M_SCB_BASE)

#define V7M_AIRCR_VECTKEY		0x5fa
#define V7M_AIRCR_VECTKEY_SHIFT		16
#define V7M_AIRCR_ENDIAN		(1 << 15)
#define V7M_AIRCR_PRIGROUP_SHIFT	8
#define V7M_AIRCR_PRIGROUP_MSK		(0x7 << V7M_AIRCR_PRIGROUP_SHIFT)
#define V7M_AIRCR_SYSRESET		(1 << 2)

#define V7M_ICSR_VECTACT_MSK		0xFF

struct v7m_mpu {
	uint32_t type;		/* Type Register */
	uint32_t ctrl;		/* Control Register */
	uint32_t rnr;		/* Region Number Register */
	uint32_t rbar;		/* Region Base Address Register */
	uint32_t rasr;		/* Region Attribute and Size Register */
};
#define V7M_MPU				((struct v7m_mpu *)V7M_MPU_BASE)

#define V7M_MPU_CTRL_ENABLE		(1 << 0)
#define V7M_MPU_CTRL_HFNMIENA		(1 << 1)

#define V7M_MPU_RASR_EN			(1 << 0)
#define V7M_MPU_RASR_SIZE_BITS		1
#define V7M_MPU_RASR_SIZE_4GB		(31 << V7M_MPU_RASR_SIZE_BITS)
#define V7M_MPU_RASR_AP_RW_RW		(3 << 24)

#endif /* !defined(__ASSEMBLY__) */
#endif /* ARMV7M_H */
