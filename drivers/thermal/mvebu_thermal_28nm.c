// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

/* #define DEBUG */
#include <common.h>
#include <asm/io.h>
#include <asm/arch-mvebu/thermal.h>

DECLARE_GLOBAL_DATA_PTR;

#define THERMAL_SEN_CTRL			0x0
#define THERMAL_SEN_TC_TRIM_OFFSET		0
#define THERMAL_SEN_TC_TRIM_MASK		\
	(0x7 << THERMAL_SEN_TC_TRIM_OFFSET)

#define THERMAL_SEN_CTRL_MSB			0x4
#define THERMAL_SEN_CTRL_MSB_RST_OFFSET		8
#define THERMAL_SEN_CTRL_MSB_RST_MASK		\
	(0x1 << THERMAL_SEN_CTRL_MSB_RST_OFFSET)

#define THERMAL_SEN_CTRL_STATS			0x8
#define THERMAL_SEN_CTRL_STATS_VALID_OFFSET	10
#define THERMAL_SEN_CTRL_STATS_VALID_MASK	\
	(0x1 << THERMAL_SEN_CTRL_STATS_VALID_OFFSET)

#define THERMAL_SEN_CTRL_STATS_TEMP_OUT_OFFSET	0
#define THERMAL_SEN_CTRL_STATS_TEMP_OUT_MASK	\
	(0x3FF << THERMAL_SEN_CTRL_STATS_TEMP_OUT_OFFSET)

s32 mvebu_thermal_sensor_read(struct thermal_unit_config *tsen, int *temp)
{
	u32 reg;

	if (!tsen->tsen_ready) {
		printf("Thermal Sensor was not initialized\n");
		return 0;
	}

	reg = readl(tsen->regs_base + THERMAL_SEN_CTRL_STATS);
	reg = ((reg & THERMAL_SEN_CTRL_STATS_TEMP_OUT_MASK) >>
	      THERMAL_SEN_CTRL_STATS_TEMP_OUT_OFFSET);

	*temp = ((tsen->tsen_gain * ((s32)reg)) - tsen->tsen_offset) /
	       tsen->tsen_divisor;

	return 0;
}

u32 mvebu_thermal_sensor_probe(struct thermal_unit_config *tsen)
{
	u32 reg, timeout = 0;

	debug("thermal.%lx Initializing sensor unit\n",
	      (uintptr_t)tsen->regs_base);

	/* Initialize thermal sensor hardware reset once */
	reg = readl(tsen->regs_base + THERMAL_SEN_CTRL_MSB);
	reg |= THERMAL_SEN_CTRL_MSB_RST_MASK;
	writel(reg, tsen->regs_base + THERMAL_SEN_CTRL_MSB);

	/* set Tsen Tc Trim to correct default value (errata #132698) */
	reg = readl(tsen->regs_base + THERMAL_SEN_CTRL);
	reg &= ~THERMAL_SEN_TC_TRIM_MASK;
	reg |= 0x3;
	writel(reg, tsen->regs_base + THERMAL_SEN_CTRL);

	/* Check that Sensor is ready */
	reg = readl(tsen->regs_base + THERMAL_SEN_CTRL_STATS);
	while ((reg & THERMAL_SEN_CTRL_STATS_VALID_MASK) == 0 &&
	       timeout < 300) {
		udelay(1);
		reg = readl(tsen->regs_base + THERMAL_SEN_CTRL_STATS);
		timeout++;
	}

	if ((reg & THERMAL_SEN_CTRL_STATS_VALID_MASK) == 0) {
		pr_err("%s: thermal.%lx: sensor is not ready\n", __func__,
		       (uintptr_t)tsen->regs_base);
		return -1;
	}

	debug("thermal.%lx: Initialization done\n", (uintptr_t)tsen->regs_base);

	return 0;
}
