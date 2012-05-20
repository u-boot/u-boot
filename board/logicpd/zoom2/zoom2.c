/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * Derived from Zoom1 code by
 *	Nishanth Menon <nm@ti.com>
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <netdev.h>
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/gpio.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include "zoom2.h"
#include "zoom2_serial.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * This the the zoom2, board specific, gpmc configuration for the
 * quad uart on the debug board.   The more general gpmc configurations
 * are setup at the cpu level in arch/arm/cpu/armv7/omap3/mem.c
 *
 * The details of the setting of the serial gpmc setup are not available.
 * The values were provided by another party.
 */
static u32 gpmc_serial_TL16CP754C[GPMC_MAX_REG] = {
	0x00011000,
	0x001F1F01,
	0x00080803,
	0x1D091D09,
	0x041D1F1F,
	0x1D0904C4, 0
};

/* Used to track the revision of the board */
static zoom2_revision revision = ZOOM2_REVISION_UNKNOWN;

/*
 * Routine: zoom2_get_revision
 * Description: Return the revision of the Zoom2 this code is running on.
 */
zoom2_revision zoom2_get_revision(void)
{
	return revision;
}

/*
 * Routine: zoom2_identify
 * Description: Detect which version of Zoom2 we are running on.
 */
void zoom2_identify(void)
{
	/*
	 * To check for production board vs beta board,
	 * check if gpio 94 is clear.
	 *
	 * No way yet to check for alpha board identity.
	 * Alpha boards were produced in very limited quantities
	 * and they are not commonly used.  They are mentioned here
	 * only for completeness.
	 */
	if (!gpio_request(94, "")) {
		unsigned int val;

		gpio_direction_input(94);
		val = gpio_get_value(94);

		if (val)
			revision = ZOOM2_REVISION_BETA;
		else
			revision = ZOOM2_REVISION_PRODUCTION;
	}

	printf("Board revision ");
	switch (revision) {
	case ZOOM2_REVISION_PRODUCTION:
		printf("Production\n");
		break;
	case ZOOM2_REVISION_BETA:
		printf("Beta\n");
		break;
	default:
		printf("Unknown\n");
		break;
	}
}

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init (void)
{
	u32 *gpmc_config;

	gpmc_init ();		/* in SRAM or SDRAM, finish GPMC */

	/* Configure console support on zoom2 */
	gpmc_config = gpmc_serial_TL16CP754C;
	enable_gpmc_cs_config(gpmc_config, &gpmc_cfg->cs[3],
			SERIAL_TL16CP754C_BASE, GPMC_SIZE_16M);

	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP_ZOOM2;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

#if defined(CONFIG_STATUS_LED) && defined(STATUS_LED_BOOT)
	status_led_set (STATUS_LED_BOOT, STATUS_LED_ON);
#endif
	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Configure zoom board specific configurations
 */
int misc_init_r(void)
{
	zoom2_identify();
	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);
	dieid_num_r();

	/*
	 * Board Reset
	 * The board is reset by holding the the large button
	 * on the top right side of the main board for
	 * eight seconds.
	 *
	 * There are reported problems of some beta boards
	 * continously resetting.  For those boards, disable resetting.
	 */
	if (ZOOM2_REVISION_PRODUCTION <= zoom2_get_revision())
		twl4030_power_reset_init();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs (void)
{
	/* platform specific muxes */
	MUX_ZOOM2 ();
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0);
	return 0;
}
#endif

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_LAN91C96
	rc = lan91c96_initialize(0, CONFIG_LAN91C96_BASE);
#endif
	return rc;
}
#endif
