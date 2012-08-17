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

#include <common.h>
#include <malloc.h>
#include <part.h>
#include <ata.h>
#include <asm/io.h>
#include "mg_disk_prv.h"

#ifndef CONFIG_MG_DISK_RES
#define CONFIG_MG_DISK_RES	0
#endif

#define MG_RES_SEC ((CONFIG_MG_DISK_RES) << 1)

static struct mg_host host;

static inline u32 mg_base(void)
{
	return host.drv_data->base;
}

static block_dev_desc_t mg_disk_dev = {
	.if_type = IF_TYPE_ATAPI,
	.part_type = PART_TYPE_UNKNOWN,
	.type = DEV_TYPE_HARDDISK,
	.blksz = MG_SECTOR_SIZE,
	.priv = &host };

static void mg_dump_status (const char *msg, unsigned int stat, unsigned err)
{
	char *name = MG_DEV_NAME;

	printf("%s: %s: status=0x%02x { ", name, msg, stat & 0xff);
	if (stat & MG_REG_STATUS_BIT_BUSY)
		printf("Busy ");
	if (stat & MG_REG_STATUS_BIT_READY)
		printf("DriveReady ");
	if (stat & MG_REG_STATUS_BIT_WRITE_FAULT)
		printf("WriteFault ");
	if (stat & MG_REG_STATUS_BIT_SEEK_DONE)
		printf("SeekComplete ");
	if (stat & MG_REG_STATUS_BIT_DATA_REQ)
		printf("DataRequest ");
	if (stat & MG_REG_STATUS_BIT_CORRECTED_ERROR)
		printf("CorrectedError ");
	if (stat & MG_REG_STATUS_BIT_ERROR)
		printf("Error ");
	printf("}\n");

	if ((stat & MG_REG_STATUS_BIT_ERROR)) {
		printf("%s: %s: error=0x%02x { ", name, msg, err & 0xff);
		if (err & MG_REG_ERR_BBK)
			printf("BadSector ");
		if (err & MG_REG_ERR_UNC)
			printf("UncorrectableError ");
		if (err & MG_REG_ERR_IDNF)
			printf("SectorIdNotFound ");
		if (err & MG_REG_ERR_ABRT)
			printf("DriveStatusError ");
		if (err & MG_REG_ERR_AMNF)
			printf("AddrMarkNotFound ");
		printf("}\n");
	}
}

static unsigned int mg_wait (u32 expect, u32 msec)
{
	u8 status;
	u32 from, cur, err;

	err = MG_ERR_NONE;
#ifdef CONFIG_SYS_LOW_RES_TIMER
	reset_timer();
#endif
	from = get_timer(0);

	status = readb(mg_base() + MG_REG_STATUS);
	do {
		cur = get_timer(from);
		if (status & MG_REG_STATUS_BIT_BUSY) {
			if (expect == MG_REG_STATUS_BIT_BUSY)
				break;
		} else {
			/* Check the error condition! */
			if (status & MG_REG_STATUS_BIT_ERROR) {
				err = readb(mg_base() + MG_REG_ERROR);
				mg_dump_status("mg_wait", status, err);
				break;
			}

			if (expect == MG_STAT_READY)
				if (MG_READY_OK(status))
					break;

			if (expect == MG_REG_STATUS_BIT_DATA_REQ)
				if (status & MG_REG_STATUS_BIT_DATA_REQ)
					break;
		}
		status = readb(mg_base() + MG_REG_STATUS);
	} while (cur < msec);

	if (cur >= msec)
		err = MG_ERR_TIMEOUT;

	return err;
}

static int mg_get_disk_id (void)
{
	u16 id[(MG_SECTOR_SIZE / sizeof(u16))];
	hd_driveid_t *iop = (hd_driveid_t *)id;
	u32 i, err, res;

	writeb(MG_CMD_ID, mg_base() + MG_REG_COMMAND);
	err = mg_wait(MG_REG_STATUS_BIT_DATA_REQ, 3000);
	if (err)
		return err;

	for(i = 0; i < (MG_SECTOR_SIZE / sizeof(u16)); i++)
		id[i] = readw(mg_base() + MG_BUFF_OFFSET + i * 2);

	writeb(MG_CMD_RD_CONF, mg_base() + MG_REG_COMMAND);
	err = mg_wait(MG_STAT_READY, 3000);
	if (err)
		return err;

	ata_swap_buf_le16(id, MG_SECTOR_SIZE / sizeof(u16));

	if((iop->field_valid & 1) == 0)
		return MG_ERR_TRANSLATION;

	ata_id_c_string(id, (unsigned char *)mg_disk_dev.revision,
			ATA_ID_FW_REV, sizeof(mg_disk_dev.revision));
	ata_id_c_string(id, (unsigned char *)mg_disk_dev.vendor,
			ATA_ID_PROD, sizeof(mg_disk_dev.vendor));
	ata_id_c_string(id, (unsigned char *)mg_disk_dev.product,
			ATA_ID_SERNO, sizeof(mg_disk_dev.product));

#ifdef __BIG_ENDIAN
	iop->lba_capacity = (iop->lba_capacity << 16) |
		(iop->lba_capacity >> 16);
#endif /* __BIG_ENDIAN */

	if (MG_RES_SEC) {
		MG_DBG("MG_RES_SEC=%d\n", MG_RES_SEC);
		iop->cyls = (iop->lba_capacity - MG_RES_SEC) /
			iop->sectors / iop->heads;
		res = iop->lba_capacity -
			iop->cyls * iop->heads * iop->sectors;
		iop->lba_capacity -= res;
		printf("mg_disk: %d sectors reserved\n", res);
	}

	mg_disk_dev.lba = iop->lba_capacity;
	return MG_ERR_NONE;
}

static int mg_disk_reset (void)
{
	struct mg_drv_data *prv_data = host.drv_data;
	s32 err;
	u8 init_status;

	/* hdd rst low */
	prv_data->mg_hdrst_pin(0);
	err = mg_wait(MG_REG_STATUS_BIT_BUSY, 300);
	if(err)
		return err;

	/* hdd rst high */
	prv_data->mg_hdrst_pin(1);
	err = mg_wait(MG_STAT_READY, 3000);
	if(err)
		return err;

	/* soft reset on */
	writeb(MG_REG_CTRL_RESET | MG_REG_CTRL_INTR_DISABLE,
		mg_base() + MG_REG_DRV_CTRL);
	err = mg_wait(MG_REG_STATUS_BIT_BUSY, 3000);
	if(err)
		return err;

	/* soft reset off */
	writeb(MG_REG_CTRL_INTR_DISABLE, mg_base() + MG_REG_DRV_CTRL);
	err = mg_wait(MG_STAT_READY, 3000);
	if(err)
		return err;

	init_status = readb(mg_base() + MG_REG_STATUS) & 0xf;

	if (init_status == 0xf)
		return MG_ERR_INIT_STAT;

	return err;
}


static unsigned int mg_out(unsigned int sect_num,
			unsigned int sect_cnt,
			unsigned int cmd)
{
	u32 err = MG_ERR_NONE;

	err = mg_wait(MG_STAT_READY, 3000);
	if (err)
		return err;

	writeb((u8)sect_cnt, mg_base() + MG_REG_SECT_CNT);
	writeb((u8)sect_num, mg_base() + MG_REG_SECT_NUM);
	writeb((u8)(sect_num >> 8), mg_base() + MG_REG_CYL_LOW);
	writeb((u8)(sect_num >> 16), mg_base() + MG_REG_CYL_HIGH);
	writeb((u8)((sect_num >> 24) | MG_REG_HEAD_LBA_MODE),
		mg_base() + MG_REG_DRV_HEAD);
	writeb(cmd, mg_base() + MG_REG_COMMAND);

	return err;
}

static unsigned int mg_do_read_sects(void *buff, u32 sect_num, u32 sect_cnt)
{
	u32 i, j, err;
	u8 *buff_ptr = buff;
	union mg_uniwb uniwb;

	err = mg_out(sect_num, sect_cnt, MG_CMD_RD);
	if (err)
		return err;

	for (i = 0; i < sect_cnt; i++) {
		err = mg_wait(MG_REG_STATUS_BIT_DATA_REQ, 3000);
		if (err)
			return err;

		if ((u32)buff_ptr & 1) {
			for (j = 0; j < MG_SECTOR_SIZE >> 1; j++) {
				uniwb.w = readw(mg_base() + MG_BUFF_OFFSET
						+ (j << 1));
				*buff_ptr++ = uniwb.b[0];
				*buff_ptr++ = uniwb.b[1];
			}
		} else {
			for(j = 0; j < MG_SECTOR_SIZE >> 1; j++) {
				*(u16 *)buff_ptr = readw(mg_base() +
						MG_BUFF_OFFSET + (j << 1));
				buff_ptr += 2;
			}
		}
		writeb(MG_CMD_RD_CONF, mg_base() + MG_REG_COMMAND);

		MG_DBG("%u (0x%8.8x) sector read", sect_num + i,
			(sect_num + i) * MG_SECTOR_SIZE);
	}

	return err;
}

unsigned int mg_disk_read_sects(void *buff, u32 sect_num, u32 sect_cnt)
{
	u32 quotient, residue, i, err;
	u8 *buff_ptr = buff;

	quotient = sect_cnt >> 8;
	residue = sect_cnt % 256;

	for (i = 0; i < quotient; i++) {
		MG_DBG("sect num : %u buff : 0x%8.8x", sect_num, (u32)buff_ptr);
		err = mg_do_read_sects(buff_ptr, sect_num, 256);
		if (err)
			return err;
		sect_num += 256;
		buff_ptr += 256 * MG_SECTOR_SIZE;
	}

	if (residue) {
		MG_DBG("sect num : %u buff : %8.8x", sect_num, (u32)buff_ptr);
		err = mg_do_read_sects(buff_ptr, sect_num, residue);
	}

	return err;
}

unsigned long mg_block_read (int dev, unsigned long start,
		lbaint_t blkcnt, void *buffer)
{
	start += MG_RES_SEC;
	if (! mg_disk_read_sects(buffer, start, blkcnt))
		return blkcnt;
	else
		return 0;
}

unsigned int mg_disk_read (u32 addr, u8 *buff, u32 len)
{
	u8 *sect_buff, *buff_ptr = buff;
	u32 cur_addr, next_sec_addr, end_addr, cnt, sect_num;
	u32 err = MG_ERR_NONE;

	/* TODO : sanity chk */
	cnt = 0;
	cur_addr = addr;
	end_addr = addr + len;

	sect_buff = malloc(MG_SECTOR_SIZE);

	if (cur_addr & MG_SECTOR_SIZE_MASK) {
		next_sec_addr = (cur_addr + MG_SECTOR_SIZE) &
				~MG_SECTOR_SIZE_MASK;
		sect_num = cur_addr >> MG_SECTOR_SIZE_SHIFT;
		err = mg_disk_read_sects(sect_buff, sect_num, 1);
		if (err)
			goto mg_read_exit;

		if (end_addr < next_sec_addr) {
			memcpy(buff_ptr,
				sect_buff + (cur_addr & MG_SECTOR_SIZE_MASK),
				end_addr - cur_addr);
			MG_DBG("copies %u byte from sector offset 0x%8.8x",
				end_addr - cur_addr, cur_addr);
			cur_addr = end_addr;
		} else {
			memcpy(buff_ptr,
				sect_buff + (cur_addr & MG_SECTOR_SIZE_MASK),
				next_sec_addr - cur_addr);
			MG_DBG("copies %u byte from sector offset 0x%8.8x",
				next_sec_addr - cur_addr, cur_addr);
			buff_ptr += (next_sec_addr - cur_addr);
			cur_addr = next_sec_addr;
		}
	}

	if (cur_addr < end_addr) {
		sect_num = cur_addr >> MG_SECTOR_SIZE_SHIFT;
		cnt = ((end_addr & ~MG_SECTOR_SIZE_MASK) - cur_addr) >>
			MG_SECTOR_SIZE_SHIFT;

		if (cnt)
			err = mg_disk_read_sects(buff_ptr, sect_num, cnt);
		if (err)
			goto mg_read_exit;

		buff_ptr += cnt * MG_SECTOR_SIZE;
		cur_addr += cnt * MG_SECTOR_SIZE;

		if (cur_addr < end_addr) {
			sect_num = cur_addr >> MG_SECTOR_SIZE_SHIFT;
			err = mg_disk_read_sects(sect_buff, sect_num, 1);
			if (err)
				goto mg_read_exit;
			memcpy(buff_ptr, sect_buff, end_addr - cur_addr);
			MG_DBG("copies %u byte", end_addr - cur_addr);
		}
	}

mg_read_exit:
	free(sect_buff);

	return err;
}
static int mg_do_write_sects(void *buff, u32 sect_num, u32 sect_cnt)
{
	u32 i, j, err;
	u8 *buff_ptr = buff;
	union mg_uniwb uniwb;

	err = mg_out(sect_num, sect_cnt, MG_CMD_WR);
	if (err)
		return err;

	for (i = 0; i < sect_cnt; i++) {
		err = mg_wait(MG_REG_STATUS_BIT_DATA_REQ, 3000);
		if (err)
			return err;

		if ((u32)buff_ptr & 1) {
			uniwb.b[0] = *buff_ptr++;
			uniwb.b[1] = *buff_ptr++;
			writew(uniwb.w, mg_base() + MG_BUFF_OFFSET + (j << 1));
		} else {
			for(j = 0; j < MG_SECTOR_SIZE >> 1; j++) {
				writew(*(u16 *)buff_ptr,
						mg_base() + MG_BUFF_OFFSET +
						(j << 1));
				buff_ptr += 2;
			}
		}
		writeb(MG_CMD_WR_CONF, mg_base() + MG_REG_COMMAND);

		MG_DBG("%u (0x%8.8x) sector write",
			sect_num + i, (sect_num + i) * MG_SECTOR_SIZE);
	}

	return err;
}

unsigned int mg_disk_write_sects(void *buff, u32 sect_num, u32 sect_cnt)
{
	u32 quotient, residue, i;
	u32 err = MG_ERR_NONE;
	u8 *buff_ptr = buff;

	quotient = sect_cnt >> 8;
	residue = sect_cnt % 256;

	for (i = 0; i < quotient; i++) {
		MG_DBG("sect num : %u buff : %8.8x", sect_num, (u32)buff_ptr);
		err = mg_do_write_sects(buff_ptr, sect_num, 256);
		if (err)
			return err;
		sect_num += 256;
		buff_ptr += 256 * MG_SECTOR_SIZE;
	}

	if (residue) {
		MG_DBG("sect num : %u buff : %8.8x", sect_num, (u32)buff_ptr);
		err = mg_do_write_sects(buff_ptr, sect_num, residue);
	}

	return err;
}

unsigned long mg_block_write (int dev, unsigned long start,
		lbaint_t blkcnt, const void *buffer)
{
	start += MG_RES_SEC;
	if (!mg_disk_write_sects((void *)buffer, start, blkcnt))
		return blkcnt;
	else
		return 0;
}

unsigned int mg_disk_write(u32 addr, u8 *buff, u32 len)
{
	u8 *sect_buff, *buff_ptr = buff;
	u32 cur_addr, next_sec_addr, end_addr, cnt, sect_num;
	u32 err = MG_ERR_NONE;

	/* TODO : sanity chk */
	cnt = 0;
	cur_addr = addr;
	end_addr = addr + len;

	sect_buff = malloc(MG_SECTOR_SIZE);

	if (cur_addr & MG_SECTOR_SIZE_MASK) {

		next_sec_addr = (cur_addr + MG_SECTOR_SIZE) &
				~MG_SECTOR_SIZE_MASK;
		sect_num = cur_addr >> MG_SECTOR_SIZE_SHIFT;
		err = mg_disk_read_sects(sect_buff, sect_num, 1);
		if (err)
			goto mg_write_exit;

		if (end_addr < next_sec_addr) {
			memcpy(sect_buff + (cur_addr & MG_SECTOR_SIZE_MASK),
				buff_ptr, end_addr - cur_addr);
			MG_DBG("copies %u byte to sector offset 0x%8.8x",
				end_addr - cur_addr, cur_addr);
			cur_addr = end_addr;
		} else {
			memcpy(sect_buff + (cur_addr & MG_SECTOR_SIZE_MASK),
				buff_ptr, next_sec_addr - cur_addr);
			MG_DBG("copies %u byte to sector offset 0x%8.8x",
				next_sec_addr - cur_addr, cur_addr);
			buff_ptr += (next_sec_addr - cur_addr);
			cur_addr = next_sec_addr;
		}

		err = mg_disk_write_sects(sect_buff, sect_num, 1);
		if (err)
			goto mg_write_exit;
	}

	if (cur_addr < end_addr) {

		sect_num = cur_addr >> MG_SECTOR_SIZE_SHIFT;
		cnt = ((end_addr & ~MG_SECTOR_SIZE_MASK) - cur_addr) >>
			MG_SECTOR_SIZE_SHIFT;

		if (cnt)
			err = mg_disk_write_sects(buff_ptr, sect_num, cnt);
		if (err)
			goto mg_write_exit;

		buff_ptr += cnt * MG_SECTOR_SIZE;
		cur_addr += cnt * MG_SECTOR_SIZE;

		if (cur_addr < end_addr) {
			sect_num = cur_addr >> MG_SECTOR_SIZE_SHIFT;
			err = mg_disk_read_sects(sect_buff, sect_num, 1);
			if (err)
				goto mg_write_exit;
			memcpy(sect_buff, buff_ptr, end_addr - cur_addr);
			MG_DBG("copies %u byte", end_addr - cur_addr);
			err = mg_disk_write_sects(sect_buff, sect_num, 1);
		}

	}

mg_write_exit:
	free(sect_buff);

	return err;
}

#ifdef CONFIG_PARTITIONS
block_dev_desc_t *mg_disk_get_dev(int dev)
{
	return ((block_dev_desc_t *) & mg_disk_dev);
}
#endif

/* must override this function */
struct mg_drv_data * __attribute__((weak)) mg_get_drv_data (void)
{
	puts ("### WARNING ### port mg_get_drv_data function\n");
	return NULL;
}

unsigned int mg_disk_init (void)
{
	struct mg_drv_data *prv_data;
	u32 err = MG_ERR_NONE;

	prv_data = mg_get_drv_data();
	if (! prv_data) {
		printf("%s:%d fail (no driver_data)\n", __func__, __LINE__);
		err = MG_ERR_NO_DRV_DATA;
		return err;
	}

	((struct mg_host *)mg_disk_dev.priv)->drv_data = prv_data;

	/* init ctrl pin */
	if (prv_data->mg_ctrl_pin_init)
		prv_data->mg_ctrl_pin_init();

	if (! prv_data->mg_hdrst_pin) {
		err = MG_ERR_CTRL_RST;
		return err;
	}

	/* disk reset */
	err = mg_disk_reset();
	if (err) {
		printf("%s:%d fail (err code : %d)\n", __func__, __LINE__, err);
		return err;
	}

	/* get disk id */
	err = mg_get_disk_id();
	if (err) {
		printf("%s:%d fail (err code : %d)\n", __func__, __LINE__, err);
		return err;
	}

	mg_disk_dev.block_read = mg_block_read;
	mg_disk_dev.block_write = mg_block_write;

	init_part(&mg_disk_dev);

	dev_print(&mg_disk_dev);

	return err;
}
