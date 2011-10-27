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
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>

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
/**
 * Print CPU information
 */
int print_cpuinfo(void)
{
	char *cpu_s, *sec_s;
	int arm_freq, ddr_freq;

	switch (get_cpu_type()) {
	case AM335X:
		cpu_s = "AM335X";
		break;
	default:
		cpu_s = "Unknown cpu type";
		break;
	}

	switch (get_device_type()) {
	case TST_DEVICE:
		sec_s = "TST";
		break;
	case EMU_DEVICE:
		sec_s = "EMU";
		break;
	case HS_DEVICE:
		sec_s = "HS";
		break;
	case GP_DEVICE:
		sec_s = "GP";
		break;
	default:
		sec_s = "?";
	}

	printf("AM%s-%s rev %d\n",
			cpu_s, sec_s, get_cpu_rev());

	/* TODO: Print ARM and DDR frequencies  */

	return 0;
}
#endif	/* CONFIG_DISPLAY_CPUINFO */
