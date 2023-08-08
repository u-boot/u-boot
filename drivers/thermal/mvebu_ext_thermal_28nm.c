// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

/* #define DEBUG */
#include <common.h>
#include <asm/io.h>
#include <asm/arch-mvebu/thermal.h>

DECLARE_GLOBAL_DATA_PTR;

#define THERMAL_TIMEOUT					1200

#define THERMAL_SEN_CTRL_LSB				0x0
#define THERMAL_SEN_CTRL_LSB_STRT_OFFSET		0
#define THERMAL_SEN_CTRL_LSB_STRT_MASK			\
	(0x1 << THERMAL_SEN_CTRL_LSB_STRT_OFFSET)
#define THERMAL_SEN_CTRL_LSB_RST_OFFSET			1
#define THERMAL_SEN_CTRL_LSB_RST_MASK			\
	(0x1 << THERMAL_SEN_CTRL_LSB_RST_OFFSET)
#define THERMAL_SEN_CTRL_LSB_EN_OFFSET			2
#define THERMAL_SEN_CTRL_LSB_EN_MASK			\
	(0x1 << THERMAL_SEN_CTRL_LSB_EN_OFFSET)

#define THERMAL_SEN_CTRL_STATS				0x8
#define THERMAL_SEN_CTRL_STATS_VALID_OFFSET		16
#define THERMAL_SEN_CTRL_STATS_VALID_MASK		\
	(0x1 << THERMAL_SEN_CTRL_STATS_VALID_OFFSET)
#define THERMAL_SEN_CTRL_STATS_TEMP_OUT_OFFSET		0
#define THERMAL_SEN_CTRL_STATS_TEMP_OUT_MASK		\
	(0x3FF << THERMAL_SEN_CTRL_STATS_TEMP_OUT_OFFSET)

#define THERMAL_SEN_OUTPUT_MSB				512
#define THERMAL_SEN_OUTPUT_COMP				1024

s32 mvebu_thermal_ext_sensor_read(struct thermal_unit_config *tsen, int *temp)
{
	u32 reg;
	int ret = 0;

	if (!tsen->tsen_ready) {
		printf("External Thermal Sensor was not initialized\n");
		return 0;
	}

	if (tsen->fw_smc_support) {
		ret = mvebu_dfx_smc_thermal(MV_SIP_DFX_THERMAL_READ, &reg);
		if (ret)
			return ret;

		*temp = reg / tsen->tsen_divisor;

		return ret;
	}

	reg = readl(tsen->regs_base + THERMAL_SEN_CTRL_STATS);
	reg = ((reg & THERMAL_SEN_CTRL_STATS_TEMP_OUT_MASK) >>
	      THERMAL_SEN_CTRL_STATS_TEMP_OUT_OFFSET);

	/*
	 * TSEN output format is signed as a 2s complement number
	 * ranging from-512 to +511. when MSB is set, need to
	 * calculate the complement number
	 */
	if (reg >= THERMAL_SEN_OUTPUT_MSB)
		reg -= THERMAL_SEN_OUTPUT_COMP;

	*temp = ((tsen->tsen_gain * ((s32)reg)) + tsen->tsen_offset) /
	       tsen->tsen_divisor;

	return ret;
}

static u32 mvebu_thermal_ext_fw_validation(struct thermal_unit_config *tsen)
{
	u32 reg = 0, timeout = 0;
	int ret = SMCCC_RET_SUCCESS;

	debug("%s: fw smc support\n", __func__);

	tsen->fw_smc_support = true;

	while (reg == 0 && timeout < THERMAL_TIMEOUT &&
	       ret == SMCCC_RET_SUCCESS) {
		udelay(10);
		ret = mvebu_dfx_smc_thermal(MV_SIP_DFX_THERMAL_IS_VALID, &reg);
		timeout++;
	}

	if (reg == 0 || ret != SMCCC_RET_SUCCESS) {
		pr_err("%s: thermal.%lx: external sensor is not ready\n",
		       __func__, (uintptr_t)tsen->regs_base);
		return -1;
	}

	debug("thermal.%lx: Initialization done\n", (uintptr_t)tsen->regs_base);

	return 0;
}

u32 mvebu_thermal_ext_sensor_probe(struct thermal_unit_config *tsen)
{
	u32 reg = 0, timeout = 0;
	int ret;

	debug("thermal.%lx Initializing sensor unit\n",
	      (uintptr_t)tsen->regs_base);

	/* Try init thermal sensor via firmware, if fails try legacy */
	ret = mvebu_dfx_smc_thermal(MV_SIP_DFX_THERMAL_INIT, 0x0);
	if (ret == SMCCC_RET_SUCCESS)
		return mvebu_thermal_ext_fw_validation(tsen);

	/* Initialize thermal sensor hardware reset once */
	reg = readl(tsen->regs_base + THERMAL_SEN_CTRL_LSB);
	/* De-assert TSEN_RESET */
	reg &= ~THERMAL_SEN_CTRL_LSB_RST_OFFSET;
	/* Set TSEN_EN to 1 */
	reg |= THERMAL_SEN_CTRL_LSB_EN_MASK;
	/* Set TSEN_START to 1 */
	reg |= THERMAL_SEN_CTRL_LSB_STRT_MASK;
	writel(reg, tsen->regs_base + THERMAL_SEN_CTRL_LSB);

	reg = readl(tsen->regs_base + THERMAL_SEN_CTRL_STATS);
	while ((reg & THERMAL_SEN_CTRL_STATS_VALID_MASK) == 0 &&
	       timeout < THERMAL_TIMEOUT) {
		udelay(10);
		reg = readl(tsen->regs_base + THERMAL_SEN_CTRL_STATS);
		timeout++;
	}
	if ((reg & THERMAL_SEN_CTRL_STATS_VALID_MASK) == 0) {
		pr_err("%s: thermal.%lx: external sensor is not ready\n",
		       __func__, (uintptr_t)tsen->regs_base);
		return -1;
	}

	debug("thermal.%lx: Initialization done\n", (uintptr_t)tsen->regs_base);

	return 0;
}
