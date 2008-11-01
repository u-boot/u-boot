/*
 * U-boot - main board file
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
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
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: ADI BF561 EZ-Kit Lite board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

phys_size_t initdram(int board_type)
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
	printf("SDRAM Begin: 0x%x\n", CONFIG_SYS_SDRAM_BASE);
	printf("Bank size = %d MB\n", CONFIG_SYS_MAX_RAM_SIZE >> 20);
#endif
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_MAX_RAM_SIZE;
	return CONFIG_SYS_MAX_RAM_SIZE;
}
