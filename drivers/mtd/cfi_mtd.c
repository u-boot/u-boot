/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Piotr Ziecik <kosmo@semihalf.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common.h>
#include <flash.h>

#include <asm/errno.h>
#include <linux/mtd/mtd.h>

extern flash_info_t flash_info[];

static struct mtd_info cfi_mtd_info[CONFIG_SYS_MAX_FLASH_BANKS];
static char cfi_mtd_names[CONFIG_SYS_MAX_FLASH_BANKS][16];

static int cfi_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	flash_info_t *fi = mtd->priv;
	size_t a_start = fi->start[0] + instr->addr;
	size_t a_end = a_start + instr->len;
	int s_first = -1;
	int s_last = -1;
	int error, sect;

	for (sect = 0; sect < fi->sector_count; sect++) {
		if (a_start == fi->start[sect])
			s_first = sect;

		if (sect < fi->sector_count - 1) {
			if (a_end == fi->start[sect + 1]) {
				s_last = sect;
				break;
			}
		} else {
			s_last = sect;
			break;
		}
	}

	if (s_first >= 0 && s_first <= s_last) {
		instr->state = MTD_ERASING;

		flash_set_verbose(0);
		error = flash_erase(fi, s_first, s_last);
		flash_set_verbose(1);

		if (error) {
			instr->state = MTD_ERASE_FAILED;
			return -EIO;
		}

		instr->state = MTD_ERASE_DONE;
		mtd_erase_callback(instr);
		return 0;
	}

	return -EINVAL;
}

static int cfi_mtd_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	flash_info_t *fi = mtd->priv;
	u_char *f = (u_char*)(fi->start[0]) + from;

	memcpy(buf, f, len);
	*retlen = len;

	return 0;
}

static int cfi_mtd_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	flash_info_t *fi = mtd->priv;
	u_long t = fi->start[0] + to;
	int error;

	flash_set_verbose(0);
	error = write_buff(fi, (u_char*)buf, t, len);
	flash_set_verbose(1);

	if (!error) {
		*retlen = len;
		return 0;
	}

	return -EIO;
}

static void cfi_mtd_sync(struct mtd_info *mtd)
{
	/*
	 * This function should wait until all pending operations
	 * finish. However this driver is fully synchronous, so
	 * this function returns immediately
	 */
}

static int cfi_mtd_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	flash_info_t *fi = mtd->priv;

	flash_set_verbose(0);
	flash_protect(FLAG_PROTECT_SET, fi->start[0] + ofs,
					fi->start[0] + ofs + len - 1, fi);
	flash_set_verbose(1);

	return 0;
}

static int cfi_mtd_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	flash_info_t *fi = mtd->priv;

	flash_set_verbose(0);
	flash_protect(FLAG_PROTECT_CLEAR, fi->start[0] + ofs,
					fi->start[0] + ofs + len - 1, fi);
	flash_set_verbose(1);

	return 0;
}

static int cfi_mtd_set_erasesize(struct mtd_info *mtd, flash_info_t *fi)
{
	int sect_size = 0;
	int sect;

	/*
	 * Select the largest sector size as erasesize (e.g. for UBI)
	 */
	for (sect = 0; sect < fi->sector_count; sect++) {
		if (flash_sector_size(fi, sect) > sect_size)
			sect_size = flash_sector_size(fi, sect);
	}

	mtd->erasesize = sect_size;

	return 0;
}

int cfi_mtd_init(void)
{
	struct mtd_info *mtd;
	flash_info_t *fi;
	int error, i;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		fi = &flash_info[i];
		mtd = &cfi_mtd_info[i];

		memset(mtd, 0, sizeof(struct mtd_info));

		error = cfi_mtd_set_erasesize(mtd, fi);
		if (error)
			continue;

		sprintf(cfi_mtd_names[i], "nor%d", i);
		mtd->name		= cfi_mtd_names[i];
		mtd->type		= MTD_NORFLASH;
		mtd->flags		= MTD_CAP_NORFLASH;
		mtd->size		= fi->size;
		mtd->writesize		= 1;

		mtd->erase		= cfi_mtd_erase;
		mtd->read		= cfi_mtd_read;
		mtd->write		= cfi_mtd_write;
		mtd->sync		= cfi_mtd_sync;
		mtd->lock		= cfi_mtd_lock;
		mtd->unlock		= cfi_mtd_unlock;
		mtd->priv		= fi;

		if (add_mtd_device(mtd))
			return -ENOMEM;
	}

	return 0;
}
