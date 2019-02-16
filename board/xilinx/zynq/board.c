// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 * (C) Copyright 2013 - 2018 Xilinx, Inc.
 */

#include <common.h>
#include <dm/uclass.h>
#include <fdtdec.h>
#include <fpga.h>
#include <malloc.h>
#include <mmc.h>
#include <watchdog.h>
#include <wdt.h>
#include <zynqpl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_WDT)
static struct udevice *watchdog_dev;
#endif

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
# if defined(CONFIG_WDT)
	/* bss is not cleared at time when watchdog_reset() is called */
	watchdog_dev = NULL;
# endif

	return 0;
}
#endif

int board_init(void)
{
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_WDT)
	if (uclass_get_device_by_seq(UCLASS_WDT, 0, &watchdog_dev)) {
		debug("Watchdog: Not found by seq!\n");
		if (uclass_get_device(UCLASS_WDT, 0, &watchdog_dev)) {
			puts("Watchdog: Not found!\n");
			return 0;
		}
	}

	wdt_start(watchdog_dev, 0, 0);
	puts("Watchdog: Started\n");
# endif

	return 0;
}

int board_late_init(void)
{
	int env_targets_len = 0;
	const char *mode;
	char *new_targets;
	char *env_targets;

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
		mode = "mmc";
		env_set("modeboot", "sdboot");
		break;
	case ZYNQ_BM_JTAG:
		mode = "pxe dhcp";
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

	return 0;
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

#if defined(CONFIG_WATCHDOG)
/* Called by macro WATCHDOG_RESET */
void watchdog_reset(void)
{
# if !defined(CONFIG_SPL_BUILD)
	static ulong next_reset;
	ulong now;

	if (!watchdog_dev)
		return;

	now = timer_get_us();

	/* Do not reset the watchdog too often */
	if (now > next_reset) {
		wdt_reset(watchdog_dev);
		next_reset = now + 1000;
	}
# endif
}
#endif
