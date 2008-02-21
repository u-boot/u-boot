/*
 * U-boot - flash.c Flash driver for PSD4256GV
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 * This file is based on BF533EzFlash.c originally written by Analog Devices, Inc.
 *
 * (C) Copyright 2000-2004
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <asm/io.h>
#include "flash-defines.h"

void flash_reset(void)
{
	reset_flash();
}

unsigned long flash_get_size(ulong baseaddr, flash_info_t * info, int bank_flag)
{
	int id = 0, i = 0;
	static int FlagDev = 1;

	id = get_codes();
	if (FlagDev) {
#ifdef DEBUG
		printf("Device ID of the Flash is %x\n", id);
#endif
		FlagDev = 0;
	}
	info->flash_id = id;

	switch (bank_flag) {
	case 0:
		for (i = PriFlashABegin; i < SecFlashABegin; i++)
			info->start[i] = (baseaddr + (i * AFP_SectorSize1));
		info->size = 0x200000;
		info->sector_count = 32;
		break;
	case 1:
		info->start[0] = baseaddr + SecFlashASec1Off;
		info->start[1] = baseaddr + SecFlashASec2Off;
		info->start[2] = baseaddr + SecFlashASec3Off;
		info->start[3] = baseaddr + SecFlashASec4Off;
		info->size = 0x10000;
		info->sector_count = 4;
		break;
	case 2:
		info->start[0] = baseaddr + SecFlashBSec1Off;
		info->start[1] = baseaddr + SecFlashBSec2Off;
		info->start[2] = baseaddr + SecFlashBSec3Off;
		info->start[3] = baseaddr + SecFlashBSec4Off;
		info->size = 0x10000;
		info->sector_count = 4;
		break;
	}
	return (info->size);
}

unsigned long flash_init(void)
{
	unsigned long size_b0, size_b1, size_b2;
	int i;

	size_b0 = size_b1 = size_b2 = 0;
#ifdef DEBUG
	printf("Flash Memory Start 0x%x\n", CFG_FLASH_BASE);
	printf("Memory Map for the Flash\n");
	printf("0x20000000 - 0x200FFFFF Flash A Primary (1MB)\n");
	printf("0x20100000 - 0x201FFFFF Flash B Primary (1MB)\n");
	printf("0x20200000 - 0x2020FFFF Flash A Secondary (64KB)\n");
	printf("0x20280000 - 0x2028FFFF Flash B Secondary (64KB)\n");
	printf("Please type command flinfo for information on Sectors \n");
#endif
	for (i = 0; i < CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	size_b0 = flash_get_size(CFG_FLASH0_BASE, &flash_info[0], 0);
	size_b1 = flash_get_size(CFG_FLASH0_BASE, &flash_info[1], 1);
	size_b2 = flash_get_size(CFG_FLASH0_BASE, &flash_info[2], 2);

	if (flash_info[0].flash_id == FLASH_UNKNOWN || size_b0 == 0) {
		printf("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
		       size_b0, size_b0 >> 20);
	}

	(void)flash_protect(FLAG_PROTECT_SET, CFG_FLASH0_BASE,
			    (flash_info[0].start[2] - 1), &flash_info[0]);

	return (size_b0 + size_b1 + size_b2);
}

void flash_print_info(flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id) {
	case FLASH_PSD4256GV:
		printf("ST Microelectronics ");
		break;
	default:
		printf("Unknown Vendor: (0x%08X) ", info->flash_id);
		break;
	}
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s",
		       info->start[i], info->protect[i] ? " (RO)" : "     ");
	}
	printf("\n");
	return;
}

int flash_erase(flash_info_t * info, int s_first, int s_last)
{
	int cnt = 0, i;
	int prot, sect;

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}

	if (prot)
		printf("- Warning: %d protected sectors will not be erased!\n",
		       prot);
	else
		printf("\n");

	cnt = s_last - s_first + 1;

	if (cnt == FLASH_TOT_SECT) {
		printf("Erasing flash, Please Wait \n");
		if (erase_flash() < 0) {
			printf("Erasing flash failed \n");
			return FLASH_FAIL;
		}
	} else {
		printf("Erasing Flash locations, Please Wait\n");
		for (i = s_first; i <= s_last; i++) {
			if (info->protect[i] == 0) {	/* not protected */
				if (erase_block_flash(i, info->start[i]) < 0) {
					printf("Error Sector erasing \n");
					return FLASH_FAIL;
				}
			}
		}
	}
	return FLASH_SUCCESS;
}

int write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	int ret;
	int d;
	if (addr % 2) {
		read_flash(addr - 1 - CFG_FLASH_BASE, &d);
		d = (int)((d & 0x00FF) | (*src++ << 8));
		ret = write_data(addr - 1, 2, (uchar *) & d);
		if (ret == FLASH_FAIL)
			return ERR_NOT_ERASED;
		ret = write_data(addr + 1, cnt - 1, src);
	} else
		ret = write_data(addr, cnt, src);
	if (ret == FLASH_FAIL)
		return ERR_NOT_ERASED;
	return FLASH_SUCCESS;
}

int write_data(long lStart, long lCount, uchar * pnData)
{
	long i = 0;
	unsigned long ulOffset = lStart - CFG_FLASH_BASE;
	int d;
	int nSector = 0;
	int flag = 0;

	if (lCount % 2) {
		flag = 1;
		lCount = lCount - 1;
	}

	for (i = 0; i < lCount - 1; i += 2, ulOffset += 2) {
		get_sector_number(ulOffset, &nSector);
		read_flash(ulOffset, &d);
		if (d != 0xffff) {
			printf
			    ("Flash not erased at offset 0x%x Please erase to reprogram \n",
			     ulOffset);
			return FLASH_FAIL;
		}
		unlock_flash(ulOffset);
		d = (int)(pnData[i] | pnData[i + 1] << 8);
		write_flash(ulOffset, d);
		if (poll_toggle_bit(ulOffset) < 0) {
			printf("Error programming the flash \n");
			return FLASH_FAIL;
		}
		if ((i > 0) && (!(i % AFP_SectorSize2)))
			printf(".");
	}
	if (flag) {
		get_sector_number(ulOffset, &nSector);
		read_flash(ulOffset, &d);
		if (d != 0xffff) {
			printf
			    ("Flash not erased at offset 0x%x Please erase to reprogram \n",
			     ulOffset);
			return FLASH_FAIL;
		}
		unlock_flash(ulOffset);
		d = (int)(pnData[i] | (d & 0xFF00));
		write_flash(ulOffset, d);
		if (poll_toggle_bit(ulOffset) < 0) {
			printf("Error programming the flash \n");
			return FLASH_FAIL;
		}
	}
	return FLASH_SUCCESS;
}

int read_data(long ulStart, long lCount, long lStride, int *pnData)
{
	long i = 0;
	int j = 0;
	long ulOffset = ulStart;
	int iShift = 0;
	int iNumWords = 2;
	int nLeftover = lCount % 4;
	int nHi, nLow;
	int nSector = 0;

	for (i = 0; (i < lCount / 4) && (i < BUFFER_SIZE); i++) {
		for (iShift = 0, j = 0; j < iNumWords; j += 2) {
			if ((ulOffset >= INVALIDLOCNSTART)
			    && (ulOffset < INVALIDLOCNEND))
				return FLASH_FAIL;

			get_sector_number(ulOffset, &nSector);
			read_flash(ulOffset, &nLow);
			ulOffset += (lStride * 2);
			read_flash(ulOffset, &nHi);
			ulOffset += (lStride * 2);
			pnData[i] = (nHi << 16) | nLow;
		}
	}
	if (nLeftover > 0) {
		if ((ulOffset >= INVALIDLOCNSTART)
		    && (ulOffset < INVALIDLOCNEND))
			return FLASH_FAIL;

		get_sector_number(ulOffset, &nSector);
		read_flash(ulOffset, &pnData[i]);
	}
	return FLASH_SUCCESS;
}

int write_flash(long nOffset, int nValue)
{
	long addr;

	addr = (CFG_FLASH_BASE + nOffset);
	SSYNC();
	*(unsigned volatile short *)addr = nValue;
	SSYNC();
	if (poll_toggle_bit(nOffset) < 0)
		return FLASH_FAIL;
	return FLASH_SUCCESS;
}

int read_flash(long nOffset, int *pnValue)
{
	int nValue = 0x0;
	long addr = (CFG_FLASH_BASE + nOffset);

	if (nOffset != 0x2)
		reset_flash();
	SSYNC();
	nValue = *(volatile unsigned short *)addr;
	SSYNC();
	*pnValue = nValue;
	return TRUE;
}

int poll_toggle_bit(long lOffset)
{
	unsigned int u1, u2;
	unsigned long timeout = 0xFFFFFFFF;
	volatile unsigned long *FB =
	    (volatile unsigned long *)(0x20000000 + lOffset);
	while (1) {
		if (timeout < 0)
			break;
		u1 = *(volatile unsigned short *)FB;
		u2 = *(volatile unsigned short *)FB;
		if ((u1 & 0x0040) == (u2 & 0x0040))
			return FLASH_SUCCESS;
		if ((u2 & 0x0020) == 0x0000)
			continue;
		u1 = *(volatile unsigned short *)FB;
		if ((u2 & 0x0040) == (u1 & 0x0040))
			return FLASH_SUCCESS;
		else {
			reset_flash();
			return FLASH_FAIL;
		}
		timeout--;
	}
	printf("Time out occured \n");
	if (timeout < 0)
		return FLASH_FAIL;
}

void reset_flash(void)
{
	write_flash(WRITESEQ1, RESET_VAL);
	/* Wait for 10 micro seconds */
	udelay(10);
}

int erase_flash(void)
{
	write_flash(WRITESEQ1, WRITEDATA1);
	write_flash(WRITESEQ2, WRITEDATA2);
	write_flash(WRITESEQ3, WRITEDATA3);
	write_flash(WRITESEQ4, WRITEDATA4);
	write_flash(WRITESEQ5, WRITEDATA5);
	write_flash(WRITESEQ6, WRITEDATA6);

	if (poll_toggle_bit(0x0000) < 0)
		return FLASH_FAIL;

	write_flash(SecFlashAOff + WRITESEQ1, WRITEDATA1);
	write_flash(SecFlashAOff + WRITESEQ2, WRITEDATA2);
	write_flash(SecFlashAOff + WRITESEQ3, WRITEDATA3);
	write_flash(SecFlashAOff + WRITESEQ4, WRITEDATA4);
	write_flash(SecFlashAOff + WRITESEQ5, WRITEDATA5);
	write_flash(SecFlashAOff + WRITESEQ6, WRITEDATA6);

	if (poll_toggle_bit(SecFlashASec1Off) < 0)
		return FLASH_FAIL;

	write_flash(PriFlashBOff + WRITESEQ1, WRITEDATA1);
	write_flash(PriFlashBOff + WRITESEQ2, WRITEDATA2);
	write_flash(PriFlashBOff + WRITESEQ3, WRITEDATA3);
	write_flash(PriFlashBOff + WRITESEQ4, WRITEDATA4);
	write_flash(PriFlashBOff + WRITESEQ5, WRITEDATA5);
	write_flash(PriFlashBOff + WRITESEQ6, WRITEDATA6);

	if (poll_toggle_bit(PriFlashBOff) < 0)
		return FLASH_FAIL;

	write_flash(SecFlashBOff + WRITESEQ1, WRITEDATA1);
	write_flash(SecFlashBOff + WRITESEQ2, WRITEDATA2);
	write_flash(SecFlashBOff + WRITESEQ3, WRITEDATA3);
	write_flash(SecFlashBOff + WRITESEQ4, WRITEDATA4);
	write_flash(SecFlashBOff + WRITESEQ5, WRITEDATA5);
	write_flash(SecFlashBOff + WRITESEQ6, WRITEDATA6);

	if (poll_toggle_bit(SecFlashBOff) < 0)
		return FLASH_FAIL;

	return FLASH_SUCCESS;
}

int erase_block_flash(int nBlock, unsigned long address)
{
	long ulSectorOff = 0x0;

	if ((nBlock < 0) || (nBlock > AFP_NumSectors))
		return FALSE;

	ulSectorOff = (address - CFG_FLASH_BASE);

	write_flash((WRITESEQ1 | ulSectorOff), WRITEDATA1);
	write_flash((WRITESEQ2 | ulSectorOff), WRITEDATA2);
	write_flash((WRITESEQ3 | ulSectorOff), WRITEDATA3);
	write_flash((WRITESEQ4 | ulSectorOff), WRITEDATA4);
	write_flash((WRITESEQ5 | ulSectorOff), WRITEDATA5);

	write_flash(ulSectorOff, BlockEraseVal);

	if (poll_toggle_bit(ulSectorOff) < 0)
		return FLASH_FAIL;

	return FLASH_SUCCESS;
}

void unlock_flash(long ulOffset)
{
	unsigned long ulOffsetAddr = ulOffset;
	ulOffsetAddr &= 0xFFFF0000;

	write_flash((WRITESEQ1 | ulOffsetAddr), UNLOCKDATA1);
	write_flash((WRITESEQ2 | ulOffsetAddr), UNLOCKDATA2);
	write_flash((WRITESEQ3 | ulOffsetAddr), UNLOCKDATA3);
}

int get_codes()
{
	int dev_id = 0;

	write_flash(WRITESEQ1, GETCODEDATA1);
	write_flash(WRITESEQ2, GETCODEDATA2);
	write_flash(WRITESEQ3, GETCODEDATA3);

	read_flash(0x0002, &dev_id);
	dev_id &= 0x00FF;

	reset_flash();

	return dev_id;
}

void get_sector_number(long ulOffset, int *pnSector)
{
	int nSector = 0;

	if (ulOffset >= SecFlashAOff) {
		if ((ulOffset < SecFlashASec1Off)
		    && (ulOffset < SecFlashASec2Off)) {
			nSector = SECT32;
		} else if ((ulOffset >= SecFlashASec2Off)
			   && (ulOffset < SecFlashASec3Off)) {
			nSector = SECT33;
		} else if ((ulOffset >= SecFlashASec3Off)
			   && (ulOffset < SecFlashASec4Off)) {
			nSector = SECT34;
		} else if ((ulOffset >= SecFlashASec4Off)
			   && (ulOffset < SecFlashAEndOff)) {
			nSector = SECT35;
		}
	} else if (ulOffset >= SecFlashBOff) {
		if ((ulOffset < SecFlashBSec1Off)
		    && (ulOffset < SecFlashBSec2Off)) {
			nSector = SECT36;
		}
		if ((ulOffset < SecFlashBSec2Off)
		    && (ulOffset < SecFlashBSec3Off)) {
			nSector = SECT37;
		}
		if ((ulOffset < SecFlashBSec3Off)
		    && (ulOffset < SecFlashBSec4Off)) {
			nSector = SECT38;
		}
		if ((ulOffset < SecFlashBSec4Off)
		    && (ulOffset < SecFlashBEndOff)) {
			nSector = SECT39;
		}
	} else if ((ulOffset >= PriFlashAOff) && (ulOffset < SecFlashAOff)) {
		nSector = ulOffset & 0xffff0000;
		nSector = ulOffset >> 16;
		nSector = nSector & 0x000ff;
	}

	if ((nSector >= 0) && (nSector < AFP_NumSectors)) {
		*pnSector = nSector;
	}
}
