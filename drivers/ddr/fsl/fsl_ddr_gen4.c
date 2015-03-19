/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <fsl_ddr_sdram.h>
#include <asm/processor.h>
#include <fsl_immap.h>
#include <fsl_ddr.h>

#ifdef CONFIG_SYS_FSL_ERRATUM_A008511
static void set_wait_for_bits_clear(void *ptr, u32 value, u32 bits)
{
	int timeout = 1000;

	ddr_out32(ptr, value);

	while (ddr_in32(ptr) & bits) {
		udelay(100);
		timeout--;
	}
	if (timeout <= 0)
		puts("Error: A007865 wait for clear timeout.\n");
}
#endif /* CONFIG_SYS_FSL_ERRATUM_A008511 */

#if (CONFIG_CHIP_SELECTS_PER_CTRL > 4)
#error Invalid setting for CONFIG_CHIP_SELECTS_PER_CTRL
#endif

/*
 * regs has the to-be-set values for DDR controller registers
 * ctrl_num is the DDR controller number
 * step: 0 goes through the initialization in one pass
 *       1 sets registers and returns before enabling controller
 *       2 resumes from step 1 and continues to initialize
 * Dividing the initialization to two steps to deassert DDR reset signal
 * to comply with JEDEC specs for RDIMMs.
 */
void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
			     unsigned int ctrl_num, int step)
{
	unsigned int i, bus_width;
	struct ccsr_ddr __iomem *ddr;
	u32 temp_sdram_cfg;
	u32 total_gb_size_per_controller;
	int timeout;
#if defined(CONFIG_SYS_FSL_ERRATUM_A008336) || \
	defined(CONFIG_SYS_FSL_ERRATUM_A008514)
	u32 *eddrtqcr1;
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A008511
	u32 temp32, mr6;
#endif
#ifdef CONFIG_FSL_DDR_BIST
	u32 mtcr, err_detect, err_sbe;
	u32 cs0_bnds, cs1_bnds, cs2_bnds, cs3_bnds, cs0_config;
#endif
#ifdef CONFIG_FSL_DDR_BIST
	char buffer[CONFIG_SYS_CBSIZE];
#endif

	switch (ctrl_num) {
	case 0:
		ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;
#if defined(CONFIG_SYS_FSL_ERRATUM_A008336) || \
	defined(CONFIG_SYS_FSL_ERRATUM_A008514)
		eddrtqcr1 = (void *)CONFIG_SYS_FSL_DCSR_DDR_ADDR + 0x800;
#endif
		break;
#if defined(CONFIG_SYS_FSL_DDR2_ADDR) && (CONFIG_NUM_DDR_CONTROLLERS > 1)
	case 1:
		ddr = (void *)CONFIG_SYS_FSL_DDR2_ADDR;
#if defined(CONFIG_SYS_FSL_ERRATUM_A008336) || \
	defined(CONFIG_SYS_FSL_ERRATUM_A008514)
		eddrtqcr1 = (void *)CONFIG_SYS_FSL_DCSR_DDR2_ADDR + 0x800;
#endif
		break;
#endif
#if defined(CONFIG_SYS_FSL_DDR3_ADDR) && (CONFIG_NUM_DDR_CONTROLLERS > 2)
	case 2:
		ddr = (void *)CONFIG_SYS_FSL_DDR3_ADDR;
#if defined(CONFIG_SYS_FSL_ERRATUM_A008336) || \
	defined(CONFIG_SYS_FSL_ERRATUM_A008514)
		eddrtqcr1 = (void *)CONFIG_SYS_FSL_DCSR_DDR3_ADDR + 0x800;
#endif
		break;
#endif
#if defined(CONFIG_SYS_FSL_DDR4_ADDR) && (CONFIG_NUM_DDR_CONTROLLERS > 3)
	case 3:
		ddr = (void *)CONFIG_SYS_FSL_DDR4_ADDR;
#if defined(CONFIG_SYS_FSL_ERRATUM_A008336) || \
	defined(CONFIG_SYS_FSL_ERRATUM_A008514)
		eddrtqcr1 = (void *)CONFIG_SYS_FSL_DCSR_DDR4_ADDR + 0x800;
#endif
		break;
#endif
	default:
		printf("%s unexpected ctrl_num = %u\n", __func__, ctrl_num);
		return;
	}

	if (step == 2)
		goto step2;

#ifdef CONFIG_SYS_FSL_ERRATUM_A008336
#ifdef CONFIG_LS2085A
	/* A008336 only applies to general DDR controllers */
	if ((ctrl_num == 0) || (ctrl_num == 1))
#endif
		ddr_out32(eddrtqcr1, 0x63b30002);
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A008514
#ifdef CONFIG_LS2085A
	/* A008514 only applies to DP-DDR controler */
	if (ctrl_num == 2)
#endif
		ddr_out32(eddrtqcr1, 0x63b20002);
#endif
	if (regs->ddr_eor)
		ddr_out32(&ddr->eor, regs->ddr_eor);

	ddr_out32(&ddr->sdram_clk_cntl, regs->ddr_sdram_clk_cntl);

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (i == 0) {
			ddr_out32(&ddr->cs0_bnds, regs->cs[i].bnds);
			ddr_out32(&ddr->cs0_config, regs->cs[i].config);
			ddr_out32(&ddr->cs0_config_2, regs->cs[i].config_2);

		} else if (i == 1) {
			ddr_out32(&ddr->cs1_bnds, regs->cs[i].bnds);
			ddr_out32(&ddr->cs1_config, regs->cs[i].config);
			ddr_out32(&ddr->cs1_config_2, regs->cs[i].config_2);

		} else if (i == 2) {
			ddr_out32(&ddr->cs2_bnds, regs->cs[i].bnds);
			ddr_out32(&ddr->cs2_config, regs->cs[i].config);
			ddr_out32(&ddr->cs2_config_2, regs->cs[i].config_2);

		} else if (i == 3) {
			ddr_out32(&ddr->cs3_bnds, regs->cs[i].bnds);
			ddr_out32(&ddr->cs3_config, regs->cs[i].config);
			ddr_out32(&ddr->cs3_config_2, regs->cs[i].config_2);
		}
	}

	ddr_out32(&ddr->timing_cfg_3, regs->timing_cfg_3);
	ddr_out32(&ddr->timing_cfg_0, regs->timing_cfg_0);
	ddr_out32(&ddr->timing_cfg_1, regs->timing_cfg_1);
	ddr_out32(&ddr->timing_cfg_2, regs->timing_cfg_2);
	ddr_out32(&ddr->timing_cfg_4, regs->timing_cfg_4);
	ddr_out32(&ddr->timing_cfg_5, regs->timing_cfg_5);
	ddr_out32(&ddr->timing_cfg_6, regs->timing_cfg_6);
	ddr_out32(&ddr->timing_cfg_7, regs->timing_cfg_7);
	ddr_out32(&ddr->timing_cfg_8, regs->timing_cfg_8);
	ddr_out32(&ddr->timing_cfg_9, regs->timing_cfg_9);
	ddr_out32(&ddr->ddr_zq_cntl, regs->ddr_zq_cntl);
	ddr_out32(&ddr->dq_map_0, regs->dq_map_0);
	ddr_out32(&ddr->dq_map_1, regs->dq_map_1);
	ddr_out32(&ddr->dq_map_2, regs->dq_map_2);
	ddr_out32(&ddr->dq_map_3, regs->dq_map_3);
	ddr_out32(&ddr->sdram_cfg_3, regs->ddr_sdram_cfg_3);
	ddr_out32(&ddr->sdram_mode, regs->ddr_sdram_mode);
	ddr_out32(&ddr->sdram_mode_2, regs->ddr_sdram_mode_2);
	ddr_out32(&ddr->sdram_mode_3, regs->ddr_sdram_mode_3);
	ddr_out32(&ddr->sdram_mode_4, regs->ddr_sdram_mode_4);
	ddr_out32(&ddr->sdram_mode_5, regs->ddr_sdram_mode_5);
	ddr_out32(&ddr->sdram_mode_6, regs->ddr_sdram_mode_6);
	ddr_out32(&ddr->sdram_mode_7, regs->ddr_sdram_mode_7);
	ddr_out32(&ddr->sdram_mode_8, regs->ddr_sdram_mode_8);
	ddr_out32(&ddr->sdram_mode_9, regs->ddr_sdram_mode_9);
	ddr_out32(&ddr->sdram_mode_10, regs->ddr_sdram_mode_10);
	ddr_out32(&ddr->sdram_mode_11, regs->ddr_sdram_mode_11);
	ddr_out32(&ddr->sdram_mode_12, regs->ddr_sdram_mode_12);
	ddr_out32(&ddr->sdram_mode_13, regs->ddr_sdram_mode_13);
	ddr_out32(&ddr->sdram_mode_14, regs->ddr_sdram_mode_14);
	ddr_out32(&ddr->sdram_mode_15, regs->ddr_sdram_mode_15);
	ddr_out32(&ddr->sdram_mode_16, regs->ddr_sdram_mode_16);
	ddr_out32(&ddr->sdram_md_cntl, regs->ddr_sdram_md_cntl);
	ddr_out32(&ddr->sdram_interval, regs->ddr_sdram_interval);
	ddr_out32(&ddr->sdram_data_init, regs->ddr_data_init);
	ddr_out32(&ddr->ddr_wrlvl_cntl, regs->ddr_wrlvl_cntl);
#ifndef CONFIG_SYS_FSL_DDR_EMU
	/*
	 * Skip these two registers if running on emulator
	 * because emulator doesn't have skew between bytes.
	 */

	if (regs->ddr_wrlvl_cntl_2)
		ddr_out32(&ddr->ddr_wrlvl_cntl_2, regs->ddr_wrlvl_cntl_2);
	if (regs->ddr_wrlvl_cntl_3)
		ddr_out32(&ddr->ddr_wrlvl_cntl_3, regs->ddr_wrlvl_cntl_3);
#endif

	ddr_out32(&ddr->ddr_sr_cntr, regs->ddr_sr_cntr);
	ddr_out32(&ddr->ddr_sdram_rcw_1, regs->ddr_sdram_rcw_1);
	ddr_out32(&ddr->ddr_sdram_rcw_2, regs->ddr_sdram_rcw_2);
	ddr_out32(&ddr->ddr_sdram_rcw_3, regs->ddr_sdram_rcw_3);
	ddr_out32(&ddr->ddr_sdram_rcw_4, regs->ddr_sdram_rcw_4);
	ddr_out32(&ddr->ddr_sdram_rcw_5, regs->ddr_sdram_rcw_5);
	ddr_out32(&ddr->ddr_sdram_rcw_6, regs->ddr_sdram_rcw_6);
	ddr_out32(&ddr->ddr_cdr1, regs->ddr_cdr1);
#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		ddr_out32(&ddr->sdram_cfg_2,
			  regs->ddr_sdram_cfg_2 & ~SDRAM_CFG2_D_INIT);
		ddr_out32(&ddr->init_addr, CONFIG_SYS_SDRAM_BASE);
		ddr_out32(&ddr->init_ext_addr, DDR_INIT_ADDR_EXT_UIA);

		/* DRAM VRef will not be trained */
		ddr_out32(&ddr->ddr_cdr2,
			  regs->ddr_cdr2 & ~DDR_CDR2_VREF_TRAIN_EN);
	} else
#endif
	{
		ddr_out32(&ddr->sdram_cfg_2, regs->ddr_sdram_cfg_2);
		ddr_out32(&ddr->init_addr, regs->ddr_init_addr);
		ddr_out32(&ddr->init_ext_addr, regs->ddr_init_ext_addr);
		ddr_out32(&ddr->ddr_cdr2, regs->ddr_cdr2);
	}
	ddr_out32(&ddr->err_disable, regs->err_disable);
	ddr_out32(&ddr->err_int_en, regs->err_int_en);
	for (i = 0; i < 32; i++) {
		if (regs->debug[i]) {
			debug("Write to debug_%d as %08x\n",
			      i+1, regs->debug[i]);
			ddr_out32(&ddr->debug[i], regs->debug[i]);
		}
	}
#ifdef CONFIG_SYS_FSL_ERRATUM_A008378
	/* Erratum applies when accumulated ECC is used, or DBI is enabled */
#define IS_ACC_ECC_EN(v) ((v) & 0x4)
#define IS_DBI(v) ((((v) >> 12) & 0x3) == 0x2)
	if (IS_ACC_ECC_EN(regs->ddr_sdram_cfg) ||
	    IS_DBI(regs->ddr_sdram_cfg_3))
		ddr_setbits32(ddr->debug[28], 0x9 << 20);
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A008511
	/* Part 1 of 2 */
	/* This erraum only applies to verion 5.2.0 */
	if (fsl_ddr_get_version(ctrl_num) == 0x50200) {
		/* Disable DRAM VRef training */
		ddr_out32(&ddr->ddr_cdr2,
			  regs->ddr_cdr2 & ~DDR_CDR2_VREF_TRAIN_EN);
		/* Disable deskew */
		ddr_out32(&ddr->debug[28], 0x400);
		/* Disable D_INIT */
		ddr_out32(&ddr->sdram_cfg_2,
			  regs->ddr_sdram_cfg_2 & ~SDRAM_CFG2_D_INIT);
		ddr_out32(&ddr->debug[25], 0x9000);
	}
#endif
	/*
	 * For RDIMMs, JEDEC spec requires clocks to be stable before reset is
	 * deasserted. Clocks start when any chip select is enabled and clock
	 * control register is set. Because all DDR components are connected to
	 * one reset signal, this needs to be done in two steps. Step 1 is to
	 * get the clocks started. Step 2 resumes after reset signal is
	 * deasserted.
	 */
	if (step == 1) {
		udelay(200);
		return;
	}

step2:
	/* Set, but do not enable the memory */
	temp_sdram_cfg = regs->ddr_sdram_cfg;
	temp_sdram_cfg &= ~(SDRAM_CFG_MEM_EN);
	ddr_out32(&ddr->sdram_cfg, temp_sdram_cfg);

	/*
	 * 500 painful micro-seconds must elapse between
	 * the DDR clock setup and the DDR config enable.
	 * DDR2 need 200 us, and DDR3 need 500 us from spec,
	 * we choose the max, that is 500 us for all of case.
	 */
	udelay(500);
	mb();
	isb();

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* enter self-refresh */
		temp_sdram_cfg = ddr_in32(&ddr->sdram_cfg_2);
		temp_sdram_cfg |= SDRAM_CFG2_FRC_SR;
		ddr_out32(&ddr->sdram_cfg_2, temp_sdram_cfg);
		/* do board specific memory setup */
		board_mem_sleep_setup();

		temp_sdram_cfg = (ddr_in32(&ddr->sdram_cfg) | SDRAM_CFG_BI);
	} else
#endif
		temp_sdram_cfg = ddr_in32(&ddr->sdram_cfg) & ~SDRAM_CFG_BI;
	/* Let the controller go */
	ddr_out32(&ddr->sdram_cfg, temp_sdram_cfg | SDRAM_CFG_MEM_EN);
	mb();
	isb();

#ifdef CONFIG_SYS_FSL_ERRATUM_A008511
	/* Part 2 of 2 */
	/* This erraum only applies to verion 5.2.0 */
	if (fsl_ddr_get_version(ctrl_num) == 0x50200) {
		/* Wait for idle */
		timeout = 200;
		while (!(ddr_in32(&ddr->debug[1]) & 0x2) &&
		       (timeout > 0)) {
			udelay(100);
			timeout--;
		}
		if (timeout <= 0) {
			printf("Controler %d timeout, debug_2 = %x\n",
			       ctrl_num, ddr_in32(&ddr->debug[1]));
		}
		/* Set VREF */
		for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
			if (!(regs->cs[i].config & SDRAM_CS_CONFIG_EN))
				continue;

			mr6 = (regs->ddr_sdram_mode_10 >> 16)		|
				 MD_CNTL_MD_EN				|
				 MD_CNTL_CS_SEL(i)			|
				 MD_CNTL_MD_SEL(6)			|
				 0x00200000;
			temp32 = mr6 | 0xc0;
			set_wait_for_bits_clear(&ddr->sdram_md_cntl,
						temp32, MD_CNTL_MD_EN);
			udelay(1);
			debug("MR6 = 0x%08x\n", temp32);
			temp32 = mr6 | 0xf0;
			set_wait_for_bits_clear(&ddr->sdram_md_cntl,
						temp32, MD_CNTL_MD_EN);
			udelay(1);
			debug("MR6 = 0x%08x\n", temp32);
			temp32 = mr6 | 0x70;
			set_wait_for_bits_clear(&ddr->sdram_md_cntl,
						temp32, MD_CNTL_MD_EN);
			udelay(1);
			debug("MR6 = 0x%08x\n", temp32);
		}
		ddr_out32(&ddr->sdram_md_cntl, 0);
		ddr_out32(&ddr->debug[28], 0);		/* Enable deskew */
		ddr_out32(&ddr->debug[1], 0x400);	/* restart deskew */
		/* wait for idle */
		timeout = 200;
		while (!(ddr_in32(&ddr->debug[1]) & 0x2) &&
		       (timeout > 0)) {
			udelay(100);
			timeout--;
		}
		if (timeout <= 0) {
			printf("Controler %d timeout, debug_2 = %x\n",
			       ctrl_num, ddr_in32(&ddr->debug[1]));
		}
		/* Restore D_INIT */
		ddr_out32(&ddr->sdram_cfg_2, regs->ddr_sdram_cfg_2);
	}
#endif /* CONFIG_SYS_FSL_ERRATUM_A008511 */

	total_gb_size_per_controller = 0;
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (!(regs->cs[i].config & 0x80000000))
			continue;
		total_gb_size_per_controller += 1 << (
			((regs->cs[i].config >> 14) & 0x3) + 2 +
			((regs->cs[i].config >> 8) & 0x7) + 12 +
			((regs->cs[i].config >> 4) & 0x3) + 0 +
			((regs->cs[i].config >> 0) & 0x7) + 8 +
			3 - ((regs->ddr_sdram_cfg >> 19) & 0x3) -
			26);			/* minus 26 (count of 64M) */
	}
	if (fsl_ddr_get_intl3r() & 0x80000000)	/* 3-way interleaving */
		total_gb_size_per_controller *= 3;
	else if (regs->cs[0].config & 0x20000000) /* 2-way interleaving */
		total_gb_size_per_controller <<= 1;
	/*
	 * total memory / bus width = transactions needed
	 * transactions needed / data rate = seconds
	 * to add plenty of buffer, double the time
	 * For example, 2GB on 666MT/s 64-bit bus takes about 402ms
	 * Let's wait for 800ms
	 */
	bus_width = 3 - ((ddr_in32(&ddr->sdram_cfg) & SDRAM_CFG_DBW_MASK)
			>> SDRAM_CFG_DBW_SHIFT);
	timeout = ((total_gb_size_per_controller << (6 - bus_width)) * 100 /
		(get_ddr_freq(ctrl_num) >> 20)) << 2;
	total_gb_size_per_controller >>= 4;	/* shift down to gb size */
	debug("total %d GB\n", total_gb_size_per_controller);
	debug("Need to wait up to %d * 10ms\n", timeout);

	/* Poll DDR_SDRAM_CFG_2[D_INIT] bit until auto-data init is done.  */
	while ((ddr_in32(&ddr->sdram_cfg_2) & SDRAM_CFG2_D_INIT) &&
		(timeout >= 0)) {
		udelay(10000);		/* throttle polling rate */
		timeout--;
	}

	if (timeout <= 0)
		printf("Waiting for D_INIT timeout. Memory may not work.\n");
#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* exit self-refresh */
		temp_sdram_cfg = ddr_in32(&ddr->sdram_cfg_2);
		temp_sdram_cfg &= ~SDRAM_CFG2_FRC_SR;
		ddr_out32(&ddr->sdram_cfg_2, temp_sdram_cfg);
	}
#endif

#ifdef CONFIG_FSL_DDR_BIST
#define BIST_PATTERN1	0xFFFFFFFF
#define BIST_PATTERN2	0x0
#define BIST_CR		0x80010000
#define BIST_CR_EN	0x80000000
#define BIST_CR_STAT	0x00000001
#define CTLR_INTLV_MASK	0x20000000
	/* Perform build-in test on memory. Three-way interleaving is not yet
	 * supported by this code. */
	if (getenv_f("ddr_bist", buffer, CONFIG_SYS_CBSIZE) >= 0) {
		puts("Running BIST test. This will take a while...");
		cs0_config = ddr_in32(&ddr->cs0_config);
		if (cs0_config & CTLR_INTLV_MASK) {
			cs0_bnds = ddr_in32(&cs0_bnds);
			cs1_bnds = ddr_in32(&cs1_bnds);
			cs2_bnds = ddr_in32(&cs2_bnds);
			cs3_bnds = ddr_in32(&cs3_bnds);
			/* set bnds to non-interleaving */
			ddr_out32(&cs0_bnds, (cs0_bnds & 0xfffefffe) >> 1);
			ddr_out32(&cs1_bnds, (cs1_bnds & 0xfffefffe) >> 1);
			ddr_out32(&cs2_bnds, (cs2_bnds & 0xfffefffe) >> 1);
			ddr_out32(&cs3_bnds, (cs3_bnds & 0xfffefffe) >> 1);
		}
		ddr_out32(&ddr->mtp1, BIST_PATTERN1);
		ddr_out32(&ddr->mtp2, BIST_PATTERN1);
		ddr_out32(&ddr->mtp3, BIST_PATTERN2);
		ddr_out32(&ddr->mtp4, BIST_PATTERN2);
		ddr_out32(&ddr->mtp5, BIST_PATTERN1);
		ddr_out32(&ddr->mtp6, BIST_PATTERN1);
		ddr_out32(&ddr->mtp7, BIST_PATTERN2);
		ddr_out32(&ddr->mtp8, BIST_PATTERN2);
		ddr_out32(&ddr->mtp9, BIST_PATTERN1);
		ddr_out32(&ddr->mtp10, BIST_PATTERN2);
		mtcr = BIST_CR;
		ddr_out32(&ddr->mtcr, mtcr);
		timeout = 100;
		while (timeout > 0 && (mtcr & BIST_CR_EN)) {
			mdelay(1000);
			timeout--;
			mtcr = ddr_in32(&ddr->mtcr);
		}
		if (timeout <= 0)
			puts("Timeout\n");
		else
			puts("Done\n");
		err_detect = ddr_in32(&ddr->err_detect);
		err_sbe = ddr_in32(&ddr->err_sbe);
		if (mtcr & BIST_CR_STAT) {
			printf("BIST test failed on controller %d.\n",
			       ctrl_num);
		}
		if (err_detect || (err_sbe & 0xffff)) {
			printf("ECC error detected on controller %d.\n",
			       ctrl_num);
		}

		if (cs0_config & CTLR_INTLV_MASK) {
			/* restore bnds registers */
			ddr_out32(&cs0_bnds, cs0_bnds);
			ddr_out32(&cs1_bnds, cs1_bnds);
			ddr_out32(&cs2_bnds, cs2_bnds);
			ddr_out32(&cs3_bnds, cs3_bnds);
		}
	}
#endif
}
