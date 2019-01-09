// SPDX-License-Identifier: GPL-2.0+
/*
 * eco5pk.c - board file for 8D Technology's AM3517 based eco5pk board
 *
 * Based on am3517evm.c
 *
 * Copyright (C) 2011-2012 8D Technologies inc.
 * Copyright (C) 2009 Texas Instruments Incorporated
 */
#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/emac_defs.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <u-boot/crc.h>
#include <asm/mach-types.h>
#include "eco5pk.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	gpio_request(30, "RESOUT");
	gpio_direction_output(30, 1);
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
	MUX_ECO5_PK();
}
