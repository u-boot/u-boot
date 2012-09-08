/*
 * (C) Copyright 2012 Xilinx
 *
 * Zynq SD Host Controller Interface
 * Controller register and bit definitions
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __SDHCI_MMC_H_
#define __SDHCI_MMC_H_

#define SD_DMA_ADDR_R        0x00

#define SD_BLOCK_SZ_R        0x04

#define SD_BLOCK_CNT_R       0x06

#define SD_ARG_R             0x08

#define SD_TRNS_MODE_R       0x0C
#define  SD_TRNS_DMA         0x01
#define  SD_TRNS_BLK_CNT_EN  0x02
#define  SD_TRNS_ACMD12      0x04
#define  SD_TRNS_READ        0x10
#define  SD_TRNS_MULTI       0x20

#define SD_CMD_R             0x0E
#define  SD_CMD_RESP_NONE    0x00
#define  SD_CMD_RESP_136     0x01
#define  SD_CMD_RESP_48      0x02
#define  SD_CMD_RESP_48_BUSY 0x03
#define  SD_CMD_CRC          0x08
#define  SD_CMD_INDEX        0x10
#define  SD_CMD_DATA         0x20

#define SD_RSP_R             0x10

#define SD_BUFF_R            0x20

#define SD_PRES_STATE_R      0x24
#define  SD_CMD_INHIBIT      0x00000001
#define  SD_DATA_INHIBIT     0x00000002
#define  SD_WRITE_ACTIVE     0x00000100
#define  SD_READ_ACTIVE      0x00000200
#define  SD_CARD_INS         0x00010000
#define  SD_CARD_DB          0x00020000
#define  SD_CARD_DPL         0x00040000
#define  SD_CARD_WP          0x00080000

#define SD_HOST_CTRL_R       0x28
#define SD_HOST_4BIT		0x02
#define  SD_CD_TEST_INS      0x40
#define  SD_CD_TEST          0x80

#define SD_PWR_CTRL_R     0x29
#define  SD_POWER_ON         0x01
#define  SD_POWER_18         0x0A
#define  SD_POWER_30         0x0C
#define  SD_POWER_33         0x0E

#define SD_BLCK_GAP_CTL_R    0x2A

#define SD_WAKE_CTL_R        0x2B

#define SD_CLK_CTL_R         0x2C
#define  SD_DIV_SHIFT        8
#define  SD_CLK_SD_EN        0x0004
#define  SD_CLK_INT_STABLE   0x0002
#define  SD_CLK_INT_EN       0x0001

#define SD_TIMEOUT_CTL_R     0x2E

#define SD_SOFT_RST_R        0x2F
#define  SD_RST_ALL          0x01
#define  SD_RST_CMD          0x02
#define  SD_RST_DATA         0x04

#define SD_INT_STAT_R        0x30
#define SD_INT_ENA_R         0x34
#define SD_SIG_ENA_R         0x38
#define  SD_INT_CMD_CMPL     0x00000001
#define  SD_INT_TRNS_CMPL    0x00000002
#define  SD_INT_DMA          0x00000008
#define  SD_INT_ERROR        0x00008000
#define  SD_INT_ERR_CTIMEOUT 0x00010000
#define  SD_INT_ERR_CCRC     0x00020000
#define  SD_INT_ERR_CEB      0x00040000
#define  SD_INT_ERR_IDX      0x00080000
#define  SD_INT_ERR_DTIMEOUT 0x00100000
#define  SD_INT_ERR_DCRC     0x00200000
#define  SD_INT_ERR_DEB      0x00400000
#define  SD_INT_ERR_CLMT     0x00800000
#define  SD_INT_ERR_ACMD12   0x01000000
#define  SD_INT_ERR_ADMA     0x02000000
#define  SD_INT_ERR_TRESP    0x10000000

#define SD_CAPABILITIES_R    0x40

#endif /* __SDHCI_MMC_H_ */
