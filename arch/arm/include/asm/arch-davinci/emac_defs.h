/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Based on:
 *
 * ----------------------------------------------------------------------------
 *
 * dm644x_emac.h
 *
 * TI DaVinci (DM644X) EMAC peripheral driver header for DV-EVM
 *
 * Copyright (C) 2005 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------

 * Modifications:
 * ver. 1.0: Sep 2005, TI PSP Team - Created EMAC version for uBoot.
 *
 */

#ifndef _DM644X_EMAC_H_
#define _DM644X_EMAC_H_

#include <asm/arch/hardware.h>

#ifdef CONFIG_SOC_DM365
#define EMAC_BASE_ADDR			(0x01d07000)
#define EMAC_WRAPPER_BASE_ADDR		(0x01d0a000)
#define EMAC_WRAPPER_RAM_ADDR		(0x01d08000)
#define EMAC_MDIO_BASE_ADDR		(0x01d0b000)
#define DAVINCI_EMAC_VERSION2
#elif defined(CONFIG_SOC_DA8XX)
#define EMAC_BASE_ADDR			DAVINCI_EMAC_CNTRL_REGS_BASE
#define EMAC_WRAPPER_BASE_ADDR		DAVINCI_EMAC_WRAPPER_CNTRL_REGS_BASE
#define EMAC_WRAPPER_RAM_ADDR		DAVINCI_EMAC_WRAPPER_RAM_BASE
#define EMAC_MDIO_BASE_ADDR		DAVINCI_MDIO_CNTRL_REGS_BASE
#define DAVINCI_EMAC_VERSION2
#else
#define EMAC_BASE_ADDR			(0x01c80000)
#define EMAC_WRAPPER_BASE_ADDR		(0x01c81000)
#define EMAC_WRAPPER_RAM_ADDR		(0x01c82000)
#define EMAC_MDIO_BASE_ADDR		(0x01c84000)
#endif

#ifdef CONFIG_SOC_DM646X
#define DAVINCI_EMAC_VERSION2
#define DAVINCI_EMAC_GIG_ENABLE
#endif

#ifdef CONFIG_SOC_DM646X
/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ		76500000
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ		2500000		/* 2.5 MHz */
#elif defined(CONFIG_SOC_DM365)
/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ		121500000
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ		2200000		/* 2.2 MHz */
#elif defined(CONFIG_SOC_DA8XX)
/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ		clk_get(DAVINCI_MDIO_CLKID)
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ		2000000		/* 2.0 MHz */
#else
/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ		99000000	/* PLL/6 - 99 MHz */
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ		2000000		/* 2.0 MHz */
#endif

#define PHY_KSZ8873	(0x00221450)
int ksz8873_is_phy_connected(int phy_addr);
int ksz8873_get_link_speed(int phy_addr);
int ksz8873_init_phy(int phy_addr);
int ksz8873_auto_negotiate(int phy_addr);

#define PHY_LXT972	(0x001378e2)
int lxt972_is_phy_connected(int phy_addr);
int lxt972_get_link_speed(int phy_addr);
int lxt972_init_phy(int phy_addr);
int lxt972_auto_negotiate(int phy_addr);

#define PHY_DP83848	(0x20005c90)
int dp83848_is_phy_connected(int phy_addr);
int dp83848_get_link_speed(int phy_addr);
int dp83848_init_phy(int phy_addr);
int dp83848_auto_negotiate(int phy_addr);

#define PHY_ET1011C	(0x282f013)
int et1011c_get_link_speed(int phy_addr);

#endif  /* _DM644X_EMAC_H_ */
