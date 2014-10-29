/*
 *  Copyright (C) 2012 Samsung Electronics
 *  Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

static const char max77686_buck_addr[] = {
	0xff, 0x10, 0x12, 0x1c, 0x26, 0x30, 0x32, 0x34, 0x36, 0x38
};

static unsigned int max77686_ldo_volt2hex(int ldo, ulong uV)
{
	unsigned int hex = 0;

	switch (ldo) {
	case 1:
	case 2:
	case 6:
	case 7:
	case 8:
	case 15:
		hex = (uV - 800000) / 25000;
		break;
	default:
		hex = (uV - 800000) / 50000;
	}

	if (hex >= 0 && hex <= MAX77686_LDO_VOLT_MAX_HEX)
		return hex;

	debug("%s: %ld is wrong voltage value for LDO%d\n", __func__, uV, ldo);
	return 0;
}

static int max77686_buck_volt2hex(int buck, ulong uV)
{
	int hex = 0;

	if (buck < 5 || buck > 9) {
		debug("%s: buck %d is not supported\n", __func__, buck);
		return -EINVAL;
	}

	hex = (uV - 750000) / 50000;

	if (hex >= 0 && hex <= MAX77686_BUCK_VOLT_MAX_HEX)
		return hex;

	debug("%s: %ld is wrong voltage value for BUCK%d\n",
	      __func__, uV, buck);
	return -EINVAL;
}

int max77686_set_ldo_voltage(struct pmic *p, int ldo, ulong uV)
{
	unsigned int val, ret, hex, adr;

	if (ldo < 1 || ldo > 26) {
		printf("%s: %d is wrong ldo number\n", __func__, ldo);
		return -1;
	}

	adr = MAX77686_REG_PMIC_LDO1CTRL1 + ldo - 1;
	hex = max77686_ldo_volt2hex(ldo, uV);

	if (!hex)
		return -1;

	ret = pmic_reg_read(p, adr, &val);
	if (ret)
		return ret;

	val &= ~MAX77686_LDO_VOLT_MASK;
	val |= hex;
	ret |= pmic_reg_write(p, adr, val);

	return ret;
}

int max77686_set_buck_voltage(struct pmic *p, int buck, ulong uV)
{
	unsigned int val, adr;
	int hex, ret;

	if (buck < 5 || buck > 9) {
		printf("%s: %d is an unsupported bucket number\n",
		       __func__, buck);
		return -EINVAL;
	}

	adr = max77686_buck_addr[buck] + 1;
	hex = max77686_buck_volt2hex(buck, uV);

	if (hex < 0)
		return hex;

	ret = pmic_reg_read(p, adr, &val);
	if (ret)
		return ret;

	val &= ~MAX77686_BUCK_VOLT_MASK;
	ret |= pmic_reg_write(p, adr, val | hex);

	return ret;
}

int max77686_set_ldo_mode(struct pmic *p, int ldo, char opmode)
{
	unsigned int val, ret, adr, mode;

	if (ldo < 1 || 26 < ldo) {
		printf("%s: %d is wrong ldo number\n", __func__, ldo);
		return -1;
	}

	adr = MAX77686_REG_PMIC_LDO1CTRL1 + ldo - 1;

	/* mode */
	switch (opmode) {
	case OPMODE_OFF:
		mode = MAX77686_LDO_MODE_OFF;
		break;
	case OPMODE_STANDBY:
		switch (ldo) {
		case 2:
		case 6:
		case 7:
		case 8:
		case 10:
		case 11:
		case 12:
		case 14:
		case 15:
		case 16:
			mode = MAX77686_LDO_MODE_STANDBY;
			break;
		default:
			mode = 0xff;
		}
		break;
	case OPMODE_LPM:
		mode = MAX77686_LDO_MODE_LPM;
		break;
	case OPMODE_ON:
		mode = MAX77686_LDO_MODE_ON;
		break;
	default:
		mode = 0xff;
	}

	if (mode == 0xff) {
		printf("%s: %d is not supported on LDO%d\n",
		       __func__, opmode, ldo);
		return -1;
	}

	ret = pmic_reg_read(p, adr, &val);
	if (ret)
		return ret;

	val &= ~MAX77686_LDO_MODE_MASK;
	val |= mode;
	ret |= pmic_reg_write(p, adr, val);

	return ret;
}

int max77686_set_buck_mode(struct pmic *p, int buck, char opmode)
{
	unsigned int val, ret, mask, adr, size, mode, mode_shift;

	size = ARRAY_SIZE(max77686_buck_addr);
	if (buck >= size) {
		printf("%s: %d is wrong buck number\n", __func__, buck);
		return -1;
	}

	adr = max77686_buck_addr[buck];

	/* mask */
	switch (buck) {
	case 2:
	case 3:
	case 4:
		mode_shift = MAX77686_BUCK_MODE_SHIFT_2;
		break;
	default:
		mode_shift = MAX77686_BUCK_MODE_SHIFT_1;
	}

	mask = MAX77686_BUCK_MODE_MASK << mode_shift;

	/* mode */
	switch (opmode) {
	case OPMODE_OFF:
		mode = MAX77686_BUCK_MODE_OFF << mode_shift;
		break;
	case OPMODE_STANDBY:
		switch (buck) {
		case 1:
		case 2:
		case 3:
		case 4:
			mode = MAX77686_BUCK_MODE_STANDBY << mode_shift;
			break;
		default:
			mode = 0xff;
		}
		break;
	case OPMODE_LPM:
		switch (buck) {
		case 2:
		case 3:
		case 4:
			mode = MAX77686_BUCK_MODE_LPM << mode_shift;
			break;
		default:
			mode = 0xff;
		}
		break;
	case OPMODE_ON:
		mode = MAX77686_BUCK_MODE_ON << mode_shift;
		break;
	default:
		mode = 0xff;
	}

	if (mode == 0xff) {
		printf("%s: %d is not supported on BUCK%d\n",
		       __func__, opmode, buck);
		return -1;
	}

	ret = pmic_reg_read(p, adr, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= mode;
	ret |= pmic_reg_write(p, adr, val);

	return ret;
}

int pmic_init(unsigned char bus)
{
	static const char name[] = "MAX77686_PMIC";
	struct pmic *p = pmic_alloc();
#ifdef CONFIG_OF_CONTROL
	const void *blob = gd->fdt_blob;
	int node, parent, tmp;
#endif

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

#ifdef CONFIG_OF_CONTROL
	node = fdtdec_next_compatible(blob, 0, COMPAT_MAXIM_MAX77686_PMIC);
	if (node < 0) {
		debug("PMIC: No node for PMIC Chip in device tree\n");
		debug("node = %d\n", node);
		return -1;
	}

	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}

	/* tmp since p->bus is unsigned */
	tmp = i2c_get_bus_num_fdt(parent);
	if (tmp < 0) {
		debug("%s: Cannot find I2C bus\n", __func__);
		return -1;
	}
	p->bus = tmp;
	p->hw.i2c.addr = fdtdec_get_int(blob, node, "reg", 9);
#else
	p->bus = bus;
	p->hw.i2c.addr = MAX77686_I2C_ADDR;
#endif

	p->name = name;
	p->interface = PMIC_I2C;
	p->number_of_regs = PMIC_NUM_OF_REGS;
	p->hw.i2c.tx_num = 1;

	puts("Board PMIC init\n");

	return 0;
}
