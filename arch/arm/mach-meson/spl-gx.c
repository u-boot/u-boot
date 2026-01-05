// SPDX-License-Identifier: GPL-2.0+
/*
 * Portions Copyright (C) 2015, Amlogic, Inc. All rights reserved.
 * Copyright (C) 2023, Ferass El Hafidi <funderscore@postmarketos.org>
 */
#include <hang.h>
#include <image.h>
#include <spl.h>
#include <vsprintf.h>
#include <asm/io.h>
#include <asm/arch/boot.h>
#include <asm/arch/clock-gx.h>
#include <asm/arch/gx.h>
#include <linux/delay.h>

/* Meson GX SPL code */

inline void cpu_pll_switch_to(int mode)
{
	u32 reg;

	reg = readl(GX_HIU_BASE + HHI_SYS_CPU_CLK_CNTL0);

	while (reg & HHI_SCC_CNTL0_BUSY)
		reg = readl(GX_HIU_BASE + HHI_SYS_CPU_CLK_CNTL0);

	reg |= HHI_SCC_CNTL0_DYN_ENABLE;

	if (mode == 1) {
		/* Switch to System PLL */
		reg |= HHI_SCC_CNTL0_FINAL_MUX_SEL;
	} else {
		if (reg & HHI_SCC_CNTL0_FINAL_DYN_MUX_SEL) {
			reg = (reg & ~(HHI_SCC_CNTL0_FINAL_DYN_MUX_SEL |
				HHI_SCC_CNTL0_MUX0_DIVN_TCNT |
				HHI_SCC_CNTL0_POSTMUX0 | HHI_SCC_CNTL0_PREMUX0));
		} else {
			reg = (reg & ~(HHI_SCC_CNTL0_FINAL_DYN_MUX_SEL |
				HHI_SCC_CNTL0_MUX1_DIVN_TCNT |
				(HHI_SCC_CNTL0_POSTMUX1 | HHI_SCC_CNTL0_PREMUX1))) |
				HHI_SCC_CNTL0_FINAL_DYN_MUX_SEL;
		}
		/* Select dynamic mux */
		reg = reg & ~(HHI_SCC_CNTL0_FINAL_MUX_SEL) /*final_mux_sel*/;
	}
	writel(reg, GX_HIU_BASE + HHI_SYS_CPU_CLK_CNTL0);
}

int meson_pll_init(void)
{
	clrbits_32(GX_HIU_BASE + HHI_MPEG_CLK_CNTL, 1 << 8);
	cpu_pll_switch_to(0);

	setbits_32(GX_HIU_BASE + HHI_MPLL_CNTL6, 1 << 26);
	udelay(100);

	while (!((readl(GX_HIU_BASE + HHI_SYS_PLL_CNTL) >> 31) & 1)) {
		if (IS_ENABLED(CONFIG_MESON_GXBB)) {
			setbits_32(GX_HIU_BASE + HHI_SYS_PLL_CNTL, 1 << 29);
			writel(0x5ac80000, GX_HIU_BASE + HHI_SYS_PLL_CNTL2);
			writel(0x8e452015, GX_HIU_BASE + HHI_SYS_PLL_CNTL3);
			writel(0x401d40c, GX_HIU_BASE + HHI_SYS_PLL_CNTL4);
			writel(0x870, GX_HIU_BASE + HHI_SYS_PLL_CNTL5);
			writel((1 << 30) | (1 << 29) |
				((0 << 16) | (1 << 9) |
				(1536 / 24)), /* 1.5 GHz */
				GX_HIU_BASE + HHI_SYS_PLL_CNTL);
			clrbits_32(GX_HIU_BASE + HHI_SYS_PLL_CNTL, 1 << 29);
		} else if (IS_ENABLED(CONFIG_MESON_GXL)) {
			writel(0xc4258100, GX_HIU_BASE + HHI_SYS_PLL_CNTL1);
			writel(0xb7400000, GX_HIU_BASE + HHI_SYS_PLL_CNTL2);
			writel(0xa59a288, GX_HIU_BASE + HHI_SYS_PLL_CNTL3);
			writel(0x40002d, GX_HIU_BASE + HHI_SYS_PLL_CNTL4);
			writel(0x7c700007, GX_HIU_BASE + HHI_SYS_PLL_CNTL5);
			writel((1 << 30) | ((1 << 9) |
				(1200 / 24)), /* 1.2 GHz */
				GX_HIU_BASE + HHI_SYS_PLL_CNTL);
		}
		udelay(20);
	}
	cpu_pll_switch_to(1); /* Hook the CPU to the PLL divider output */

	if (IS_ENABLED(CONFIG_MESON_GXBB))
		writel(0x10007, GX_HIU_BASE + HHI_MPLL_CNTL4);
	else if (IS_ENABLED(CONFIG_MESON_GXL))
		writel(0x10006, GX_HIU_BASE + HHI_MPLL_CNTL4);

	setbits_32(GX_HIU_BASE + HHI_MPLL_CNTL, 1 << 29);
	udelay(200);

	writel(0x59C80000, GX_HIU_BASE + HHI_MPLL_CNTL2);
	writel(0xCA45B822, GX_HIU_BASE + HHI_MPLL_CNTL3);

	if (IS_ENABLED(CONFIG_MESON_GXBB))
		writel(0xB5500E1A, GX_HIU_BASE + HHI_MPLL_CNTL5);
	else if (IS_ENABLED(CONFIG_MESON_GXL))
		writel(0x95520E1A, GX_HIU_BASE + HHI_MPLL_CNTL5);

	writel(0xFC454545, GX_HIU_BASE + HHI_MPLL_CNTL6);

	if (IS_ENABLED(CONFIG_MESON_GXBB)) {
		writel((1 << 30) | (1 << 29) | (3 << 9) | (250 << 0), GX_HIU_BASE + HHI_MPLL_CNTL);
		clrbits_32(GX_HIU_BASE + HHI_MPLL_CNTL, 1 << 29);
	} else if (IS_ENABLED(CONFIG_MESON_GXL)) {
		writel((1 << 30) | (3 << 9) | (250 << 0), GX_HIU_BASE + HHI_MPLL_CNTL);
	}
	udelay(800);

	setbits_32(GX_HIU_BASE + HHI_MPLL_CNTL4, 1 << 14);

	while (!((readl(GX_HIU_BASE + HHI_MPLL_CNTL) >> 31) & 1)) {
		if ((readl(GX_HIU_BASE + HHI_MPLL_CNTL) & (1 << 31)) != 0)
			break;
		setbits_32(GX_HIU_BASE + HHI_MPLL_CNTL, 1 << 29);
		udelay(1000);
		clrbits_32(GX_HIU_BASE + HHI_MPLL_CNTL, 1 << 29);
		udelay(1000);
	}

	if (IS_ENABLED(CONFIG_MESON_GXBB)) {
		writel(0xFFF << 16, GX_HIU_BASE + HHI_MPLL_CNTL10);
		writel(((7 << 16) | (1 << 15) | (1 << 14) | (4681 << 0)),
		       GX_HIU_BASE + HHI_MPLL_CNTL7);
		writel(((readl(GX_HIU_BASE + HHI_MPEG_CLK_CNTL) & (~((0x7 << 12) | (1 << 7) |
			(0x7F << 0)))) | ((5 << 12) | (1 << 7)	| (2 << 0))),
			GX_HIU_BASE + HHI_MPEG_CLK_CNTL);
		setbits_32(GX_HIU_BASE + HHI_MPEG_CLK_CNTL, 1 << 8);
		writel(((5 << 16) | (1 << 15) | (1 << 14) | (12524 << 0)),
		       GX_HIU_BASE + HHI_MPLL_CNTL8);
	} else if (IS_ENABLED(CONFIG_MESON_GXL)) {
		writel((1 << 12) | 3, GX_HIU_BASE + HHI_MPLL_CNTL10);
		writel(0x5edb7, GX_HIU_BASE + HHI_MPLL_CNTL7);
		clrbits_32(GX_HIU_BASE + HHI_MPEG_CLK_CNTL,
			   (3 << 13) | (1 << 12) | (15 << 4) | 15);
		setbits_32(GX_HIU_BASE + HHI_MPEG_CLK_CNTL,
			   (1 << 14) | (1 << 12) | (1 << 8) | (2 << 6) | (1 << 1));
		writel((4 << 16) | (7 << 13) | (1 << 8) | (5 << 4) | 10,
		       GX_HIU_BASE + HHI_MPLL_CNTL8);
	}

	udelay(200);

	/* TODO: Some error handling and timeouts... */
	return 0;
}

#if CONFIG_IS_ENABLED(MMC) && !CONFIG_IS_ENABLED(DM_MMC)
int board_mmc_init(struct bd_info *bis)
{
	int mmc_device;

	switch (meson_get_boot_device()) {
	case BOOT_DEVICE_SD:
		mmc_device = 0;
		break;
	case BOOT_DEVICE_EMMC:
		mmc_device = 1;
		break;
	default:
		return -1;
	}

	if (!meson_mmc_init(mmc_device))
		return -1;

	return 0;
}
#endif
