/*
 * (C) Copyright 2009 mGine co.
 * unsik Kim <donari75@gmail.com>
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

#ifndef __MG_DISK_PRV_H__
#define __MG_DISK_PRV_H__

#include <mg_disk.h>

/* name for block device */
#define MG_DISK_NAME "mgd"
/* name for platform device */
#define MG_DEV_NAME "mg_disk"

#define MG_DISK_MAJ 240
#define MG_DISK_MAX_PART 16
#define MG_SECTOR_SIZE 512
#define MG_SECTOR_SIZE_MASK (512 - 1)
#define MG_SECTOR_SIZE_SHIFT (9)
#define MG_MAX_SECTS 256

/* Register offsets */
#define MG_BUFF_OFFSET			0x8000
#define MG_STORAGE_BUFFER_SIZE		0x200
#define MG_REG_OFFSET			0xC000
#define MG_REG_FEATURE			(MG_REG_OFFSET + 2)	/* write case */
#define MG_REG_ERROR			(MG_REG_OFFSET + 2)	/* read case */
#define MG_REG_SECT_CNT			(MG_REG_OFFSET + 4)
#define MG_REG_SECT_NUM			(MG_REG_OFFSET + 6)
#define MG_REG_CYL_LOW			(MG_REG_OFFSET + 8)
#define MG_REG_CYL_HIGH			(MG_REG_OFFSET + 0xA)
#define MG_REG_DRV_HEAD			(MG_REG_OFFSET + 0xC)
#define MG_REG_COMMAND			(MG_REG_OFFSET + 0xE)	/* write case */
#define MG_REG_STATUS			(MG_REG_OFFSET + 0xE)	/* read  case */
#define MG_REG_DRV_CTRL			(MG_REG_OFFSET + 0x10)
#define MG_REG_BURST_CTRL		(MG_REG_OFFSET + 0x12)

/* "Drive Select/Head Register" bit values */
#define MG_REG_HEAD_MUST_BE_ON		0xA0 /* These 2 bits are always on */
#define MG_REG_HEAD_DRIVE_MASTER	(0x00 | MG_REG_HEAD_MUST_BE_ON)
#define MG_REG_HEAD_DRIVE_SLAVE		(0x10 | MG_REG_HEAD_MUST_BE_ON)
#define MG_REG_HEAD_LBA_MODE		(0x40 | MG_REG_HEAD_MUST_BE_ON)


/* "Device Control Register" bit values */
#define MG_REG_CTRL_INTR_ENABLE			0x0
#define MG_REG_CTRL_INTR_DISABLE		(0x1 << 1)
#define MG_REG_CTRL_RESET			(0x1 << 2)
#define MG_REG_CTRL_INTR_POLA_ACTIVE_HIGH	0x0
#define MG_REG_CTRL_INTR_POLA_ACTIVE_LOW	(0x1 << 4)
#define MG_REG_CTRL_DPD_POLA_ACTIVE_LOW		0x0
#define MG_REG_CTRL_DPD_POLA_ACTIVE_HIGH	(0x1 << 5)
#define MG_REG_CTRL_DPD_DISABLE			0x0
#define MG_REG_CTRL_DPD_ENABLE			(0x1 << 6)

/* Status register bit */
 /* error bit in status register */
#define MG_REG_STATUS_BIT_ERROR			0x01
 /* corrected error in status register */
#define MG_REG_STATUS_BIT_CORRECTED_ERROR	0x04
 /* data request bit in status register */
#define MG_REG_STATUS_BIT_DATA_REQ		0x08
 /* DSC - Drive Seek Complete */
#define MG_REG_STATUS_BIT_SEEK_DONE		0x10
 /* DWF - Drive Write Fault */
#define MG_REG_STATUS_BIT_WRITE_FAULT		0x20
#define MG_REG_STATUS_BIT_READY			0x40
#define MG_REG_STATUS_BIT_BUSY			0x80

/* handy status */
#define MG_STAT_READY	(MG_REG_STATUS_BIT_READY | MG_REG_STATUS_BIT_SEEK_DONE)
#define MG_READY_OK(s)	(((s) & (MG_STAT_READY | \
				(MG_REG_STATUS_BIT_BUSY | \
				 MG_REG_STATUS_BIT_WRITE_FAULT | \
				 MG_REG_STATUS_BIT_ERROR))) == MG_STAT_READY)

/* Error register */
#define MG_REG_ERR_AMNF		0x01
#define MG_REG_ERR_ABRT		0x04
#define MG_REG_ERR_IDNF		0x10
#define MG_REG_ERR_UNC		0x40
#define MG_REG_ERR_BBK		0x80

/* error code for others */
#define MG_ERR_NONE		0
#define MG_ERR_TIMEOUT		0x100
#define MG_ERR_INIT_STAT	0x101
#define MG_ERR_TRANSLATION	0x102
#define MG_ERR_CTRL_RST		0x103
#define MG_ERR_NO_DRV_DATA	0x104

#define MG_MAX_ERRORS	16	/* Max read/write errors/sector */
#define MG_RESET_FREQ	4	/* Reset controller every 4th retry */

/* command */
#define MG_CMD_RD	0x20
#define MG_CMD_WR	0x30
#define MG_CMD_SLEEP	0x99
#define MG_CMD_WAKEUP	0xC3
#define MG_CMD_ID	0xEC
#define MG_CMD_WR_CONF	0x3C
#define MG_CMD_RD_CONF	0x40

union mg_uniwb{
	u16 w;
	u8 b[2];
};

/* main structure for mflash driver */
struct mg_host {
	struct mg_drv_data *drv_data;
	/* for future use */
};

/*
 * Debugging macro and defines
 */
#undef DO_MG_DEBUG
#ifdef DO_MG_DEBUG
# define MG_DBG(fmt, args...) printf("%s:%d "fmt"\n", __func__, __LINE__,##args)
#else /* CONFIG_MG_DEBUG */
# define MG_DBG(fmt, args...) do { } while(0)
#endif /* CONFIG_MG_DEBUG */

#endif
