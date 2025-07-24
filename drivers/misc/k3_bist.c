// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' BIST (Built-In Self-Test) driver
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 *      Neha Malcom Francis <n-francis@ti.com>
 *
 */

#include <dm.h>
#include <errno.h>
#include <clk.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <asm/arch/hardware.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <remoteproc.h>
#include <power-domain.h>
#include <k3_bist.h>

#include "k3_bist_static_data.h"

/* PBIST Timeout Value */
#define PBIST_MAX_TIMEOUT_VALUE		100000000

/**
 * struct k3_bist_privdata - K3 BIST structure
 * @dev: device pointer
 * @pbist_base: base of register set for PBIST
 * @instance: PBIST instance number
 * @intr_num: corresponding interrupt ID of the PBIST instance
 * @lbist_ctrl_mmr: base of CTRL MMR register set for LBIST
 */
struct k3_bist_privdata {
	struct udevice *dev;
	void *pbist_base;
	u32 instance;
	u32 intr_num;
	void *lbist_ctrl_mmr;
	struct pbist_inst_info *pbist_info;
	struct lbist_inst_info *lbist_info;
};

static struct k3_bist_privdata *k3_bist_priv;

/**
 * check_post_pbist_result() - Check POST results
 *
 * Function to check whether HW Power-On Self Test, i.e. POST has run
 * successfully on the MCU domain.
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static int check_post_pbist_result(void)
{
	bool is_done, timed_out;
	u32 mask;
	u32 post_reg_val, shift;

	/* Read HW POST status register */
	post_reg_val = readl(WKUP_CTRL_MMR0_BASE + WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT);

	/* Check if HW POST PBIST was performed */
	shift = WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_PBIST_DONE_SHIFT;
	is_done = (((post_reg_val >> shift) & 0x1u) == 0x1u) ? (bool)true : (bool)false;

	if (!is_done) {
		/* HW POST: PBIST not completed, check if it timed out */
		shift = WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_PBIST_TIMEOUT_SHIFT;
		timed_out = (((post_reg_val >> shift) & 0x1u) == 0x1u) ? (bool)true : (bool)false;

		if (!timed_out) {
			printf("%s: PBIST was not performed at all on this device for this core\n",
			       __func__);
			return -EINVAL;
		}
		printf("%s: PBIST was attempted but timed out for this section\n",
		       __func__);
		return -ETIMEDOUT;

	} else {
		/* HW POST: PBIST was completed on this device, check the result */
		mask = WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_PBIST_FAIL_MASK;

		if ((post_reg_val & mask) != 0) {
			printf("%s: PBIST was completed, but the test failed\n", __func__);
			return -EINVAL;
		}
		debug("%s: HW POST PBIST completed, test passed\n", __func__);
	}

	return 0;
}

/**
 * check_post_lbist_result() - Check POST results
 *
 * Function to check whether HW Power-On Self Test, i.e. POST has run
 * successfully on the MCU domain.
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static int check_post_lbist_result(void)
{
	bool is_done, timed_out;
	u32 post_reg_val, shift;
	u32 calculated_misr, expected_misr;

	/* Read HW POST status register */
	post_reg_val = readl(WKUP_CTRL_MMR0_BASE + WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT);

	/* Check if HW POST LBIST was performed */
	shift = WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_LBIST_DONE_SHIFT;
	is_done = (((post_reg_val >> shift) & 0x1u) == 0x1u) ? (bool)true : (bool)false;

	if (!is_done) {
		/* HW POST: PBIST not completed, check if it timed out */
		shift = WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_LBIST_TIMEOUT_SHIFT;
		timed_out = (((post_reg_val >> shift) & 0x1u) == 0x1u) ? (bool)true : (bool)false;

		if (!timed_out) {
			printf("%s: PBIST was not performed at all on this device for this core\n",
			       __func__);
			return -EINVAL;
		}
		printf("%s: PBIST was attempted but timed out for this section\n",
		       __func__);
		return -ETIMEDOUT;

	} else {
		/* Get the output MISR and the expected MISR which 0 for MCU domain */
		lbist_get_misr((void *)MCU_LBIST_BASE, &calculated_misr);
		expected_misr = readl(MCU_CTRL_MMR0_CFG0_BASE + MCU_CTRL_MMR_CFG0_MCU_LBIST_SIG);

		if (calculated_misr != expected_misr) {
			/* HW POST: LBIST was completed, but the test failed for this core */
			printf("%s: calculated MISR != expected MISR\n", __func__);
			debug("%s: calculated MISR = %x\n", __func__, calculated_misr);
			debug("%s: expected MISR = %x\n", __func__, expected_misr);
			return -EINVAL;
		}
		debug("%s: HW POST LBIST completed, test passed\n", __func__);
	}

	return 0;
}

/**
 * pbist_self_test() - Run PBIST_TEST on specified cores
 * @config: pbist_config structure for PBIST test
 *
 * Function to run PBIST_TEST
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static int pbist_self_test(struct pbist_config *config)
{
	void *base = k3_bist_priv->pbist_base;

	/* Turns on PBIST clock in PBIST ACTivate register */
	writel(PBIST_PACT_PACT_MASK, base + PBIST_PACT);

	/* Set Margin mode register for Test mode */
	writel(PBIST_TEST_MODE, base + PBIST_MARGIN_MODE);

	/* Zero out Loop counter 0 */
	writel(0x0, base + PBIST_L0);

	/* Set algorithm bitmap */
	writel(config->algorithms_bit_map, base + PBIST_ALGO);

	/* Set Memory group bitmap */
	writel(config->memory_groups_bit_map, base + PBIST_RINFO);

	/* Zero out override register */
	writel(config->override, base + PBIST_OVER);

	/* Set Scramble value - 64 bit*/
	writel(config->scramble_value_lo, base + PBIST_SCR_LO);
	writel(config->scramble_value_hi, base + PBIST_SCR_HI);

	/* Set DLR register for ROM based testing and Config Access */
	writel(PBIST_DLR_DLR0_ROM_MASK
	| PBIST_DLR_DLR0_CAM_MASK, base + PBIST_DLR);

	/* Allow time for completion of test*/
	udelay(1000);

	if (readl(base + PBIST_FSRF)) {
		printf("%s: test failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/**
 * pbist_neg_self_test() - Run PBIST_negTEST on specified cores
 * @config: pbist_config_neg structure for PBIST negative test
 *
 * Function to run PBIST failure insertion test
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static int pbist_neg_self_test(struct pbist_config_neg *config)
{
	void *base = k3_bist_priv->pbist_base;

	/* Turns on PBIST clock in PBIST ACTivate register */
	writel(PBIST_PACT_PACT_MASK, base + PBIST_PACT);

	/* Set Margin mode register for Test mode */
	writel(PBIST_FAILURE_INSERTION_TEST_MODE, base + PBIST_MARGIN_MODE);

	/* Zero out Loop counter 0 */
	writel(0x0, base + PBIST_L0);

	/* Set DLR register */
	writel(0x10, base + PBIST_DLR);

	/* Set Registers*/
	writel(0x00000001, base + PBIST_RF0L);
	writel(0x00003123, base + PBIST_RF0U);
	writel(0x0513FC02, base + PBIST_RF1L);
	writel(0x00000002, base + PBIST_RF1U);
	writel(0x00000003, base + PBIST_RF2L);
	writel(0x00000000, base + PBIST_RF2U);
	writel(0x00000004, base + PBIST_RF3L);
	writel(0x00000028, base + PBIST_RF3U);
	writel(0x64000044, base + PBIST_RF4L);
	writel(0x00000000, base + PBIST_RF4U);
	writel(0x0006A006, base + PBIST_RF5L);
	writel(0x00000000, base + PBIST_RF5U);
	writel(0x00000007, base + PBIST_RF6L);
	writel(0x0000A0A0, base + PBIST_RF6U);
	writel(0x00000008, base + PBIST_RF7L);
	writel(0x00000064, base + PBIST_RF7U);
	writel(0x00000009, base + PBIST_RF8L);
	writel(0x0000A5A5, base + PBIST_RF8U);
	writel(0x0000000A, base + PBIST_RF9L);
	writel(0x00000079, base + PBIST_RF9U);
	writel(0x00000000, base + PBIST_RF10L);
	writel(0x00000001, base + PBIST_RF10U);
	writel(0xAAAAAAAA, base + PBIST_D);
	writel(0xAAAAAAAA, base + PBIST_E);

	writel(config->CA2, base + PBIST_CA2);
	writel(config->CL0, base + PBIST_CL0);
	writel(config->CA3, base + PBIST_CA3);
	writel(config->I0, base + PBIST_I0);
	writel(config->CL1, base + PBIST_CL1);
	writel(config->I3, base + PBIST_I3);
	writel(config->I2, base + PBIST_I2);
	writel(config->CL2, base + PBIST_CL2);
	writel(config->CA1, base + PBIST_CA1);
	writel(config->CA0, base + PBIST_CA0);
	writel(config->CL3, base + PBIST_CL3);
	writel(config->I1, base + PBIST_I1);
	writel(config->RAMT, base + PBIST_RAMT);
	writel(config->CSR, base + PBIST_CSR);
	writel(config->CMS, base + PBIST_CMS);

	writel(0x00000009, base + PBIST_STR);

	/* Start PBIST */
	writel(0x00000001, base + PBIST_STR);

	/* Allow time for completion of test*/
	udelay(1000);

	if (readl(base + PBIST_FSRF) == 0) {
		printf("%s: test failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/**
 * pbist_rom_self_test() - Run PBIST_ROM_TEST on specified cores
 * @config: pbist_config_rom structure for PBIST negative test
 *
 * Function to run PBIST test of ROM
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static int pbist_rom_self_test(struct pbist_config_rom *config)
{
	void *base = k3_bist_priv->pbist_base;

	/* Turns on PBIST clock in PBIST ACTivate register */
	writel(0x1, base + PBIST_PACT);

	/* Set Margin mode register for Test mode */
	writel(0xf, base + PBIST_MARGIN_MODE);

	/* Zero out Loop counter 0 */
	writel(0x0, base + PBIST_L0);

	/* Set DLR register */
	writel(0x310, base + PBIST_DLR);

	/* Set Registers*/
	writel(0x00000001, base + PBIST_RF0L);
	writel(0x00003123, base + PBIST_RF0U);
	writel(0x7A400183, base + PBIST_RF1L);
	writel(0x00000060, base + PBIST_RF1U);
	writel(0x00000184, base + PBIST_RF2L);
	writel(0x00000000, base + PBIST_RF2U);
	writel(0x7B600181, base + PBIST_RF3L);
	writel(0x00000061, base + PBIST_RF3U);
	writel(0x00000000, base + PBIST_RF4L);
	writel(0x00000000, base + PBIST_RF4U);

	writel(config->D, base + PBIST_D);
	writel(config->E, base + PBIST_E);
	writel(config->CA2, base + PBIST_CA2);
	writel(config->CL0, base + PBIST_CL0);
	writel(config->CA3, base + PBIST_CA3);
	writel(config->I0, base + PBIST_I0);
	writel(config->CL1, base + PBIST_CL1);
	writel(config->I3, base + PBIST_I3);
	writel(config->I2, base + PBIST_I2);
	writel(config->CL2, base + PBIST_CL2);
	writel(config->CA1, base + PBIST_CA1);
	writel(config->CA0, base + PBIST_CA0);
	writel(config->CL3, base + PBIST_CL3);
	writel(config->I1, base + PBIST_I1);
	writel(config->RAMT, base + PBIST_RAMT);
	writel(config->CSR, base + PBIST_CSR);
	writel(config->CMS, base + PBIST_CMS);

	writel(0x00000009, base + PBIST_STR);

	/* Start PBIST */
	writel(0x00000001, base + PBIST_STR);

	/* Allow time for completion of test*/
	udelay(1000);

	if (readl(base + PBIST_FSRF)) {
		printf("%s: test failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/**
 * lbist_program_config() - Program LBIST config
 * @config: lbist_config structure for LBIST test
 */
static void lbist_program_config(struct lbist_config *config)
{
	void *base = k3_bist_priv->lbist_ctrl_mmr;

	lbist_set_clock_delay(base, config->dc_def);
	lbist_set_divide_ratio(base, config->divide_ratio);
	lbist_clear_load_div(base);
	lbist_set_load_div(base);
	lbist_set_num_stuck_at_patterns(base, config->static_pc_def);
	lbist_set_num_set_patterns(base, config->set_pc_def);
	lbist_set_num_reset_patterns(base, config->reset_pc_def);
	lbist_set_num_chain_test_patterns(base, config->scan_pc_def);
	lbist_set_seed(base, config->prpg_def_l, config->prpg_def_u);
}

/**
 * lbist_enable_isolation() - LBIST Enable Isolation
 * @config: lbist_config structure for LBIST test
 */
void lbist_enable_isolation(void)
{
	void *base = k3_bist_priv->lbist_ctrl_mmr;
	u32 reg_val;

	reg_val = readl(base + LBIST_SPARE0);
	writel(reg_val | (LBIST_SPARE0_LBIST_SELFTEST_EN_MASK), base + LBIST_SPARE0);
}

/**
 * lbist_disable_isolation() - LBIST Disable Isolation
 * @config: lbist_config structure for LBIST test
 */
void lbist_disable_isolation(void)
{
	void *base = k3_bist_priv->lbist_ctrl_mmr;
	u32 reg_val;

	reg_val = readl(base + LBIST_SPARE0);
	writel(reg_val & (~(LBIST_SPARE0_LBIST_SELFTEST_EN_MASK)), base + LBIST_SPARE0);
}

/**
 * lbist_enable_run_bist_mode() - LBIST Enable run BIST mode
 * @config: lbist_config structure for LBIST test
 */
static void lbist_enable_run_bist_mode(struct lbist_config *config)
{
	void *base = k3_bist_priv->lbist_ctrl_mmr;
	u32 reg_val;

	reg_val = readl(base + LBIST_CTRL);
	writel(reg_val | (LBIST_CTRL_RUNBIST_MODE_MAX << LBIST_CTRL_RUNBIST_MODE_SHIFT),
	       base + LBIST_CTRL);
}

/**
 * lbist_start() - Start LBIST test
 * @config: lbist_config structure for LBIST test
 */
static void lbist_start(struct lbist_config *config)
{
	struct udevice *dev = k3_bist_priv->dev;
	void *base = k3_bist_priv->lbist_ctrl_mmr;
	u32 reg_val;
	u32 timeout_count = 0;

	reg_val = readl(base + LBIST_CTRL);
	writel(reg_val | (LBIST_CTRL_BIST_RESET_MAX << LBIST_CTRL_BIST_RESET_SHIFT),
	       base + LBIST_CTRL);

	reg_val = readl(base + LBIST_CTRL);
	writel(reg_val | (LBIST_CTRL_BIST_RUN_MAX << LBIST_CTRL_BIST_RUN_SHIFT),
	       base + LBIST_CTRL);

	reg_val = readl(base + LBIST_STAT);
	if ((reg_val & LBIST_STAT_BIST_RUNNING_MASK) != 0)
		debug("%s(dev=%p): LBIST is running\n", __func__, dev);

	while (((!(readl(base + LBIST_STAT) & LBIST_STAT_BIST_DONE_MASK))) &&
	       (timeout_count++ < PBIST_MAX_TIMEOUT_VALUE)) {
	}

	if (!(readl(base + LBIST_STAT) & LBIST_STAT_BIST_DONE_MASK))
		printf("%s(dev=%p): test failed\n", __func__, dev);
}

/**
 * lbist_check_result() - Check LBIST test result
 * @config: lbist_config structure for LBIST test
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static int lbist_check_result(struct lbist_config *config)
{
	void *base = k3_bist_priv->lbist_ctrl_mmr;
	struct lbist_inst_info *info = k3_bist_priv->lbist_info;
	u32 calculated_misr;
	u32 expected_misr;

	lbist_get_misr(base, &calculated_misr);
	expected_misr = info->expected_misr;
	lbist_clear_run_bist_mode(base);
	lbist_stop(base);
	lbist_reset(base);

	if (calculated_misr != expected_misr) {
		printf("calculated_misr != expected_misr\n %x %x\n",
		       calculated_misr, expected_misr);
		return -EINVAL;
	}

	return 0;
}

static int k3_run_lbist(void)
{
	/* Check whether HW POST successfully completely LBIST on the MCU domain */
	struct lbist_inst_info *info_lbist = k3_bist_priv->lbist_info;

	lbist_program_config(&info_lbist->lbist_conf);
	lbist_enable_isolation();
	lbist_reset(&info_lbist->lbist_conf);
	lbist_enable_run_bist_mode(&info_lbist->lbist_conf);
	lbist_start(&info_lbist->lbist_conf);
	if (lbist_check_result(&info_lbist->lbist_conf)) {
		printf("%s: test failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int k3_run_lbist_post(void)
{
	if (check_post_lbist_result()) {
		printf("HW POST LBIST failed to run successfully\n");
		return -EINVAL;
	}

	return 0;
}

static int k3_run_pbist_post(void)
{
	/* Check whether HW POST successfully completely PBIST on the MCU domain */
	if (check_post_pbist_result()) {
		printf("HW POST failed to run successfully\n");
		return -EINVAL;
	}

	return 0;
}

static int k3_run_pbist(void)
{
	/* Run PBIST test */
	struct pbist_inst_info *info = k3_bist_priv->pbist_info;
	int num_runs = info->num_pbist_runs;

	for (int j = 0; j < num_runs; j++) {
		if (pbist_self_test(&info->pbist_config_run[j])) {
			printf("failed to run PBIST test\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int k3_run_pbist_neg(void)
{
	/* Run PBIST failure insertion test */
	struct pbist_inst_info *info = k3_bist_priv->pbist_info;

	if (pbist_neg_self_test(&info->pbist_neg_config_run)) {
		printf("failed to run PBIST negative test\n");
		return -EINVAL;
	}

	return 0;
}

static int k3_run_pbist_rom(void)
{
	/* Run PBIST test on ROM */
	struct pbist_inst_info *info = k3_bist_priv->pbist_info;
	int num_runs = info->num_pbist_rom_test_runs;

	for (int j = 0; j < num_runs; j++) {
		if (pbist_rom_self_test(&info->pbist_rom_test_config_run[j])) {
			printf("failed to run ROM PBIST test\n");
			return -EINVAL;
		}
	}

	return 0;
}

int prepare_pbist(struct ti_sci_handle *handle)
{
	struct ti_sci_proc_ops *proc_ops = &handle->ops.proc_ops;
	struct ti_sci_dev_ops *dev_ops = &handle->ops.dev_ops;
	struct pbist_inst_info *info_pbist = k3_bist_priv->pbist_info;
	struct core_under_test *cut = info_pbist->cut;

	if (proc_ops->proc_request(handle, cut[0].proc_id)) {
		printf("%s: requesting primary core failed\n", __func__);
		return -EINVAL;
	}

	if (proc_ops->proc_request(handle, cut[1].proc_id)) {
		printf("%s: requesting secondary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->set_device_resets(handle, cut[0].dev_id, 0x1)) {
		printf("%s: local reset primary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->set_device_resets(handle, cut[1].dev_id, 0x1)) {
		printf("%s: local reset secondary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->get_device(handle, cut[0].dev_id)) {
		printf("%s: power on primary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->get_device(handle, cut[1].dev_id)) {
		printf("%s: power on secondary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->get_device(handle, info_pbist->dev_id)) {
		printf("%s: power on PBIST failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

int deprepare_pbist(struct ti_sci_handle *handle)
{
	struct ti_sci_proc_ops *proc_ops = &handle->ops.proc_ops;
	struct ti_sci_dev_ops *dev_ops = &handle->ops.dev_ops;
	struct pbist_inst_info *info_pbist = k3_bist_priv->pbist_info;
	struct core_under_test *cut = info_pbist->cut;

	if (dev_ops->put_device(handle, info_pbist->dev_id)) {
		printf("%s: power off PBIST failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->put_device(handle, cut[1].dev_id)) {
		printf("%s: power off secondary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->put_device(handle, cut[0].dev_id)) {
		printf("%s: power off primary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->set_device_resets(handle, cut[0].dev_id, 0)) {
		printf("%s: putting primary core out of local reset failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->set_device_resets(handle, cut[1].dev_id, 0)) {
		printf("%s: putting secondary core out of local reset failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->put_device(handle, cut[0].dev_id)) {
		printf("%s: power off primary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->put_device(handle, cut[1].dev_id)) {
		printf("%s: power off secondary core failed\n", __func__);
		return -EINVAL;
	}

	if (proc_ops->proc_release(handle, cut[0].proc_id)) {
		printf("%s: release primary core failed\n", __func__);
		return -EINVAL;
	}

	if (proc_ops->proc_release(handle, cut[1].proc_id)) {
		printf("%s: release secondary core failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

int prepare_lbist(struct ti_sci_handle *handle)
{
	struct ti_sci_proc_ops *proc_ops = &handle->ops.proc_ops;
	struct ti_sci_dev_ops *dev_ops = &handle->ops.dev_ops;
	struct lbist_inst_info *info_lbist = k3_bist_priv->lbist_info;
	struct core_under_test *cut = &info_lbist->cut;

	if (proc_ops->proc_request(handle, cut->proc_id)) {
		printf("%s: requesting primary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->set_device_resets(handle, cut->dev_id, 0x3)) {
		printf("%s: module and local reset primary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->idle_device(handle, cut->dev_id)) {
		printf("%s: putting primary core into retention failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

int deprepare_lbist(struct ti_sci_handle *handle)
{
	struct ti_sci_proc_ops *proc_ops = &handle->ops.proc_ops;
	struct ti_sci_dev_ops *dev_ops = &handle->ops.dev_ops;
	struct lbist_inst_info *info_lbist = k3_bist_priv->lbist_info;
	struct core_under_test *cut = &info_lbist->cut;

	if (dev_ops->put_device(handle, 0)) {
		printf("%s: power off secondary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->put_device(handle, cut->dev_id)) {
		printf("%s: power off primary core failed\n", __func__);
		return -EINVAL;
	}

	lbist_disable_isolation();

	if (dev_ops->idle_device(handle, cut->dev_id)) {
		printf("%s: retention primary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->idle_device(handle, 0)) {
		printf("%s: retention secondary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->put_device(handle, 0)) {
		printf("%s: power off secondary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->put_device(handle, cut->dev_id)) {
		printf("%s: power off primary core failed\n", __func__);
		return -EINVAL;
	}

	if (dev_ops->set_device_resets(handle, cut->dev_id, 0)) {
		printf("%s: putting primary core out of local reset failed\n", __func__);
		return -EINVAL;
	}

	if (proc_ops->proc_release(handle, cut->proc_id)) {
		printf("%s: release primary core failed\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/**
 * k3_bist_probe() - Basic probe
 * @dev: corresponding BIST device
 *
 * Parses BIST info from device tree, and configures the module accordingly.
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_bist_probe(struct udevice *dev)
{
	int ret = 0;
	struct k3_bist_privdata *priv = dev_get_priv(dev);
	struct pbist_inst_info *info;
	struct lbist_inst_info *info_lbist;
	void *reg;

	debug("%s(dev=%p)\n", __func__, dev);

	priv = dev_get_priv(dev);
	priv->dev = dev;

	k3_bist_priv = priv;

	reg = dev_read_addr_name_ptr(dev, "cfg");
	if (!reg) {
		dev_err(dev, "No reg property for BIST\n");
		return -EINVAL;
	}
	priv->pbist_base = reg;

	reg = dev_read_addr_name_ptr(dev, "ctrl_mmr");
	if (!reg) {
		dev_err(dev, "No reg property for CTRL MMR\n");
		return -EINVAL;
	}
	priv->lbist_ctrl_mmr = reg;

	ret = dev_read_u32(dev, "ti,sci-dev-id", &priv->instance);
	if (!priv->instance)
		return -ENODEV;

	switch (priv->instance) {
	case PBIST14_DEV_ID:
		priv->pbist_info = &pbist14_inst_info;
		priv->lbist_info = &lbist_inst_info_main_r5f2_x;
		info = priv->pbist_info;
		info_lbist = priv->lbist_info;
		priv->intr_num = info->intr_num;
		break;
	default:
		dev_err(dev, "%s: PBIST instance %d not supported\n", __func__, priv->instance);
		return -ENODEV;
	};

	return 0;
}

static const struct bist_ops k3_bist_ops = {
	.run_lbist = k3_run_lbist,
	.run_lbist_post = k3_run_lbist_post,
	.run_pbist = k3_run_pbist,
	.run_pbist_post = k3_run_pbist_post,
	.run_pbist_neg = k3_run_pbist_neg,
	.run_pbist_rom = k3_run_pbist_rom,
};

static const struct udevice_id k3_bist_ids[] = {
	{ .compatible = "ti,j784s4-bist" },
	{}
};

U_BOOT_DRIVER(k3_bist) = {
	.name = "k3_bist",
	.of_match = k3_bist_ids,
	.id = UCLASS_MISC,
	.ops = &k3_bist_ops,
	.probe = k3_bist_probe,
	.priv_auto = sizeof(struct k3_bist_privdata),
};
