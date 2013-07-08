/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Steve Sakoman <steve@sakoman.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>

struct gpmc *gpmc_cfg;

/*****************************************************
 * gpmc_init(): init gpmc bus
 * This code can only be executed from SRAM or SDRAM.
 *****************************************************/
void gpmc_init(void)
{
	gpmc_cfg = (struct gpmc *)GPMC_BASE;

	/* global settings */
	writel(0, &gpmc_cfg->irqenable); /* isr's sources masked */
	writel(0, &gpmc_cfg->timeout_control);/* timeout disable */

	/*
	 * Disable the GPMC0 config set by ROM code
	 * It conflicts with our MPDB (both at 0x08000000)
	 */
	writel(0, &gpmc_cfg->cs[0].config7);
}
