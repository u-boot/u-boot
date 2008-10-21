/*
 * (C) Copyright 2005
 * BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
 *
 * Based On
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include  "cfm_flash.h"

#define PHYS_FLASH_1 CONFIG_SYS_FLASH_BASE
#define FLASH_BANK_SIZE 0x200000

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

void flash_print_info (flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (AMD_MANUFACT & FLASH_VENDMASK):
		printf ("AMD: ");
		switch (info->flash_id & FLASH_TYPEMASK) {
		case (AMD_ID_LV160B & FLASH_TYPEMASK):
			printf ("AM29LV160B (16Bit)\n");
			break;
		default:
			printf ("Unknown Chip Type\n");
			break;
		}
		break;
	case FREESCALE_MANUFACT & FLASH_VENDMASK:
		cfm_flash_print_info (info);
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	puts ("  Size: ");
	if ((info->size >> 20) > 0)
	{
		printf ("%ld MiB",info->size >> 20);
	}
	else
	{
		printf ("%ld KiB",info->size >> 10);
	}
	printf (" in %d Sectors\n", info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 4) == 0) {
			printf ("\n    ");
		}
		printf ("%02d: %08lX%s  ", i,info->start[i],
			info->protect[i] ? " P" : "  ");
	}
	printf ("\n\n");
}

unsigned long flash_init (void)
{
	int i, j;
	ulong size = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		ulong flashbase = 0;

		switch (i)
		{
		case 1:
			flash_info[i].flash_id =
				(AMD_MANUFACT & FLASH_VENDMASK) |
				(AMD_ID_LV160B & FLASH_TYPEMASK);
			flash_info[i].size = FLASH_BANK_SIZE;
			flash_info[i].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
			memset (flash_info[i].protect, 0, CONFIG_SYS_MAX_FLASH_SECT);
			flashbase = PHYS_FLASH_1;
			for (j = 0; j < flash_info[i].sector_count; j++) {
				if (j == 0) {
					/* 1st is 16 KiB */
					flash_info[i].start[j] = flashbase;
				}
				if ((j >= 1) && (j <= 2)) {
				/* 2nd and 3rd are 8 KiB */
					flash_info[i].start[j] =
						flashbase + 0x4000 + 0x2000 * (j - 1);
				}
				if (j == 3) {
					/* 4th is 32 KiB */
					flash_info[i].start[j] = flashbase + 0x8000;
				}
				if ((j >= 4) && (j <= 34)) {
					/* rest is 256 KiB */
					flash_info[i].start[j] =
						flashbase + 0x10000 + 0x10000 * (j - 4);
				}
			}
			break;
		case 0:
			cfm_flash_init (&flash_info[i]);
			break;
		default:
			panic ("configured to many flash banks!\n");
		}

		size += flash_info[i].size;
	}

	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_SYS_FLASH_BASE,
		       CONFIG_SYS_FLASH_BASE + 0xffff, &flash_info[0]);

	return size;
}

#define CMD_READ_ARRAY		0x00F0
#define CMD_UNLOCK1		0x00AA
#define CMD_UNLOCK2		0x0055
#define CMD_ERASE_SETUP		0x0080
#define CMD_ERASE_CONFIRM	0x0030
#define CMD_PROGRAM		0x00A0
#define CMD_UNLOCK_BYPASS	0x0020

#define MEM_FLASH_ADDR1		(*(volatile u16 *)(info->start[0] + (0x00000555<<1)))
#define MEM_FLASH_ADDR2		(*(volatile u16 *)(info->start[0] + (0x000002AA<<1)))


#define BIT_ERASE_DONE		0x0080
#define BIT_RDY_MASK		0x0080
#define BIT_PROGRAM_ERROR	0x0020
#define BIT_TIMEOUT		0x80000000	/* our flag */

#define ERR_READY -1

int amd_flash_erase_sector(flash_info_t * info, int sector)
{
	int state;
	ulong result;

	volatile u16 *addr =
				(volatile u16 *) (info->start[sector]);

	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	MEM_FLASH_ADDR1 = CMD_ERASE_SETUP;

	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	*addr = CMD_ERASE_CONFIRM;

	/* wait until flash is ready */
	state = 0;
	set_timer (0);

	do {
		result = *addr;

		/* check timeout */
		if (get_timer (0) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			MEM_FLASH_ADDR1 = CMD_READ_ARRAY;
			state = ERR_TIMOUT;
		}

		if (!state && (result & 0xFFFF) & BIT_ERASE_DONE)
			state = ERR_READY;
	}
	while (!state);
	if (state == ERR_READY)
		state = ERR_OK;

	MEM_FLASH_ADDR1 = CMD_READ_ARRAY;

	return state;
}

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int iflag, cflag;
	int sector;
	int rc;

	rc = ERR_OK;

	if (info->flash_id == FLASH_UNKNOWN)
	{
		rc = ERR_UNKNOWN_FLASH_TYPE;
	} /* (info->flash_id == FLASH_UNKNOWN) */

	if ((s_first < 0) || (s_first > s_last) || s_last >= info->sector_count)
	{
		rc = ERR_INVAL;
	}

	cflag = icache_status ();
	icache_disable ();
	iflag = disable_interrupts ();

	for (sector = s_first; (sector <= s_last) && (rc == ERR_OK); sector++) {

		if (info->protect[sector])
		{
			putc('P'); /*  protected sector will not erase */
		}
		else
		{
			/* erase on unprotected sector */
			puts("E\b");
			switch (info->flash_id & FLASH_VENDMASK)
			{
			case (AMD_MANUFACT & FLASH_VENDMASK):
				rc = amd_flash_erase_sector(info,sector);
				break;
			case (FREESCALE_MANUFACT & FLASH_VENDMASK):
				rc = cfm_flash_erase_sector(info,sector);
				break;
			default:
				return ERR_UNKNOWN_FLASH_VENDOR;
			}
			putc('.');
		}
	}
	if (rc!=ERR_OK)
	{
		printf ("\n   ");
		flash_perror (rc);
	}
	else
	{
		printf (" done\n");
	}

	udelay (10000);	/* allow flash to settle - wait 10 ms */

	if (iflag)
		enable_interrupts ();

	if (cflag)
		icache_enable ();

	return rc;
}

volatile static int amd_write_word (flash_info_t * info, ulong dest, u16 data)
{
	volatile u16 *addr;
	ulong result;
	int cflag, iflag;
	int state;

	/*
	 * Check if Flash is (sufficiently) erased
	 */
	addr = (volatile u16 *) dest;

	result = *addr;
	if ((result & data) != data)
		return ERR_NOT_ERASED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */

	cflag = icache_status ();
	icache_disable ();
	iflag = disable_interrupts ();

	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	MEM_FLASH_ADDR1 = CMD_PROGRAM;
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	set_timer (0);

	/* wait until flash is ready */
	state = 0;
	do {
		result = *addr;

		/* check timeout */
		if (get_timer (0) > CONFIG_SYS_FLASH_ERASE_TOUT) {
				state = ERR_TIMOUT;
		}
		if (!state && ((result & BIT_RDY_MASK) == (data & BIT_RDY_MASK)))
			state = ERR_READY;

	} while (!state);

	*addr = CMD_READ_ARRAY;

	if (state == ERR_READY)
		state = ERR_OK;
	if ((*addr != data) && (state != ERR_TIMOUT))
		state = ERR_PROG_ERROR;

	if (iflag)
		enable_interrupts ();

	if (cflag)
		icache_enable ();

	return state;
}

int amd_flash_write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	int rc;
	ulong dest;
	u16 data;

	rc = ERR_OK;
	if (addr & 1)
	{
		debug ("Byte alignment not supported\n");
		rc = ERR_ALIGN;
	}
	if (cnt & 1)
	{
		debug ("Byte transfer not supported\n");
		rc = ERR_ALIGN;
	}

	dest = addr;
	while ((cnt>=2) && (rc == ERR_OK))
	{
		data = *((volatile u16 *) src);
		rc=amd_write_word (info,dest,data);
		src +=2;
		dest +=2;
		cnt -=2;
	}
	return rc;
}

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	int rc;

	switch (info->flash_id & FLASH_VENDMASK)
	{
		case (AMD_MANUFACT & FLASH_VENDMASK):
			rc = amd_flash_write_buff(info,src,addr,cnt);
			break;
		case (FREESCALE_MANUFACT & FLASH_VENDMASK):
			rc = cfm_flash_write_buff(info,src,addr,cnt);
			break;
		default:
			rc = ERR_UNKNOWN_FLASH_VENDOR;
	}
	return rc;

}
int amd_flash_protect(flash_info_t * info,long sector,int prot)
{
	int rc;
	rc= ERR_OK;
	if (prot)
	{
		info->protect[sector]=1;
	}
	else
	{
		info->protect[sector]=0;
	}
	return rc;
}

#ifdef CONFIG_SYS_FLASH_PROTECTION

int flash_real_protect(flash_info_t * info,long sector,int prot)
{
	int rc;

	switch (info->flash_id & FLASH_VENDMASK)
	{
		case (AMD_MANUFACT & FLASH_VENDMASK):
			rc = amd_flash_protect(info,sector,prot);
			break;
		case (FREESCALE_MANUFACT & FLASH_VENDMASK):
			rc = cfm_flash_protect(info,sector,prot);
			break;
		default:
			rc = ERR_UNKNOWN_FLASH_VENDOR;
	}
	return rc;
}

#endif
