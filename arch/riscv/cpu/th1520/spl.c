// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Yao Zi <ziyao@disroot.org>
 */
#include <asm/arch/iopmp.h>
#include <asm/io.h>
#include <dm.h>
#include <linux/sizes.h>
#include <log.h>
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

#define TH1520_SUBSYS_CLK		(void __iomem *)(0xffff011000 + 0x220)
#define  TH1520_SUBSYS_CLK_VO_EN	BIT(2)
#define  TH1520_SUBSYS_CLK_VI_EN	BIT(1)
#define  TH1520_SUBSYS_CLK_DSP_EN	BIT(0)
#define TH1520_SUBSYS_RST		(void __iomem *)(0xffff015000 + 0x220)
#define  TH1520_SUBSYS_RST_VP_N		BIT(3)
#define  TH1520_SUBSYS_RST_VO_N		BIT(2)
#define  TH1520_SUBSYS_RST_VI_N		BIT(1)
#define  TH1520_SUBSYS_RST_DSP_N	BIT(0)

int spl_dram_init(void)
{
	int ret;
	struct udevice *dev;

	ret = fdtdec_setup_mem_size_base();
	if (ret) {
		printf("failed to setup memory size and base: %d\n", ret);
		return ret;
	}

	/* DDR init */
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		printf("DRAM init failed: %d\n", ret);
		return ret;
	}

	return 0;
}

static void __iomem *th1520_iopmp_regs[] = {
	TH1520_IOPMP_EMMC,
	TH1520_IOPMP_SDIO0,
	TH1520_IOPMP_SDIO1,
	TH1520_IOPMP_USB0,
	TH1520_IOPMP_AO,
	TH1520_IOPMP_AUD,
	TH1520_IOPMP_CHIP_DBG,
	TH1520_IOPMP_EIP120I,
	TH1520_IOPMP_EIP120II,
	TH1520_IOPMP_EIP120III,
	TH1520_IOPMP_ISP0,
	TH1520_IOPMP_ISP1,
	TH1520_IOPMP_DW200,
	TH1520_IOPMP_VIPRE,
	TH1520_IOPMP_VENC,
	TH1520_IOPMP_VDEC,
	TH1520_IOPMP_G2D,
	TH1520_IOPMP_FCE,
	TH1520_IOPMP_NPU,
	TH1520_IOPMP_DPU0,
	TH1520_IOPMP_DPU1,
	TH1520_IOPMP_GPU,
	TH1520_IOPMP_GMAC1,
	TH1520_IOPMP_GMAC2,
	TH1520_IOPMP_DMAC,
	TH1520_IOPMP_TEE_DMAC,
	TH1520_IOPMP_DSP0,
	TH1520_IOPMP_DSP1,
};

void harts_early_init(void)
{
	int i;

	/*
	 * Set IOPMPs to the default attribute, allowing the application
	 * processor to access various peripherals. Subsystem clocks should be
	 * enabled and resets should be deasserted ahead of time, or the HART
	 * will hang when configuring corresponding IOPMP entries.
	 */
	setbits_le32(TH1520_SUBSYS_CLK, TH1520_SUBSYS_CLK_VO_EN |
					TH1520_SUBSYS_CLK_VI_EN |
					TH1520_SUBSYS_CLK_DSP_EN);
	setbits_le32(TH1520_SUBSYS_RST, TH1520_SUBSYS_RST_VP_N |
					TH1520_SUBSYS_RST_VO_N |
					TH1520_SUBSYS_RST_VI_N |
					TH1520_SUBSYS_RST_DSP_N);

	for (i = 0; i < ARRAY_SIZE(th1520_iopmp_regs); i++)
		writel(TH1520_IOPMP_DEFAULT_ATTR, th1520_iopmp_regs[i]);
}
