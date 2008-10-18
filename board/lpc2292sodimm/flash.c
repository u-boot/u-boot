/*
 * (C) Copyright 2006 Embedded Artists AB <www.embeddedartists.com>
 *
 * Modified to use the routines in cpu/arm720t/lpc2292/flash.c by
 * Gary Jennejohn <garyj@denx,de>
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
#include <asm/arch/hardware.h>

#define SST_BASEADDR 0x80000000
#define SST_ADDR1 ((volatile ushort*)(SST_BASEADDR + (0x5555 << 1)))
#define SST_ADDR2 ((volatile ushort*)(SST_BASEADDR + (0x2AAA << 1)))


flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

extern int lpc2292_copy_buffer_to_flash(flash_info_t *, ulong);
extern int lpc2292_flash_erase(flash_info_t *, int, int);
extern int lpc2292_write_buff (flash_info_t *, uchar *, ulong, ulong);

/*-----------------------------------------------------------------------
 *
 */
void write_word_sst(ulong addr, ushort data)
{
	ushort tmp;

	*SST_ADDR1 = 0x00AA;
	*SST_ADDR2 = 0x0055;
	*SST_ADDR1 = 0x00A0;
	*((volatile ushort*)addr) = data;
	/* do data polling */
	do {
		tmp = *((volatile ushort*)addr);
	} while (tmp != data);
}

/*-----------------------------------------------------------------------
 */

ulong flash_init (void)
{
	int j, k;
	ulong size = 0;
	ulong flashbase = 0;

	flash_info[0].flash_id = (PHILIPS_LPC2292 & FLASH_VENDMASK);
	flash_info[0].size = 0x003E000;	/* 256 - 8 KB */
	flash_info[0].sector_count = 17;
	memset (flash_info[0].protect, 0, 17);
	flashbase = 0x00000000;
	for (j = 0, k = 0; j < 8; j++, k++) {
		flash_info[0].start[k] = flashbase;
		flashbase += 0x00002000;
	}
	for (j = 0; j < 2; j++, k++) {
		flash_info[0].start[k] = flashbase;
		flashbase += 0x00010000;
	}
	for (j = 0; j < 7; j++, k++) {
		flash_info[0].start[k] = flashbase;
		flashbase += 0x00002000;
	}
	size += flash_info[0].size;

	flash_info[1].flash_id = (SST_MANUFACT & FLASH_VENDMASK);
	flash_info[1].size = 0x00200000; /* 2 MB */
	flash_info[1].sector_count = 512;
	memset (flash_info[1].protect, 0, 512);
	flashbase = SST_BASEADDR;
	for (j=0; j<512; j++) {
		flash_info[1].start[j] = flashbase;
		flashbase += 0x1000;	/* 4 KB sectors */
	}
	size += flash_info[1].size;

	/* Protect monitor and environment sectors */
	flash_protect (FLAG_PROTECT_SET,
		 0x0,
		 0x0 + monitor_flash_len - 1,
		 &flash_info[0]);

	flash_protect (FLAG_PROTECT_SET,
		 CONFIG_ENV_ADDR,
		 CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
		 &flash_info[0]);

	return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;
	int erased = 0;
	unsigned long j;
	unsigned long count;
	unsigned char *p;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (SST_MANUFACT & FLASH_VENDMASK):
		printf("SST: ");
		break;
	case (PHILIPS_LPC2292 & FLASH_VENDMASK):
		printf("Philips: ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	printf ("  Size: %ld KB in %d Sectors\n",
	  info->size >> 10, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}
		if (i < (info->sector_count - 1)) {
			count = info->start[i+1] - info->start[i];
		}
		else {
			count = info->start[0] + info->size - info->start[i];
		}
		p = (unsigned char*)(info->start[i]);
		erased = 1;
		for (j = 0; j < count; j++) {
			if (*p != 0xFF) {
				erased = 0;
				break;
			}
			p++;
		}
		printf (" %08lX%s%s", info->start[i], info->protect[i] ? " RO" : "   ",
			erased ? " E" : "  ");
	}
	printf ("\n");
}

int flash_erase_sst (flash_info_t * info, int s_first, int s_last)
{
	int i;

	for (i = s_first; i <= s_last; i++) {
		*SST_ADDR1 = 0x00AA;
		*SST_ADDR2 = 0x0055;
		*SST_ADDR1 = 0x0080;
		*SST_ADDR1 = 0x00AA;
		*SST_ADDR2 = 0x0055;
		*((volatile ushort*)(info->start[i])) = 0x0030;
		/* wait for erase to finish */
		udelay(25000);
	}

	return ERR_OK;
}

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	switch (info->flash_id & FLASH_VENDMASK) {
		case (SST_MANUFACT & FLASH_VENDMASK):
			return flash_erase_sst(info, s_first, s_last);
		case (PHILIPS_LPC2292 & FLASH_VENDMASK):
			return lpc2292_flash_erase(info, s_first, s_last);
		default:
			return ERR_PROTECTED;
	}
	return ERR_PROTECTED;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 *
 * cnt is in bytes
 */

int write_buff_sst (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ushort tmp;
	ulong i;
	uchar* src_org;
	uchar* dst_org;
	ulong cnt_org = cnt;
	int ret = ERR_OK;

	src_org = src;
	dst_org = (uchar*)addr;

	if (addr & 1) {		/* if odd address */
		tmp = *((uchar*)(addr - 1)); /* little endian */
		tmp |= (*src << 8);
		write_word_sst(addr - 1, tmp);
		addr += 1;
		cnt -= 1;
		src++;
	}
	while (cnt > 1) {
		tmp = ((*(src+1)) << 8) + (*src); /* little endian */
		write_word_sst(addr, tmp);
		addr += 2;
		src += 2;
		cnt -= 2;
	}
	if (cnt > 0) {
		tmp = (*((uchar*)(addr + 1))) << 8;
		tmp |= *src;
		write_word_sst(addr, tmp);
	}

	for (i = 0; i < cnt_org; i++) {
		if (*dst_org != *src_org) {
			printf("Write failed. Byte %lX differs\n", i);
			ret = ERR_PROG_ERROR;
			break;
		}
		dst_org++;
		src_org++;
	}

	return ret;
}

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	switch (info->flash_id & FLASH_VENDMASK) {
		case (SST_MANUFACT & FLASH_VENDMASK):
			return write_buff_sst(info, src, addr, cnt);
		case (PHILIPS_LPC2292 & FLASH_VENDMASK):
			return lpc2292_write_buff(info, src, addr, cnt);
		default:
			return ERR_PROG_ERROR;
	}
	return ERR_PROG_ERROR;
}
