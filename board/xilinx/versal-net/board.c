// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 - 2022, Xilinx, Inc.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <fdtdec.h>
#include <init.h>
#include <log.h>
#include <malloc.h>
#include <time.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include "../common/board.h"

#include <linux/bitfield.h>
#include <debug_uart.h>
#include <generated/dt.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	printf("EL Level:\tEL%d\n", current_el());

	return 0;
}

static u32 platform_id, platform_version;

char *soc_name_decode(void)
{
	char *name, *platform_name;

	switch (platform_id) {
	case VERSAL_NET_SPP:
		platform_name = "ipp";
		break;
	case VERSAL_NET_EMU:
		platform_name = "emu";
		break;
	case VERSAL_NET_QEMU:
		platform_name = "qemu";
		break;
	default:
		return NULL;
	}

	/*
	 * --rev. are 6 chars
	 * max platform name is qemu which is 4 chars
	 * platform version number are 1+1
	 * Plus 1 char for \n
	 */
	name = calloc(1, strlen(CONFIG_SYS_BOARD) + 13);
	if (!name)
		return NULL;

	sprintf(name, "%s-%s-rev%d.%d", CONFIG_SYS_BOARD,
		platform_name, platform_version / 10,
		platform_version % 10);

	return name;
}

bool soc_detection(void)
{
	u32 version;

	version = readl(PMC_TAP_VERSION);
	platform_id = FIELD_GET(PLATFORM_MASK, version);

	debug("idcode %x, version %x, usercode %x\n",
	      readl(PMC_TAP_IDCODE), version,
	      readl(PMC_TAP_USERCODE));

	debug("pmc_ver %lx, ps version %lx, rtl version %lx\n",
	      FIELD_GET(PMC_VERSION_MASK, version),
	      FIELD_GET(PS_VERSION_MASK, version),
	      FIELD_GET(RTL_VERSION_MASK, version));

	platform_version = FIELD_GET(PLATFORM_VERSION_MASK, version);

	if (platform_id == VERSAL_NET_SPP ||
	    platform_id == VERSAL_NET_EMU) {
		/*
		 * 9 is diff for
		 * 0 means 0.9 version
		 * 1 means 1.0 version
		 * 2 means 1.1 version
		 * etc,
		 */
		platform_version += 9;
	}

	debug("Platform id: %d version: %d.%d\n", platform_id,
	      platform_version / 10, platform_version % 10);

	return true;
}

int board_early_init_f(void)
{
	if (IS_ENABLED(CONFIG_DEBUG_UART)) {
		/* Uart debug for sure */
		debug_uart_init();
		puts("Debug uart enabled\n"); /* or printch() */
	}

	return 0;
}

int board_early_init_r(void)
{
	return 0;
}

int board_late_init(void)
{
	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	if (!CONFIG_IS_ENABLED(ENV_VARS_UBOOT_RUNTIME_CONFIG))
		return 0;

	return board_late_init_xilinx();
}

int dram_init_banksize(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;

	mem_map_fill();

	return 0;
}

int dram_init(void)
{
	int ret;

	if (CONFIG_IS_ENABLED(SYS_MEM_RSVD_FOR_MMU))
		ret = fdtdec_setup_mem_size_base();
	else
		ret = fdtdec_setup_mem_size_base_lowest();

	if (ret)
		return -EINVAL;

	return 0;
}

void reset_cpu(void)
{
}
