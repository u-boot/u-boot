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
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include "zoom2.h"
#include "zoom2_serial.h"

/*
 * This the the zoom2, board specific, gpmc configuration for the
 * quad uart on the debug board.   The more general gpmc configurations
 * are setup at the cpu level in cpu/arm_cortexa8/omap3/mem.c
 *
 * The details of the setting of the serial gpmc setup are not available.
 * The values were provided by another party.
 */
extern void enable_gpmc_config(u32 *gpmc_config, gpmc_csx_t *gpmc_cs_base,
			       u32 base, u32 size);

static u32 gpmc_serial_TL16CP754C[GPMC_MAX_REG] = {
	0x00011000,
	0x001F1F01,
	0x00080803,
	0x1D091D09,
	0x041D1F1F,
	0x1D0904C4, 0
};

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	gpmc_csx_t *serial_cs_base;
	u32 *gpmc_config;

	gpmc_init ();		/* in SRAM or SDRAM, finish GPMC */

	/* Configure console support on zoom2 */
	gpmc_config = gpmc_serial_TL16CP754C;
	serial_cs_base = (gpmc_csx_t *) (GPMC_CONFIG_CS0_BASE +
					 (3 * GPMC_CONFIG_WIDTH));
	enable_gpmc_config(gpmc_config,
			   serial_cs_base,
			   SERIAL_TL16CP754C_BASE,
			   GPMC_SIZE_16M);

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
int misc_init_r (void)
{
	power_init_r ();
	dieid_num_r ();
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
