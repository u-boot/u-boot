/*
 * Copyright 2011 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * P3060 QDS board configuration file
 */
#define CONFIG_P3060QDS
#define CONFIG_PHYS_64BIT
#define CONFIG_PPC_P3060
#define CONFIG_FSL_QIXIS

#define CONFIG_NAND_FSL_ELBC

#define CONFIG_ICS307_REFCLK_HZ	25000000  /* ICS307 ref clk freq */

#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SPI_FLASH_EON
#define CONFIG_SPI_FLASH_SST

#include "corenet_ds.h"

#define SGMII_CARD_PORT1_PHY_ADDR 0x1C
#define SGMII_CARD_PORT2_PHY_ADDR 0x1D
#define SGMII_CARD_PORT3_PHY_ADDR 0x1E
#define SGMII_CARD_PORT4_PHY_ADDR 0x1F

/* There is a PCA9547 8-channel I2C-bus multiplexer on P3060QDS board */
#define CONFIG_I2C_MUX
#define CONFIG_I2C_MULTI_BUS
