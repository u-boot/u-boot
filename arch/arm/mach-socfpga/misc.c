/*
 *  Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <altera.h>
#include <miiphy.h>
#include <netdev.h>
#include <watchdog.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/nic301.h>
#include <asm/arch/scu.h>
#include <asm/pl310.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;

struct bsel bsel_str[] = {
	{ "rsvd", "Reserved", },
	{ "fpga", "FPGA (HPS2FPGA Bridge)", },
	{ "nand", "NAND Flash (1.8V)", },
	{ "nand", "NAND Flash (3.0V)", },
	{ "sd", "SD/MMC External Transceiver (1.8V)", },
	{ "sd", "SD/MMC Internal Transceiver (3.0V)", },
	{ "qspi", "QSPI Flash (1.8V)", },
	{ "qspi", "QSPI Flash (3.0V)", },
};

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	return 0;
}

void enable_caches(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
	icache_enable();
#endif
#ifndef CONFIG_SYS_DCACHE_OFF
	dcache_enable();
#endif
}

void v7_outer_cache_enable(void)
{
	/* Disable the L2 cache */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);

	/* enable BRESP, instruction and data prefetch, full line of zeroes */
	setbits_le32(&pl310->pl310_aux_ctrl,
		     L310_AUX_CTRL_DATA_PREFETCH_MASK |
		     L310_AUX_CTRL_INST_PREFETCH_MASK |
		     L310_SHARED_ATT_OVERRIDE_ENABLE);

	/* Enable the L2 cache */
	setbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}

void v7_outer_cache_disable(void)
{
	/* Disable the L2 cache */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}

#if defined(CONFIG_SYS_CONSOLE_IS_IN_ENV) && \
defined(CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE)
int overwrite_console(void)
{
	return 0;
}
#endif

#ifdef CONFIG_FPGA
/*
 * FPGA programming support for SoC FPGA Cyclone V
 */
static Altera_desc altera_fpga[] = {
	{
		/* Family */
		Altera_SoCFPGA,
		/* Interface type */
		fast_passive_parallel,
		/* No limitation as additional data will be ignored */
		-1,
		/* No device function table */
		NULL,
		/* Base interface address specified in driver */
		NULL,
		/* No cookie implementation */
		0
	},
};

/* add device descriptor to FPGA device table */
void socfpga_fpga_add(void)
{
	int i;
	fpga_init();
	for (i = 0; i < ARRAY_SIZE(altera_fpga); i++)
		fpga_add(fpga_altera, &altera_fpga[i]);
}
#endif

int arch_cpu_init(void)
{
#ifdef CONFIG_HW_WATCHDOG
	/*
	 * In case the watchdog is enabled, make sure to (re-)configure it
	 * so that the defined timeout is valid. Otherwise the SPL (Perloader)
	 * timeout value is still active which might too short for Linux
	 * booting.
	 */
	hw_watchdog_init();
#else
	/*
	 * If the HW watchdog is NOT enabled, make sure it is not running,
	 * for example because it was enabled in the preloader. This might
	 * trigger a watchdog-triggered reboot of Linux kernel later.
	 * Toggle watchdog reset, so watchdog in not running state.
	 */
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 1);
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 0);
#endif

	return 0;
}
