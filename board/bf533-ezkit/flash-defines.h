/*
 * U-boot - flash-defines.h
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
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

#ifndef __FLASHDEFINES_H__
#define __FLASHDEFINES_H__

#include <common.h>

#define V_ULONG(a)		(*(volatile unsigned long *)( a ))
#define V_BYTE(a)		(*(volatile unsigned char *)( a ))
#define TRUE			0x1
#define FALSE			0x0
#define BUFFER_SIZE		0x80000
#define NO_COMMAND		0
#define GET_CODES		1
#define RESET			2
#define WRITE			3
#define FILL			4
#define ERASE_ALL		5
#define ERASE_SECT		6
#define READ			7
#define GET_SECTNUM		8
#define FLASH_START_L 		0x0000
#define FLASH_START_H 		0x2000
#define FLASH_TOT_SECT		40
#define FLASH_SIZE 		0x220000
#define FLASH_MAN_ST 		2
#define CFG_FLASH0_BASE		0x20000000
#define RESET_VAL		0xF0

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

int get_codes(void);
int poll_toggle_bit(long lOffset);
void reset_flash(void);
int erase_flash(void);
int erase_block_flash(int, unsigned long);
void unlock_flash(long lOffset);
int write_data(long lStart, long lCount, uchar *pnData);
int FillData(long lStart, long lCount, long lStride, int *pnData);
int read_data(long lStart, long lCount, long lStride, int *pnData);
int read_flash(long nOffset, int *pnValue);
int write_flash(long nOffset, int nValue);
void get_sector_number(long lOffset, int *pnSector);
int GetSectorProtectionStatus(flash_info_t * info, int nSector);
int GetOffset(int nBlock);
int AFP_NumSectors = 40;
long AFP_SectorSize1 = 0x10000;
int AFP_SectorSize2 = 0x4000;

#define WRITESEQ1		0x0AAA
#define WRITESEQ2		0x0554
#define WRITESEQ3		0x0AAA
#define WRITESEQ4		0x0AAA
#define WRITESEQ5		0x0554
#define WRITESEQ6		0x0AAA
#define WRITEDATA1		0xaa
#define WRITEDATA2		0x55
#define WRITEDATA3		0x80
#define WRITEDATA4		0xaa
#define WRITEDATA5		0x55
#define WRITEDATA6		0x10
#define PriFlashABegin		0
#define SecFlashABegin		32
#define SecFlashBBegin		36
#define PriFlashAOff		0x0
#define PriFlashBOff		0x100000
#define SecFlashAOff		0x200000
#define SecFlashBOff		0x280000
#define INVALIDLOCNSTART	0x20270000
#define INVALIDLOCNEND		0x20280000
#define BlockEraseVal		0x30
#define UNLOCKDATA1		0xaa
#define UNLOCKDATA2		0x55
#define UNLOCKDATA3		0xa0
#define GETCODEDATA1		0xaa
#define GETCODEDATA2		0x55
#define GETCODEDATA3		0x90
#define SecFlashASec1Off	0x200000
#define SecFlashASec2Off	0x204000
#define SecFlashASec3Off	0x206000
#define SecFlashASec4Off	0x208000
#define SecFlashAEndOff		0x210000
#define SecFlashBSec1Off	0x280000
#define SecFlashBSec2Off	0x284000
#define SecFlashBSec3Off	0x286000
#define SecFlashBSec4Off	0x288000
#define SecFlashBEndOff		0x290000

#define SECT32			32
#define SECT33			33
#define SECT34			34
#define SECT35			35
#define SECT36			36
#define SECT37			37
#define SECT38			38
#define SECT39			39

#define FLASH_SUCCESS	0
#define FLASH_FAIL	-1

#endif
