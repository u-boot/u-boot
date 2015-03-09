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

	ret = rsb_init();
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

static int axp221_clrbits(u8 reg, u8 bits)
{
	int ret;
	u8 val;

	ret = pmic_bus_read(reg, &val);
	if (ret)
		return ret;

	val &= ~bits;
	return pmic_bus_write(reg, val);
}

int axp221_set_dcdc1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 1600, 3400, 100);

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL1,
				      AXP221_OUTPUT_CTRL1_DCDC1_EN);

	ret = pmic_bus_write(AXP221_DCDC1_CTRL, cfg);
	if (ret)
		return ret;

	ret = axp221_setbits(AXP221_OUTPUT_CTRL2,
			     AXP221_OUTPUT_CTRL2_DCDC1SW_EN);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL1,
			      AXP221_OUTPUT_CTRL1_DCDC1_EN);
}

int axp221_set_dcdc2(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 600, 1540, 20);

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL1,
				      AXP221_OUTPUT_CTRL1_DCDC2_EN);

	ret = pmic_bus_write(AXP221_DCDC2_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL1,
			      AXP221_OUTPUT_CTRL1_DCDC2_EN);
}

int axp221_set_dcdc3(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 600, 1860, 20);

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL1,
				      AXP221_OUTPUT_CTRL1_DCDC3_EN);

	ret = pmic_bus_write(AXP221_DCDC3_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL1,
			      AXP221_OUTPUT_CTRL1_DCDC3_EN);
}

int axp221_set_dcdc4(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 600, 1540, 20);

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL1,
				      AXP221_OUTPUT_CTRL1_DCDC4_EN);

	ret = pmic_bus_write(AXP221_DCDC4_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL1,
			      AXP221_OUTPUT_CTRL1_DCDC4_EN);
}

int axp221_set_dcdc5(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 1000, 2550, 50);

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL1,
				      AXP221_OUTPUT_CTRL1_DCDC5_EN);

	ret = pmic_bus_write(AXP221_DCDC5_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL1,
			      AXP221_OUTPUT_CTRL1_DCDC5_EN);
}

int axp221_set_dldo1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL2,
				      AXP221_OUTPUT_CTRL2_DLDO1_EN);

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

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL2,
				      AXP221_OUTPUT_CTRL2_DLDO2_EN);

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

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL2,
				      AXP221_OUTPUT_CTRL2_DLDO3_EN);

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

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL2,
				      AXP221_OUTPUT_CTRL2_DLDO4_EN);

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

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL1,
				      AXP221_OUTPUT_CTRL1_ALDO1_EN);

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

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL1,
				      AXP221_OUTPUT_CTRL1_ALDO2_EN);

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

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL3,
				      AXP221_OUTPUT_CTRL3_ALDO3_EN);

	ret = pmic_bus_write(AXP221_ALDO3_CTRL, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL3,
			      AXP221_OUTPUT_CTRL3_ALDO3_EN);
}

int axp221_set_eldo(int eldo_num, unsigned int mvolt)
{
	int ret;
	u8 cfg = axp221_mvolt_to_cfg(mvolt, 700, 3300, 100);
	u8 addr, bits;

	switch (eldo_num) {
	case 3:
		addr = AXP221_ELDO3_CTRL;
		bits = AXP221_OUTPUT_CTRL2_ELDO3_EN;
		break;
	case 2:
		addr = AXP221_ELDO2_CTRL;
		bits = AXP221_OUTPUT_CTRL2_ELDO2_EN;
		break;
	case 1:
		addr = AXP221_ELDO1_CTRL;
		bits = AXP221_OUTPUT_CTRL2_ELDO1_EN;
		break;
	default:
		return -EINVAL;
	}

	if (mvolt == 0)
		return axp221_clrbits(AXP221_OUTPUT_CTRL2, bits);

	ret = pmic_bus_write(addr, cfg);
	if (ret)
		return ret;

	return axp221_setbits(AXP221_OUTPUT_CTRL2, bits);
}

int axp221_init(void)
{
	/* This cannot be 0 because it is used in SPL before BSS is ready */
	static int needs_init = 1;
	u8 axp_chip_id;
	int ret;

	if (!needs_init)
		return 0;

	ret = pmic_bus_init();
	if (ret)
		return ret;

	ret = pmic_bus_read(AXP221_CHIP_ID, &axp_chip_id);
	if (ret)
		return ret;

	if (!(axp_chip_id == 0x6 || axp_chip_id == 0x7 || axp_chip_id == 0x17))
		return -ENODEV;

	needs_init = 0;
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

int axp_get_vbus(void)
{
	int ret;
	u8 val;

	ret = axp221_init();
	if (ret)
		return ret;

	ret = pmic_bus_read(AXP221_POWER_STATUS, &val);
	if (ret)
		return ret;

	return (val & AXP221_POWER_STATUS_VBUS_USABLE) ? 1 : 0;
}

static int axp_drivebus_setup(void)
{
	int ret;

	ret = axp221_init();
	if (ret)
		return ret;

	/* Set N_VBUSEN pin to output / DRIVEBUS function */
	return axp221_clrbits(AXP221_MISC_CTRL, AXP221_MISC_CTRL_N_VBUSEN_FUNC);
}

int axp_drivebus_enable(void)
{
	int ret;

	ret = axp_drivebus_setup();
	if (ret)
		return ret;

	/* Set DRIVEBUS high */
	return axp221_setbits(AXP221_VBUS_IPSOUT, AXP221_VBUS_IPSOUT_DRIVEBUS);
}

int axp_drivebus_disable(void)
{
	int ret;

	ret = axp_drivebus_setup();
	if (ret)
		return ret;

	/* Set DRIVEBUS low */
	return axp221_clrbits(AXP221_VBUS_IPSOUT, AXP221_VBUS_IPSOUT_DRIVEBUS);
}
