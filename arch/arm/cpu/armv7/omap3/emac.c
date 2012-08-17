/*
 *
 * DaVinci EMAC initialization.
 *
 * (C) Copyright 2011, Ilya Yanok, Emcraft Systems
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
 * Foundation, Inc.
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/am35x_def.h>

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
	u32 reset;

	/* ensure that the module is out of reset */
	reset = readl(&am35x_scm_general_regs->ip_sw_reset);
	reset &= ~CPGMACSS_SW_RST;
	writel(reset, &am35x_scm_general_regs->ip_sw_reset);

	return davinci_emac_initialize();
}
