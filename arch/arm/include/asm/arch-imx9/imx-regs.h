/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 */

#ifndef __ASM_ARCH_IMX9_REGS_H__
#define __ASM_ARCH_IMX9_REGS_H__

#define ARCH_MXC

#define IOMUXC_BASE_ADDR	0x443C0000UL
#define CCM_BASE_ADDR		0x44450000UL
#define CCM_CCGR_BASE_ADDR	0x44458000UL
#define SYSCNT_CTRL_BASE_ADDR	0x44290000

#define ANATOP_BASE_ADDR    0x44480000UL

#define WDG3_BASE_ADDR      0x42490000UL
#define WDG4_BASE_ADDR      0x424a0000UL
#define WDG5_BASE_ADDR      0x424b0000UL

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#include <stdbool.h>

struct mu_type {
	u32 ver;
	u32 par;
	u32 cr;
	u32 sr;
	u32 reserved0[60];
	u32 fcr;
	u32 fsr;
	u32 reserved1[2];
	u32 gier;
	u32 gcr;
	u32 gsr;
	u32 reserved2;
	u32 tcr;
	u32 tsr;
	u32 rcr;
	u32 rsr;
	u32 reserved3[52];
	u32 tr[16];
	u32 reserved4[16];
	u32 rr[16];
	u32 reserved5[14];
	u32 mu_attr;
};
#endif

#endif
