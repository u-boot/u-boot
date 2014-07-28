/*
 * Keystone EVM : Board initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mon.h>
#include <asm/arch/psc_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/hardware.h>

/**
 * cpu_to_bus - swap bytes of the 32-bit data if the device is BE
 * @ptr - array of data
 * @length - lenght of data array
 */
int cpu_to_bus(u32 *ptr, u32 length)
{
	u32 i;

	if (!(readl(KS2_DEVSTAT) & 0x1))
		for (i = 0; i < length; i++, ptr++)
			*ptr = cpu_to_be32(*ptr);

	return 0;
}

static int turn_off_myself(void)
{
	printf("Turning off ourselves\r\n");
	mon_power_off(0);

	psc_disable_module(KS2_LPSC_TETRIS);
	psc_disable_domain(KS2_TETRIS_PWR_DOMAIN);

	asm volatile ("isb\n"
		      "dsb\n"
		      "wfi\n");

	printf("What! Should not see that\n");
	return 0;
}

static void turn_off_all_dsps(int num_dsps)
{
	int i;

	for (i = 0; i < num_dsps; i++) {
		if (psc_disable_module(i + KS2_LPSC_GEM_0))
			printf("Cannot disable module for #%d DSP", i);

		if (psc_disable_domain(i + 8))
			printf("Cannot disable domain for #%d DSP", i);
	}
}

int do_killme_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return turn_off_myself();
}

U_BOOT_CMD(
	killme, 1,      0,      do_killme_cmd,
	"turn off main ARM core",
	"turn off main ARM core. Should not live after that :(\n"
);

int misc_init_r(void)
{
	char *env;
	long ks2_debug = 0;

	env = getenv("ks2_debug");

	if (env)
		ks2_debug = simple_strtol(env, NULL, 0);

	if ((ks2_debug & DBG_LEAVE_DSPS_ON) == 0)
		turn_off_all_dsps(KS2_NUM_DSPS);

	return 0;
}
