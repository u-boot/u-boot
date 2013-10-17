/*
 * (C) Copyright 2011 Comelit Group SpA
 * Luca Ceresoli <luca.ceresoli@comelit.it>
 *
 * Based on board/ti/beagle/beagle.c:
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
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/omap3-regs.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include "dig297.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CMD_NET
static void setup_net_chip(void);

#define NET_LAN9221_RESET_GPIO 12

/* GPMC CS 5 connected to an SMSC LAN9220 ethernet controller */
#define NET_LAN9220_GPMC_CONFIG1	(DEVICESIZE_16BIT)
#define NET_LAN9220_GPMC_CONFIG2	(CSWROFFTIME(8) | \
					 CSRDOFFTIME(7) | \
					 ADVONTIME(1))
#define NET_LAN9220_GPMC_CONFIG3	(ADVWROFFTIME(2) | \
					 ADVRDOFFTIME(2) | \
					 ADVONTIME(1))
#define NET_LAN9220_GPMC_CONFIG4	(WEOFFTIME(8) | \
					 WEONTIME(1) |  \
					 OEOFFTIME(7)|	\
					 OEONTIME(1))
#define NET_LAN9220_GPMC_CONFIG5	(PAGEBURSTACCESSTIME(0) | \
					 RDACCESSTIME(6)        | \
					 WRCYCLETIME(0x1D)      | \
					 RDCYCLETIME(0x1D))
#define NET_LAN9220_GPMC_CONFIG6	((1 << 31)          | \
					 WRACCESSTIME(0x1D) | \
					 WRDATAONADMUXBUS(3))

static const u32 gpmc_lan_config[] = {
	NET_LAN9220_GPMC_CONFIG1,
	NET_LAN9220_GPMC_CONFIG2,
	NET_LAN9220_GPMC_CONFIG3,
	NET_LAN9220_GPMC_CONFIG4,
	NET_LAN9220_GPMC_CONFIG5,
	NET_LAN9220_GPMC_CONFIG6,
	/* CONFIG7: computed by enable_gpmc_cs_config() */
};
#endif /* CONFIG_CMD_NET */

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init();		/* in SRAM or SDRAM, finish GPMC */
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
	struct gpio *gpio1_base = (struct gpio *)OMAP34XX_GPIO1_BASE;
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;

	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);

	/*
	 * GPIO list
	 * - 159 OUT (GPIO5+31): reset for remote camera interface connector.
	 * - 19  OUT (GPIO1+19): integrated speaker amplifier (1=on, 0=shdn).
	 * - 20  OUT (GPIO1+20): handset amplifier (1=on, 0=shdn).
	 */

	/* Configure GPIOs to output */
	writel(~(GPIO19 | GPIO20), &gpio1_base->oe);
	writel(~(GPIO31), &gpio5_base->oe);

	/* Set GPIO values */
	writel((GPIO19 | GPIO20), &gpio1_base->setdataout);
	writel(0, &gpio5_base->setdataout);

#if defined(CONFIG_CMD_NET)
	setup_net_chip();
#endif

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
	MUX_DIG297();
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#ifdef CONFIG_CMD_NET
/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *	      Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	/* Configure GPMC registers */
	enable_gpmc_cs_config(gpmc_lan_config, &gpmc_cfg->cs[5],
			      CONFIG_SMC911X_BASE, GPMC_SIZE_16M);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
	       &ctrl_base->gpmc_nadv_ale);

	/* Make GPIO 12 as output pin and send a magic pulse through it */
	if (!gpio_request(NET_LAN9221_RESET_GPIO, "")) {
		gpio_direction_output(NET_LAN9221_RESET_GPIO, 0);
		gpio_set_value(NET_LAN9221_RESET_GPIO, 1);
		udelay(1);
		gpio_set_value(NET_LAN9221_RESET_GPIO, 0);
		udelay(31000);	/* Should be >= 30ms according to datasheet */
		gpio_set_value(NET_LAN9221_RESET_GPIO, 1);
	}
}
#endif /* CONFIG_CMD_NET */

int board_eth_init(bd_t *bis)
{
	int rc = 0;
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
	return rc;
}
