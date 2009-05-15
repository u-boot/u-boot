/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc. All rights reserved.
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
#include <asm/io.h>

#include "bcsr.h"

void enable_8569mds_flash_write()
{
	setbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 17), BCSR17_FLASH_nWP);
}

void disable_8569mds_flash_write()
{
	clrbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 17), BCSR17_FLASH_nWP);
}

void enable_8569mds_qe_mdio()
{
	setbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 7),
			BCSR7_UCC1_GETH_EN | BCSR7_UCC1_RGMII_EN);
	setbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 8),
			BCSR8_UCC2_GETH_EN | BCSR8_UCC2_RGMII_EN);
}

void disable_8569mds_brd_eeprom_write_protect()
{
	clrbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 7), BCSR7_BRD_WRT_PROTECT);
}
