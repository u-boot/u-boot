/*
 * NVIDIA Tegra SPI-SLINK controller
 *
 * Copyright 2010-2013 NVIDIA Corporation
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

#ifndef _TEGRA_SLINK_H_
#define _TEGRA_SLINK_H_

#include <asm/types.h>

struct slink_tegra {
	u32 command;	/* SLINK_COMMAND_0 register  */
	u32 command2;	/* SLINK_COMMAND2_0 reg */
	u32 status;	/* SLINK_STATUS_0 register */
	u32 reserved;	/* Reserved offset 0C */
	u32 mas_data;	/* SLINK_MAS_DATA_0 reg */
	u32 slav_data;	/* SLINK_SLAVE_DATA_0 reg */
	u32 dma_ctl;	/* SLINK_DMA_CTL_0 register */
	u32 status2;	/* SLINK_STATUS2_0 reg */
	u32 rsvd[56];	/* 0x20 to 0xFF reserved */
	u32 tx_fifo;	/* SLINK_TX_FIFO_0 reg off 100h */
	u32 rsvd2[31];	/* 0x104 to 0x17F reserved */
	u32 rx_fifo;	/* SLINK_RX_FIFO_0 reg off 180h */
};

/* COMMAND */
#define SLINK_CMD_ENB			(1 << 31)
#define SLINK_CMD_GO			(1 << 30)
#define SLINK_CMD_M_S			(1 << 28)
#define SLINK_CMD_CK_SDA		(1 << 21)
#define SLINK_CMD_CS_POL		(1 << 13)
#define SLINK_CMD_CS_VAL		(1 << 12)
#define SLINK_CMD_CS_SOFT		(1 << 11)
#define SLINK_CMD_BIT_LENGTH		(1 << 4)
#define SLINK_CMD_BIT_LENGTH_MASK	0x0000001F
/* COMMAND2 */
#define SLINK_CMD2_TXEN			(1 << 30)
#define SLINK_CMD2_RXEN			(1 << 31)
#define SLINK_CMD2_SS_EN		(1 << 18)
#define SLINK_CMD2_SS_EN_SHIFT		18
#define SLINK_CMD2_SS_EN_MASK		0x000C0000
#define SLINK_CMD2_CS_ACTIVE_BETWEEN	(1 << 17)
/* STATUS */
#define SLINK_STAT_BSY			(1 << 31)
#define SLINK_STAT_RDY			(1 << 30)
#define SLINK_STAT_ERR			(1 << 29)
#define SLINK_STAT_RXF_FLUSH		(1 << 27)
#define SLINK_STAT_TXF_FLUSH		(1 << 26)
#define SLINK_STAT_RXF_OVF		(1 << 25)
#define SLINK_STAT_TXF_UNR		(1 << 24)
#define SLINK_STAT_RXF_EMPTY		(1 << 23)
#define SLINK_STAT_RXF_FULL		(1 << 22)
#define SLINK_STAT_TXF_EMPTY		(1 << 21)
#define SLINK_STAT_TXF_FULL		(1 << 20)
#define SLINK_STAT_TXF_OVF		(1 << 19)
#define SLINK_STAT_RXF_UNR		(1 << 18)
#define SLINK_STAT_CUR_BLKCNT		(1 << 15)
/* STATUS2 */
#define SLINK_STAT2_RXF_FULL_CNT	(1 << 16)
#define SLINK_STAT2_TXF_FULL_CNT	(1 << 0)

#define SPI_TIMEOUT		1000
#define TEGRA_SPI_MAX_FREQ	52000000

#endif	/* _TEGRA_SLINK_H_ */
