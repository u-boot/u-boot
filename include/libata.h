/*
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *		Dave Liu <daveliu@freescale.com>
 *		port from libata of linux kernel
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
 *
 */

#ifndef __LIBATA_H__
#define __LIBATA_H__

#include <common.h>

/* command
 */
enum ata_cmd {
	/* non-NCQ command */
	ATA_CMD_DEV_RESET		= 0x08, /* ATAPI device reset */
	ATA_CMD_FLUSH_CACHE		= 0xE7,
	ATA_CMD_FLUSH_CACHE_EXT		= 0xEA,
	ATA_CMD_ID_ATA			= 0xEC,
	ATA_CMD_ID_ATAPI		= 0xA1,
	ATA_CMD_READ_DMA		= 0xC8,
	ATA_CMD_READ_DMA_EXT		= 0x25,
	ATA_CMD_WRITE_DMA		= 0xCA,
	ATA_CMD_WRITE_DMA_EXT		= 0x35,
	ATA_CMD_PIO_READ		= 0x20,
	ATA_CMD_PIO_READ_EXT		= 0x24,
	ATA_CMD_PIO_WRITE		= 0x30,
	ATA_CMD_PIO_WRITE_EXT		= 0x34,
	ATA_CMD_SET_FEATURES		= 0xEF,

	/* NCQ command */
	ATA_CMD_READ_FPDMA_QUEUED	= 0x60,
	ATA_CMD_WRITE_FPDMA_QUEUED	= 0x61,
};

/* SETFEATURES stuff
 */
enum ata_set_features {
	/* SETFEATURES stuff */
	SETFEATURES_XFER	= 0x03,
	XFER_UDMA_7		= 0x47,
	XFER_UDMA_6		= 0x46,
	XFER_UDMA_5		= 0x45,
	XFER_UDMA_4		= 0x44,
	XFER_UDMA_3		= 0x43,
	XFER_UDMA_2		= 0x42,
	XFER_UDMA_1		= 0x41,
	XFER_UDMA_0		= 0x40,
	XFER_MW_DMA_4		= 0x24, /* CFA only */
	XFER_MW_DMA_3		= 0x23, /* CFA only */
	XFER_MW_DMA_2		= 0x22,
	XFER_MW_DMA_1		= 0x21,
	XFER_MW_DMA_0		= 0x20,
	XFER_PIO_6		= 0x0E, /* CFA only */
	XFER_PIO_5		= 0x0D, /* CFA only */
	XFER_PIO_4		= 0x0C,
	XFER_PIO_3		= 0x0B,
	XFER_PIO_2		= 0x0A,
	XFER_PIO_1		= 0x09,
	XFER_PIO_0		= 0x08,
	XFER_PIO_SLOW		= 0x00,

	SETFEATURES_WC_ON	= 0x02, /* Enable write cache */
	SETFEATURES_WC_OFF	= 0x82, /* Disable write cache */

	SETFEATURES_SPINUP	= 0x07, /* Spin-up drive */
};

enum ata_protocol {
	ATA_PROT_UNKNOWN,	/* unknown */
	ATA_PROT_NODATA,	/* no data */
	ATA_PROT_PIO,		/* PIO data xfer */
	ATA_PROT_DMA,		/* DMA */
	ATA_PROT_NCQ,		/* NCQ */
	ATA_PROT_ATAPI,		/* packet command, PIO data xfer */
	ATA_PROT_ATAPI_NODATA,	/* packet command, no data */
	ATA_PROT_ATAPI_DMA,	/* packet command, DMA */
};

enum ata_dev_typed {
	ATA_DEV_ATA,		/* ATA device */
	ATA_DEV_ATAPI,		/* ATAPI device */
	ATA_DEV_PMP,		/* Port Multiplier Port */
	ATA_DEV_UNKNOWN,	/* unknown */
};

enum {
	ATA_SECT_SIZE		= 512,
	ATA_MAX_SECTORS_128	= 128,
	ATA_MAX_SECTORS		= 256,
	ATA_MAX_SECTORS_LBA48	= 65535,

	/* bits in ATA command block registers */
	ATA_HOB			= (1 << 7),	/* LBA48 selector */
	ATA_NIEN		= (1 << 1),	/* disable-irq flag */
	ATA_LBA			= (1 << 6),	/* LBA28 selector */
	ATA_DEV1		= (1 << 4),	/* Select Device 1 (slave) */
	ATA_BUSY		= (1 << 7),	/* BSY status bit */
	ATA_DRDY		= (1 << 6),	/* device ready */
	ATA_DF			= (1 << 5),	/* device fault */
	ATA_DRQ			= (1 << 3),	/* data request i/o */
	ATA_ERR			= (1 << 0),	/* have an error */
	ATA_SRST		= (1 << 2),	/* software reset */
	ATA_ICRC		= (1 << 7),	/* interface CRC error */
	ATA_UNC			= (1 << 6),	/* uncorrectable media error */
	ATA_IDNF		= (1 << 4),	/* ID not found */
	ATA_ABORTED		= (1 << 2),	/* command aborted */

	ATA_ID_WORDS		= 256,
	ATA_ID_SERNO		= 10,
	ATA_ID_FW_REV		= 23,
	ATA_ID_PROD		= 27,
	ATA_ID_FIELD_VALID	= 53,
	ATA_ID_LBA_SECTORS	= 60,
	ATA_ID_MWDMA_MODES	= 63,
	ATA_ID_PIO_MODES	= 64,
	ATA_ID_QUEUE_DEPTH	= 75,
	ATA_ID_SATA_CAP		= 76,
	ATA_ID_SATA_FEATURES	= 78,
	ATA_ID_SATA_FEATURES_EN	= 79,
	ATA_ID_MAJOR_VER	= 80,
	ATA_ID_MINOR_VER	= 81,
	ATA_ID_UDMA_MODES	= 88,
	ATA_ID_LBA48_SECTORS	= 100,

	ATA_ID_SERNO_LEN	= 20,
	ATA_ID_FW_REV_LEN	= 8,
	ATA_ID_PROD_LEN		= 40,

	ATA_PIO3		= (1 << 0),
	ATA_PIO4		= ATA_PIO3 | (1 << 1),

	ATA_UDMA0		= (1 << 0),
	ATA_UDMA1		= ATA_UDMA0 | (1 << 1),
	ATA_UDMA2		= ATA_UDMA1 | (1 << 2),
	ATA_UDMA3		= ATA_UDMA2 | (1 << 3),
	ATA_UDMA4		= ATA_UDMA3 | (1 << 4),
	ATA_UDMA5		= ATA_UDMA4 | (1 << 5),
	ATA_UDMA6		= ATA_UDMA5 | (1 << 6),
	ATA_UDMA7		= ATA_UDMA6 | (1 << 7),
};

#define ata_id_is_ata(id)		(((id)[0] & (1 << 15)) == 0)
#define ata_id_wcache_enabled(id)	((id)[85] & (1 << 5))
#define ata_id_has_fua(id)		((id)[84] & (1 << 6))
#define ata_id_has_flush(id)		((id)[83] & (1 << 12))
#define ata_id_has_flush_ext(id)	((id)[83] & (1 << 13))
#define ata_id_has_lba48(id)		((id)[83] & (1 << 10))
#define ata_id_has_wcache(id)		((id)[82] & (1 << 5))
#define ata_id_has_lba(id)		((id)[49] & (1 << 9))
#define ata_id_has_dma(id)		((id)[49] & (1 << 8))
#define ata_id_has_ncq(id)		((id)[76] & (1 << 8))
#define ata_id_queue_depth(id)		(((id)[75] & 0x1f) + 1)

#define ata_id_u32(id,n)        \
        (((u32) (id)[(n) + 1] << 16) | ((u32) (id)[(n)]))
#define ata_id_u64(id,n)        \
        ( ((u64) (id)[(n) + 3] << 48) | \
          ((u64) (id)[(n) + 2] << 32) | \
          ((u64) (id)[(n) + 1] << 16) | \
          ((u64) (id)[(n) + 0]) )


static inline unsigned int ata_id_major_version(const u16 *id)
{
	unsigned int mver;

	if (id[ATA_ID_MAJOR_VER] == 0xFFFF)
		return 0;

	for (mver = 14; mver >= 1; mver--)
		if (id[ATA_ID_MAJOR_VER] & (1 << mver))
			break;
	return mver;
}

static inline int ata_id_is_sata(const u16 *id)
{
	return ata_id_major_version(id) >= 5 && id[93] == 0;
}

u64 ata_id_n_sectors(u16 *id);
u32 ata_dev_classify(u32 sig);
void ata_id_c_string(const u16 *id, unsigned char *s,
			 unsigned int ofs, unsigned int len);
void ata_dump_id(u16 *id);
void ata_swap_buf_le16(u16 *buf, unsigned int buf_words);

#endif /* __LIBATA_H__ */
