/*
 * (C) Copyright 2006 Embedded Artists AB <www.embeddedartists.com>
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

/* IAP commands use 32 bytes at the top of CPU internal sram, we
   use 512 bytes below that */
#define COPY_BUFFER_LOCATION 0x40003de0

#define IAP_LOCATION 0x7ffffff1
#define IAP_CMD_PREPARE 50
#define IAP_CMD_COPY 51
#define IAP_CMD_ERASE 52
#define IAP_CMD_CHECK 53
#define IAP_CMD_ID 54
#define IAP_CMD_VERSION 55
#define IAP_CMD_COMPARE 56

#define IAP_RET_CMD_SUCCESS 0

#define SST_BASEADDR 0x80000000
#define SST_ADDR1 ((volatile ushort*)(SST_BASEADDR + (0x5555 << 1)))
#define SST_ADDR2 ((volatile ushort*)(SST_BASEADDR + (0x2AAA << 1)))


static unsigned long command[5];
static unsigned long result[2];

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

extern void iap_entry(unsigned long * command, unsigned long * result);

/*-----------------------------------------------------------------------
 *
 */
int get_flash_sector(flash_info_t * info, ulong flash_addr)
{
	int i;

	for(i=1; i < (info->sector_count); i++) {
		if (flash_addr < (info->start[i]))
			break;
	}

	return (i-1);
}

/*-----------------------------------------------------------------------
 * This function assumes that flash_addr is aligned on 512 bytes boundary
 * in flash. This function also assumes that prepare have been called
 * for the sector in question.
 */
int copy_buffer_to_flash(flash_info_t * info, ulong flash_addr)
{
	int first_sector;
	int last_sector;

	first_sector = get_flash_sector(info, flash_addr);
	last_sector = get_flash_sector(info, flash_addr + 512 - 1);

	/* prepare sectors for write */
	command[0] = IAP_CMD_PREPARE;
	command[1] = first_sector;
	command[2] = last_sector;
	iap_entry(command, result);
	if (result[0] != IAP_RET_CMD_SUCCESS) {
		printf("IAP prepare failed\n");
		return ERR_PROG_ERROR;
	}

	command[0] = IAP_CMD_COPY;
	command[1] = flash_addr;
	command[2] = COPY_BUFFER_LOCATION;
	command[3] = 512;
	command[4] = CFG_SYS_CLK_FREQ >> 10;
	iap_entry(command, result);
	if (result[0] != IAP_RET_CMD_SUCCESS) {
		printf("IAP copy failed\n");
		return 1;
	}

	return 0;
}

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
		 CFG_ENV_ADDR,
		 CFG_ENV_ADDR + CFG_ENV_SIZE - 1,
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

/*-----------------------------------------------------------------------
 */

int flash_erase_philips (flash_info_t * info, int s_first, int s_last)
{
	int flag;
	int prot;
	int sect;

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}
	if (prot)
		return ERR_PROTECTED;


	flag = disable_interrupts();

	printf ("Erasing %d sectors starting at sector %2d.\n"
	"This make take some time ... ",
	s_last - s_first + 1, s_first);

	command[0] = IAP_CMD_PREPARE;
	command[1] = s_first;
	command[2] = s_last;
	iap_entry(command, result);
	if (result[0] != IAP_RET_CMD_SUCCESS) {
		printf("IAP prepare failed\n");
		return ERR_PROTECTED;
	}

	command[0] = IAP_CMD_ERASE;
	command[1] = s_first;
	command[2] = s_last;
	command[3] = CFG_SYS_CLK_FREQ >> 10;
	iap_entry(command, result);
	if (result[0] != IAP_RET_CMD_SUCCESS) {
		printf("IAP erase failed\n");
		return ERR_PROTECTED;
	}

	if (flag)
		enable_interrupts();

	return ERR_OK;
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
			return flash_erase_philips(info, s_first, s_last);
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

int write_buff_philips (flash_info_t * info,
			uchar * src,
			ulong addr,
			ulong cnt)
{
	int first_copy_size;
	int last_copy_size;
	int first_block;
	int last_block;
	int nbr_mid_blocks;
	uchar memmap_value;
	ulong i;
	uchar* src_org;
	uchar* dst_org;
	int ret = ERR_OK;

	src_org = src;
	dst_org = (uchar*)addr;

	first_block = addr / 512;
	last_block = (addr + cnt) / 512;
	nbr_mid_blocks = last_block - first_block - 1;

	first_copy_size = 512 - (addr % 512);
	last_copy_size = (addr + cnt) % 512;

#if 0
	printf("\ncopy first block: (1) %lX -> %lX 0x200 bytes, "
		"(2) %lX -> %lX 0x%X bytes, (3) %lX -> %lX 0x200 bytes\n",
	(ulong)(first_block * 512),
	(ulong)COPY_BUFFER_LOCATION,
	(ulong)src,
	(ulong)(COPY_BUFFER_LOCATION + 512 - first_copy_size),
	first_copy_size,
	(ulong)COPY_BUFFER_LOCATION,
	(ulong)(first_block * 512));
#endif

	/* copy first block */
	memcpy((void*)COPY_BUFFER_LOCATION,
		(void*)(first_block * 512), 512);
	memcpy((void*)(COPY_BUFFER_LOCATION + 512 - first_copy_size),
		src, first_copy_size);
	copy_buffer_to_flash(info, first_block * 512);
	src += first_copy_size;
	addr += first_copy_size;

	/* copy middle blocks */
	for (i = 0; i < nbr_mid_blocks; i++) {
#if 0
		printf("copy middle block: %lX -> %lX 512 bytes, "
		"%lX -> %lX 512 bytes\n",
		(ulong)src,
		(ulong)COPY_BUFFER_LOCATION,
		(ulong)COPY_BUFFER_LOCATION,
		(ulong)addr);
#endif
		memcpy((void*)COPY_BUFFER_LOCATION, src, 512);
		copy_buffer_to_flash(info, addr);
		src += 512;
		addr += 512;
	}


	if (last_copy_size > 0) {
#if 0
		printf("copy last block: (1) %lX -> %lX 0x200 bytes, "
		"(2) %lX -> %lX 0x%X bytes, (3) %lX -> %lX x200 bytes\n",
		(ulong)(last_block * 512),
		(ulong)COPY_BUFFER_LOCATION,
		(ulong)src,
		(ulong)(COPY_BUFFER_LOCATION),
		last_copy_size,
		(ulong)COPY_BUFFER_LOCATION,
		(ulong)addr);
#endif
		/* copy last block */
		memcpy((void*)COPY_BUFFER_LOCATION,
			(void*)(last_block * 512), 512);
		memcpy((void*)COPY_BUFFER_LOCATION,
			src, last_copy_size);
		copy_buffer_to_flash(info, addr);
	}

	/* verify write */
	memmap_value = GET8(MEMMAP);

	disable_interrupts();

	PUT8(MEMMAP, 01);		/* we must make sure that initial 64
							   bytes are taken from flash when we
							   do the compare */

	for (i = 0; i < cnt; i++) {
		if (*dst_org != *src_org){
			printf("Write failed. Byte %lX differs\n", i);
			ret = ERR_PROG_ERROR;
			break;
		}
		dst_org++;
		src_org++;
	}

	PUT8(MEMMAP, memmap_value);
	enable_interrupts();

	return ret;
}

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	switch (info->flash_id & FLASH_VENDMASK) {
		case (SST_MANUFACT & FLASH_VENDMASK):
			return write_buff_sst(info, src, addr, cnt);
		case (PHILIPS_LPC2292 & FLASH_VENDMASK):
			return write_buff_philips(info, src, addr, cnt);
		default:
			return ERR_PROG_ERROR;
	}
	return ERR_PROG_ERROR;
}
