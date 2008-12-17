/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <i2c.h>

#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>

static void
get_spd(ddr1_spd_eeprom_t *spd, unsigned char i2c_address)
{
	i2c_read(i2c_address, 0, 1, (uchar *)spd, sizeof(ddr1_spd_eeprom_t));
}


unsigned int
fsl_ddr_get_mem_data_rate(void)
{
	return get_ddr_freq(0);
}


void
fsl_ddr_get_spd(ddr1_spd_eeprom_t *ctrl_dimms_spd,
		      unsigned int ctrl_num)
{
	unsigned int i;
	unsigned int i2c_address = 0;

	for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
		if (ctrl_num == 0 && i == 0) {
			i2c_address = SPD_EEPROM_ADDRESS;
		}
		get_spd(&(ctrl_dimms_spd[i]), i2c_address);
	}
}

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	/*
	 * Factors to consider for CPO:
	 *	- frequency
	 *	- ddr1 vs. ddr2
	 */
	popts->cpo_override = 0;

	/*
	 * Factors to consider for write data delay:
	 *	- number of DIMMs
	 *
	 * 1 = 1/4 clock delay
	 * 2 = 1/2 clock delay
	 * 3 = 3/4 clock delay
	 * 4 = 1   clock delay
	 * 5 = 5/4 clock delay
	 * 6 = 3/2 clock delay
	 */
	popts->write_data_delay = 3;

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
}
