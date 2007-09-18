/*
 * U-boot - ezkit533.c
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

#include <common.h>
#if defined(CONFIG_MISC_INIT_R)
#include "psd4256.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
#if (BFIN_CPU == ADSP_BF531)
	printf("CPU:   ADSP BF531 Rev.: 0.%d\n", *pCHIPID >> 28);
#elif (BFIN_CPU == ADSP_BF532)
	printf("CPU:   ADSP BF532 Rev.: 0.%d\n", *pCHIPID >> 28);
#else
	printf("CPU:   ADSP BF533 Rev.: 0.%d\n", *pCHIPID >> 28);
#endif
	printf("Board: ADI BF533 EZ-Kit Lite board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

long int initdram(int board_type)
{
#ifdef DEBUG
	int brate;
	char *tmp = getenv("baudrate");
	brate = simple_strtoul(tmp, NULL, 16);
	printf("Serial Port initialized with Baud rate = %x\n", brate);
	printf("SDRAM attributes:\n");
	printf("tRCD %d SCLK Cycles,tRP %d SCLK Cycles,tRAS %d SCLK Cycles"
	       "tWR %d SCLK Cycles,CAS Latency %d SCLK cycles \n",
	       3, 3, 6, 2, 3);
	printf("SDRAM Begin: 0x%x\n", CFG_SDRAM_BASE);
	printf("Bank size = %d MB\n", CFG_MAX_RAM_SIZE >> 20);
#endif
	gd->bd->bi_memstart = CFG_SDRAM_BASE;
	gd->bd->bi_memsize = CFG_MAX_RAM_SIZE;
	return CFG_MAX_RAM_SIZE;
}

#if defined(CONFIG_MISC_INIT_R)
/* miscellaneous platform dependent initialisations */
int misc_init_r(void)
{
	/* Set direction bits for Video en/decoder reset as output      */
	*(volatile unsigned char *)(CFG_FLASH1_BASE + PSD_PORTA_DIR) =
	    PSDA_VDEC_RST | PSDA_VENC_RST;
	/* Deactivate Video en/decoder reset lines                      */
	*(volatile unsigned char *)(CFG_FLASH1_BASE + PSD_PORTA_DOUT) =
	    PSDA_VDEC_RST | PSDA_VENC_RST;

	return 0;
}
#endif
