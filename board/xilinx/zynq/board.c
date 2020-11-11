// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 * (C) Copyright 2013 - 2018 Xilinx, Inc.
 */

#include <common.h>
#include <init.h>
#include <log.h>
#include <dm/uclass.h>
#include <env.h>
#include <fdtdec.h>
#include <fpga.h>
#include <malloc.h>
#include <mmc.h>
#include <watchdog.h>
#include <wdt.h>
#include <zynqpl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include "../common/board.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	int env_targets_len = 0;
	const char *mode;
	char *new_targets;
	char *env_targets;

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	if (!CONFIG_IS_ENABLED(ENV_VARS_UBOOT_RUNTIME_CONFIG))
		return 0;

	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
	case ZYNQ_BM_QSPI:
		mode = "qspi";
		env_set("modeboot", "qspiboot");
		break;
	case ZYNQ_BM_NAND:
		mode = "nand";
		env_set("modeboot", "nandboot");
		break;
	case ZYNQ_BM_NOR:
		mode = "nor";
		env_set("modeboot", "norboot");
		break;
	case ZYNQ_BM_SD:
		mode = "mmc0";
		env_set("modeboot", "sdboot");
		break;
	case ZYNQ_BM_JTAG:
		mode = "jtag pxe dhcp";
		env_set("modeboot", "jtagboot");
		break;
	default:
		mode = "";
		env_set("modeboot", "");
		break;
	}

	/*
	 * One terminating char + one byte for space between mode
	 * and default boot_targets
	 */
	env_targets = env_get("boot_targets");
	if (env_targets)
		env_targets_len = strlen(env_targets);

	new_targets = calloc(1, strlen(mode) + env_targets_len + 2);
	if (!new_targets)
		return -ENOMEM;

	sprintf(new_targets, "%s %s", mode,
		env_targets ? env_targets : "");

	env_set("boot_targets", new_targets);

	return board_late_init_xilinx();
}

#if !defined(CONFIG_SYS_SDRAM_BASE) && !defined(CONFIG_SYS_SDRAM_SIZE)
int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	zynq_ddrc_init();

	return 0;
}
#else
int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	zynq_ddrc_init();

	return 0;
}
#endif
