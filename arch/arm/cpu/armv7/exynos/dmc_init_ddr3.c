/*
 * DDR3 mem setup file for board based on EXYNOS5
 *
 * Copyright (C) 2012 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dmc.h>
#include <asm/arch/power.h>
#include "common_setup.h"
#include "exynos5_setup.h"
#include "clock_init.h"

#define TIMEOUT	10000

#ifdef CONFIG_EXYNOS5250
static void reset_phy_ctrl(void)
{
	struct exynos5_clock *clk =
		(struct exynos5_clock *)samsung_get_base_clock();

	writel(DDR3PHY_CTRL_PHY_RESET_OFF, &clk->lpddr3phy_ctrl);
	writel(DDR3PHY_CTRL_PHY_RESET, &clk->lpddr3phy_ctrl);
}

int ddr3_mem_ctrl_init(struct mem_timings *mem, unsigned long mem_iv_size,
		       int reset)
{
	unsigned int val;
	struct exynos5_phy_control *phy0_ctrl, *phy1_ctrl;
	struct exynos5_dmc *dmc;
	int i;

	phy0_ctrl = (struct exynos5_phy_control *)samsung_get_base_dmc_phy();
	phy1_ctrl = (struct exynos5_phy_control *)(samsung_get_base_dmc_phy()
							+ DMC_OFFSET);
	dmc = (struct exynos5_dmc *)samsung_get_base_dmc_ctrl();

	if (reset)
		reset_phy_ctrl();

	/* Set Impedance Output Driver */
	val = (mem->impedance << CA_CK_DRVR_DS_OFFSET) |
		(mem->impedance << CA_CKE_DRVR_DS_OFFSET) |
		(mem->impedance << CA_CS_DRVR_DS_OFFSET) |
		(mem->impedance << CA_ADR_DRVR_DS_OFFSET);
	writel(val, &phy0_ctrl->phy_con39);
	writel(val, &phy1_ctrl->phy_con39);

	/* Set Read Latency and Burst Length for PHY0 and PHY1 */
	val = (mem->ctrl_bstlen << PHY_CON42_CTRL_BSTLEN_SHIFT) |
		(mem->ctrl_rdlat << PHY_CON42_CTRL_RDLAT_SHIFT);
	writel(val, &phy0_ctrl->phy_con42);
	writel(val, &phy1_ctrl->phy_con42);

	/* ZQ Calibration */
	if (dmc_config_zq(mem, &phy0_ctrl->phy_con16, &phy1_ctrl->phy_con16,
			  &phy0_ctrl->phy_con17, &phy1_ctrl->phy_con17))
		return SETUP_ERR_ZQ_CALIBRATION_FAILURE;

	/* DQ Signal */
	writel(mem->phy0_pulld_dqs, &phy0_ctrl->phy_con14);
	writel(mem->phy1_pulld_dqs, &phy1_ctrl->phy_con14);

	writel(mem->concontrol | (mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT)
		| (mem->dfi_init_start << CONCONTROL_DFI_INIT_START_SHIFT),
		&dmc->concontrol);

	update_reset_dll(&dmc->phycontrol0, DDR_MODE_DDR3);

	/* DQS Signal */
	writel(mem->phy0_dqs, &phy0_ctrl->phy_con4);
	writel(mem->phy1_dqs, &phy1_ctrl->phy_con4);

	writel(mem->phy0_dq, &phy0_ctrl->phy_con6);
	writel(mem->phy1_dq, &phy1_ctrl->phy_con6);

	writel(mem->phy0_tFS, &phy0_ctrl->phy_con10);
	writel(mem->phy1_tFS, &phy1_ctrl->phy_con10);

	val = (mem->ctrl_start_point << PHY_CON12_CTRL_START_POINT_SHIFT) |
		(mem->ctrl_inc << PHY_CON12_CTRL_INC_SHIFT) |
		(mem->ctrl_dll_on << PHY_CON12_CTRL_DLL_ON_SHIFT) |
		(mem->ctrl_ref << PHY_CON12_CTRL_REF_SHIFT);
	writel(val, &phy0_ctrl->phy_con12);
	writel(val, &phy1_ctrl->phy_con12);

	/* Start DLL locking */
	writel(val | (mem->ctrl_start << PHY_CON12_CTRL_START_SHIFT),
	       &phy0_ctrl->phy_con12);
	writel(val | (mem->ctrl_start << PHY_CON12_CTRL_START_SHIFT),
	       &phy1_ctrl->phy_con12);

	update_reset_dll(&dmc->phycontrol0, DDR_MODE_DDR3);

	writel(mem->concontrol | (mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT),
	       &dmc->concontrol);

	/* Memory Channel Inteleaving Size */
	writel(mem->iv_size, &dmc->ivcontrol);

	writel(mem->memconfig, &dmc->memconfig0);
	writel(mem->memconfig, &dmc->memconfig1);
	writel(mem->membaseconfig0, &dmc->membaseconfig0);
	writel(mem->membaseconfig1, &dmc->membaseconfig1);

	/* Precharge Configuration */
	writel(mem->prechconfig_tp_cnt << PRECHCONFIG_TP_CNT_SHIFT,
	       &dmc->prechconfig);

	/* Power Down mode Configuration */
	writel(mem->dpwrdn_cyc << PWRDNCONFIG_DPWRDN_CYC_SHIFT |
		mem->dsref_cyc << PWRDNCONFIG_DSREF_CYC_SHIFT,
		&dmc->pwrdnconfig);

	/* TimingRow, TimingData, TimingPower and Timingaref
	 * values as per Memory AC parameters
	 */
	writel(mem->timing_ref, &dmc->timingref);
	writel(mem->timing_row, &dmc->timingrow);
	writel(mem->timing_data, &dmc->timingdata);
	writel(mem->timing_power, &dmc->timingpower);

	/* Send PALL command */
	dmc_config_prech(mem, &dmc->directcmd);

	/* Send NOP, MRS and ZQINIT commands */
	dmc_config_mrs(mem, &dmc->directcmd);

	if (mem->gate_leveling_enable) {
		val = PHY_CON0_RESET_VAL;
		val |= P0_CMD_EN;
		writel(val, &phy0_ctrl->phy_con0);
		writel(val, &phy1_ctrl->phy_con0);

		val = PHY_CON2_RESET_VAL;
		val |= INIT_DESKEW_EN;
		writel(val, &phy0_ctrl->phy_con2);
		writel(val, &phy1_ctrl->phy_con2);

		val = PHY_CON0_RESET_VAL;
		val |= P0_CMD_EN;
		val |= BYTE_RDLVL_EN;
		writel(val, &phy0_ctrl->phy_con0);
		writel(val, &phy1_ctrl->phy_con0);

		val = (mem->ctrl_start_point <<
				PHY_CON12_CTRL_START_POINT_SHIFT) |
			(mem->ctrl_inc << PHY_CON12_CTRL_INC_SHIFT) |
			(mem->ctrl_force << PHY_CON12_CTRL_FORCE_SHIFT) |
			(mem->ctrl_start << PHY_CON12_CTRL_START_SHIFT) |
			(mem->ctrl_ref << PHY_CON12_CTRL_REF_SHIFT);
		writel(val, &phy0_ctrl->phy_con12);
		writel(val, &phy1_ctrl->phy_con12);

		val = PHY_CON2_RESET_VAL;
		val |= INIT_DESKEW_EN;
		val |= RDLVL_GATE_EN;
		writel(val, &phy0_ctrl->phy_con2);
		writel(val, &phy1_ctrl->phy_con2);

		val = PHY_CON0_RESET_VAL;
		val |= P0_CMD_EN;
		val |= BYTE_RDLVL_EN;
		val |= CTRL_SHGATE;
		writel(val, &phy0_ctrl->phy_con0);
		writel(val, &phy1_ctrl->phy_con0);

		val = PHY_CON1_RESET_VAL;
		val &= ~(CTRL_GATEDURADJ_MASK);
		writel(val, &phy0_ctrl->phy_con1);
		writel(val, &phy1_ctrl->phy_con1);

		writel(CTRL_RDLVL_GATE_ENABLE, &dmc->rdlvl_config);
		i = TIMEOUT;
		while ((readl(&dmc->phystatus) &
			(RDLVL_COMPLETE_CHO | RDLVL_COMPLETE_CH1)) !=
			(RDLVL_COMPLETE_CHO | RDLVL_COMPLETE_CH1) && i > 0) {
			/*
			 * TODO(waihong): Comment on how long this take to
			 * timeout
			 */
			sdelay(100);
			i--;
		}
		if (!i)
			return SETUP_ERR_RDLV_COMPLETE_TIMEOUT;
		writel(CTRL_RDLVL_GATE_DISABLE, &dmc->rdlvl_config);

		writel(0, &phy0_ctrl->phy_con14);
		writel(0, &phy1_ctrl->phy_con14);

		val = (mem->ctrl_start_point <<
				PHY_CON12_CTRL_START_POINT_SHIFT) |
			(mem->ctrl_inc << PHY_CON12_CTRL_INC_SHIFT) |
			(mem->ctrl_force << PHY_CON12_CTRL_FORCE_SHIFT) |
			(mem->ctrl_start << PHY_CON12_CTRL_START_SHIFT) |
			(mem->ctrl_dll_on << PHY_CON12_CTRL_DLL_ON_SHIFT) |
			(mem->ctrl_ref << PHY_CON12_CTRL_REF_SHIFT);
		writel(val, &phy0_ctrl->phy_con12);
		writel(val, &phy1_ctrl->phy_con12);

		update_reset_dll(&dmc->phycontrol0, DDR_MODE_DDR3);
	}

	/* Send PALL command */
	dmc_config_prech(mem, &dmc->directcmd);

	writel(mem->memcontrol, &dmc->memcontrol);

	/* Set DMC Concontrol and enable auto-refresh counter */
	writel(mem->concontrol | (mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT)
		| (mem->aref_en << CONCONTROL_AREF_EN_SHIFT), &dmc->concontrol);
	return 0;
}
#endif

#ifdef CONFIG_EXYNOS5420
int ddr3_mem_ctrl_init(struct mem_timings *mem, unsigned long mem_iv_size,
		       int reset)
{
	struct exynos5420_clock *clk =
		(struct exynos5420_clock *)samsung_get_base_clock();
	struct exynos5420_power *power =
		(struct exynos5420_power *)samsung_get_base_power();
	struct exynos5420_phy_control *phy0_ctrl, *phy1_ctrl;
	struct exynos5420_dmc *drex0, *drex1;
	struct exynos5420_tzasc *tzasc0, *tzasc1;
	uint32_t val, n_lock_r, n_lock_w_phy0, n_lock_w_phy1;
	int chip;
	int i;

	phy0_ctrl = (struct exynos5420_phy_control *)samsung_get_base_dmc_phy();
	phy1_ctrl = (struct exynos5420_phy_control *)(samsung_get_base_dmc_phy()
							+ DMC_OFFSET);
	drex0 = (struct exynos5420_dmc *)samsung_get_base_dmc_ctrl();
	drex1 = (struct exynos5420_dmc *)(samsung_get_base_dmc_ctrl()
							+ DMC_OFFSET);
	tzasc0 = (struct exynos5420_tzasc *)samsung_get_base_dmc_tzasc();
	tzasc1 = (struct exynos5420_tzasc *)(samsung_get_base_dmc_tzasc()
							+ DMC_OFFSET);

	/* Enable PAUSE for DREX */
	setbits_le32(&clk->pause, ENABLE_BIT);

	/* Enable BYPASS mode */
	setbits_le32(&clk->bpll_con1, BYPASS_EN);

	writel(MUX_BPLL_SEL_FOUTBPLL, &clk->src_cdrex);
	do {
		val = readl(&clk->mux_stat_cdrex);
		val &= BPLL_SEL_MASK;
	} while (val != FOUTBPLL);

	clrbits_le32(&clk->bpll_con1, BYPASS_EN);

	/* Specify the DDR memory type as DDR3 */
	val = readl(&phy0_ctrl->phy_con0);
	val &= ~(PHY_CON0_CTRL_DDR_MODE_MASK << PHY_CON0_CTRL_DDR_MODE_SHIFT);
	val |= (DDR_MODE_DDR3 << PHY_CON0_CTRL_DDR_MODE_SHIFT);
	writel(val, &phy0_ctrl->phy_con0);

	val = readl(&phy1_ctrl->phy_con0);
	val &= ~(PHY_CON0_CTRL_DDR_MODE_MASK << PHY_CON0_CTRL_DDR_MODE_SHIFT);
	val |= (DDR_MODE_DDR3 << PHY_CON0_CTRL_DDR_MODE_SHIFT);
	writel(val, &phy1_ctrl->phy_con0);

	/* Set Read Latency and Burst Length for PHY0 and PHY1 */
	val = (mem->ctrl_bstlen << PHY_CON42_CTRL_BSTLEN_SHIFT) |
		(mem->ctrl_rdlat << PHY_CON42_CTRL_RDLAT_SHIFT);
	writel(val, &phy0_ctrl->phy_con42);
	writel(val, &phy1_ctrl->phy_con42);

	val = readl(&phy0_ctrl->phy_con26);
	val &= ~(T_WRDATA_EN_MASK << T_WRDATA_EN_OFFSET);
	val |= (T_WRDATA_EN_DDR3 << T_WRDATA_EN_OFFSET);
	writel(val, &phy0_ctrl->phy_con26);

	val = readl(&phy1_ctrl->phy_con26);
	val &= ~(T_WRDATA_EN_MASK << T_WRDATA_EN_OFFSET);
	val |= (T_WRDATA_EN_DDR3 << T_WRDATA_EN_OFFSET);
	writel(val, &phy1_ctrl->phy_con26);

	/*
	 * Set Driver strength for CK, CKE, CS & CA to 0x7
	 * Set Driver strength for Data Slice 0~3 to 0x7
	 */
	val = (0x7 << CA_CK_DRVR_DS_OFFSET) | (0x7 << CA_CKE_DRVR_DS_OFFSET) |
		(0x7 << CA_CS_DRVR_DS_OFFSET) | (0x7 << CA_ADR_DRVR_DS_OFFSET);
	val |= (0x7 << DA_3_DS_OFFSET) | (0x7 << DA_2_DS_OFFSET) |
		(0x7 << DA_1_DS_OFFSET) | (0x7 << DA_0_DS_OFFSET);
	writel(val, &phy0_ctrl->phy_con39);
	writel(val, &phy1_ctrl->phy_con39);

	/* ZQ Calibration */
	if (dmc_config_zq(mem, &phy0_ctrl->phy_con16, &phy1_ctrl->phy_con16,
			  &phy0_ctrl->phy_con17, &phy1_ctrl->phy_con17))
		return SETUP_ERR_ZQ_CALIBRATION_FAILURE;

	clrbits_le32(&phy0_ctrl->phy_con16, ZQ_CLK_DIV_EN);
	clrbits_le32(&phy1_ctrl->phy_con16, ZQ_CLK_DIV_EN);

	/* DQ Signal */
	val = readl(&phy0_ctrl->phy_con14);
	val |= mem->phy0_pulld_dqs;
	writel(val, &phy0_ctrl->phy_con14);
	val = readl(&phy1_ctrl->phy_con14);
	val |= mem->phy1_pulld_dqs;
	writel(val, &phy1_ctrl->phy_con14);

	val = MEM_TERM_EN | PHY_TERM_EN;
	writel(val, &drex0->phycontrol0);
	writel(val, &drex1->phycontrol0);

	writel(mem->concontrol |
		(mem->dfi_init_start << CONCONTROL_DFI_INIT_START_SHIFT) |
		(mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT),
		&drex0->concontrol);
	writel(mem->concontrol |
		(mem->dfi_init_start << CONCONTROL_DFI_INIT_START_SHIFT) |
		(mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT),
		&drex1->concontrol);

	do {
		val = readl(&drex0->phystatus);
	} while ((val & DFI_INIT_COMPLETE) != DFI_INIT_COMPLETE);
	do {
		val = readl(&drex1->phystatus);
	} while ((val & DFI_INIT_COMPLETE) != DFI_INIT_COMPLETE);

	clrbits_le32(&drex0->concontrol, DFI_INIT_START);
	clrbits_le32(&drex1->concontrol, DFI_INIT_START);

	update_reset_dll(&drex0->phycontrol0, DDR_MODE_DDR3);
	update_reset_dll(&drex1->phycontrol0, DDR_MODE_DDR3);

	/*
	 * Set Base Address:
	 * 0x2000_0000 ~ 0x5FFF_FFFF
	 * 0x6000_0000 ~ 0x9FFF_FFFF
	 */
	/* MEMBASECONFIG0 */
	val = DMC_MEMBASECONFIGX_CHIP_BASE(DMC_CHIP_BASE_0) |
		DMC_MEMBASECONFIGX_CHIP_MASK(DMC_CHIP_MASK);
	writel(val, &tzasc0->membaseconfig0);
	writel(val, &tzasc1->membaseconfig0);

	/* MEMBASECONFIG1 */
	val = DMC_MEMBASECONFIGX_CHIP_BASE(DMC_CHIP_BASE_1) |
		DMC_MEMBASECONFIGX_CHIP_MASK(DMC_CHIP_MASK);
	writel(val, &tzasc0->membaseconfig1);
	writel(val, &tzasc1->membaseconfig1);

	/*
	 * Memory Channel Inteleaving Size
	 * Ares Channel interleaving = 128 bytes
	 */
	/* MEMCONFIG0/1 */
	writel(mem->memconfig, &tzasc0->memconfig0);
	writel(mem->memconfig, &tzasc1->memconfig0);
	writel(mem->memconfig, &tzasc0->memconfig1);
	writel(mem->memconfig, &tzasc1->memconfig1);

	/* Precharge Configuration */
	writel(mem->prechconfig_tp_cnt << PRECHCONFIG_TP_CNT_SHIFT,
	       &drex0->prechconfig0);
	writel(mem->prechconfig_tp_cnt << PRECHCONFIG_TP_CNT_SHIFT,
	       &drex1->prechconfig0);

	/*
	 * TimingRow, TimingData, TimingPower and Timingaref
	 * values as per Memory AC parameters
	 */
	writel(mem->timing_ref, &drex0->timingref);
	writel(mem->timing_ref, &drex1->timingref);
	writel(mem->timing_row, &drex0->timingrow0);
	writel(mem->timing_row, &drex1->timingrow0);
	writel(mem->timing_data, &drex0->timingdata0);
	writel(mem->timing_data, &drex1->timingdata0);
	writel(mem->timing_power, &drex0->timingpower0);
	writel(mem->timing_power, &drex1->timingpower0);

	if (reset) {
		/*
		 * Send NOP, MRS and ZQINIT commands
		 * Sending MRS command will reset the DRAM. We should not be
		 * reseting the DRAM after resume, this will lead to memory
		 * corruption as DRAM content is lost after DRAM reset
		 */
		dmc_config_mrs(mem, &drex0->directcmd);
		dmc_config_mrs(mem, &drex1->directcmd);
	} else {
		/*
		 * During Suspend-Resume & S/W-Reset, as soon as PMU releases
		 * pad retention, CKE goes high. This causes memory contents
		 * not to be retained during DRAM initialization. Therfore,
		 * there is a new control register(0x100431e8[28]) which lets us
		 * release pad retention and retain the memory content until the
		 * initialization is complete.
		 */
		writel(PAD_RETENTION_DRAM_COREBLK_VAL,
		       &power->pad_retention_dram_coreblk_option);
		do {
			val = readl(&power->pad_retention_dram_status);
		} while (val != 0x1);

		/*
		 * CKE PAD retention disables DRAM self-refresh mode.
		 * Send auto refresh command for DRAM refresh.
		 */
		for (i = 0; i < 128; i++) {
			for (chip = 0; chip < mem->chips_to_configure; chip++) {
				writel(DIRECT_CMD_REFA |
				       (chip << DIRECT_CMD_CHIP_SHIFT),
				       &drex0->directcmd);
				writel(DIRECT_CMD_REFA |
				       (chip << DIRECT_CMD_CHIP_SHIFT),
				       &drex1->directcmd);
			}
		}
	}

	if (mem->gate_leveling_enable) {
		writel(PHY_CON0_RESET_VAL, &phy0_ctrl->phy_con0);
		writel(PHY_CON0_RESET_VAL, &phy1_ctrl->phy_con0);

		setbits_le32(&phy0_ctrl->phy_con0, P0_CMD_EN);
		setbits_le32(&phy1_ctrl->phy_con0, P0_CMD_EN);

		val = PHY_CON2_RESET_VAL;
		val |= INIT_DESKEW_EN;
		writel(val, &phy0_ctrl->phy_con2);
		writel(val, &phy1_ctrl->phy_con2);

		val =  readl(&phy0_ctrl->phy_con1);
		val |= (RDLVL_PASS_ADJ_VAL << RDLVL_PASS_ADJ_OFFSET);
		writel(val, &phy0_ctrl->phy_con1);

		val =  readl(&phy1_ctrl->phy_con1);
		val |= (RDLVL_PASS_ADJ_VAL << RDLVL_PASS_ADJ_OFFSET);
		writel(val, &phy1_ctrl->phy_con1);

		n_lock_r = readl(&phy0_ctrl->phy_con13);
		n_lock_w_phy0 = (n_lock_r & CTRL_LOCK_COARSE_MASK) >> 2;
		n_lock_r = readl(&phy0_ctrl->phy_con12);
		n_lock_r &= ~CTRL_DLL_ON;
		n_lock_r |= n_lock_w_phy0;
		writel(n_lock_r, &phy0_ctrl->phy_con12);

		n_lock_r = readl(&phy1_ctrl->phy_con13);
		n_lock_w_phy1 = (n_lock_r & CTRL_LOCK_COARSE_MASK) >> 2;
		n_lock_r = readl(&phy1_ctrl->phy_con12);
		n_lock_r &= ~CTRL_DLL_ON;
		n_lock_r |= n_lock_w_phy1;
		writel(n_lock_r, &phy1_ctrl->phy_con12);

		val = (0x3 << DIRECT_CMD_BANK_SHIFT) | 0x4;
		for (chip = 0; chip < mem->chips_to_configure; chip++) {
			writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
			       &drex0->directcmd);
			writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
			       &drex1->directcmd);
		}

		setbits_le32(&phy0_ctrl->phy_con2, RDLVL_GATE_EN);
		setbits_le32(&phy1_ctrl->phy_con2, RDLVL_GATE_EN);

		setbits_le32(&phy0_ctrl->phy_con0, CTRL_SHGATE);
		setbits_le32(&phy1_ctrl->phy_con0, CTRL_SHGATE);

		val = readl(&phy0_ctrl->phy_con1);
		val &= ~(CTRL_GATEDURADJ_MASK);
		writel(val, &phy0_ctrl->phy_con1);

		val = readl(&phy1_ctrl->phy_con1);
		val &= ~(CTRL_GATEDURADJ_MASK);
		writel(val, &phy1_ctrl->phy_con1);

		writel(CTRL_RDLVL_GATE_ENABLE, &drex0->rdlvl_config);
		i = TIMEOUT;
		while (((readl(&drex0->phystatus) & RDLVL_COMPLETE_CHO) !=
			RDLVL_COMPLETE_CHO) && (i > 0)) {
			/*
			 * TODO(waihong): Comment on how long this take to
			 * timeout
			 */
			sdelay(100);
			i--;
		}
		if (!i)
			return SETUP_ERR_RDLV_COMPLETE_TIMEOUT;
		writel(CTRL_RDLVL_GATE_DISABLE, &drex0->rdlvl_config);

		writel(CTRL_RDLVL_GATE_ENABLE, &drex1->rdlvl_config);
		i = TIMEOUT;
		while (((readl(&drex1->phystatus) & RDLVL_COMPLETE_CHO) !=
			RDLVL_COMPLETE_CHO) && (i > 0)) {
			/*
			 * TODO(waihong): Comment on how long this take to
			 * timeout
			 */
			sdelay(100);
			i--;
		}
		if (!i)
			return SETUP_ERR_RDLV_COMPLETE_TIMEOUT;
		writel(CTRL_RDLVL_GATE_DISABLE, &drex1->rdlvl_config);

		writel(0, &phy0_ctrl->phy_con14);
		writel(0, &phy1_ctrl->phy_con14);

		val = (0x3 << DIRECT_CMD_BANK_SHIFT);
		for (chip = 0; chip < mem->chips_to_configure; chip++) {
			writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
			       &drex0->directcmd);
			writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
			       &drex1->directcmd);
		}

		if (mem->read_leveling_enable) {
			/* Set Read DQ Calibration */
			val = (0x3 << DIRECT_CMD_BANK_SHIFT) | 0x4;
			for (chip = 0; chip < mem->chips_to_configure; chip++) {
				writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
				       &drex0->directcmd);
				writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
				       &drex1->directcmd);
			}

			val = readl(&phy0_ctrl->phy_con1);
			val |= READ_LEVELLING_DDR3;
			writel(val, &phy0_ctrl->phy_con1);
			val = readl(&phy1_ctrl->phy_con1);
			val |= READ_LEVELLING_DDR3;
			writel(val, &phy1_ctrl->phy_con1);

			val = readl(&phy0_ctrl->phy_con2);
			val |= (RDLVL_EN | RDLVL_INCR_ADJ);
			writel(val, &phy0_ctrl->phy_con2);
			val = readl(&phy1_ctrl->phy_con2);
			val |= (RDLVL_EN | RDLVL_INCR_ADJ);
			writel(val, &phy1_ctrl->phy_con2);

			setbits_le32(&drex0->rdlvl_config,
				     CTRL_RDLVL_DATA_ENABLE);
			i = TIMEOUT;
			while (((readl(&drex0->phystatus) & RDLVL_COMPLETE_CHO)
				 != RDLVL_COMPLETE_CHO) && (i > 0)) {
				/*
				 * TODO(waihong): Comment on how long this take
				 * to timeout
				 */
				sdelay(100);
				i--;
			}
			if (!i)
				return SETUP_ERR_RDLV_COMPLETE_TIMEOUT;

			clrbits_le32(&drex0->rdlvl_config,
				     CTRL_RDLVL_DATA_ENABLE);
			setbits_le32(&drex1->rdlvl_config,
				     CTRL_RDLVL_DATA_ENABLE);
			i = TIMEOUT;
			while (((readl(&drex1->phystatus) & RDLVL_COMPLETE_CHO)
				 != RDLVL_COMPLETE_CHO) && (i > 0)) {
				/*
				 * TODO(waihong): Comment on how long this take
				 * to timeout
				 */
				sdelay(100);
				i--;
			}
			if (!i)
				return SETUP_ERR_RDLV_COMPLETE_TIMEOUT;

			clrbits_le32(&drex1->rdlvl_config,
				     CTRL_RDLVL_DATA_ENABLE);

			val = (0x3 << DIRECT_CMD_BANK_SHIFT);
			for (chip = 0; chip < mem->chips_to_configure; chip++) {
				writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
				       &drex0->directcmd);
				writel(val | (chip << DIRECT_CMD_CHIP_SHIFT),
				       &drex1->directcmd);
			}

			update_reset_dll(&drex0->phycontrol0, DDR_MODE_DDR3);
			update_reset_dll(&drex1->phycontrol0, DDR_MODE_DDR3);
		}

		/* Common Settings for Leveling */
		val = PHY_CON12_RESET_VAL;
		writel((val + n_lock_w_phy0), &phy0_ctrl->phy_con12);
		writel((val + n_lock_w_phy1), &phy1_ctrl->phy_con12);

		setbits_le32(&phy0_ctrl->phy_con2, DLL_DESKEW_EN);
		setbits_le32(&phy1_ctrl->phy_con2, DLL_DESKEW_EN);
	}

	/* Send PALL command */
	dmc_config_prech(mem, &drex0->directcmd);
	dmc_config_prech(mem, &drex1->directcmd);

	writel(mem->memcontrol, &drex0->memcontrol);
	writel(mem->memcontrol, &drex1->memcontrol);

	/*
	 * Set DMC Concontrol: Enable auto-refresh counter, provide
	 * read data fetch cycles and enable DREX auto set powerdown
	 * for input buffer of I/O in none read memory state.
	 */
	writel(mem->concontrol | (mem->aref_en << CONCONTROL_AREF_EN_SHIFT) |
		(mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT)|
		DMC_CONCONTROL_IO_PD_CON(0x2),
		&drex0->concontrol);
	writel(mem->concontrol | (mem->aref_en << CONCONTROL_AREF_EN_SHIFT) |
		(mem->rd_fetch << CONCONTROL_RD_FETCH_SHIFT)|
		DMC_CONCONTROL_IO_PD_CON(0x2),
		&drex1->concontrol);

	/*
	 * Enable Clock Gating Control for DMC
	 * this saves around 25 mw dmc power as compared to the power
	 * consumption without these bits enabled
	 */
	setbits_le32(&drex0->cgcontrol, DMC_INTERNAL_CG);
	setbits_le32(&drex1->cgcontrol, DMC_INTERNAL_CG);

	return 0;
}
#endif
