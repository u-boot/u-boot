// SPDX-License-Identifier: GPL-2.0
/*
 * https://beagleplay.org/
 *
 * Copyright (C) 2022-2023 Texas Instruments Incorporated - https://www.ti.com/
 * Copyright (C) 2022-2023 Robert Nelson, BeagleBoard.org Foundation
 */

#include <cpu_func.h>
#include <env.h>
#include <fdt_support.h>
#include <spl.h>

#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	char fdtfile[50];

	snprintf(fdtfile, sizeof(fdtfile), "%s/%s.dtb",
		 CONFIG_TI_FDT_FOLDER_PATH, CONFIG_DEFAULT_DEVICE_TREE);

	env_set("fdtfile", fdtfile);

	return 0;
}
#endif

#ifdef CONFIG_SPL_BOARD_INIT

/*
 * Enable the 32k Crystal: needed for accurate 32k clock
 * and external clock sources such as wlan 32k input clock
 * supplied from the SoC to the wlan chip.
 *
 * The trim setup can be very highly board type specific choice of the crystal
 * So this is done in the board file, though, in this case, no specific trim
 * is necessary.
 */
static void crystal_32k_enable(void)
{
	/* Only mess with 32k at the start of boot from R5 */
	if (IS_ENABLED(CONFIG_CPU_V7R)) {
		/*
		 * We have external 32k crystal, so lets enable it (0x0)
		 * and disable bypass (0x0)
		 */
		writel(0x0, MCU_CTRL_LFXOSC_CTRL);

		/* Add any crystal specific TRIM needed here.. */

		/* Make sure to mux the SoC 32k from the crystal */
		writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
		       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);
	}
}

static void debounce_configure(void)
{
	/* Configure debounce one time from R5 */
	if (IS_ENABLED(CONFIG_CPU_V7R)) {
		/*
		 * Setup debounce time registers.
		 * arbitrary values. Times are approx
		 */
		/* 1.9ms debounce @ 32k */
		writel(0x1, CTRLMMR_DBOUNCE_CFG(1));
		/* 5ms debounce @ 32k */
		writel(0x5, CTRLMMR_DBOUNCE_CFG(2));
		/* 20ms debounce @ 32k */
		writel(0x14, CTRLMMR_DBOUNCE_CFG(3));
		/* 46ms debounce @ 32k */
		writel(0x18, CTRLMMR_DBOUNCE_CFG(4));
		/* 100ms debounce @ 32k */
		writel(0x1c, CTRLMMR_DBOUNCE_CFG(5));
		/* 156ms debounce @ 32k */
		writel(0x1f, CTRLMMR_DBOUNCE_CFG(6));
	}
}

void spl_board_init(void)
{
	crystal_32k_enable();
	debounce_configure();
}
#endif
