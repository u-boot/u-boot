// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2019 NXP
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

void ddr_cfg_umctl2(struct dram_cfg_param *ddrc_cfg, int num)
{
	int i = 0;

	for (i = 0; i < num; i++) {
		reg32_write(ddrc_cfg->reg, ddrc_cfg->val);
		ddrc_cfg++;
	}
}

void ddr_init(struct dram_timing_info *dram_timing)
{
	unsigned int tmp, initial_drate, target_freq;

	printf("DDRINFO: start DRAM init\n");

	/* Step1: Follow the power up procedure */
	if (is_imx8mq()) {
		reg32_write(SRC_DDRC_RCR_ADDR + 0x04, 0x8F00000F);
		reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00000F);
		reg32_write(SRC_DDRC_RCR_ADDR + 0x04, 0x8F000000);
	} else {
		reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00001F);
		reg32_write(SRC_DDRC_RCR_ADDR, 0x8F00000F);
	}

	debug("DDRINFO: cfg clk\n");
	/* change the clock source of dram_apb_clk_root: source 4 800MHz /4 = 200MHz */
	clock_set_target_val(DRAM_APB_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(4) |
			     CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV4));

	/* disable iso */
	reg32_write(0x303A00EC, 0x0000ffff); /* PGC_CPU_MAPPING */
	reg32setbit(0x303A00F8, 5); /* PU_PGC_SW_PUP_REQ */

	initial_drate = dram_timing->fsp_msg[0].drate;
	/* default to the frequency point 0 clock */
	ddrphy_init_set_dfi_clk(initial_drate);

	/* D-aasert the presetn */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000006);

	/* Step2: Program the dwc_ddr_umctl2 registers */
	debug("DDRINFO: ddrc config start\n");
	ddr_cfg_umctl2(dram_timing->ddrc_cfg, dram_timing->ddrc_cfg_num);
	debug("DDRINFO: ddrc config done\n");

	/* Step3: De-assert reset signal(core_ddrc_rstn & aresetn_n) */
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000004);
	reg32_write(SRC_DDRC_RCR_ADDR, 0x8F000000);

	/*
	 * Step4: Disable auto-refreshes, self-refresh, powerdown, and
	 * assertion of dfi_dram_clk_disable by setting RFSHCTL3.dis_auto_refresh = 1,
	 * PWRCTL.powerdown_en = 0, and PWRCTL.selfref_en = 0, PWRCTL.en_dfi_dram_clk_disable = 0
	 */
	reg32_write(DDRC_DBG1(0), 0x00000000);
	reg32_write(DDRC_RFSHCTL3(0), 0x0000001);
	reg32_write(DDRC_PWRCTL(0), 0xa0);

	/* if ddr type is LPDDR4, do it */
	tmp = reg32_read(DDRC_MSTR(0));
	if (tmp & (0x1 << 5))
		reg32_write(DDRC_DDR_SS_GPR0, 0x01); /* LPDDR4 mode */

	/* determine the initial boot frequency */
	target_freq = reg32_read(DDRC_MSTR2(0)) & 0x3;
	target_freq = (tmp & (0x1 << 29)) ? target_freq : 0x0;

	/* Step5: Set SWCT.sw_done to 0 */
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	/* Set the default boot frequency point */
	clrsetbits_le32(DDRC_DFIMISC(0), (0x1f << 8), target_freq << 8);
	/* Step6: Set DFIMISC.dfi_init_complete_en to 0 */
	clrbits_le32(DDRC_DFIMISC(0), 0x1);

	/* Step7: Set SWCTL.sw_done to 1; need to polling SWSTAT.sw_done_ack */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
	do {
		tmp = reg32_read(DDRC_SWSTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/*
	 * Step8 ~ Step13: Start PHY initialization and training by
	 * accessing relevant PUB registers
	 */
	debug("DDRINFO:ddrphy config start\n");
	ddr_cfg_phy(dram_timing);
	debug("DDRINFO: ddrphy config done\n");

	/*
	 * step14 CalBusy.0 =1, indicates the calibrator is actively
	 * calibrating. Wait Calibrating done.
	 */
	do {
		tmp = reg32_read(DDRPHY_CalBusy(0));
	} while ((tmp & 0x1));

	printf("DDRINFO:ddrphy calibration done\n");

	/* Step15: Set SWCTL.sw_done to 0 */
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	/* Step16: Set DFIMISC.dfi_init_start to 1 */
	setbits_le32(DDRC_DFIMISC(0), (0x1 << 5));

	/* Step17: Set SWCTL.sw_done to 1; need to polling SWSTAT.sw_done_ack */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
	do {
		tmp = reg32_read(DDRC_SWSTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/* Step18: Polling DFISTAT.dfi_init_complete = 1 */
	do {
		tmp = reg32_read(DDRC_DFISTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/* Step19: Set SWCTL.sw_done to 0 */
	reg32_write(DDRC_SWCTL(0), 0x00000000);

	/* Step20: Set DFIMISC.dfi_init_start to 0 */
	clrbits_le32(DDRC_DFIMISC(0), (0x1 << 5));

	/* Step21: optional */

	/* Step22: Set DFIMISC.dfi_init_complete_en to 1 */
	setbits_le32(DDRC_DFIMISC(0), 0x1);

	/* Step23: Set PWRCTL.selfref_sw to 0 */
	clrbits_le32(DDRC_PWRCTL(0), (0x1 << 5));

	/* Step24: Set SWCTL.sw_done to 1; need polling SWSTAT.sw_done_ack */
	reg32_write(DDRC_SWCTL(0), 0x00000001);
	do {
		tmp = reg32_read(DDRC_SWSTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/* Step25: Wait for dwc_ddr_umctl2 to move to normal operating mode by monitoring
	 * STAT.operating_mode signal */
	do {
		tmp = reg32_read(DDRC_STAT(0));
	} while ((tmp & 0x3) != 0x1);

	/* Step26: Set back register in Step4 to the original values if desired */
	reg32_write(DDRC_RFSHCTL3(0), 0x0000000);
	/* enable selfref_en by default */
	setbits_le32(DDRC_PWRCTL(0), 0x1 << 3);

	/* enable port 0 */
	reg32_write(DDRC_PCTRL_0(0), 0x00000001);
	printf("DDRINFO: ddrmix config done\n");

	/* save the dram timing config into memory */
	dram_config_save(dram_timing, CONFIG_SAVED_DRAM_TIMING_BASE);
}
