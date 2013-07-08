/*
 * MATRIX VISION GmbH mvBlueLYNX-X
 *
 * Derived from Beagle and Overo
 *
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <netdev.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "mvblx.h"
#include "fpga.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_NET)
static void setup_net_chip(void);
#endif /* CONFIG_CMD_NET */

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init();	/* in SRAM or SDRAM, finish GPMC */
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	printf("mvBlueLYNX-X\n");
	if (get_cpu_family() == CPU_OMAP36XX)
		setenv("mpurate", "1000");
	else
		setenv("mpurate", "600");

	twl4030_power_init();

#if defined(CONFIG_CMD_NET)
	setup_net_chip();
#endif /* CONFIG_CMD_NET */

	mvblx_init_fpga();

	mac_read_from_eeprom();

	dieid_num_r();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_MVBLX();
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);
	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}
#endif

#if defined(CONFIG_CMD_NET)
/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	/* Configure GPMC registers */
	writel(NET_GPMC_CONFIG1, &gpmc_cfg->cs[0].config1);
	writel(NET_GPMC_CONFIG2, &gpmc_cfg->cs[0].config2);
	writel(NET_GPMC_CONFIG3, &gpmc_cfg->cs[0].config3);
	writel(NET_GPMC_CONFIG4, &gpmc_cfg->cs[0].config4);
	writel(NET_GPMC_CONFIG5, &gpmc_cfg->cs[0].config5);
	writel(NET_GPMC_CONFIG6, &gpmc_cfg->cs[0].config6);
	writel(NET_GPMC_CONFIG7, &gpmc_cfg->cs[0].config7);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);

	/* Make GPIO 139 as output pin */
	writel(readl(&gpio5_base->oe) & ~(GPIO11), &gpio5_base->oe);

	/* Now send a pulse on the GPIO pin */
	writel(GPIO11, &gpio5_base->setdataout);
	udelay(1);
	writel(GPIO11, &gpio5_base->cleardataout);
	udelay(1);
	writel(GPIO11, &gpio5_base->setdataout);
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

int overwrite_console(void)
{
	/* return true if console should be overwritten */
	return 0;
}

#endif /* CONFIG_CMD_NET */
