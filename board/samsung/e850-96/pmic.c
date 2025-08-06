// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 *
 * This file contains functions for S2MPU12 PMIC regulators configuration.
 *
 * Example of voltage calculation for LDO24 and LDO32:
 *   - V_min = 1800 mV
 *   - V_step = 25 mV
 *   - V_wanted = 3300 mV
 *   - register value: (V_wanted - V_min) / V_step = 60 = 0x3c
 *
 * NOTE: 0x3c value might mean different voltage for other LDOs.
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include "pmic.h"

/* PMIC definitions */
#define S2MPU12_CHANNEL			0	/* I3C bus number of PMIC */
#define S2MPU12_PM_ADDR			0x1	/* I3C slave addr of PM part */

/* PMIC I3C registers */
#define S2MPU12_PM_LDO1_CTRL		0x2b
#define S2MPU12_PM_LDO_CTRL(n)		(S2MPU12_PM_LDO1_CTRL + (n) - 1)

/* LDOx_CTRL values */
#define S2MPU12_LDO_CTRL_OUT_MASK	(0x3 << 6)
#define S2MPU12_LDO_CTRL_OUT_ALWAYS_ON	(0x3 << 6)

struct pmic_ldo {
	u8 num;	/* LDO number */
	u8 en;	/* "enable" bits value in LDOx_CTRL register */
	u8 out;	/* "output voltage" bits value in LDOx_CTRL register */
};

/* List of LDOs to enable only */
static u8 pmic_ldos_en[] = {
	2,	/* 1.8V/450mA:	multiple lines */
	11,	/* 3.0V/150mA:	AVDD33_USB20 */
	23,	/* 2.85V/800mA:	VDD_EMMC_2P85 */
	27,	/* 3.0V/150mA:	MIPI_SWITCH_3V3 */
	28,	/* 1.8V/150mA:	HDMI_CONV_1V8 */
	30,	/* 1.8V/150mA:	NPU_VDD18 */
};

/* List of LDOs to enable and set output voltage */
static struct pmic_ldo pmic_ldos_en_out[] = {
	{
		.num	= 24, /* 3.0V/800mA:	VDD_LAN (LAN9514) */
		.en	= S2MPU12_LDO_CTRL_OUT_ALWAYS_ON,
		.out	= 0x3c, /* means 3.3V for LDO24 */
	}, {
		.num	= 32, /* 3.3V/300mA:	CAM_VDD (RPi camera module) */
		.en	= S2MPU12_LDO_CTRL_OUT_ALWAYS_ON,
		.out	= 0x3c, /* means 3.3V for LDO32 */
	},
};

/* Enable specified LDO */
static int pmic_ldo_set_en(struct acpm *acpm, u8 ldo)
{
	const u8 reg = S2MPU12_PM_LDO_CTRL(ldo);
	u8 val;
	int err;

	err = acpm_i3c_read(acpm, S2MPU12_CHANNEL, S2MPU12_PM_ADDR, reg, &val);
	if (err)
		return err;

	val &= ~S2MPU12_LDO_CTRL_OUT_MASK;
	val |= S2MPU12_LDO_CTRL_OUT_ALWAYS_ON;

	return acpm_i3c_write(acpm, S2MPU12_CHANNEL, S2MPU12_PM_ADDR, reg, val);
}

/* Enable specified LDO and set its voltage to 0xc0 value */
static int pmic_ldo_set_en_out(struct acpm *acpm, struct pmic_ldo *ldo)
{
	const u8 reg = S2MPU12_PM_LDO_CTRL(ldo->num);
	const u8 val = ldo->en | ldo->out;

	return acpm_i3c_write(acpm, S2MPU12_CHANNEL, S2MPU12_PM_ADDR, reg, val);
}

#ifdef DEBUG
static void pmic_trace_ldo(struct acpm *acpm, u8 ldo)
{
	const u8 reg = S2MPU12_PM_LDO_CTRL(ldo);
	u8 val;
	int err;

	err = acpm_i3c_read(acpm, S2MPU12_CHANNEL, S2MPU12_PM_ADDR, reg, &val);
	if (err)
		printf("  S2MPU12_PM_LDO%u_CTRL: Read error!\n", ldo);
	else
		printf("  S2MPU12_PM_LDO%u_CTRL: 0x%x\n", ldo, val);
}

static void pmic_trace_ldos(struct acpm *acpm)
{
	size_t i;

	printf("Tracing LDOs...\n");
	for (i = 0; i < ARRAY_SIZE(pmic_ldos_en); ++i)
		pmic_trace_ldo(acpm, pmic_ldos_en[i]);
	for (i = 0; i < ARRAY_SIZE(pmic_ldos_en_out); ++i)
		pmic_trace_ldo(acpm, pmic_ldos_en_out[i].num);
}
#endif

/**
 * pmic_init() - Enable power regulators in S2MPU12 PMIC.
 * @acpm: Data for I3C communication with PMIC over ACPM protocol
 *
 * Enable LDOs needed for devices used in the bootloader and kernel.
 *
 * Return: 0 on success or non-zero code on error.
 */
int pmic_init(struct acpm *acpm)
{
	size_t i;
	int err;

	for (i = 0; i < ARRAY_SIZE(pmic_ldos_en); ++i) {
		err = pmic_ldo_set_en(acpm, pmic_ldos_en[i]);
		if (err)
			return -EIO;
	}

	for (i = 0; i < ARRAY_SIZE(pmic_ldos_en_out); ++i) {
		err = pmic_ldo_set_en_out(acpm, &pmic_ldos_en_out[i]);
		if (err)
			return -EIO;
	}

#ifdef DEBUG
	pmic_trace_ldos(acpm);
#endif

	return 0;
}
