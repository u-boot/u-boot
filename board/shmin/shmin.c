/*
 * Copyright (C) 2007 - 2010
 *     Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * (C) Copyright 2000-2003
 *     Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 *
 * board/shmin/shmin.c
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
 *
 * Copy board_flash_get_legacy() from board/freescale/m54455evb/m54455evb.c
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>

int checkboard(void)
{
	puts("BOARD: T-SH7706LAN ");
	if(readb(0xb0008006) == 0xab)
		puts("v2\n");
	else
		puts("v1\n");
	return 0;
}

int board_init(void)
{
	writew(0x2980, BCR2);
	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	printf("DRAM:  %dMB\n", CONFIG_SYS_SDRAM_SIZE / (1024 * 1024));
	return 0;
}

void led_set_state(unsigned short value)
{

}

#if defined(CONFIG_FLASH_CFI_LEGACY)
#include <flash.h>
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	int sect[] = CONFIG_SYS_ATMEL_SECT;
	int sectsz[] = CONFIG_SYS_ATMEL_SECTSZ;
		int i, j, k;

	if (base != CONFIG_SYS_ATMEL_BASE)
		return 0;

	info->flash_id			= 0x01000000;
	info->portwidth			= 1;
	info->chipwidth			= 1;
	info->buffer_size		= 1;
	info->erase_blk_tout	= 16384;
	info->write_tout		= 2;
	info->buffer_write_tout	= 5;
	info->vendor			= 0xFFF0; /* CFI_CMDSET_AMD_LEGACY */
	info->cmd_reset			= 0x00F0;
	info->interface			= FLASH_CFI_X8;
	info->legacy_unlock		= 0;
	info->manufacturer_id	= (u16) ATM_MANUFACT;
	info->device_id			= ATM_ID_LV040;
	info->device_id2		= 0;
	info->ext_addr			= 0;
	info->cfi_version		= 0x3133;
	info->cfi_offset		= 0x0000;
	info->addr_unlock1		= 0x00000555;
	info->addr_unlock2		= 0x000002AA;
	info->name				= "CFI conformant";
	info->size				= 0;
	info->sector_count		= CONFIG_SYS_ATMEL_TOTALSECT;
	info->start[0] = base;

	for (k = 0, i = 0; i < CONFIG_SYS_ATMEL_REGION; i++) {
		info->size += sect[i] * sectsz[i];
		for (j = 0; j < sect[i]; j++, k++) {
			info->start[k + 1] = info->start[k] + sectsz[i];
			info->protect[k] = 0;
		}
	}

	return 1;
}
#endif /* CONFIG_FLASH_CFI_LEGACY */
