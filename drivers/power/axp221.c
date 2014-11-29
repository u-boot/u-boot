/*
 * AXP221 and AXP223 driver
 *
 * IMPORTANT when making changes to this file check that the registers
 * used are the same for the axp221 and axp223.
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/p2wi.h>
#include <asm/arch/rsb.h>
#include <axp221.h>

/*
 * The axp221 uses the p2wi bus, the axp223 is identical (for all registers
 * used sofar) but uses the rsb bus. These functions abstract this.
 */
static int pmic_bus_init(void)
{
#ifdef CONFIG_MACH_SUN6I
	p2wi_init();
	return p2wi_change_to_p2wi_mode(AXP221_CHIP_ADDR, AXP221_CTRL_ADDR,
					AXP221_INIT_DATA);
#else
	int ret;

	rsb_init();

	ret = rsb_set_device_mode(AXP223_DEVICE_MODE_DATA);
	if (ret)
		return ret;

	return rsb_set_device_address(AXP223_DEVICE_ADDR, AXP223_RUNTIME_ADDR);
#endif
}

static int pmic_bus_read(const u8 addr, u8 *data)
{
#ifdef CONFIG_MACH_SUN6I
	return p2wi_read(addr, data);
#else
	return rsb_read(AXP223_RUNTIME_ADDR, addr, data);
#endif
}

static int pmic_bus_write(const u8 addr, u8 data)
{
#ifdef CONFIG_MACH_SUN6I
	return p2wi_write(addr, data);
#else
	return rsb_write(AXP223_RUNTIME_ADDR, addr, data);
#endif
}

static u8 axp221_mvolt_to_cfg(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

static int axp221_setbits(u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(reg, &val);
	if (ret)
		return ret;

	val |= bits;
	return pmic_bus_write(reg, val);
}

int axp221_set_dcdc1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 1600, 3400, 100);

	ret = pmic_bus_write(AXP221_DCDC1_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL2,
			      AXP221_OUTPUT_CTRL2_DCDC1_EN);
}

int axp221_set_dcdc2(unsigned int mvolt)
{
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 600, 1540, 20);

	return pmic_bus_write(AXP221_DCDC2_CTRL, cfg);
}

int axp221_set_dcdc3(unsigned int mvolt)
{
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 600, 1860, 20);

	return pmic_bus_write(AXP221_DCDC3_CTRL, cfg);
}

int axp221_set_dcdc4(unsigned int mvolt)
{
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 600, 1540, 20);

	return pmic_bus_write(AXP221_DCDC4_CTRL, cfg);
}

int axp221_set_dcdc5(unsigned int mvolt)
{
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 1000, 2550, 50);

	return pmic_bus_write(AXP221_DCDC5_CTRL, cfg);
}

int axp221_set_dldo1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = pmic_bus_write(AXP221_DLDO1_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL2,
			      AXP221_OUTPUT_CTRL2_DLDO1_EN);
}

int axp221_set_dldo2(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = pmic_bus_write(AXP221_DLDO2_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL2,
			      AXP221_OUTPUT_CTRL2_DLDO2_EN);
}

int axp221_set_dldo3(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = pmic_bus_write(AXP221_DLDO3_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL2,
			      AXP221_OUTPUT_CTRL2_DLDO3_EN);
}

int axp221_set_dldo4(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = pmic_bus_write(AXP221_DLDO4_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL2,
			      AXP221_OUTPUT_CTRL2_DLDO4_EN);
}

int axp221_set_aldo1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = pmic_bus_write(AXP221_ALDO1_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL1,
			      AXP221_OUTPUT_CTRL1_ALDO1_EN);
}

int axp221_set_aldo2(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = pmic_bus_write(AXP221_ALDO2_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL1,
			      AXP221_OUTPUT_CTRL1_ALDO2_EN);
}

int axp221_set_aldo3(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	ret = pmic_bus_write(AXP221_ALDO3_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL3,
			      AXP221_OUTPUT_CTRL3_ALDO3_EN);
}

int axp221_init(void)
{
	u8 axp_chip_id;
	int ret;

	ret = pmic_bus_init();
	if (ret)
		return ret;

	ret = pmic_bus_read(AXP221_CHIP_ID, &axp_chip_id);
	if (ret)
		return ret;

	if (!(axp_chip_id == 0x6 || axp_chip_id == 0x7 || axp_chip_id == 0x17))
		return -ENODEV;

	return 0;
}

int axp221_get_sid(unsigned int *sid)
{
	u8 *dest = (u8 *)sid;
	int i, ret;

	ret = axp221_init();
	if (ret)
		return ret;

	ret = pmic_bus_write(AXP221_PAGE, 1);
	if (ret)
		return ret;

	for (i = 0; i < 16; i++) {
		ret = pmic_bus_read(AXP221_SID + i, &dest[i]);
		if (ret)
			return ret;
	}

	pmic_bus_write(AXP221_PAGE, 0);

	for (i = 0; i < 4; i++)
		sid[i] = be32_to_cpu(sid[i]);

	return 0;
}
