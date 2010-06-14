/*
 * flash.c: Support code for the flash chips on the Xilinx PELE board
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License (GPL) version 2, incorporated herein by
 * reference. Drivers based on or derived from this code fall under the GPL
 * and must retain the authorship, copyright and this license notice. This
 * file is not a complete program and may only be used when the entire program
 * is licensed under the GPL.
 *
 */

#include <common.h>
#include <asm/u-boot.h>
#include <configs/xpele.h>
#include "pl353_x8.h"

#ifndef CONFIG_SYS_NO_FLASH

#define FLASH_BANK_SIZE 	(16*1024*1024)	//16MB
#define FLASH_SIZE		CONFIG_SYS_FLASH_SIZE
#define SECT_SIZE		(64*1024)	//a.k.a BLOCK_SIZE
#define PAGE_SIZE		32

#define FLASH_STATUS_DONE               0x80
#define FLASH_STATUS_ESS                0x40
#define FLASH_STATUS_ECLBS              0x20
#define FLASH_STATUS_PSLBS              0x10
#define FLASH_STATUS_VPENS              0x08
#define FLASH_STATUS_PSS                0x04
#define FLASH_STATUS_DPS                0x02
#define FLASH_STATUS_PROTECT            0x01

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

static int flash_full_status_check(ulong addr);

static initialize = 0;

ulong flash_init(void)
{

	int i, j;
	ulong size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		ulong flashbase = 0;

		flash_info[i].flash_id = (INTEL_MANUFACT & FLASH_VENDMASK) |
		    (INTEL_ID_28F256P30T & FLASH_TYPEMASK);
		flash_info[i].size = FLASH_BANK_SIZE;
		flash_info[i].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
		memset(flash_info[i].protect, 1, CONFIG_SYS_MAX_FLASH_SECT);
		if (i == 0)
			flashbase = CONFIG_SYS_FLASH_BASE;

		for (j = 0; j < flash_info[i].sector_count; j++)
			flash_info[i].start[j] = flashbase + j * SECT_SIZE;

		size += flash_info[i].size;
	}

	return size;
}

int do_flinit(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	if (initialize) {
		printf("FLASH already initialized\r\n");
		return 1;
	}
	initialize = 1;
	printf("FLASH Initialized\r\n");
	init_nor_flash();
	return 0;
}

U_BOOT_CMD(flinit, 1, 0, do_flinit,
	   "FLASH memory initialization",
	   "\n    - flinit initialize the FLASH memory\n"
	   "flinit \n    - initialize the FLASH ");

void flash_print_info(flash_info_t * info)
{

	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (INTEL_MANUFACT & FLASH_VENDMASK):
		printf("Intel: ");
		break;
	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case (INTEL_ID_28F256P30T & FLASH_TYPEMASK):
		printf("INTEL_ID_28F256P30T 16MByte)\n");
		break;
	default:
		printf("Unknown Chip Type\n");
		break;
	}

	printf("  Size: %ld MB in %d Sectors\n", info->size >> 20,
	       info->sector_count);
	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s", info->start[i],
		       info->protect[i] ? " (RO)" : "     ");
	}
	printf("\n");
}

int flash_erase(flash_info_t * info, int s_first, int s_last)
{

	int rc = ERR_OK;
	int sect;

	if (!initialize) {
		printf("FLASH not initialized. Use flinit command. \r\n");
		return ERR_UNKNOWN_FLASH_TYPE;
	}
	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	if ((s_first < 0) || (s_first > s_last))
		return ERR_INVAL;

	if ((info->flash_id & FLASH_VENDMASK) !=
	    (INTEL_MANUFACT & FLASH_VENDMASK))
		return ERR_UNKNOWN_FLASH_VENDOR;

	for (sect = s_first; sect <= s_last; ++sect)
		if (info->protect[sect]) {
			printf("Flash is protected.\n");
			return ERR_PROTECTED;
		}
	printf("\n");
	for (sect = s_first; sect <= s_last && !ctrlc(); sect++) {
		volatile unsigned long long *addr =
		    (unsigned long long *)(info->start[sect]);

		printf("Erasing sector %d ... \n", sect);
		block_erase_nor_flash((u32) addr);
		rc = flash_full_status_check(info->start[sect]);
	}

	if (ctrlc())
		printf("User Interrupt!\n");

	return rc;
}

int flash_real_protect(flash_info_t * info, long sector, int prot)
{

	if (!initialize) {
		printf("FLASH not initialized. Use flinit command. \r\n");
		return ERR_UNKNOWN_FLASH_TYPE;
	}
	int retcode = 0;
	if (prot)
		lock_nor_flash(info->start[sector]);
	else
		unlock_nor_flash(info->start[sector]);

	info->protect[sector] = prot;
	retcode = flash_full_status_check(info->start[sector]);
	return retcode;
}

static int flash_full_status_check(ulong addr)
{
	int retcode;
	retcode = read_status_reg_nor_flash(addr);
	if (retcode != FLASH_STATUS_DONE) {
		printf("\nFlash error at address %lx with retcode=0x%x\n", addr,
		       retcode);
		if (retcode & (FLASH_STATUS_ECLBS | FLASH_STATUS_PSLBS))
			printf("Command Sequence Error.\n");
		else if (retcode & FLASH_STATUS_ECLBS)
			printf("Block Erase Error.\n");
		else if (retcode & FLASH_STATUS_PSLBS)
			printf("Locking Error\n");
		if (retcode & FLASH_STATUS_DPS)
			printf("Block locked.\n");
		if (retcode & FLASH_STATUS_VPENS)
			printf("Vpp Low Error.\n");
		retcode = 1;
	} else
		retcode = 0;

	Xil_Out8(addr, 0xFF);
	return retcode;
}

static int write_word(flash_info_t * info, ulong dest, unsigned long data)
{

	volatile unsigned char *addr1 = (unsigned char *)dest;
	volatile unsigned long *addr2 = (unsigned long *)dest;
	unsigned long result;
	int rc = ERR_OK;
	int i;
	char real_data;
	result = *addr2;
	if ((result & data) != data)
		return ERR_NOT_ERASED;
	for (i = 0; i < 4; i++) {
		real_data = (data >> (i * 8)) & 0xFF;
		write_byte_nor_flash((u32) (addr1 + i), real_data);
	}
	rc = flash_full_status_check((u32) addr1);
	return rc;

}

int write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{

	if (!initialize) {
		printf("FLASH not initialized. Use flinit command. \r\n");
		return ERR_UNKNOWN_FLASH_TYPE;
	}
#if CONFIG_SYS_FLASH_USE_BUFFER_WRITE
	ulong pages, remain;
	int i, rc = 0;
	char *src_ptr, *dst_ptr;

	src_ptr = (char *)src;
	dst_ptr = (char *)addr;
	pages = cnt / PAGE_SIZE;
	remain = cnt % PAGE_SIZE;
	if (pages) {
		for (i = 0; i < pages; i++) {
			if ((i % 500) == 0)
				putc('.');
			buffered_wirte_nor_flash((u32) dst_ptr, (u8 *) src_ptr,
						 (u16) PAGE_SIZE);
			if ((rc = flash_full_status_check((u32) dst_ptr)) != 0)
				break;
			dst_ptr += PAGE_SIZE;
			src_ptr += PAGE_SIZE;
		}
	}

	if (remain) {
		buffered_wirte_nor_flash((u32) dst_ptr, (u8 *) src_ptr,
					 (u16) remain);
		rc = flash_full_status_check((u32) dst_ptr);
	}

	printf("\nCopied 0x%x bytes from 0x%x to 0x%x\r\n", cnt, src, addr);
	return rc;

#else
	u32 *sp, *dp;
	char *src_ptr, *dst_ptr;
	u32 len, remain, i;
	int rc = 0;
	sp = (u32 *) src;
	dp = (u32 *) addr;
	len = cnt & ~0x3;
	remain = cnt % 4;

	while (len) {
		if ((len % 2000) == 0)
			putc('.');
		if ((rc = write_word(info, dp, *sp)) != 0)
			break;
		dp++;
		sp++;
		len -= 4;
	}
	src_ptr = (char *)sp;
	dst_ptr = (char *)dp;
	if (remain) {
		for (i = 0; i < remain; i++) {
			write_byte_nor_flash((dst_ptr + i), *(src_ptr + i));
		}
		rc = flash_full_status_check((u32) dst_ptr);
	}
	printf("\nCopied 0x%xbytes from 0x%x to 0x%x\r\n", cnt, src, addr);
	return rc;

#endif
}

#endif
