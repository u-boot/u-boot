/*
 * Copyright (C) 2016 Socionext Inc.
 *
 * based on commit f7a4c9efe333fb1536efa86f9e96dc0ee109fedd of Diag
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/processor.h>

#include "../init.h"
#include "ddrphy-ld20-regs.h"
#include "umc-ld20-regs.h"

#define DRAM_CH_NR	3

enum dram_freq {
	DRAM_FREQ_1866M,
	DRAM_FREQ_NR,
};

enum dram_size {
	DRAM_SZ_256M,
	DRAM_SZ_512M,
	DRAM_SZ_NR,
};

/* umc */
static u32 umc_initctla[DRAM_FREQ_NR] = {0x71016D11};
static u32 umc_initctlb[DRAM_FREQ_NR] = {0x07E390AC};
static u32 umc_initctlc[DRAM_FREQ_NR] = {0x00FF00FF};
static u32 umc_drmmr0[DRAM_FREQ_NR] = {0x00000114};
static u32 umc_drmmr2[DRAM_FREQ_NR] = {0x000002a0};

static u32 umc_memconf0a[DRAM_FREQ_NR] = {0x00000801};
static u32 umc_memconf0b[DRAM_FREQ_NR] = {0x00000130};
static u32 umc_memconfch[DRAM_FREQ_NR] = {0x00033803};

static u32 umc_cmdctla[DRAM_FREQ_NR] = {0x060D0D20};
static u32 umc_cmdctlb[DRAM_FREQ_NR] = {0x2D211C08};
static u32 umc_cmdctlc[DRAM_FREQ_NR] = {0x00150C04};
static u32 umc_cmdctle[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	{0x0049071D, 0x0078071D},
};

static u32 umc_rdatactl_d0[DRAM_FREQ_NR] = {0x00000610};
static u32 umc_rdatactl_d1[DRAM_FREQ_NR] = {0x00000610};
static u32 umc_wdatactl_d0[DRAM_FREQ_NR] = {0x00000204};
static u32 umc_wdatactl_d1[DRAM_FREQ_NR] = {0x00000204};
static u32 umc_odtctl_d0[DRAM_FREQ_NR] = {0x02000002};
static u32 umc_odtctl_d1[DRAM_FREQ_NR] = {0x02000002};
static u32 umc_dataset[DRAM_FREQ_NR] = {0x04000000};

static u32 umc_flowctla[DRAM_FREQ_NR] = {0x0081E01E};
static u32 umc_directbusctrla[DRAM_CH_NR] = {
	0x00000000, 0x00000001, 0x00000001
};

/* DDR PHY */
static void ddrphy_init(void __iomem *phy_base, enum dram_freq freq)
{
	writel(0x00000001, phy_base + PHY_UNIQUIFY_TSMC_IO_1);
	while ((readl(phy_base + PHY_UNIQUIFY_TSMC_IO_1) & BIT(1)))
		cpu_relax();

	writel(0x00000000, phy_base + PHY_DLL_INCR_TRIM_3);
	writel(0x00000000, phy_base + PHY_DLL_INCR_TRIM_1);
	writel(0x00000000, phy_base + PHY_LANE_SEL);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	writel(0x00000006, phy_base + PHY_LANE_SEL);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	writel(0x0000000c, phy_base + PHY_LANE_SEL);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	writel(0x00000012, phy_base + PHY_LANE_SEL);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	writel(0x00000001, phy_base + PHY_SCL_WINDOW_TRIM);
	writel(0x00000000, phy_base + PHY_UNQ_ANALOG_DLL_1);
	writel(0x50bb40b1, phy_base + PHY_PAD_CTRL);
	writel(0x00000070, phy_base + PHY_VREF_TRAINING);
	writel(0x01000075, phy_base + PHY_SCL_CONFIG_1);
	writel(0x00000501, phy_base + PHY_SCL_CONFIG_2);
	writel(0x00000000, phy_base + PHY_SCL_CONFIG_3);
	writel(0x000261c0, phy_base + PHY_DYNAMIC_WRITE_BIT_LVL);
	writel(0x00000000, phy_base + PHY_SCL_CONFIG_4);
	writel(0x000000a0, phy_base + PHY_SCL_GATE_TIMING);
	writel(0x02a000a0, phy_base + PHY_WRLVL_DYN_ODT);
	writel(0x00840004, phy_base + PHY_WRLVL_ON_OFF);
	writel(0x0000020d, phy_base + PHY_DLL_ADRCTRL);
	writel(0x00000000, phy_base + PHY_LANE_SEL);
	writel(0x0000008d, phy_base + PHY_DLL_TRIM_CLK);
	writel(0xa800100d, phy_base + PHY_DLL_RECALIB);
	writel(0x00005076, phy_base + PHY_SCL_LATENCY);
}

static int ddrphy_training(void __iomem *phy_base)
{
	writel(0x0000000f, phy_base + PHY_WRLVL_AUTOINC_TRIM);
	writel(0x00010000, phy_base + PHY_DLL_TRIM_2);
	writel(0x50000000, phy_base + PHY_SCL_START);

	while ((readl(phy_base + PHY_SCL_START) & BIT(28)))
		cpu_relax();

	writel(0x00000000, phy_base + PHY_DISABLE_GATING_FOR_SCL);
	writel(0xff00ff00, phy_base + PHY_SCL_DATA_0);
	writel(0xff00ff00, phy_base + PHY_SCL_DATA_1);
	writel(0x00080000, phy_base + PHY_SCL_START_ADDR);
	writel(0x11000000, phy_base + PHY_SCL_START);

	while ((readl(phy_base + PHY_SCL_START) & BIT(28)))
		cpu_relax();

	writel(0x00000000, phy_base + PHY_SCL_START_ADDR);
	writel(0x30500000, phy_base + PHY_SCL_START);

	while ((readl(phy_base + PHY_SCL_START) & BIT(28)))
		cpu_relax();

	writel(0x00000001, phy_base + PHY_DISABLE_GATING_FOR_SCL);
	writel(0x00000010, phy_base + PHY_SCL_MAIN_CLK_DELTA);
	writel(0x789b3de0, phy_base + PHY_SCL_DATA_0);
	writel(0xf10e4a56, phy_base + PHY_SCL_DATA_1);
	writel(0x11000000, phy_base + PHY_SCL_START);

	while ((readl(phy_base + PHY_SCL_START) & BIT(28)))
		cpu_relax();

	writel(0x34000000, phy_base + PHY_SCL_START);

	while ((readl(phy_base + PHY_SCL_START) & BIT(28)))
		cpu_relax();

	writel(0x00000003, phy_base + PHY_DISABLE_GATING_FOR_SCL);

	return 0;
}

static int umc_dc_init(void __iomem *dc_base, enum dram_freq freq,
		       unsigned long size, int ch)
{
	enum dram_size size_e;

	switch (size) {
	case 0:
		return 0;
	case SZ_256M:
		size_e = DRAM_SZ_256M;
		break;
	case SZ_512M:
		size_e = DRAM_SZ_512M;
		break;
	default:
		pr_err("unsupported DRAM size 0x%08lx (per 16bit) for ch%d\n",
		       size, ch);
		return -EINVAL;
	}

	/* Wait for PHY Init Complete */
	while (!(readl(dc_base + UMC_DFISTCTLC) & BIT(0)))
		cpu_relax();

	writel(0x00000001, dc_base + UMC_DFICSOVRRD);
	writel(0x00000000, dc_base + UMC_DFITURNOFF);

	writel(umc_initctla[freq], dc_base + UMC_INITCTLA);
	writel(umc_initctlb[freq], dc_base + UMC_INITCTLB);
	writel(umc_initctlc[freq], dc_base + UMC_INITCTLC);

	writel(umc_drmmr0[freq], dc_base + UMC_DRMMR0);
	writel(0x00000004, dc_base + UMC_DRMMR1);
	writel(umc_drmmr2[freq], dc_base + UMC_DRMMR2);
	writel(0x00000000, dc_base + UMC_DRMMR3);

	writel(umc_memconf0a[freq], dc_base + UMC_MEMCONF0A);
	writel(umc_memconf0b[freq], dc_base + UMC_MEMCONF0B);
	writel(umc_memconfch[freq], dc_base + UMC_MEMCONFCH);
	writel(0x00000008, dc_base + UMC_MEMMAPSET);

	writel(umc_cmdctla[freq], dc_base + UMC_CMDCTLA);
	writel(umc_cmdctlb[freq], dc_base + UMC_CMDCTLB);
	writel(umc_cmdctlc[freq], dc_base + UMC_CMDCTLC);
	writel(umc_cmdctle[freq][size_e], dc_base + UMC_CMDCTLE);

	writel(umc_rdatactl_d0[freq], dc_base + UMC_RDATACTL_D0);
	writel(umc_rdatactl_d1[freq], dc_base + UMC_RDATACTL_D1);

	writel(umc_wdatactl_d0[freq], dc_base + UMC_WDATACTL_D0);
	writel(umc_wdatactl_d1[freq], dc_base + UMC_WDATACTL_D1);
	writel(umc_odtctl_d0[freq], dc_base + UMC_ODTCTL_D0);
	writel(umc_odtctl_d1[freq], dc_base + UMC_ODTCTL_D1);
	writel(umc_dataset[freq], dc_base + UMC_DATASET);

	writel(0x00400020, dc_base + UMC_DCCGCTL);
	writel(0x00000003, dc_base + UMC_ACSCTLA);
	writel(0x00000103, dc_base + UMC_FLOWCTLG);
	writel(0x00010200, dc_base + UMC_ACSSETA);

	writel(umc_flowctla[freq], dc_base + UMC_FLOWCTLA);
	writel(0x00004444, dc_base + UMC_FLOWCTLC);
	writel(0x00000000, dc_base + UMC_DFICUPDCTLA);

	writel(0x00202000, dc_base + UMC_FLOWCTLB);
	writel(0x00000000, dc_base + UMC_BSICMAPSET);
	writel(0x00000000, dc_base + UMC_ERRMASKA);
	writel(0x00000000, dc_base + UMC_ERRMASKB);

	writel(umc_directbusctrla[ch], dc_base + UMC_DIRECTBUSCTRLA);

	writel(0x00000001, dc_base + UMC_INITSET);
	/* Wait for PHY Init Complete */
	while (readl(dc_base + UMC_INITSTAT) & BIT(0))
		cpu_relax();

	writel(0x2A0A0A00, dc_base + UMC_SPCSETB);
	writel(0x00000000, dc_base + UMC_DFICSOVRRD);

	return 0;
}

static int umc_ch_init(void __iomem *umc_ch_base, void __iomem *phy_ch_base,
		       enum dram_freq freq, unsigned long size, int ch)
{
	void __iomem *dc_base = umc_ch_base + 0x00011000;
	void __iomem *phy_base = phy_ch_base;
	int ret;

	/* PHY Update Mode (ON) */
	writel(0x8000003f, dc_base + UMC_DFIPUPDCTLA);

	/* deassert PHY reset signals */
	writel(UMC_DIOCTLA_CTL_NRST | UMC_DIOCTLA_CFG_NRST,
	       dc_base + UMC_DIOCTLA);

	ddrphy_init(phy_base, freq);

	ret = umc_dc_init(dc_base, freq, size, ch);
	if (ret)
		return ret;

	ret = ddrphy_training(phy_base);
	if (ret)
		return ret;

	return 0;
}

static void um_init(void __iomem *um_base)
{
	writel(0x000000ff, um_base + UMC_MBUS0);
	writel(0x000000ff, um_base + UMC_MBUS1);
	writel(0x000000ff, um_base + UMC_MBUS2);
	writel(0x00000001, um_base + UMC_MBUS3);
	writel(0x00000001, um_base + UMC_MBUS4);
	writel(0x00000001, um_base + UMC_MBUS5);
	writel(0x00000001, um_base + UMC_MBUS6);
	writel(0x00000001, um_base + UMC_MBUS7);
	writel(0x00000001, um_base + UMC_MBUS8);
	writel(0x00000001, um_base + UMC_MBUS9);
	writel(0x00000001, um_base + UMC_MBUS10);
}

int uniphier_ld20_umc_init(const struct uniphier_board_data *bd)
{
	void __iomem *um_base = (void __iomem *)0x5b600000;
	void __iomem *umc_ch_base = (void __iomem *)0x5b800000;
	void __iomem *phy_ch_base = (void __iomem *)0x6e200000;
	enum dram_freq freq;
	int ch, ret;

	switch (bd->dram_freq) {
	case 1866:
		freq = DRAM_FREQ_1866M;
		break;
	default:
		pr_err("unsupported DRAM frequency %d MHz\n", bd->dram_freq);
		return -EINVAL;
	}

	for (ch = 0; ch < bd->dram_nr_ch; ch++) {
		unsigned long size = bd->dram_ch[ch].size;
		unsigned int width = bd->dram_ch[ch].width;

		ret = umc_ch_init(umc_ch_base, phy_ch_base, freq,
				  size / (width / 16), ch);
		if (ret) {
			pr_err("failed to initialize UMC ch%d\n", ch);
			return ret;
		}

		umc_ch_base += 0x00200000;
		phy_ch_base += 0x00004000;
	}

	um_init(um_base);

	return 0;
}
