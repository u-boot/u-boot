// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <errno.h>
#include <div64.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <wait_bit.h>
#include <asm/arch/firewall_s10.h>
#include <asm/arch/sdram_s10.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/reset_manager.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_system_manager *sysmgr_regs =
		(void *)SOCFPGA_SYSMGR_ADDRESS;

#define DDR_CONFIG(A, B, C, R)	(((A) << 24) | ((B) << 16) | ((C) << 8) | (R))

/* The followring are the supported configurations */
u32 ddr_config[] = {
	/* DDR_CONFIG(Address order,Bank,Column,Row) */
	/* List for DDR3 or LPDDR3 (pinout order > chip, row, bank, column) */
	DDR_CONFIG(0, 3, 10, 12),
	DDR_CONFIG(0, 3,  9, 13),
	DDR_CONFIG(0, 3, 10, 13),
	DDR_CONFIG(0, 3,  9, 14),
	DDR_CONFIG(0, 3, 10, 14),
	DDR_CONFIG(0, 3, 10, 15),
	DDR_CONFIG(0, 3, 11, 14),
	DDR_CONFIG(0, 3, 11, 15),
	DDR_CONFIG(0, 3, 10, 16),
	DDR_CONFIG(0, 3, 11, 16),
	DDR_CONFIG(0, 3, 12, 15),	/* 0xa */
	/* List for DDR4 only (pinout order > chip, bank, row, column) */
	DDR_CONFIG(1, 3, 10, 14),
	DDR_CONFIG(1, 4, 10, 14),
	DDR_CONFIG(1, 3, 10, 15),
	DDR_CONFIG(1, 4, 10, 15),
	DDR_CONFIG(1, 3, 10, 16),
	DDR_CONFIG(1, 4, 10, 16),
	DDR_CONFIG(1, 3, 10, 17),
	DDR_CONFIG(1, 4, 10, 17),
};

static u32 hmc_readl(u32 reg)
{
	return readl(((void __iomem *)SOCFPGA_HMC_MMR_IO48_ADDRESS + (reg)));
}

static u32 hmc_ecc_readl(u32 reg)
{
	return readl((void __iomem *)SOCFPGA_SDR_ADDRESS + (reg));
}

static u32 hmc_ecc_writel(u32 data, u32 reg)
{
	return writel(data, (void __iomem *)SOCFPGA_SDR_ADDRESS + (reg));
}

static u32 ddr_sch_writel(u32 data, u32 reg)
{
	return writel(data,
		      (void __iomem *)SOCFPGA_SDR_SCHEDULER_ADDRESS + (reg));
}

int match_ddr_conf(u32 ddr_conf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ddr_config); i++) {
		if (ddr_conf == ddr_config[i])
			return i;
	}
	return 0;
}

static int emif_clear(void)
{
	hmc_ecc_writel(0, RSTHANDSHAKECTRL);

	return wait_for_bit_le32((const void *)(SOCFPGA_SDR_ADDRESS +
				 RSTHANDSHAKESTAT),
				 DDR_HMC_RSTHANDSHAKE_MASK,
				 false, 1000, false);
}

static int emif_reset(void)
{
	u32 c2s, s2c, ret;

	c2s = hmc_ecc_readl(RSTHANDSHAKECTRL) & DDR_HMC_RSTHANDSHAKE_MASK;
	s2c = hmc_ecc_readl(RSTHANDSHAKESTAT) & DDR_HMC_RSTHANDSHAKE_MASK;

	debug("DDR: c2s=%08x s2c=%08x nr0=%08x nr1=%08x nr2=%08x dst=%08x\n",
	      c2s, s2c, hmc_readl(NIOSRESERVED0), hmc_readl(NIOSRESERVED1),
	      hmc_readl(NIOSRESERVED2), hmc_readl(DRAMSTS));

	if (s2c && emif_clear()) {
		printf("DDR: emif_clear() failed\n");
		return -1;
	}

	debug("DDR: Triggerring emif reset\n");
	hmc_ecc_writel(DDR_HMC_CORE2SEQ_INT_REQ, RSTHANDSHAKECTRL);

	/* if seq2core[3] = 0, we are good */
	ret = wait_for_bit_le32((const void *)(SOCFPGA_SDR_ADDRESS +
				 RSTHANDSHAKESTAT),
				 DDR_HMC_SEQ2CORE_INT_RESP_MASK,
				 false, 1000, false);
	if (ret) {
		printf("DDR: failed to get ack from EMIF\n");
		return ret;
	}

	ret = emif_clear();
	if (ret) {
		printf("DDR: emif_clear() failed\n");
		return ret;
	}

	debug("DDR: %s triggered successly\n", __func__);
	return 0;
}

static int poll_hmc_clock_status(void)
{
	return wait_for_bit_le32(&sysmgr_regs->hmc_clk,
				 SYSMGR_HMC_CLK_STATUS_MSK, true, 1000, false);
}

static void sdram_size_check(bd_t *bd)
{
	phys_size_t total_ram_check = 0;
	phys_size_t ram_check = 0;
	phys_addr_t start = 0;
	int bank;

	/* Sanity check ensure correct SDRAM size specified */
	debug("DDR: Running SDRAM size sanity check\n");

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start = bd->bi_dram[bank].start;
		while (ram_check < bd->bi_dram[bank].size) {
			ram_check += get_ram_size((void *)(start + ram_check),
						 (phys_size_t)SZ_1G);
		}
		total_ram_check += ram_check;
		ram_check = 0;
	}

	/* If the ram_size is 2GB smaller, we can assume the IO space is
	 * not mapped in.  gd->ram_size is the actual size of the dram
	 * not the accessible size.
	 */
	if (total_ram_check != gd->ram_size) {
		puts("DDR: SDRAM size check failed!\n");
		hang();
	}

	debug("DDR: SDRAM size check passed!\n");
}

/**
 * sdram_mmr_init_full() - Function to initialize SDRAM MMR
 *
 * Initialize the SDRAM MMR.
 */
int sdram_mmr_init_full(unsigned int unused)
{
	u32 update_value, io48_value, ddrioctl;
	u32 i;
	int ret;
	phys_size_t hw_size;
	bd_t bd = {0};

	/* Enable access to DDR from CPU master */
	clrbits_le32(CCU_REG_ADDR(CCU_CPU0_MPRT_ADBASE_DDRREG),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_CPU0_MPRT_ADBASE_MEMSPACE0),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_CPU0_MPRT_ADBASE_MEMSPACE1A),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_CPU0_MPRT_ADBASE_MEMSPACE1B),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_CPU0_MPRT_ADBASE_MEMSPACE1C),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_CPU0_MPRT_ADBASE_MEMSPACE1D),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_CPU0_MPRT_ADBASE_MEMSPACE1E),
		     CCU_ADBASE_DI_MASK);

	/* Enable access to DDR from IO master */
	clrbits_le32(CCU_REG_ADDR(CCU_IOM_MPRT_ADBASE_MEMSPACE0),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_IOM_MPRT_ADBASE_MEMSPACE1A),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_IOM_MPRT_ADBASE_MEMSPACE1B),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_IOM_MPRT_ADBASE_MEMSPACE1C),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_IOM_MPRT_ADBASE_MEMSPACE1D),
		     CCU_ADBASE_DI_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_IOM_MPRT_ADBASE_MEMSPACE1E),
		     CCU_ADBASE_DI_MASK);

	/* this enables nonsecure access to DDR */
	/* mpuregion0addr_limit */
	FW_MPU_DDR_SCR_WRITEL(0xFFFF0000, FW_MPU_DDR_SCR_MPUREGION0ADDR_LIMIT);
	FW_MPU_DDR_SCR_WRITEL(0x1F, FW_MPU_DDR_SCR_MPUREGION0ADDR_LIMITEXT);

	/* nonmpuregion0addr_limit */
	FW_MPU_DDR_SCR_WRITEL(0xFFFF0000,
			      FW_MPU_DDR_SCR_NONMPUREGION0ADDR_LIMIT);
	FW_MPU_DDR_SCR_WRITEL(0x1F, FW_MPU_DDR_SCR_NONMPUREGION0ADDR_LIMITEXT);

	/* Enable mpuregion0enable and nonmpuregion0enable */
	FW_MPU_DDR_SCR_WRITEL(MPUREGION0_ENABLE | NONMPUREGION0_ENABLE,
			      FW_MPU_DDR_SCR_EN_SET);

	/* Ensure HMC clock is running */
	if (poll_hmc_clock_status()) {
		puts("DDR: Error as HMC clock not running\n");
		return -1;
	}

	/* release DDR scheduler from reset */
	socfpga_per_reset(SOCFPGA_RESET(SDR), 0);

	/* Try 3 times to do a calibration */
	for (i = 0; i < 3; i++) {
		ret = wait_for_bit_le32((const void *)(SOCFPGA_SDR_ADDRESS +
					DDRCALSTAT),
					DDR_HMC_DDRCALSTAT_CAL_MSK, true, 1000,
					false);
		if (!ret)
			break;

		emif_reset();
	}

	if (ret) {
		puts("DDR: Error as SDRAM calibration failed\n");
		return -1;
	}
	debug("DDR: Calibration success\n");

	u32 ctrlcfg0 = hmc_readl(CTRLCFG0);
	u32 ctrlcfg1 = hmc_readl(CTRLCFG1);
	u32 dramaddrw = hmc_readl(DRAMADDRW);
	u32 dramtim0 = hmc_readl(DRAMTIMING0);
	u32 caltim0 = hmc_readl(CALTIMING0);
	u32 caltim1 = hmc_readl(CALTIMING1);
	u32 caltim2 = hmc_readl(CALTIMING2);
	u32 caltim3 = hmc_readl(CALTIMING3);
	u32 caltim4 = hmc_readl(CALTIMING4);
	u32 caltim9 = hmc_readl(CALTIMING9);

	/*
	 * Configure the DDR IO size [0xFFCFB008]
	 * niosreserve0: Used to indicate DDR width &
	 *	bit[7:0] = Number of data bits (bit[6:5] 0x01=32bit, 0x10=64bit)
	 *	bit[8]   = 1 if user-mode OCT is present
	 *	bit[9]   = 1 if warm reset compiled into EMIF Cal Code
	 *	bit[10]  = 1 if warm reset is on during generation in EMIF Cal
	 * niosreserve1: IP ADCDS version encoded as 16 bit value
	 *	bit[2:0] = Variant (0=not special,1=FAE beta, 2=Customer beta,
	 *			    3=EAP, 4-6 are reserved)
	 *	bit[5:3] = Service Pack # (e.g. 1)
	 *	bit[9:6] = Minor Release #
	 *	bit[14:10] = Major Release #
	 */
	update_value = hmc_readl(NIOSRESERVED0);
	hmc_ecc_writel(((update_value & 0xFF) >> 5), DDRIOCTRL);
	ddrioctl = hmc_ecc_readl(DDRIOCTRL);

	/* enable HPS interface to HMC */
	hmc_ecc_writel(DDR_HMC_HPSINTFCSEL_ENABLE_MASK, HPSINTFCSEL);

	/* Set the DDR Configuration */
	io48_value = DDR_CONFIG(CTRLCFG1_CFG_ADDR_ORDER(ctrlcfg1),
				(DRAMADDRW_CFG_BANK_ADDR_WIDTH(dramaddrw) +
				 DRAMADDRW_CFG_BANK_GRP_ADDR_WIDTH(dramaddrw)),
				DRAMADDRW_CFG_COL_ADDR_WIDTH(dramaddrw),
				DRAMADDRW_CFG_ROW_ADDR_WIDTH(dramaddrw));

	update_value = match_ddr_conf(io48_value);
	if (update_value)
		ddr_sch_writel(update_value, DDR_SCH_DDRCONF);

	/* Configure HMC dramaddrw */
	hmc_ecc_writel(hmc_readl(DRAMADDRW), DRAMADDRWIDTH);

	/*
	 * Configure DDR timing
	 *  RDTOMISS = tRTP + tRP + tRCD - BL/2
	 *  WRTOMISS = WL + tWR + tRP + tRCD and
	 *    WL = RL + BL/2 + 2 - rd-to-wr ; tWR = 15ns  so...
	 *  First part of equation is in memory clock units so divide by 2
	 *  for HMC clock units. 1066MHz is close to 1ns so use 15 directly.
	 *  WRTOMISS = ((RL + BL/2 + 2 + tWR) >> 1)- rd-to-wr + tRP + tRCD
	 */
	u32 burst_len = CTRLCFG0_CFG_CTRL_BURST_LEN(ctrlcfg0);

	update_value = CALTIMING2_CFG_RD_TO_WR_PCH(caltim2) +
		       CALTIMING4_CFG_PCH_TO_VALID(caltim4) +
		       CALTIMING0_CFG_ACT_TO_RDWR(caltim0) -
		       (burst_len >> 2);
	io48_value = (((DRAMTIMING0_CFG_TCL(dramtim0) + 2 + DDR_TWR +
		       (burst_len >> 1)) >> 1) -
		      /* Up to here was in memory cycles so divide by 2 */
		      CALTIMING1_CFG_RD_TO_WR(caltim1) +
		      CALTIMING0_CFG_ACT_TO_RDWR(caltim0) +
		      CALTIMING4_CFG_PCH_TO_VALID(caltim4));

	ddr_sch_writel(((CALTIMING0_CFG_ACT_TO_ACT(caltim0) <<
			 DDR_SCH_DDRTIMING_ACTTOACT_OFF) |
			(update_value << DDR_SCH_DDRTIMING_RDTOMISS_OFF) |
			(io48_value << DDR_SCH_DDRTIMING_WRTOMISS_OFF) |
			((burst_len >> 2) << DDR_SCH_DDRTIMING_BURSTLEN_OFF) |
			(CALTIMING1_CFG_RD_TO_WR(caltim1) <<
			 DDR_SCH_DDRTIMING_RDTOWR_OFF) |
			(CALTIMING3_CFG_WR_TO_RD(caltim3) <<
			 DDR_SCH_DDRTIMING_WRTORD_OFF) |
			(((ddrioctl == 1) ? 1 : 0) <<
			 DDR_SCH_DDRTIMING_BWRATIO_OFF)),
			DDR_SCH_DDRTIMING);

	/* Configure DDR mode [precharge = 0] */
	ddr_sch_writel(((ddrioctl ? 0 : 1) <<
			 DDR_SCH_DDRMOD_BWRATIOEXTENDED_OFF),
			DDR_SCH_DDRMODE);

	/* Configure the read latency */
	ddr_sch_writel((DRAMTIMING0_CFG_TCL(dramtim0) >> 1) +
			DDR_READ_LATENCY_DELAY,
			DDR_SCH_READ_LATENCY);

	/*
	 * Configuring timing values concerning activate commands
	 * [FAWBANK alway 1 because always 4 bank DDR]
	 */
	ddr_sch_writel(((CALTIMING0_CFG_ACT_TO_ACT_DB(caltim0) <<
			 DDR_SCH_ACTIVATE_RRD_OFF) |
			(CALTIMING9_CFG_4_ACT_TO_ACT(caltim9) <<
			 DDR_SCH_ACTIVATE_FAW_OFF) |
			(DDR_ACTIVATE_FAWBANK <<
			 DDR_SCH_ACTIVATE_FAWBANK_OFF)),
			DDR_SCH_ACTIVATE);

	/*
	 * Configuring timing values concerning device to device data bus
	 * ownership change
	 */
	ddr_sch_writel(((CALTIMING1_CFG_RD_TO_RD_DC(caltim1) <<
			 DDR_SCH_DEVTODEV_BUSRDTORD_OFF) |
			(CALTIMING1_CFG_RD_TO_WR_DC(caltim1) <<
			 DDR_SCH_DEVTODEV_BUSRDTOWR_OFF) |
			(CALTIMING3_CFG_WR_TO_RD_DC(caltim3) <<
			 DDR_SCH_DEVTODEV_BUSWRTORD_OFF)),
			DDR_SCH_DEVTODEV);

	/* assigning the SDRAM size */
	unsigned long long size = sdram_calculate_size();
	/* If the size is invalid, use default Config size */
	if (size <= 0)
		hw_size = PHYS_SDRAM_1_SIZE;
	else
		hw_size = size;

	/* Get bank configuration from devicetree */
	ret = fdtdec_decode_ram_size(gd->fdt_blob, NULL, 0, NULL,
				     (phys_size_t *)&gd->ram_size, &bd);
	if (ret) {
		puts("DDR: Failed to decode memory node\n");
		return -1;
	}

	if (gd->ram_size != hw_size)
		printf("DDR: Warning: DRAM size from device tree mismatch with hardware.\n");

	printf("DDR: %lld MiB\n", gd->ram_size >> 20);

	/* Enable or disable the SDRAM ECC */
	if (CTRLCFG1_CFG_CTRL_EN_ECC(ctrlcfg1)) {
		setbits_le32(SOCFPGA_SDR_ADDRESS + ECCCTRL1,
			     (DDR_HMC_ECCCTL_AWB_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_ECC_EN_SET_MSK));
		clrbits_le32(SOCFPGA_SDR_ADDRESS + ECCCTRL1,
			     (DDR_HMC_ECCCTL_AWB_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_CNT_RST_SET_MSK));
		setbits_le32(SOCFPGA_SDR_ADDRESS + ECCCTRL2,
			     (DDR_HMC_ECCCTL2_RMW_EN_SET_MSK |
			      DDR_HMC_ECCCTL2_AWB_EN_SET_MSK));
	} else {
		clrbits_le32(SOCFPGA_SDR_ADDRESS + ECCCTRL1,
			     (DDR_HMC_ECCCTL_AWB_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_ECC_EN_SET_MSK));
		clrbits_le32(SOCFPGA_SDR_ADDRESS + ECCCTRL2,
			     (DDR_HMC_ECCCTL2_RMW_EN_SET_MSK |
			      DDR_HMC_ECCCTL2_AWB_EN_SET_MSK));
	}

	sdram_size_check(&bd);

	debug("DDR: HMC init success\n");
	return 0;
}

/**
 * sdram_calculate_size() - Calculate SDRAM size
 *
 * Calculate SDRAM device size based on SDRAM controller parameters.
 * Size is specified in bytes.
 */
phys_size_t sdram_calculate_size(void)
{
	u32 dramaddrw = hmc_readl(DRAMADDRW);

	phys_size_t size = 1 << (DRAMADDRW_CFG_CS_ADDR_WIDTH(dramaddrw) +
			 DRAMADDRW_CFG_BANK_GRP_ADDR_WIDTH(dramaddrw) +
			 DRAMADDRW_CFG_BANK_ADDR_WIDTH(dramaddrw) +
			 DRAMADDRW_CFG_ROW_ADDR_WIDTH(dramaddrw) +
			 DRAMADDRW_CFG_COL_ADDR_WIDTH(dramaddrw));

	size *= (2 << (hmc_ecc_readl(DDRIOCTRL) &
			DDR_HMC_DDRIOCTRL_IOSIZE_MSK));

	return size;
}
