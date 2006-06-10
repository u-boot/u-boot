/*
 * Basic Flash Driver for Freescale MCF 5281/5282 internal FLASH
 *
 * (C) Copyright 2005 BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
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
#include <asm/m5282.h>
#include  "cfm_flash.h"

#if defined(CONFIG_M5281) || defined(CONFIG_M5282)

#if (CFG_CLK>20000000)
	#define CFM_CLK  (((long) CFG_CLK / (400000 * 8) + 1) | 0x40)
#else
	#define CFM_CLK  ((long) CFG_CLK / 400000 + 1)
#endif

#define cmf_backdoor_address(addr)	(((addr) & 0x0007FFFF) | 0x04000000 | \
					 (CFG_MBAR & 0xC0000000))

void cfm_flash_print_info (flash_info_t * info)
{
	printf ("Freescale: ");
	switch (info->flash_id & FLASH_TYPEMASK) {
	case FREESCALE_ID_MCF5281 & FLASH_TYPEMASK:
		printf ("MCF5281 internal FLASH\n");
		break;
	case FREESCALE_ID_MCF5282 & FLASH_TYPEMASK:
		printf ("MCF5282 internal FLASH\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		break;
	}
}

void cfm_flash_init (flash_info_t * info)
{
	int sector;
	ulong protection;
	MCFCFM_MCR = 0;
	MCFCFM_CLKD = CFM_CLK;
	debug ("CFM Clock divider: %ld (%d Hz @ %ld Hz)\n",CFM_CLK,\
	 	CFG_CLK / (2* ((CFM_CLK & 0x3F)+1) * (1+((CFM_CLK & 0x40)>>6)*7)),\
		CFG_CLK);
	MCFCFM_SACC = 0;
	MCFCFM_DACC = 0;

	if (MCFCFM_SEC & MCFCFM_SEC_KEYEN)
		puts("CFM backdoor access is enabled\n");
	if (MCFCFM_SEC & MCFCFM_SEC_SECSTAT)
		puts("CFM securety is enabled\n");

	#ifdef CONFIG_M5281
		info->flash_id = (FREESCALE_MANUFACT & FLASH_VENDMASK) |
				 (FREESCALE_ID_MCF5281 & FLASH_TYPEMASK);
		info->size = 256*1024;
		info->sector_count = 16;
	#else
		info->flash_id = (FREESCALE_MANUFACT & FLASH_VENDMASK) |
				 (FREESCALE_ID_MCF5282 & FLASH_TYPEMASK);
		info->size = 512*1024;
		info->sector_count = 32;
	#endif
	protection = MCFCFM_PROT;
	for (sector = 0; sector < info->sector_count; sector++)
	{
		if (sector == 0)
		{
			info->start[sector] = CFG_INT_FLASH_BASE;
		}
		else
		{
			info->start[sector] = info->start[sector-1] + 0x04000;
		}
		info->protect[sector] = protection & 1;
		protection >>= 1;
	}
}

int cfm_flash_readycheck(int checkblank)
{
	int	rc;
	unsigned char state;

	rc	= ERR_OK;
	while (!(MCFCFM_USTAT & MCFCFM_USTAT_CCIF));
	state = MCFCFM_USTAT;
	if (state & MCFCFM_USTAT_ACCERR)
	{
		debug ("%s(): CFM access error",__FUNCTION__);
		rc = ERR_PROG_ERROR;
	}
	if (state & MCFCFM_USTAT_PVIOL)
	{
		debug ("%s(): CFM protection violation",__FUNCTION__);
		rc = ERR_PROTECTED;
	}
	if (checkblank)
	{
		if (!(state & MCFCFM_USTAT_BLANK))
		{
			debug ("%s(): CFM erras error",__FUNCTION__);
			rc = ERR_NOT_ERASED;
		}
	}
	MCFCFM_USTAT = state & 0x34; /* reset state */
	return rc;
}

/* Erase 16KiB = 8 2KiB pages */

int cfm_flash_erase_sector (flash_info_t * info, int sector)
{
	ulong address;
	int page;
	int rc;
	rc= ERR_OK;
	address = cmf_backdoor_address(info->start[sector]);
	for (page=0; (page<8) && (rc==ERR_OK); page++)
	{
		*(volatile __u32*) address = 0;
		MCFCFM_CMD = MCFCFM_CMD_PGERS;
		MCFCFM_USTAT = MCFCFM_USTAT_CBEIF;
		rc = cfm_flash_readycheck(0);
		if (rc==ERR_OK)
		{
			*(volatile __u32*) address = 0;
			MCFCFM_CMD = MCFCFM_CMD_PGERSVER;
			MCFCFM_USTAT = MCFCFM_USTAT_CBEIF;
			rc = cfm_flash_readycheck(1);
		}
		address += 0x800;
	}
	return rc;
}

int cfm_flash_write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	int rc;
	ulong dest, data;

	rc = ERR_OK;
	if (addr & 3)
	{
		debug ("Byte and Word alignment not supported\n");
		rc = ERR_ALIGN;
	}
	if (cnt & 3)
	{
		debug ("Byte and Word transfer not supported\n");
		rc = ERR_ALIGN;
	}
	dest = cmf_backdoor_address(addr);
	while ((cnt>=4) && (rc == ERR_OK))
	{
		data =*((volatile u32 *) src);
		*(volatile u32*) dest = data;
		MCFCFM_CMD = MCFCFM_CMD_PGM;
		MCFCFM_USTAT = MCFCFM_USTAT_CBEIF;
		rc = cfm_flash_readycheck(0);
		if (*(volatile u32*) addr != data) rc = ERR_PROG_ERROR;
		src +=4;
		dest +=4;
		addr +=4;
		cnt -=4;
	}
	return rc;
}

#ifdef CFG_FLASH_PROTECTION

int cfm_flash_protect(flash_info_t * info,long sector,int prot)
{
	int rc;

	rc= ERR_OK;
	if (prot)
	{
		MCFCFM_PROT |= (1<<sector);
		info->protect[sector]=1;
	}
	else
	{
		MCFCFM_PROT &= ~(1<<sector);
		info->protect[sector]=0;
	}
	return rc;
}

#endif

#endif
