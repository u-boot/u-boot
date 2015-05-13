/*
 *  Copyright (C) 2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SANDBOX_PMIC_H_
#define  _SANDBOX_PMIC_H_

#define SANDBOX_LDO_DRIVER		"sandbox_ldo"
#define SANDBOX_OF_LDO_PREFIX		"ldo"
#define SANDBOX_BUCK_DRIVER		"sandbox_buck"
#define SANDBOX_OF_BUCK_PREFIX		"buck"

#define SANDBOX_BUCK_COUNT	2
#define SANDBOX_LDO_COUNT	2
/*
 * Sandbox PMIC registers:
 * We have only 12 significant registers, but we alloc 16 for padding.
 */
enum {
	SANDBOX_PMIC_REG_BUCK1_UV = 0,
	SANDBOX_PMIC_REG_BUCK1_UA,
	SANDBOX_PMIC_REG_BUCK1_OM,

	SANDBOX_PMIC_REG_BUCK2_UV,
	SANDBOX_PMIC_REG_BUCK2_UA,
	SANDBOX_PMIC_REG_BUCK2_OM,

	SANDBOX_PMIC_REG_LDO_OFFSET,
	SANDBOX_PMIC_REG_LDO1_UV = SANDBOX_PMIC_REG_LDO_OFFSET,
	SANDBOX_PMIC_REG_LDO1_UA,
	SANDBOX_PMIC_REG_LDO1_OM,

	SANDBOX_PMIC_REG_LDO2_UV,
	SANDBOX_PMIC_REG_LDO2_UA,
	SANDBOX_PMIC_REG_LDO2_OM,

	SANDBOX_PMIC_REG_COUNT = 16,
};

/* Register offset for output: micro Volts, micro Amps, Operation Mode */
enum {
	OUT_REG_UV = 0,
	OUT_REG_UA,
	OUT_REG_OM,
	OUT_REG_COUNT,
};

/* Buck operation modes */
enum {
	BUCK_OM_OFF = 0,
	BUCK_OM_ON,
	BUCK_OM_PWM,
	BUCK_OM_COUNT,
};

/* Ldo operation modes */
enum {
	LDO_OM_OFF = 0,
	LDO_OM_ON,
	LDO_OM_SLEEP,
	LDO_OM_STANDBY,
	LDO_OM_COUNT,
};

/* BUCK1 Voltage: min: 0.8V, step: 25mV, max 2.4V */
#define OUT_BUCK1_UV_MIN	800000
#define OUT_BUCK1_UV_MAX	2400000
#define OUT_BUCK1_UV_STEP	25000

/* BUCK1 Amperage: min: 150mA, step: 25mA, max: 250mA */
#define OUT_BUCK1_UA_MIN	150000
#define OUT_BUCK1_UA_MAX	250000
#define OUT_BUCK1_UA_STEP	25000

/* BUCK2 Voltage: min: 0.75V, step: 50mV, max 3.95V */
#define OUT_BUCK2_UV_MIN	750000
#define OUT_BUCK2_UV_MAX	3950000
#define OUT_BUCK2_UV_STEP	50000

/* LDO1 Voltage: min: 0.8V, step: 25mV, max 2.4V */
#define OUT_LDO1_UV_MIN		800000
#define OUT_LDO1_UV_MAX		2400000
#define OUT_LDO1_UV_STEP	25000

/* LDO1 Amperage: min: 100mA, step: 50mA, max: 200mA */
#define OUT_LDO1_UA_MIN		100000
#define OUT_LDO1_UA_MAX		200000
#define OUT_LDO1_UA_STEP	50000

/* LDO2 Voltage: min: 0.75V, step: 50mV, max 3.95V */
#define OUT_LDO2_UV_MIN		750000
#define OUT_LDO2_UV_MAX		3950000
#define OUT_LDO2_UV_STEP	50000

/* register <-> value conversion */
#define REG2VAL(min, step, reg)		((min) + ((step) * (reg)))
#define VAL2REG(min, step, val)		(((val) - (min)) / (step))

/* Operation mode id -> register value conversion */
#define OM2REG(x)			(x)

#endif
