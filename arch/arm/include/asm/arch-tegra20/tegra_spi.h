/*
 * NVIDIA Tegra20 SPI-FLASH controller
 *
 * Copyright 2010-2012 NVIDIA Corporation
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

#ifndef _TEGRA_SPI_H_
#define _TEGRA_SPI_H_

#include <asm/types.h>

struct spi_tegra {
	u32 command;	/* SPI_COMMAND_0 register  */
	u32 status;	/* SPI_STATUS_0 register */
	u32 rx_cmp;	/* SPI_RX_CMP_0 register  */
	u32 dma_ctl;	/* SPI_DMA_CTL_0 register */
	u32 tx_fifo;	/* SPI_TX_FIFO_0 register */
	u32 rsvd[3];	/* offsets 0x14 to 0x1F reserved */
	u32 rx_fifo;	/* SPI_RX_FIFO_0 register */
};

#define SPI_CMD_GO			(1 << 30)
#define SPI_CMD_ACTIVE_SCLK_SHIFT	26
#define SPI_CMD_ACTIVE_SCLK_MASK	(3 << SPI_CMD_ACTIVE_SCLK_SHIFT)
#define SPI_CMD_CK_SDA			(1 << 21)
#define SPI_CMD_ACTIVE_SDA_SHIFT	18
#define SPI_CMD_ACTIVE_SDA_MASK		(3 << SPI_CMD_ACTIVE_SDA_SHIFT)
#define SPI_CMD_CS_POL			(1 << 16)
#define SPI_CMD_TXEN			(1 << 15)
#define SPI_CMD_RXEN			(1 << 14)
#define SPI_CMD_CS_VAL			(1 << 13)
#define SPI_CMD_CS_SOFT			(1 << 12)
#define SPI_CMD_CS_DELAY		(1 << 9)
#define SPI_CMD_CS3_EN			(1 << 8)
#define SPI_CMD_CS2_EN			(1 << 7)
#define SPI_CMD_CS1_EN			(1 << 6)
#define SPI_CMD_CS0_EN			(1 << 5)
#define SPI_CMD_BIT_LENGTH		(1 << 4)
#define SPI_CMD_BIT_LENGTH_MASK		0x0000001F

#define SPI_STAT_BSY			(1 << 31)
#define SPI_STAT_RDY			(1 << 30)
#define SPI_STAT_RXF_FLUSH		(1 << 29)
#define SPI_STAT_TXF_FLUSH		(1 << 28)
#define SPI_STAT_RXF_UNR		(1 << 27)
#define SPI_STAT_TXF_OVF		(1 << 26)
#define SPI_STAT_RXF_EMPTY		(1 << 25)
#define SPI_STAT_RXF_FULL		(1 << 24)
#define SPI_STAT_TXF_EMPTY		(1 << 23)
#define SPI_STAT_TXF_FULL		(1 << 22)
#define SPI_STAT_SEL_TXRX_N		(1 << 16)
#define SPI_STAT_CUR_BLKCNT		(1 << 15)

#define SPI_TIMEOUT		1000
#define TEGRA_SPI_MAX_FREQ	52000000

#endif	/* _TEGRA_SPI_H_ */
