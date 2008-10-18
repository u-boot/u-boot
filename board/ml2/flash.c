/*
 * flash.c: Support code for the flash chips on the Xilinx ML2 board
 *
 * Copyright 2002 Mind NV
 *
 * http://www.mind.be/
 *
 * Author : Peter De Schrijver (p2@mind.be)
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
#include <configs/ML2.h>

#define FLASH_BANK_SIZE (64*1024*1024)

flash_info_t    flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

#define SECT_SIZE		(512*1024)

#define CMD_READ_ARRAY	0x00FF00FF00FF00FULL
#define CMD_IDENTIFY        0x0090009000900090ULL
#define CMD_ERASE_SETUP     0x0020002000200020ULL
#define CMD_ERASE_CONFIRM   0x00D000D000D000D0ULL
#define CMD_PROGRAM     0x0040004000400040ULL
#define CMD_RESUME      0x00D000D000D000D0ULL
#define CMD_SUSPEND     0x00B000B000B000B0ULL
#define CMD_STATUS_READ     0x0070007000700070ULL
#define CMD_STATUS_RESET    0x0050005000500050ULL

#define BIT_BUSY        0x0080008000800080ULL
#define BIT_ERASE_SUSPEND   0x004000400400040ULL
#define BIT_ERASE_ERROR     0x0020002000200020ULL
#define BIT_PROGRAM_ERROR   0x0010001000100010ULL
#define BIT_VPP_RANGE_ERROR 0x0008000800080008ULL
#define BIT_PROGRAM_SUSPEND 0x0004000400040004ULL
#define BIT_PROTECT_ERROR   0x0002000200020002ULL
#define BIT_UNDEFINED       0x0001000100010001ULL

#define BIT_SEQUENCE_ERROR  0x0030003000300030ULL

#define BIT_TIMEOUT     0x80000000


inline void eieio(void) {

	__asm__ __volatile__ ("eieio" : : : "memory");

}

ulong flash_init(void) {

	int i, j;
	ulong size = 0;

	for(i=0;i<CONFIG_SYS_MAX_FLASH_BANKS;i++) {
		ulong flashbase = 0;

		flash_info[i].flash_id = (INTEL_MANUFACT & FLASH_VENDMASK) |
								 (INTEL_ID_28F128J3A & FLASH_TYPEMASK);
		flash_info[i].size = FLASH_BANK_SIZE;
		flash_info[i].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
		memset(flash_info[i].protect, 0, CONFIG_SYS_MAX_FLASH_SECT);
		if (i==0)
			flashbase = CONFIG_SYS_FLASH_BASE;
		else
			panic("configured too many flash banks!\n");
		for (j = 0; j < flash_info[i].sector_count; j++)
				flash_info[i].start[j]=flashbase + j * SECT_SIZE;

		size += flash_info[i].size;
	}

	return size;
}

void flash_print_info  (flash_info_t *info) {

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
		case (INTEL_ID_28F128J3A & FLASH_TYPEMASK):
			printf("4x 28F128J3A (128Mbit)\n");
			break;
		default:
			printf("Unknown Chip Type\n");
			break;
	}

	printf("  Size: %ld MB in %d Sectors\n", info->size >> 20, info->sector_count);
	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0)
			printf("\n   ");
		printf (" %08lX%s", info->start[i],
				 info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
}

int flash_error (unsigned long long code) {

	if (code & BIT_TIMEOUT) {
		printf ("Timeout\n");
		return ERR_TIMOUT;
	}

	if (~code & BIT_BUSY) {
		printf ("Busy\n");
		return ERR_PROG_ERROR;
	}

	if (code & BIT_VPP_RANGE_ERROR) {
		printf ("Vpp range error\n");
		return ERR_PROG_ERROR;
	}

	if (code & BIT_PROTECT_ERROR) {
		printf ("Device protect error\n");
		return ERR_PROG_ERROR;
	}

	if (code & BIT_SEQUENCE_ERROR) {
		printf ("Command seqence error\n");
		return ERR_PROG_ERROR;
	}

	if (code & BIT_ERASE_ERROR) {
		printf ("Block erase error\n");
		return ERR_PROG_ERROR;
	}

	if (code & BIT_PROGRAM_ERROR) {
		printf ("Program error\n");
		return ERR_PROG_ERROR;
	}

	if (code & BIT_ERASE_SUSPEND) {
		printf ("Block erase suspended\n");
		return ERR_PROG_ERROR;
	}

	if (code & BIT_PROGRAM_SUSPEND) {
		printf ("Program suspended\n");
		return ERR_PROG_ERROR;
	}

	return ERR_OK;

}

int flash_erase (flash_info_t *info, int s_first, int s_last) {

	int rc = ERR_OK;
	int sect;
	unsigned long long result;

	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	if ((s_first < 0) || (s_first > s_last))
		return ERR_INVAL;

	if ((info->flash_id & FLASH_VENDMASK) != (INTEL_MANUFACT & FLASH_VENDMASK))
		return ERR_UNKNOWN_FLASH_VENDOR;

	for (sect=s_first; sect<=s_last; ++sect)
		if (info->protect[sect])
			return ERR_PROTECTED;

	for (sect = s_first; sect<=s_last && !ctrlc(); sect++) {
		volatile unsigned long long *addr=
									(unsigned long long *)(info->start[sect]);

		printf("Erasing sector %2d ... ", sect);

		*addr=CMD_STATUS_RESET;
		eieio();
		*addr=CMD_ERASE_SETUP;
		eieio();
		*addr=CMD_ERASE_CONFIRM;
		eieio();

		do {
			result = *addr;
		} while(~result & BIT_BUSY);

		*addr=CMD_READ_ARRAY;

		if ((rc = flash_error(result)) == ERR_OK)
			printf("ok.\n");
		else
			break;
	}

	if (ctrlc())
		printf("User Interrupt!\n");

	return rc;
}

static int write_word (flash_info_t *info, ulong dest, unsigned long long data) {

	volatile unsigned long long *addr=(unsigned long long *)dest;
	unsigned long long result;
	int rc = ERR_OK;

	result = *addr;
	if ((result & data) != data)
		return ERR_NOT_ERASED;

	*addr=CMD_STATUS_RESET;
	eieio();
	*addr=CMD_PROGRAM;
	eieio();
	*addr=data;
	eieio();

	do {
		result = *addr;
	} while(~result & BIT_BUSY);

	*addr=CMD_READ_ARRAY;

	rc = flash_error(result);

	return rc;

}

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt) {

	ulong cp, wp;
	unsigned long long data;
	int l;
	int i,rc;

	wp=(addr & ~7);

	if((l=addr-wp) != 0) {
		data=0;
		for(i=0,cp=wp;i<l;++i,++cp)
			data = (data >> 8) | (*(uchar *)cp << 24);

		for (; i<8 && cnt>0; ++i) {
			data = (data >> 8) | (*src++ << 24);
			--cnt;
			++cp;
		}

		for (; i<8; ++i, ++cp)
			data = (data >> 8) | (*(uchar *)cp << 24);

		if ((rc = write_word(info, wp, data)) != 0)
			return rc;

		wp+=8;
	}

	while(cnt>=8) {
		data = *((unsigned long long *)src);
		if ((rc = write_word(info, wp, data)) != 0)
			return rc;
		src+=8;
		wp+=8;
		cnt-=8;
	}

	if(cnt == 0)
		return ERR_OK;

	data = 0;
	for (i=0, cp=wp; i<8 && cnt>0; ++i, ++cp) {
		data = (data >> 8) | (*src++ << 24);
		--cnt;
	}
	for (; i<8; ++i, ++cp) {
		data = (data >> 8) | (*(uchar *)cp << 24);
	}

	return write_word(info, wp, data);

}
