/*
 * Copyright (c) 2004	Cucy Systems (http://www.cucy.com)
 * Curt Brune <curt@cucy.com>
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
#include <asm/hardware.h>
#include <command.h>

/* ------------------------------------------------------------------------- */


#define PUT_LED(val)       (PUT_REG(REG_IOPDATA, (~val)&0xFF))
#define GET_LED()          ((~GET_REG( REG_IOPDATA)) & 0xFF)
#define SET_LED(val)       { u32 led = GET_LED(); led |= 1 << (val);  PUT_LED( led); }
#define CLR_LED(val)       { u32 led = GET_LED(); led &= ~(1 << (val));  PUT_LED( led); }

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	icache_enable();

	/* address for the kernel command line */
	gd->bd->bi_boot_params = 0x800;

	/* enable board LEDs for output */
	PUT_REG( REG_IOPDATA, 0x0);
	PUT_REG( REG_IOPMODE, 0xFFFF);
	PUT_REG( REG_IOPDATA, 0xFF);

	/* enable LED 7 to show we're alive */
	SET_LED( 7);

	/* configure free running timer 1 */
	/* Stop timer 1 */
	CLR_REG( REG_TMOD, TM1_RUN);

	/* Configure for toggle mode */
	SET_REG( REG_TMOD, TM1_TOGGLE);

	/* Load Timer data register with count down value */
	PUT_REG( REG_TDATA1, 0xFFFFFFFF);

	/* Clear timer counter register */
	PUT_REG( REG_TCNT1, 0x0);

	/* Start timer -- count down timer */
	SET_REG( REG_TMOD, TM1_RUN);

	return 0;
}

int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;
#if CONFIG_NR_DRAM_BANKS == 2
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size  = PHYS_SDRAM_2_SIZE;
#endif
	return 0;
}
