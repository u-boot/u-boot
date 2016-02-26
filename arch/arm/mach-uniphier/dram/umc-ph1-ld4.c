/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "../init.h"
#include "ddrphy-regs.h"
#include "umc-regs.h"

enum dram_freq {
	DRAM_FREQ_1333M,
	DRAM_FREQ_1600M,
	DRAM_FREQ_NR,
};

enum dram_size {
	DRAM_SZ_128M,
	DRAM_SZ_256M,
	DRAM_SZ_NR,
};

static u32 umc_cmdctla_plus[DRAM_FREQ_NR] = {0x45990b11, 0x36bb0f17};
static u32 umc_cmdctlb_plus[DRAM_FREQ_NR] = {0x16958924, 0x18c6aa24};
static u32 umc_spcctla[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	{0x00240512, 0x00350512},
	{0x002b0617, 0x003f0617},
};
static u32 umc_spcctlb[DRAM_FREQ_NR] = {0x00ff0006, 0x00ff0008};
static u32 umc_rdatactl[DRAM_FREQ_NR] = {0x000a00ac, 0x000c00ae};

static void umc_start_ssif(void __iomem *ssif_base)
{
	writel(0x00000000, ssif_base + 0x0000b004);
	writel(0xffffffff, ssif_base + 0x0000c004);
	writel(0x000fffcf, ssif_base + 0x0000c008);
	writel(0x00000001, ssif_base + 0x0000b000);
	writel(0x00000001, ssif_base + 0x0000c000);
	writel(0x03010101, ssif_base + UMC_MDMCHSEL);
	writel(0x03010100, ssif_base + UMC_DMDCHSEL);

	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_FETCH);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMQUE0);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMWC0);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMRC0);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMQUE1);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMWC1);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_COMRC1);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_WC);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_RC);
	writel(0x00000000, ssif_base + UMC_CLKEN_SSIF_DST);

	writel(0x00000001, ssif_base + UMC_CPURST);
	writel(0x00000001, ssif_base + UMC_IDSRST);
	writel(0x00000001, ssif_base + UMC_IXMRST);
	writel(0x00000001, ssif_base + UMC_MDMRST);
	writel(0x00000001, ssif_base + UMC_MDDRST);
	writel(0x00000001, ssif_base + UMC_SIORST);
	writel(0x00000001, ssif_base + UMC_VIORST);
	writel(0x00000001, ssif_base + UMC_FRCRST);
	writel(0x00000001, ssif_base + UMC_RGLRST);
	writel(0x00000001, ssif_base + UMC_AIORST);
	writel(0x00000001, ssif_base + UMC_DMDRST);
}

static int umc_dramcont_init(void __iomem *dramcont, void __iomem *ca_base,
			     int size, int freq)
{
	enum dram_freq freq_e;
	enum dram_size size_e;

	switch (freq) {
	case 1333:
		freq_e = DRAM_FREQ_1333M;
		break;
	case 1600:
		freq_e = DRAM_FREQ_1600M;
		break;
	default:
		pr_err("unsupported DRAM frequency %d MHz\n", freq);
		return -EINVAL;
	}

	switch (size) {
	case 0:
		return 0;
	case 1:
		size_e = DRAM_SZ_128M;
		break;
	case 2:
		size_e = DRAM_SZ_256M;
		break;
	default:
		pr_err("unsupported DRAM size\n");
		return -EINVAL;
	}

	writel(umc_cmdctla_plus[freq_e], dramcont + UMC_CMDCTLA);
	writel(umc_cmdctlb_plus[freq_e], dramcont + UMC_CMDCTLB);
	writel(umc_spcctla[freq_e][size_e], dramcont + UMC_SPCCTLA);
	writel(umc_spcctlb[freq_e], dramcont + UMC_SPCCTLB);
	writel(umc_rdatactl[freq_e], dramcont + UMC_RDATACTL_D0);
	writel(0x04060806, dramcont + UMC_WDATACTL_D0);
	writel(0x04a02000, dramcont + UMC_DATASET);
	writel(0x00000000, ca_base + 0x2300);
	writel(0x00400020, dramcont + UMC_DCCGCTL);
	writel(0x00000003, dramcont + 0x7000);
	writel(0x0000000f, dramcont + 0x8000);
	writel(0x000000c3, dramcont + 0x8004);
	writel(0x00000071, dramcont + 0x8008);
	writel(0x0000003b, dramcont + UMC_DICGCTLA);
	writel(0x020a0808, dramcont + UMC_DICGCTLB);
	writel(0x00000004, dramcont + UMC_FLOWCTLG);
	writel(0x80000201, ca_base + 0xc20);
	writel(0x0801e01e, dramcont + UMC_FLOWCTLA);
	writel(0x00200000, dramcont + UMC_FLOWCTLB);
	writel(0x00004444, dramcont + UMC_FLOWCTLC);
	writel(0x200a0a00, dramcont + UMC_SPCSETB);
	writel(0x00000000, dramcont + UMC_SPCSETD);
	writel(0x00000520, dramcont + UMC_DFICUPDCTLA);

	return 0;
}

static int umc_init_sub(int freq, int size_ch0, int size_ch1, bool ddr3plus)
{
	void __iomem *ssif_base = (void __iomem *)UMC_SSIF_BASE;
	void __iomem *ca_base0 = (void __iomem *)UMC_CA_BASE(0);
	void __iomem *ca_base1 = (void __iomem *)UMC_CA_BASE(1);
	void __iomem *dramcont0 = (void __iomem *)UMC_DRAMCONT_BASE(0);
	void __iomem *dramcont1 = (void __iomem *)UMC_DRAMCONT_BASE(1);
	void __iomem *phy0_0 = (void __iomem *)DDRPHY_BASE(0, 0);
	void __iomem *phy1_0 = (void __iomem *)DDRPHY_BASE(1, 0);

	umc_dram_init_start(dramcont0);
	umc_dram_init_start(dramcont1);
	umc_dram_init_poll(dramcont0);
	umc_dram_init_poll(dramcont1);

	writel(0x00000101, dramcont0 + UMC_DIOCTLA);

	ph1_ld4_ddrphy_init(phy0_0, freq, ddr3plus);

	ddrphy_prepare_training(phy0_0, 0);
	ddrphy_training(phy0_0);

	writel(0x00000101, dramcont1 + UMC_DIOCTLA);

	ph1_ld4_ddrphy_init(phy1_0, freq, ddr3plus);

	ddrphy_prepare_training(phy1_0, 1);
	ddrphy_training(phy1_0);

	umc_dramcont_init(dramcont0, ca_base0, size_ch0, freq);
	umc_dramcont_init(dramcont1, ca_base1, size_ch1, freq);

	umc_start_ssif(ssif_base);

	return 0;
}

int ph1_ld4_umc_init(const struct uniphier_board_data *bd)
{
	if ((bd->dram_ch[0].size == SZ_128M || bd->dram_ch[0].size == SZ_256M) &&
	    (bd->dram_ch[1].size == SZ_128M || bd->dram_ch[1].size == SZ_256M) &&
	    (bd->dram_freq == 1333 || bd->dram_freq == 1600) &&
	    bd->dram_ch[0].width == 16 && bd->dram_ch[1].width == 16) {
		return umc_init_sub(bd->dram_freq,
				    bd->dram_ch[0].size / SZ_128M,
				    bd->dram_ch[1].size / SZ_128M,
				    bd->dram_ddr3plus);
	} else {
		pr_err("Unsupported DDR configuration\n");
		return -EINVAL;
	}
}
