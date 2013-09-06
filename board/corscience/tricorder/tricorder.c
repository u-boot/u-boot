/*
 * (C) Copyright 2012
 * Corscience GmbH & Co. KG, <www.corscience.de>
 * Thomas Weber <weber@corscience.de>
 * Sunil Kumar <sunilsaini05@gmail.com>
 * Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Devkit8000 code by
 * Frederik Kriewitz <frederik@kriewitz.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include "tricorder.h"
#include "tricorder-eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/**
 * get_eeprom - read the eeprom
 *
 * @eeprom - pointer to a eeprom struct to fill
 *
 * This function will panic() on wrong EEPROM content
 */
static void get_eeprom(struct tricorder_eeprom *eeprom)
{
	int ret;

	if (!eeprom)
		panic("No eeprom given!\n");

	ret = gpio_request(7, "BMS");
	if (ret)
		panic("gpio: requesting BMS pin failed\n");

	ret = gpio_direction_input(7);
	if (ret)
		panic("gpio: set BMS as input failed\n");

	ret = gpio_get_value(7);
	if (ret < 0)
		panic("gpio: get BMS pin state failed\n");

	gpio_free(7);

	if (ret == 0) {
		/* BMS is _not_ set, do the EEPROM check */
		ret = tricorder_get_eeprom(0x51, eeprom);
		if (!ret) {
			if (strncmp(eeprom->board_name, "CS10411", 7) != 0)
				panic("Wrong board name '%.*s'\n",
				      sizeof(eeprom->board_name),
						eeprom->board_name);
			if (eeprom->board_version[0] < 'D')
				panic("Wrong board version '%.*s'\n",
				      sizeof(eeprom->board_version),
						eeprom->board_version);
		} else {
			panic("Could not get board revision\n");
		}
	}
}

/**
 * print_hwversion - print out a HW version string
 *
 * @eeprom - pointer to the eeprom
 */
static void print_hwversion(struct tricorder_eeprom *eeprom)
{
	size_t len;
	if (!eeprom)
		panic("No eeprom given!");

	printf("Board %.*s:%.*s serial %.*s",
	       sizeof(eeprom->board_name), eeprom->board_name,
	       sizeof(eeprom->board_version), eeprom->board_version,
	       sizeof(eeprom->board_serial), eeprom->board_serial);

	len = strnlen(eeprom->interface_version,
		      sizeof(eeprom->interface_version));
	if (len > 0)
		printf(" HW interface version %.*s",
		       sizeof(eeprom->interface_version),
		       eeprom->interface_version);
	puts("\n");
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	struct tricorder_eeprom eeprom;
	get_eeprom(&eeprom);
	print_hwversion(&eeprom);

	twl4030_power_init();
#ifdef CONFIG_TWL4030_LED
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);
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
	MUX_TRICORDER();
}

#if defined(CONFIG_GENERIC_MMC) && !(defined(CONFIG_SPL_BUILD))
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on the first bank.  This
 * provides the timing values back to the function that configures
 * the memory.  We have either one or two banks of 128MB DDR.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	/* General SDRC config */
	timings->mcfg = MICRON_V_MCFG_165(128 << 20);
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;

	/* AC timings */
	timings->ctrla = MICRON_V_ACTIMA_165;
	timings->ctrlb = MICRON_V_ACTIMB_165;
	timings->mr = MICRON_V_MR_165;
}
