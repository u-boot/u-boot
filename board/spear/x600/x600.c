/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 *
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
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
#include <nand.h>
#include <netdev.h>
#include <phy.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_defs.h>
#include <asm/arch/spr_misc.h>
#include <linux/mtd/fsmc_nand.h>
#include "fpga.h"

static struct nand_chip nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];

int board_init(void)
{
	/*
	 * X600 is equipped with an M41T82 RTC. This RTC has the
	 * HT bit (Halt Update), which needs to be cleared upon
	 * power-up. Otherwise the RTC is halted.
	 */
	rtc_reset();

	return spear_board_init(MACH_TYPE_SPEAR600);
}

int board_late_init(void)
{
	/*
	 * Monitor and env protection on by default
	 */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE +
		      CONFIG_SYS_SPL_LEN + CONFIG_SYS_MONITOR_LEN +
		      2 * CONFIG_ENV_SECT_SIZE - 1,
		      &flash_info[0]);

	/* Init FPGA subsystem */
	x600_init_fpga();

	return 0;
}

/*
 * board_nand_init - Board specific NAND initialization
 * @nand:	mtd private chip structure
 *
 * Called by nand_init_chip to initialize the board specific functions
 */

void board_nand_init(void)
{
	struct misc_regs *const misc_regs_p =
		(struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	struct nand_chip *nand = &nand_chip[0];

	if (!(readl(&misc_regs_p->auto_cfg_reg) & MISC_NANDDIS))
		fsmc_nand_init(nand);
}

int designware_board_phy_init(struct eth_device *dev, int phy_addr,
	int (*mii_write)(struct eth_device *, u8, u8, u16),
	int dw_reset_phy(struct eth_device *))
{
	/* Extended PHY control 1, select GMII */
	mii_write(dev, phy_addr, 23, 0x0020);

	/* Software reset necessary after GMII mode selction */
	dw_reset_phy(dev);

	/* Enable extended page register access */
	mii_write(dev, phy_addr, 31, 0x0001);

	/* 17e: Enhanced LED behavior, needs to be written twice */
	mii_write(dev, phy_addr, 17, 0x09ff);
	mii_write(dev, phy_addr, 17, 0x09ff);

	/* 16e: Enhanced LED method select */
	mii_write(dev, phy_addr, 16, 0xe0ea);

	/* Disable extended page register access */
	mii_write(dev, phy_addr, 31, 0x0000);

	/* Enable clock output pin */
	mii_write(dev, phy_addr, 18, 0x0049);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = 0;

	if (designware_initialize(0, CONFIG_SPEAR_ETHBASE, CONFIG_PHY_ADDR,
				  PHY_INTERFACE_MODE_GMII) >= 0)
		ret++;

	return ret;
}
