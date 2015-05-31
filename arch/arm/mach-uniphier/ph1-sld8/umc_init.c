/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <mach/umc-regs.h>
#include <mach/ddrphy-regs.h>

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

static void umc_dramcont_init(void __iomem *dramcont, void __iomem *ca_base,
			      int size, int freq)
{
#ifdef CONFIG_DDR_STANDARD
	writel(0x55990b11, dramcont + UMC_CMDCTLA);
	writel(0x16958944, dramcont + UMC_CMDCTLB);
#else
	writel(0x45990b11, dramcont + UMC_CMDCTLA);
	writel(0x16958924, dramcont + UMC_CMDCTLB);
#endif

	writel(0x5101046A, dramcont + UMC_INITCTLA);

	if (size == 1)
		writel(0x27028B0A, dramcont + UMC_INITCTLB);
	else if (size == 2)
		writel(0x38028B0A, dramcont + UMC_INITCTLB);

	writel(0x00FF00FF, dramcont + UMC_INITCTLC);
	writel(0x00000b51, dramcont + UMC_DRMMR0);
	writel(0x00000006, dramcont + UMC_DRMMR1);
	writel(0x00000290, dramcont + UMC_DRMMR2);

#ifdef CONFIG_DDR_STANDARD
	writel(0x00000000, dramcont + UMC_DRMMR3);
#else
	writel(0x00000800, dramcont + UMC_DRMMR3);
#endif

	if (size == 1)
		writel(0x00240512, dramcont + UMC_SPCCTLA);
	else if (size == 2)
		writel(0x00350512, dramcont + UMC_SPCCTLA);

	writel(0x00ff0006, dramcont + UMC_SPCCTLB);
	writel(0x000a00ac, dramcont + UMC_RDATACTL_D0);
	writel(0x04060806, dramcont + UMC_WDATACTL_D0);
	writel(0x04a02000, dramcont + UMC_DATASET);
	writel(0x00000000, ca_base + 0x2300);
	writel(0x00400020, dramcont + UMC_DCCGCTL);
	writel(0x00000003, dramcont + 0x7000);
	writel(0x0000004f, dramcont + 0x8000);
	writel(0x000000c3, dramcont + 0x8004);
	writel(0x00000077, dramcont + 0x8008);
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
}

static int umc_init_sub(int freq, int size_ch0, int size_ch1)
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

	ddrphy_init(phy0_0, freq, size_ch0);

	ddrphy_prepare_training(phy0_0, 0);
	ddrphy_training(phy0_0);

	writel(0x00000101, dramcont1 + UMC_DIOCTLA);

	ddrphy_init(phy1_0, freq, size_ch1);

	ddrphy_prepare_training(phy1_0, 1);
	ddrphy_training(phy1_0);

	umc_dramcont_init(dramcont0, ca_base0, size_ch0, freq);
	umc_dramcont_init(dramcont1, ca_base1, size_ch1, freq);

	umc_start_ssif(ssif_base);

	return 0;
}

int umc_init(void)
{
	return umc_init_sub(CONFIG_DDR_FREQ, CONFIG_SDRAM0_SIZE / 0x08000000,
					CONFIG_SDRAM1_SIZE / 0x08000000);
}

#if (CONFIG_SDRAM0_SIZE == 0x08000000 || CONFIG_SDRAM0_SIZE == 0x10000000) && \
    (CONFIG_SDRAM1_SIZE == 0x08000000 || CONFIG_SDRAM1_SIZE == 0x10000000) && \
    CONFIG_DDR_NUM_CH0 == 1 && CONFIG_DDR_NUM_CH1 == 1
/* OK */
#else
#error Unsupported DDR configuration.
#endif
