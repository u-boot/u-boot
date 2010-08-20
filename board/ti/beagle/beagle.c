/*
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "beagle.h"

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3_BEAGLE;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Routine: get_board_revision
 * Description: Detect if we are running on a Beagle revision Ax/Bx,
 *		C1/2/3, C4 or xM. This can be done by reading
 *		the level of GPIO173, GPIO172 and GPIO171. This should
 *		result in
 *		GPIO173, GPIO172, GPIO171: 1 1 1 => Ax/Bx
 *		GPIO173, GPIO172, GPIO171: 1 1 0 => C1/2/3
 *		GPIO173, GPIO172, GPIO171: 1 0 1 => C4
 *		GPIO173, GPIO172, GPIO171: 0 0 0 => xM
 */
int get_board_revision(void)
{
	int revision;

	if (!omap_request_gpio(171) &&
	    !omap_request_gpio(172) &&
	    !omap_request_gpio(173)) {

		omap_set_gpio_direction(171, 1);
		omap_set_gpio_direction(172, 1);
		omap_set_gpio_direction(173, 1);

		revision = omap_get_gpio_datain(173) << 2 |
			   omap_get_gpio_datain(172) << 1 |
			   omap_get_gpio_datain(171);

		omap_free_gpio(171);
		omap_free_gpio(172);
		omap_free_gpio(173);
	} else {
		printf("Error: unable to acquire board revision GPIOs\n");
		revision = -1;
	}

	return revision;
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;
	struct gpio *gpio6_base = (struct gpio *)OMAP34XX_GPIO6_BASE;

	switch (get_board_revision()) {
	case REVISION_AXBX:
		printf("Beagle Rev Ax/Bx\n");
		setenv("beaglerev", "AxBx");
		setenv("mpurate", "600");
		break;
	case REVISION_CX:
		printf("Beagle Rev C1/C2/C3\n");
		setenv("beaglerev", "Cx");
		setenv("mpurate", "600");
		MUX_BEAGLE_C();
		break;
	case REVISION_C4:
		printf("Beagle Rev C4\n");
		setenv("beaglerev", "C4");
		setenv("mpurate", "720");
		MUX_BEAGLE_C();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		break;
	case REVISION_XM:
		printf("Beagle xM Rev A\n");
		setenv("beaglerev", "xMA");
		setenv("mpurate", "1000");
		MUX_BEAGLE_XM();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		break;
	default:
		printf("Beagle unknown 0x%02x\n", get_board_revision());
	}

	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);

	/* Configure GPIOs to output */
	writel(~(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1), &gpio6_base->oe);
	writel(~(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
		GPIO15 | GPIO14 | GPIO13 | GPIO12), &gpio5_base->oe);

	/* Set GPIOs */
	writel(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1,
		&gpio6_base->setdataout);
	writel(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
		GPIO15 | GPIO14 | GPIO13 | GPIO12, &gpio5_base->setdataout);

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
	MUX_BEAGLE();
}
