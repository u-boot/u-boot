// SPDX-License-Identifier: GPL-2.0+
/*
 * NEORV32 SPI definitions (subset)
 */

#ifndef __NEORV32_SPI_H
#define __NEORV32_SPI_H

#include <linux/types.h>

enum NEORV32_SPI_CTRL_enum {
	SPI_CTRL_EN           =  0,
	SPI_CTRL_CPHA         =  1,
	SPI_CTRL_CPOL         =  2,
	SPI_CTRL_PRSC0        =  3,
	SPI_CTRL_PRSC1        =  4,
	SPI_CTRL_PRSC2        =  5,
	SPI_CTRL_CDIV0        =  6,
	SPI_CTRL_CDIV1        =  7,
	SPI_CTRL_CDIV2        =  8,
	SPI_CTRL_CDIV3        =  9,
	SPI_CTRL_RX_AVAIL     = 16,
	SPI_CTRL_TX_EMPTY     = 17,
	SPI_CTRL_TX_FULL      = 18,
	SPI_CTRL_FIFO_LSB     = 24,
	SPI_CTRL_FIFO_MSB     = 27,
	SPI_CS_ACTIVE         = 30,
	SPI_CTRL_BUSY         = 31,
};

enum NEORV32_SPI_DATA_enum {
	SPI_DATA_LSB  =  0,
	SPI_DATA_CSEN =  3,
	SPI_DATA_MSB  =  7,
	SPI_DATA_CMD  = 31,
};

#define NEORV32_SPI_CTRL 0x00
#define NEORV32_SPI_DATA 0x04

#endif /* __NEORV32_SPI_H */
