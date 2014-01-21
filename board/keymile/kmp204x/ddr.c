/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 *
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
  */

#include <common.h>
#include <i2c.h>
#include <hwconfig.h>
#include <asm/mmu.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	if (ctrl_num) {
		printf("Wrong parameter for controller number %d", ctrl_num);
		return;
	}

	/* automatic calibration for nb of cycles between read and DQS pre */
	popts->cpo_override = 0xFF;

	/* 1/2 clk delay between wr command and data strobe */
	popts->write_data_delay = 4;
	/* clk lauched 1/2 applied cylcle after address command */
	popts->clk_adjust = 4;
	/* 1T timing: command/address held for only 1 cycle */
	popts->twot_en = 0;

	/* we have only one module, half str should be OK */
	popts->half_strength_driver_enable = 1;

	/* wrlvl values overriden as recommended by ddr init func */
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;
	popts->wrlvl_start = 0x6;

	/* Enable ZQ calibration */
	popts->zq_en = 1;

	/* DHC_EN =1, ODT = 75 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR_ODT_75ohm;
}

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size = 0;

	puts("Initializing with SPD\n");

	dram_size = fsl_ddr_sdram();

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

	debug("    DDR: ");
	return dram_size;
}
