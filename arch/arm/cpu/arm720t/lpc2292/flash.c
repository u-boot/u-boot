/*
 * (C) Copyright 2006 Embedded Artists AB <www.embeddedartists.com>
 *
 * Modified to remove all but the IAP-command related code by
 * Gary Jennejohn <garyj@denx.de>
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

static unsigned long command[5];
static unsigned long result[2];

extern void iap_entry(unsigned long * command, unsigned long * result);

/*-----------------------------------------------------------------------
 *
 */
static int get_flash_sector(flash_info_t * info, ulong flash_addr)
{
	int i;

	for(i = 1; i < (info->sector_count); i++) {
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
int lpc2292_copy_buffer_to_flash(flash_info_t * info, ulong flash_addr)
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
	command[4] = CONFIG_SYS_SYS_CLK_FREQ >> 10;
	iap_entry(command, result);
	if (result[0] != IAP_RET_CMD_SUCCESS) {
		printf("IAP copy failed\n");
		return 1;
	}

	return 0;
}

/*-----------------------------------------------------------------------
 */

int lpc2292_flash_erase (flash_info_t * info, int s_first, int s_last)
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
	command[3] = CONFIG_SYS_SYS_CLK_FREQ >> 10;
	iap_entry(command, result);
	if (result[0] != IAP_RET_CMD_SUCCESS) {
		printf("IAP erase failed\n");
		return ERR_PROTECTED;
	}

	if (flag)
		enable_interrupts();

	return ERR_OK;
}

int lpc2292_write_buff (flash_info_t * info, uchar * src, ulong addr,
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

	debug("\ncopy first block: (1) %lX -> %lX 0x200 bytes, "
		"(2) %lX -> %lX 0x%X bytes, (3) %lX -> %lX 0x200 bytes\n",
	(ulong)(first_block * 512),
	(ulong)COPY_BUFFER_LOCATION,
	(ulong)src,
	(ulong)(COPY_BUFFER_LOCATION + 512 - first_copy_size),
	first_copy_size,
	(ulong)COPY_BUFFER_LOCATION,
	(ulong)(first_block * 512));

	/* copy first block */
	memcpy((void*)COPY_BUFFER_LOCATION,
		(void*)(first_block * 512), 512);
	memcpy((void*)(COPY_BUFFER_LOCATION + 512 - first_copy_size),
		src, first_copy_size);
	lpc2292_copy_buffer_to_flash(info, first_block * 512);
	src += first_copy_size;
	addr += first_copy_size;

	/* copy middle blocks */
	for (i = 0; i < nbr_mid_blocks; i++) {
		debug("copy middle block: %lX -> %lX 512 bytes, "
		"%lX -> %lX 512 bytes\n",
		(ulong)src,
		(ulong)COPY_BUFFER_LOCATION,
		(ulong)COPY_BUFFER_LOCATION,
		(ulong)addr);

		memcpy((void*)COPY_BUFFER_LOCATION, src, 512);
		lpc2292_copy_buffer_to_flash(info, addr);
		src += 512;
		addr += 512;
	}


	if (last_copy_size > 0) {
		debug("copy last block: (1) %lX -> %lX 0x200 bytes, "
		"(2) %lX -> %lX 0x%X bytes, (3) %lX -> %lX x200 bytes\n",
		(ulong)(last_block * 512),
		(ulong)COPY_BUFFER_LOCATION,
		(ulong)src,
		(ulong)(COPY_BUFFER_LOCATION),
		last_copy_size,
		(ulong)COPY_BUFFER_LOCATION,
		(ulong)addr);

		/* copy last block */
		memcpy((void*)COPY_BUFFER_LOCATION,
			(void*)(last_block * 512), 512);
		memcpy((void*)COPY_BUFFER_LOCATION,
			src, last_copy_size);
		lpc2292_copy_buffer_to_flash(info, addr);
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
