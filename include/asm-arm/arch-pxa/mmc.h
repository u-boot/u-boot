/*
 *  linux/drivers/mmc/mmc_pxa.h
 *
 *  Author: Vladimir Shebordaev, Igor Oblakov
 *  Copyright:  MontaVista Software Inc.
 *
 *  $Id: mmc_pxa.h,v 0.3.1.6 2002/09/25 19:25:48 ted Exp ted $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __MMC_PXA_P_H__
#define __MMC_PXA_P_H__

/* PXA-250 MMC controller registers */

/* MMC_STRPCL */
#define MMC_STRPCL_STOP_CLK		(0x0001UL)
#define MMC_STRPCL_START_CLK		(0x0002UL)

/* MMC_STAT */
#define MMC_STAT_END_CMD_RES		(0x0001UL << 13)
#define MMC_STAT_PRG_DONE		(0x0001UL << 12)
#define MMC_STAT_DATA_TRAN_DONE		(0x0001UL << 11)
#define MMC_STAT_CLK_EN			(0x0001UL << 8)
#define MMC_STAT_RECV_FIFO_FULL		(0x0001UL << 7)
#define MMC_STAT_XMIT_FIFO_EMPTY	(0x0001UL << 6)
#define MMC_STAT_RES_CRC_ERROR		(0x0001UL << 5)
#define MMC_STAT_SPI_READ_ERROR_TOKEN   (0x0001UL << 4)
#define MMC_STAT_CRC_READ_ERROR		(0x0001UL << 3)
#define MMC_STAT_CRC_WRITE_ERROR	(0x0001UL << 2)
#define MMC_STAT_TIME_OUT_RESPONSE	(0x0001UL << 1)
#define MMC_STAT_READ_TIME_OUT		(0x0001UL)

#define MMC_STAT_ERRORS (MMC_STAT_RES_CRC_ERROR|MMC_STAT_SPI_READ_ERROR_TOKEN\
	|MMC_STAT_CRC_READ_ERROR|MMC_STAT_TIME_OUT_RESPONSE\
	|MMC_STAT_READ_TIME_OUT|MMC_STAT_CRC_WRITE_ERROR)

/* MMC_CLKRT */
#define MMC_CLKRT_20MHZ			(0x0000UL)
#define MMC_CLKRT_10MHZ			(0x0001UL)
#define MMC_CLKRT_5MHZ			(0x0002UL)
#define MMC_CLKRT_2_5MHZ		(0x0003UL)
#define MMC_CLKRT_1_25MHZ		(0x0004UL)
#define MMC_CLKRT_0_625MHZ		(0x0005UL)
#define MMC_CLKRT_0_3125MHZ		(0x0006UL)

/* MMC_SPI */
#define MMC_SPI_DISABLE			(0x00UL)
#define MMC_SPI_EN			(0x01UL)
#define MMC_SPI_CS_EN			(0x01UL << 2)
#define MMC_SPI_CS_ADDRESS		(0x01UL << 3)
#define MMC_SPI_CRC_ON			(0x01UL << 1)

/* MMC_CMDAT */
#define MMC_CMDAT_SD_4DAT		(0x0001UL << 8)
#define MMC_CMDAT_MMC_DMA_EN		(0x0001UL << 7)
#define MMC_CMDAT_INIT			(0x0001UL << 6)
#define MMC_CMDAT_BUSY			(0x0001UL << 5)
#define MMC_CMDAT_BCR			(0x0003UL << 5)
#define MMC_CMDAT_STREAM		(0x0001UL << 4)
#define MMC_CMDAT_BLOCK			(0x0000UL << 4)
#define MMC_CMDAT_WRITE			(0x0001UL << 3)
#define MMC_CMDAT_READ			(0x0000UL << 3)
#define MMC_CMDAT_DATA_EN		(0x0001UL << 2)
#define MMC_CMDAT_R0			(0)
#define MMC_CMDAT_R1			(0x0001UL)
#define MMC_CMDAT_R2			(0x0002UL)
#define MMC_CMDAT_R3			(0x0003UL)

/* MMC_RESTO */
#define MMC_RES_TO_MAX			(0x007fUL) /* [6:0] */

/* MMC_RDTO */
#define MMC_READ_TO_MAX			(0x0ffffUL) /* [15:0] */

/* MMC_BLKLEN */
#define MMC_BLK_LEN_MAX			(0x03ffUL) /* [9:0] */

/* MMC_PRTBUF */
#define MMC_PRTBUF_BUF_PART_FULL	(0x01UL)
#define MMC_PRTBUF_BUF_FULL		(0x00UL    )

/* MMC_I_MASK */
#define MMC_I_MASK_TXFIFO_WR_REQ	(0x01UL << 6)
#define MMC_I_MASK_RXFIFO_RD_REQ	(0x01UL << 5)
#define MMC_I_MASK_CLK_IS_OFF		(0x01UL << 4)
#define MMC_I_MASK_STOP_CMD		(0x01UL << 3)
#define MMC_I_MASK_END_CMD_RES		(0x01UL << 2)
#define MMC_I_MASK_PRG_DONE		(0x01UL << 1)
#define MMC_I_MASK_DATA_TRAN_DONE       (0x01UL)
#define MMC_I_MASK_ALL			(0x07fUL)


/* MMC_I_REG */
#define MMC_I_REG_TXFIFO_WR_REQ		(0x01UL << 6)
#define MMC_I_REG_RXFIFO_RD_REQ		(0x01UL << 5)
#define MMC_I_REG_CLK_IS_OFF		(0x01UL << 4)
#define MMC_I_REG_STOP_CMD		(0x01UL << 3)
#define MMC_I_REG_END_CMD_RES		(0x01UL << 2)
#define MMC_I_REG_PRG_DONE		(0x01UL << 1)
#define MMC_I_REG_DATA_TRAN_DONE	(0x01UL)
#define MMC_I_REG_ALL			(0x007fUL)

/* MMC_CMD */
#define MMC_CMD_INDEX_MAX		(0x006fUL)  /* [5:0] */
#define CMD(x)  (x)

#define MMC_DEFAULT_RCA			1

#define MMC_BLOCK_SIZE			512
#define MMC_MAX_BLOCK_SIZE		512

#define MMC_R1_IDLE_STATE		0x01
#define MMC_R1_ERASE_STATE		0x02
#define MMC_R1_ILLEGAL_CMD		0x04
#define MMC_R1_COM_CRC_ERR		0x08
#define MMC_R1_ERASE_SEQ_ERR		0x01
#define MMC_R1_ADDR_ERR			0x02
#define MMC_R1_PARAM_ERR		0x04

#define MMC_R1B_WP_ERASE_SKIP		0x0002
#define MMC_R1B_ERR			0x0004
#define MMC_R1B_CC_ERR			0x0008
#define MMC_R1B_CARD_ECC_ERR		0x0010
#define MMC_R1B_WP_VIOLATION		0x0020
#define MMC_R1B_ERASE_PARAM		0x0040
#define MMC_R1B_OOR			0x0080
#define MMC_R1B_IDLE_STATE		0x0100
#define MMC_R1B_ERASE_RESET		0x0200
#define MMC_R1B_ILLEGAL_CMD		0x0400
#define MMC_R1B_COM_CRC_ERR		0x0800
#define MMC_R1B_ERASE_SEQ_ERR		0x1000
#define MMC_R1B_ADDR_ERR		0x2000
#define MMC_R1B_PARAM_ERR		0x4000

typedef struct mmc_cid
{
/* FIXME: BYTE_ORDER */
   uchar year:4,
   month:4;
   uchar sn[3];
   uchar fwrev:4,
   hwrev:4;
   uchar name[6];
   uchar id[3];
} mmc_cid_t;

typedef struct mmc_csd
{
	uint8_t		csd_structure:2,
			spec_ver:4,
			rsvd1:2;
	uint8_t		taac;
	uint8_t		nsac;
	uint8_t		tran_speed;
	uint16_t	ccc:12,
			read_bl_len:4;
	uint64_t	read_bl_partial:1,
			write_blk_misalign:1,
			read_blk_misalign:1,
			dsr_imp:1,
			rsvd2:2,
			c_size:12,
			vdd_r_curr_min:3,
			vdd_r_curr_max:3,
			vdd_w_curr_min:3,
			vdd_w_curr_max:3,
			c_size_mult:3,
			erase_blk_en:1,
			sector_size:7,
			wp_grp_size:7,
			wp_grp_enable:1,
			default_ecc:2,
			r2w_factor:3,
			write_bl_len:4,
			write_bl_partial:1,
			rsvd3:4,
			content_prot_app:1;
	uint8_t		file_format_grp:1,
			copy:1,
			perm_write_protect:1,
			tmp_write_protect:1,
			file_format:2,
			ecc:2;
} mmc_csd_t;

#endif /* __MMC_PXA_P_H__ */
