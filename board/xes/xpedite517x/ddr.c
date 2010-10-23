/*
 * Copyright 2009 Extreme Engineering Solutions, Inc.
 * Copyright 2007-2008 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <i2c.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>

static void get_spd(ddr2_spd_eeprom_t *spd, unsigned char i2c_address)
{
	i2c_read(i2c_address, SPD_EEPROM_OFFSET, 2, (uchar *)spd,
		sizeof(ddr2_spd_eeprom_t));
}

unsigned int fsl_ddr_get_mem_data_rate(void)
{
	return get_bus_freq(0);
}

void fsl_ddr_get_spd(ddr2_spd_eeprom_t *ctrl_dimms_spd,
			unsigned int ctrl_num)
{
	unsigned int i;
	unsigned int i2c_address = 0;

	for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
		if (ctrl_num == 0) {
			i2c_address = SPD_EEPROM_ADDRESS1;
#ifdef SPD_EEPROM_ADDRESS2
		} else if (ctrl_num == 1) {
			i2c_address = SPD_EEPROM_ADDRESS2;
#endif
		} else {
			/* An inalid ctrl number was give, use default SPD */
			printf("ERROR: invalid DDR ctrl: %d\n", ctrl_num);
			i2c_address = SPD_EEPROM_ADDRESS1;
		}

		get_spd(&(ctrl_dimms_spd[i]), i2c_address);
	}
}

/*
 * There are four board-specific SDRAM timing parameters which must be
 * calculated based on the particular PCB artwork.  These are:
 *   1.) CPO (Read Capture Delay)
 *           - TIMING_CFG_2 register
 *           Source: Calculation based on board trace lengths and
 *                   chip-specific internal delays.
 *   2.) WR_DATA_DELAY (Write Command to Data Strobe Delay)
 *           - TIMING_CFG_2 register
 *           Source: Calculation based on board trace lengths.
 *                   Unless clock and DQ lanes are very different
 *                   lengths (>2"), this should be set to the nominal value
 *                   of 1/2 clock delay.
 *   3.) CLK_ADJUST (Clock and Addr/Cmd alignment control)
 *           - DDR_SDRAM_CLK_CNTL register
 *           Source: Signal Integrity Simulations
 *   4.) 2T Timing on Addr/Ctl
 *           - TIMING_CFG_2 register
 *           Source: Signal Integrity Simulations
 *           Usually only needed with heavy load/very high speed (>DDR2-800)
 *
 *     PCB routing on the XPedite5170 is nearly identical to the XPedite5370
 *     so we use the XPedite5370 settings as a basis for the XPedite5170.
 */

typedef struct board_memctl_options {
	uint16_t datarate_mhz_low;
	uint16_t datarate_mhz_high;
	uint8_t clk_adjust;
	uint8_t cpo_override;
	uint8_t write_data_delay;
} board_memctl_options_t;

static struct board_memctl_options bopts_ctrl[][2] = {
	{
		/* Controller 0 */
		{
			/* DDR2 600/667 */
			.datarate_mhz_low	= 500,
			.datarate_mhz_high	= 750,
			.clk_adjust		= 5,
			.cpo_override		= 8,
			.write_data_delay	= 2,
		},
		{
			/* DDR2 800 */
			.datarate_mhz_low	= 750,
			.datarate_mhz_high	= 850,
			.clk_adjust		= 5,
			.cpo_override		= 9,
			.write_data_delay	= 2,
		},
	},
	{
		/* Controller 1 */
		{
			/* DDR2 600/667 */
			.datarate_mhz_low	= 500,
			.datarate_mhz_high	= 750,
			.clk_adjust		= 5,
			.cpo_override		= 7,
			.write_data_delay	= 2,
		},
		{
			/* DDR2 800 */
			.datarate_mhz_low	= 750,
			.datarate_mhz_high	= 850,
			.clk_adjust		= 5,
			.cpo_override		= 8,
			.write_data_delay	= 2,
		},
	},
};

void fsl_ddr_board_options(memctl_options_t *popts,
			dimm_params_t *pdimm,
			unsigned int ctrl_num)
{
	struct board_memctl_options *bopts = bopts_ctrl[ctrl_num];
	sys_info_t sysinfo;
	int i;
	unsigned int datarate;

	get_sys_info(&sysinfo);
	datarate = fsl_ddr_get_mem_data_rate() / 1000000;

	for (i = 0; i < ARRAY_SIZE(bopts_ctrl[ctrl_num]); i++) {
		if ((bopts[i].datarate_mhz_low <= datarate) &&
		    (bopts[i].datarate_mhz_high >= datarate)) {
			debug("controller %d:\n", ctrl_num);
			debug(" clk_adjust = %d\n", bopts[i].clk_adjust);
			debug(" cpo = %d\n", bopts[i].cpo_override);
			debug(" write_data_delay = %d\n",
				bopts[i].write_data_delay);
			popts->clk_adjust = bopts[i].clk_adjust;
			popts->cpo_override = bopts[i].cpo_override;
			popts->write_data_delay = bopts[i].write_data_delay;
		}
	}

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
}
