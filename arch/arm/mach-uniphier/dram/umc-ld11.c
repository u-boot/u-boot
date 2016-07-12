/*
 * Copyright (C) 2016 Socionext Inc.
 */

#include <common.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/processor.h>

#include "../init.h"
#include "umc64-regs.h"

#define CONFIG_DDR_FREQ		1866

#define DRAM_CH_NR	2

enum dram_freq {
	DRAM_FREQ_1600M,
	DRAM_FREQ_NR,
};

enum dram_size {
	DRAM_SZ_256M,
	DRAM_SZ_512M,
	DRAM_SZ_NR,
};

/* umc */
static u32 umc_cmdctla[DRAM_FREQ_NR] = {0x060D0D20};
static u32 umc_cmdctlb[DRAM_FREQ_NR] = {0x2D211C08};
static u32 umc_cmdctlc[DRAM_FREQ_NR] = {0x00150C04};
static u32 umc_cmdctle[DRAM_FREQ_NR] = {0x0078071D};
static u32 umc_cmdctlf[DRAM_FREQ_NR] = {0x02000200};
static u32 umc_cmdctlg[DRAM_FREQ_NR] = {0x08080808};

static u32 umc_rdatactl_d0[DRAM_FREQ_NR] = {0x00000810};
static u32 umc_rdatactl_d1[DRAM_FREQ_NR] = {0x00000810};
static u32 umc_wdatactl_d0[DRAM_FREQ_NR] = {0x00000004};
static u32 umc_wdatactl_d1[DRAM_FREQ_NR] = {0x00000004};
static u32 umc_odtctl_d0[DRAM_FREQ_NR] = {0x02000002};
static u32 umc_odtctl_d1[DRAM_FREQ_NR] = {0x02000002};
static u32 umc_acssetb[DRAM_CH_NR] = {0x00000200, 0x00000203};
static u32 umc_memconfch[DRAM_FREQ_NR] = {0x00023605};

static int umc_dc_init(void __iomem *dc_base, enum dram_freq freq,
		       unsigned long size, int ch)
{
	writel(umc_cmdctla[freq], dc_base + UMC_CMDCTLA);
	writel(umc_cmdctlb[freq], dc_base + UMC_CMDCTLB);
	writel(umc_cmdctlc[freq], dc_base + UMC_CMDCTLC);
	writel(umc_cmdctle[freq], dc_base + UMC_CMDCTLE);
	writel(umc_cmdctlf[freq], dc_base + UMC_CMDCTLF);
	writel(umc_cmdctlg[freq], dc_base + UMC_CMDCTLG);

	writel(umc_rdatactl_d0[freq], dc_base + UMC_RDATACTL_D0);
	writel(umc_rdatactl_d1[freq], dc_base + UMC_RDATACTL_D1);

	writel(umc_wdatactl_d0[freq], dc_base + UMC_WDATACTL_D0);
	writel(umc_wdatactl_d1[freq], dc_base + UMC_WDATACTL_D1);

	writel(umc_odtctl_d0[freq], dc_base + UMC_ODTCTL_D0);
	writel(umc_odtctl_d1[freq], dc_base + UMC_ODTCTL_D1);

	writel(0x00000003, dc_base + UMC_ACSSETA);
	writel(0x00000103, dc_base + UMC_FLOWCTLG);
	writel(umc_acssetb[ch], dc_base + UMC_ACSSETB);
	writel(0x02020200, dc_base + UMC_SPCSETB);
	writel(umc_memconfch[freq], dc_base + UMC_MEMCONFCH);
	writel(0x00000002, dc_base + UMC_ACFETCHCTRL);

	return 0;
}

static int umc_ch_init(void __iomem *umc_ch_base,
		       enum dram_freq freq, unsigned long size, int ch)
{
	void __iomem *dc_base  = umc_ch_base;

	return umc_dc_init(dc_base, freq, size, ch);
}

static void um_init(void __iomem *um_base)
{
	writel(0x00000001, um_base + UMC_SIORST);
	writel(0x00000001, um_base + UMC_VO0RST);
	writel(0x00000001, um_base + UMC_VPERST);
	writel(0x00000001, um_base + UMC_RGLRST);
	writel(0x00000001, um_base + UMC_A2DRST);
	writel(0x00000001, um_base + UMC_DMDRST);
}

int uniphier_ld11_umc_init(const struct uniphier_board_data *bd)
{
	void __iomem *um_base = (void __iomem *)0x5B800000;
	void __iomem *umc_ch_base = (void __iomem *)0x5BC00000;
	enum dram_freq freq;
	int ch, ret;

	switch (bd->dram_freq) {
	case 1600:
		freq = DRAM_FREQ_1600M;
		break;
	default:
		pr_err("unsupported DRAM frequency %d MHz\n", bd->dram_freq);
		return -EINVAL;
	}

	for (ch = 0; ch < bd->dram_nr_ch; ch++) {
		unsigned long size = bd->dram_ch[ch].size;
		unsigned int width = bd->dram_ch[ch].width;

		ret = umc_ch_init(umc_ch_base, freq, size / (width / 16), ch);
		if (ret) {
			pr_err("failed to initialize UMC ch%d\n", ch);
			return ret;
		}

		umc_ch_base += 0x00200000;
	}

	um_init(um_base);

	return 0;
}
