// SPDX-License-Identifier: GPL-2.0+
/*
 * AXP PMIC SPL driver
 * (C) Copyright 2024 Arm Ltd.
 */

#include <errno.h>
#include <linux/types.h>
#include <asm/arch/pmic_bus.h>
#include <axp_pmic.h>

struct axp_reg_desc_spl {
	u8	enable_reg;
	u8	enable_mask;
	u8	volt_reg;
	u8	volt_mask;
	u16	min_mV;
	u16	max_mV;
	u8	step_mV;
	u8	split;
};

#define NA 0xff

#if defined(CONFIG_AXP717_POWER)				/* AXP717 */

static const struct axp_reg_desc_spl axp_spl_dcdc_regulators[] = {
	{ 0x80, BIT(0), 0x83, 0x7f,  500, 1540,  10, 70 },
	{ 0x80, BIT(1), 0x84, 0x7f,  500, 1540,  10, 70 },
	{ 0x80, BIT(2), 0x85, 0x7f,  500, 1840,  10, 70 },
};

#define AXP_CHIP_VERSION	0x0
#define AXP_CHIP_VERSION_MASK	0x0
#define AXP_CHIP_ID		0x0
#define AXP_SHUTDOWN_REG	0x27
#define AXP_SHUTDOWN_MASK	BIT(0)

#elif defined(CONFIG_AXP313_POWER)				/* AXP313 */

static const struct axp_reg_desc_spl axp_spl_dcdc_regulators[] = {
	{ 0x10, BIT(0), 0x13, 0x7f,  500, 1540,  10, 70 },
	{ 0x10, BIT(1), 0x14, 0x7f,  500, 1540,  10, 70 },
	{ 0x10, BIT(2), 0x15, 0x7f,  500, 1840,  10, 70 },
};

#define AXP_CHIP_VERSION	0x3
#define AXP_CHIP_VERSION_MASK	0xc8
#define AXP_CHIP_ID		0x48
#define AXP_SHUTDOWN_REG	0x1a
#define AXP_SHUTDOWN_MASK	BIT(7)

#elif defined(CONFIG_AXP305_POWER)				/* AXP305 */

static const struct axp_reg_desc_spl axp_spl_dcdc_regulators[] = {
	{ 0x10, BIT(0), 0x12, 0x7f,  600, 1520,  10, 50 },
	{ 0x10, BIT(1), 0x13, 0x1f, 1000, 2550,  50, NA },
	{ 0x10, BIT(2), 0x14, 0x7f,  600, 1520,  10, 50 },
	{ 0x10, BIT(3), 0x15, 0x3f,  600, 1500,  20, NA },
	{ 0x10, BIT(4), 0x16, 0x1f, 1100, 3400, 100, NA },
};

#define AXP_CHIP_VERSION	0x3
#define AXP_CHIP_VERSION_MASK	0xcf
#define AXP_CHIP_ID		0x40
#define AXP_SHUTDOWN_REG	0x32
#define AXP_SHUTDOWN_MASK	BIT(7)

#else

	#error "Please define the regulator registers in axp_spl_regulators[]."

#endif

static u8 axp_mvolt_to_cfg(int mvolt, const struct axp_reg_desc_spl *reg)
{
	if (mvolt < reg->min_mV)
		mvolt = reg->min_mV;
	else if (mvolt > reg->max_mV)
		mvolt = reg->max_mV;

	mvolt -= reg->min_mV;

	/* voltage in the first range ? */
	if (mvolt <= reg->split * reg->step_mV)
		return mvolt / reg->step_mV;

	mvolt -= reg->split * reg->step_mV;

	return reg->split + mvolt / (reg->step_mV * 2);
}

static int axp_set_dcdc(int dcdc_num, unsigned int mvolt)
{
	const struct axp_reg_desc_spl *reg;
	int ret;

	if (dcdc_num < 1 || dcdc_num > ARRAY_SIZE(axp_spl_dcdc_regulators))
		return -EINVAL;

	reg = &axp_spl_dcdc_regulators[dcdc_num - 1];

	if (mvolt == 0)
		return pmic_bus_clrbits(reg->enable_reg, reg->enable_mask);

	ret = pmic_bus_write(reg->volt_reg, axp_mvolt_to_cfg(mvolt, reg));
	if (ret)
		return ret;

	return pmic_bus_setbits(reg->enable_reg, reg->enable_mask);
}

int axp_set_dcdc1(unsigned int mvolt)
{
	return axp_set_dcdc(1, mvolt);
}

int axp_set_dcdc2(unsigned int mvolt)
{
	return axp_set_dcdc(2, mvolt);
}

int axp_set_dcdc3(unsigned int mvolt)
{
	return axp_set_dcdc(3, mvolt);
}

int axp_set_dcdc4(unsigned int mvolt)
{
	return axp_set_dcdc(4, mvolt);
}

int axp_set_dcdc5(unsigned int mvolt)
{
	return axp_set_dcdc(5, mvolt);
}

int axp_init(void)
{
	int ret = pmic_bus_init();

	if (ret)
		return ret;

	if (AXP_CHIP_VERSION_MASK) {
		u8 axp_chip_id;

		ret = pmic_bus_read(AXP_CHIP_VERSION, &axp_chip_id);
		if (ret)
			return ret;

		if ((axp_chip_id & AXP_CHIP_VERSION_MASK) != AXP_CHIP_ID) {
			debug("unknown PMIC: 0x%x\n", axp_chip_id);
			return -EINVAL;
		}
	}

	return 0;
}

#if !CONFIG_IS_ENABLED(ARM_PSCI_FW) && !IS_ENABLED(CONFIG_SYSRESET_CMD_POWEROFF)
int do_poweroff(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	pmic_bus_setbits(AXP_SHUTDOWN_REG, AXP_SHUTDOWN_MASK);

	/* infinite loop during shutdown */
	while (1)
		;

	/* not reached */
	return 0;
}
#endif
