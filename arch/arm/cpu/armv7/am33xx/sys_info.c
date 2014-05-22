/*
 * sys_info.c
 *
 * System information functions
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * Derived from Beagle Board and 3430 SDP code by
 *      Richard Woodruff <r-woodruff2@ti.com>
 *      Syed Mohammed Khasim <khasim@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <power/tps65910.h>

struct ctrl_stat *cstat = (struct ctrl_stat *)CTRL_BASE;

/**
 * get_cpu_rev(void) - extract rev info
 */
u32 get_cpu_rev(void)
{
	u32 id;
	u32 rev;

	id = readl(DEVICE_ID);
	rev = (id >> 28) & 0xff;

	return rev;
}

/**
 * get_cpu_type(void) - extract cpu info
 */
u32 get_cpu_type(void)
{
	u32 id = 0;
	u32 partnum;

	id = readl(DEVICE_ID);
	partnum = (id >> 12) & 0xffff;

	return partnum;
}

/**
 * get_board_rev() - setup to pass kernel board revision information
 * returns:(bit[0-3] sub version, higher bit[7-4] is higher version)
 */
u32 get_board_rev(void)
{
	return BOARD_REV_ID;
}

/**
 * get_device_type(): tell if GP/HS/EMU/TST
 */
u32 get_device_type(void)
{
	int mode;
	mode = readl(&cstat->statusreg) & (DEVICE_MASK);
	return mode >>= 8;
}

/**
 * get_sysboot_value(void) - return SYS_BOOT[4:0]
 */
u32 get_sysboot_value(void)
{
	int mode;
	mode = readl(&cstat->statusreg) & (SYSBOOT_MASK);
	return mode;
}

#ifdef CONFIG_DISPLAY_CPUINFO
static char *cpu_revs[] = {
		"1.0",
		"2.0",
		"2.1"};


static char *dev_types[] = {
		"TST",
		"EMU",
		"HS",
		"GP"};

/**
 * Print CPU information
 */
int print_cpuinfo(void)
{
	char *cpu_s, *sec_s, *rev_s;

	switch (get_cpu_type()) {
	case AM335X:
		cpu_s = "AM335X";
		break;
	case TI81XX:
		cpu_s = "TI81XX";
		break;
	default:
		cpu_s = "Unknown CPU type";
		break;
	}

	if (get_cpu_rev() < ARRAY_SIZE(cpu_revs))
		rev_s = cpu_revs[get_cpu_rev()];
	else
		rev_s = "?";

	if (get_device_type() < ARRAY_SIZE(dev_types))
		sec_s = dev_types[get_device_type()];
	else
		sec_s = "?";

	printf("%s-%s rev %s\n", cpu_s, sec_s, rev_s);

	return 0;
}
#endif	/* CONFIG_DISPLAY_CPUINFO */

#ifdef CONFIG_AM33XX
int am335x_get_efuse_mpu_max_freq(struct ctrl_dev *cdev)
{
	int sil_rev;

	sil_rev = readl(&cdev->deviceid) >> 28;

	if (sil_rev == 1)
		/* PG 2.0, efuse may not be set. */
		return MPUPLL_M_800;
	else if (sil_rev >= 2) {
		/* Check what the efuse says our max speed is. */
		int efuse_arm_mpu_max_freq;
		efuse_arm_mpu_max_freq = readl(&cdev->efuse_sma);
		switch ((efuse_arm_mpu_max_freq & DEVICE_ID_MASK)) {
		case AM335X_ZCZ_1000:
			return MPUPLL_M_1000;
		case AM335X_ZCZ_800:
			return MPUPLL_M_800;
		case AM335X_ZCZ_720:
			return MPUPLL_M_720;
		case AM335X_ZCZ_600:
		case AM335X_ZCE_600:
			return MPUPLL_M_600;
		case AM335X_ZCZ_300:
		case AM335X_ZCE_300:
			return MPUPLL_M_300;
		}
	}

	/* PG 1.0 or otherwise unknown, use the PG1.0 max */
	return MPUPLL_M_720;
}

int am335x_get_tps65910_mpu_vdd(int sil_rev, int frequency)
{
	/* For PG2.1 and later, we have one set of values. */
	if (sil_rev >= 2) {
		switch (frequency) {
		case MPUPLL_M_1000:
			return TPS65910_OP_REG_SEL_1_3_2_5;
		case MPUPLL_M_800:
			return TPS65910_OP_REG_SEL_1_2_6;
		case MPUPLL_M_720:
			return TPS65910_OP_REG_SEL_1_2_0;
		case MPUPLL_M_600:
		case MPUPLL_M_300:
			return TPS65910_OP_REG_SEL_1_1_3;
		}
	}

	/* Default to PG1.0/PG2.0 values. */
	return TPS65910_OP_REG_SEL_1_1_3;
}
#endif
