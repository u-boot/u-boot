/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <altera.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/dwmmc.h>

DECLARE_GLOBAL_DATA_PTR;

static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	return 0;
}

/*
 * DesignWare Ethernet initialization
 */
#ifdef CONFIG_DESIGNWARE_ETH
int cpu_eth_init(bd_t *bis)
{
#if CONFIG_EMAC_BASE == SOCFPGA_EMAC0_ADDRESS
	const int physhift = SYSMGR_EMACGRP_CTRL_PHYSEL0_LSB;
#elif CONFIG_EMAC_BASE == SOCFPGA_EMAC1_ADDRESS
	const int physhift = SYSMGR_EMACGRP_CTRL_PHYSEL1_LSB;
#else
#error "Incorrect CONFIG_EMAC_BASE value!"
#endif

	/* Initialize EMAC. This needs to be done at least once per boot. */

	/*
	 * Putting the EMAC controller to reset when configuring the PHY
	 * interface select at System Manager
	 */
	socfpga_emac_reset(1);

	/* Clearing emac0 PHY interface select to 0 */
	clrbits_le32(&sysmgr_regs->emacgrp_ctrl,
		     SYSMGR_EMACGRP_CTRL_PHYSEL_MASK << physhift);

	/* configure to PHY interface select choosed */
	setbits_le32(&sysmgr_regs->emacgrp_ctrl,
		     SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII << physhift);

	/* Release the EMAC controller from reset */
	socfpga_emac_reset(0);

	/* initialize and register the emac */
	return designware_initialize(CONFIG_EMAC_BASE,
				     CONFIG_PHY_INTERFACE_MODE);
}
#endif

#ifdef CONFIG_DWMMC
/*
 * Initializes MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
	return socfpga_dwmmc_init(SOCFPGA_SDMMC_ADDRESS,
				  CONFIG_HPS_SDMMC_BUSWIDTH, 0);
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
/*
 * Print CPU information
 */
int print_cpuinfo(void)
{
	puts("CPU:   Altera SoCFPGA Platform\n");
	return 0;
}
#endif

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
static void socfpga_fpga_add(void)
{
	int i;
	fpga_init();
	for (i = 0; i < ARRAY_SIZE(altera_fpga); i++)
		fpga_add(fpga_altera, &altera_fpga[i]);
}
#else
static inline void socfpga_fpga_add(void) {}
#endif

int arch_cpu_init(void)
{
	/*
	 * If the HW watchdog is NOT enabled, make sure it is not running,
	 * for example because it was enabled in the preloader. This might
	 * trigger a watchdog-triggered reboot of Linux kernel later.
	 */
#ifndef CONFIG_HW_WATCHDOG
	socfpga_watchdog_reset();
#endif
	return 0;
}

int misc_init_r(void)
{
	/* Add device descriptor to FPGA device table */
	socfpga_fpga_add();
	return 0;
}
