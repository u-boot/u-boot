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
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include "tricorder.h"

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

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
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
