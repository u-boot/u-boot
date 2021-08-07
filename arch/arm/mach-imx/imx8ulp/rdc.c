// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <div64.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define XRDC_ADDR	0x292f0000
#define MRC_OFFSET	0x2000
#define MRC_STEP	0x200

#define SP(X)		((X) << 9)
#define SU(X)		((X) << 6)
#define NP(X)		((X) << 3)
#define NU(X)		((X) << 0)

#define RWX		7
#define RW		6
#define R		4
#define X		1

#define D7SEL_CODE	(SP(RW) | SU(RW) | NP(RWX) | NU(RWX))
#define D6SEL_CODE	(SP(RW) | SU(RW) | NP(RWX))
#define D5SEL_CODE	(SP(RW) | SU(RWX))
#define D4SEL_CODE	SP(RWX)
#define D3SEL_CODE	(SP(X) | SU(X) | NP(X) | NU(X))
#define D0SEL_CODE	0

#define D7SEL_DAT	(SP(RW) | SU(RW) | NP(RW) | NU(RW))
#define D6SEL_DAT	(SP(RW) | SU(RW) | NP(RW))
#define D5SEL_DAT	(SP(RW) | SU(RW) | NP(R) | NU(R))
#define D4SEL_DAT	(SP(RW) | SU(RW))
#define D3SEL_DAT	SP(RW)

union dxsel_perm {
	struct {
		u8 dx;
		u8 perm;
	};

	u32 dom_perm;
};

int xrdc_config_mrc_dx_perm(u32 mrc_con, u32 region, u32 dom, u32 dxsel)
{
	ulong w2_addr;
	u32 val = 0;

	w2_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20 + 0x8;

	val = (readl(w2_addr) & (~(7 << (3 * dom)))) | (dxsel << (3 * dom));
	writel(val, w2_addr);

	return 0;
}

int xrdc_config_mrc_w0_w1(u32 mrc_con, u32 region, u32 w0, u32 size)
{
	ulong w0_addr, w1_addr;

	w0_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20;
	w1_addr = w0_addr + 4;

	if ((size % 32) != 0)
		return -EINVAL;

	writel(w0 & ~0x1f, w0_addr);
	writel(w0 + size - 1, w1_addr);

	return 0;
}

int xrdc_config_mrc_w3_w4(u32 mrc_con, u32 region, u32 w3, u32 w4)
{
	ulong w3_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20 + 0xC;
	ulong w4_addr = w3_addr + 4;

	writel(w3, w3_addr);
	writel(w4, w4_addr);

	return 0;
}

int xrdc_config_pdac_openacc(u32 bridge, u32 index)
{
	ulong w0_addr;
	u32 val;

	switch (bridge) {
	case 3:
		w0_addr = XRDC_ADDR + 0x1000 + 0x8 * index;
		break;
	case 4:
		w0_addr = XRDC_ADDR + 0x1400 + 0x8 * index;
		break;
	case 5:
		w0_addr = XRDC_ADDR + 0x1800 + 0x8 * index;
		break;
	default:
		return -EINVAL;
	}
	writel(0xffffff, w0_addr);

	val = readl(w0_addr + 4);
	writel(val | BIT(31), w0_addr + 4);

	return 0;
}

int xrdc_config_pdac(u32 bridge, u32 index, u32 dom, u32 perm)
{
	ulong w0_addr;
	u32 val;

	switch (bridge) {
	case 3:
		w0_addr = XRDC_ADDR + 0x1000 + 0x8 * index;
		break;
	case 4:
		w0_addr = XRDC_ADDR + 0x1400 + 0x8 * index;
		break;
	case 5:
		w0_addr = XRDC_ADDR + 0x1800 + 0x8 * index;
		break;
	default:
		return -EINVAL;
	}
	val = readl(w0_addr);
	writel((val & ~(0x7 << (dom * 3))) | (perm << (dom * 3)), w0_addr);

	val = readl(w0_addr + 4);
	writel(val | BIT(31), w0_addr + 4);

	return 0;
}
