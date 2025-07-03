// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Yao Zi <ziyao@disroot.org>
 */
#include <asm/arch/iopmp.h>
#include <asm/io.h>
#include <cpu_func.h>
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

#define CSR_MXSTATUS			0x7c0
#define  CSR_MXSTATUS_THEADISAEE	BIT(22)
#define  CSR_MXSTATUS_MAEE		BIT(21)
#define  CSR_MXSTATUS_CLINTEE		BIT(17)
#define  CSR_MXSTATUS_UCME		BIT(16)
#define  CSR_MXSTATUS_MM		BIT(15)
#define CSR_MHCR			0x7c1
#define  CSR_MHCR_WBR			BIT(8)
#define  CSR_MHCR_BTB			BIT(6)
#define  CSR_MHCR_BPE			BIT(5)
#define  CSR_MHCR_RS			BIT(4)
#define  CSR_MHCR_WB			BIT(3)
#define  CSR_MHCR_WA			BIT(2)
#define  CSR_MHCR_DE			BIT(1)
#define  CSR_MHCR_IE			BIT(0)
#define CSR_MCOR			0x7c2
#define  CSR_MCOR_IBP_INV		BIT(18)
#define  CSR_MCOR_BTB_INV		BIT(17)
#define  CSR_MCOR_BHT_INV		BIT(16)
#define  CSR_MCOR_CACHE_INV		BIT(4)
#define CSR_MCCR2			0x7c3
#define  CSR_MCCR2_TPRF			BIT(31)
#define  CSR_MCCR2_IPRF(n)		((n) << 29)
#define  CSR_MCCR2_TSETUP		BIT(25)
#define  CSR_MCCR2_TLNTCY(n)		((n) << 22)
#define  CSR_MCCR2_DSETUP		BIT(19)
#define  CSR_MCCR2_DLNTCY(n)		((n) << 16)
#define  CSR_MCCR2_L2EN			BIT(3)
#define  CSR_MCCR2_RFE			BIT(0)
#define CSR_MHINT			0x7c5
#define  CSR_MHINT_FENCERW_BROAD_DIS	BIT(22)
#define  CSR_MHINT_TLB_BRAOD_DIS	BIT(21)
#define  CSR_MHINT_NSFE			BIT(18)
#define  CSR_MHINT_L2_PREF_DIST(n)	((n) << 16)
#define  CSR_MHINT_L2PLD		BIT(15)
#define  CSR_MHINT_DCACHE_PREF_DIST(n)	((n) << 13)
#define  CSR_MHINT_LPE			BIT(9)
#define  CSR_MHINT_ICACHE_PREF		BIT(8)
#define  CSR_MHINT_AMR			BIT(3)
#define  CSR_MHINT_DCACHE_PREF		BIT(2)
#define CSR_MHINT2			0x7cc
#define  CSR_MHINT2_LOCAL_ICG_EN(n)	BIT((n) + 14)
#define CSR_MHINT4			0x7ce
#define CSR_MSMPR			0x7f3
#define  CSR_MSMPR_SMPEN		BIT(0)

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

	/* Invalidate cache and buffer entries */
	csr_write(CSR_MCOR, CSR_MCOR_IBP_INV | CSR_MCOR_BTB_INV |
			    CSR_MCOR_BHT_INV | CSR_MCOR_CACHE_INV | 0x3);

	/* Enable cache snooping */
	csr_write(CSR_MSMPR, CSR_MSMPR_SMPEN);

	/*
	 * Configure and enable L2 cache,
	 *	Enable tag/data RAM prefetch, both cost 2 cycles
	 *	Prefetch 3 cache lines of instructions
	 *	Enable read allocation
	 */
	csr_write(CSR_MCCR2, CSR_MCCR2_TPRF   | CSR_MCCR2_IPRF(3)	|
			     CSR_MCCR2_TSETUP | CSR_MCCR2_TLNTCY(1)	|
			     CSR_MCCR2_DSETUP | CSR_MCCR2_DLNTCY(1)	|
			     CSR_MCCR2_L2EN   | CSR_MCCR2_RFE);
	csr_write(CSR_MXSTATUS, CSR_MXSTATUS_THEADISAEE | CSR_MXSTATUS_MAEE |
				CSR_MXSTATUS_CLINTEE    | CSR_MXSTATUS_UCME |
				CSR_MXSTATUS_MM);
	csr_write(CSR_MHINT, CSR_MHINT_FENCERW_BROAD_DIS	|
			     CSR_MHINT_TLB_BRAOD_DIS		|
			     CSR_MHINT_NSFE			|
			     CSR_MHINT_L2_PREF_DIST(2)		|
			     CSR_MHINT_L2PLD			|
			     CSR_MHINT_DCACHE_PREF_DIST(3)	|
			     CSR_MHINT_LPE			|
			     CSR_MHINT_ICACHE_PREF		|
			     CSR_MHINT_AMR			|
			     CSR_MHINT_DCACHE_PREF);
	csr_write(CSR_MHCR, CSR_MHCR_WBR | CSR_MHCR_BTB | CSR_MHCR_BPE |
			    CSR_MHCR_RS  | CSR_MHCR_WB  | CSR_MHCR_WA | 0x3);
	csr_write(CSR_MHINT2, CSR_MHINT2_LOCAL_ICG_EN(8)	|
			      CSR_MHINT2_LOCAL_ICG_EN(3));
	csr_write(CSR_MHINT4, 0x410);

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
