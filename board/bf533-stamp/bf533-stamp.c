/*
 * U-boot - stamp.c STAMP board specific routines
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
#include <asm/mem_init.h>
#include <asm/io.h>
#include "bf533-stamp.h"

DECLARE_GLOBAL_DATA_PTR;

#define STATUS_LED_OFF 0
#define STATUS_LED_ON  1

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# define SHOW_BOOT_PROGRESS(arg)	show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

int checkboard(void)
{
	printf("Board: ADI BF533 Stamp board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

phys_size_t initdram(int board_type)
{
#ifdef DEBUG
	printf("SDRAM attributes:\n");
	printf
	    ("  tRCD:%d Cycles; tRP:%d Cycles; tRAS:%d Cycles; tWR:%d Cycles; "
	     "CAS Latency:%d cycles\n", (SDRAM_tRCD >> 15), (SDRAM_tRP >> 11),
	     (SDRAM_tRAS >> 6), (SDRAM_tWR >> 19), (SDRAM_CL >> 2));
	printf("SDRAM Begin: 0x%x\n", CFG_SDRAM_BASE);
	printf("Bank size = %d MB\n", 128);
#endif
	gd->bd->bi_memstart = CFG_SDRAM_BASE;
	gd->bd->bi_memsize = CFG_MAX_RAM_SIZE;
	return (gd->bd->bi_memsize);
}

void swap_to(int device_id)
{

	if (device_id == ETHERNET) {
		*pFIO_DIR = PF0;
		SSYNC();
		*pFIO_FLAG_S = PF0;
		SSYNC();
	} else if (device_id == FLASH) {
		*pFIO_DIR = (PF4 | PF3 | PF2 | PF1 | PF0);
		*pFIO_FLAG_S = (PF4 | PF3 | PF2);
		*pFIO_MASKA_D = (PF8 | PF6 | PF5);
		*pFIO_MASKB_D = (PF7);
		*pFIO_POLAR = (PF8 | PF6 | PF5);
		*pFIO_EDGE = (PF8 | PF7 | PF6 | PF5);
		*pFIO_INEN = (PF8 | PF7 | PF6 | PF5);
		*pFIO_FLAG_D = (PF4 | PF3 | PF2);
		SSYNC();
	} else {
		printf("Unknown bank to switch\n");
	}

	return;
}

#if defined(CONFIG_MISC_INIT_R)
/* miscellaneous platform dependent initialisations */
int misc_init_r(void)
{
	int i;
	int cf_stat = 0;

	/* Check whether CF card is inserted */
	*pFIO_EDGE = FIO_EDGE_CF_BITS;
	*pFIO_POLAR = FIO_POLAR_CF_BITS;
	for (i = 0; i < 0x300; i++)
		asm("nop;");

	if ((*pFIO_FLAG_S) & CF_STAT_BITS) {
		cf_stat = 0;
	} else {
		cf_stat = 1;
	}

	*pFIO_EDGE = FIO_EDGE_BITS;
	*pFIO_POLAR = FIO_POLAR_BITS;

	if (cf_stat) {
		printf("Booting from COMPACT flash\n");

		/* Set cycle time for CF */
		*(volatile unsigned long *)ambctl1 = CF_AMBCTL1VAL;

		for (i = 0; i < 0x1000; i++)
			asm("nop;");
		for (i = 0; i < 0x1000; i++)
			asm("nop;");
		for (i = 0; i < 0x1000; i++)
			asm("nop;");

		serial_setbrg();
		ide_init();

		setenv("bootargs", "");
		setenv("bootcmd",
		       "fatload ide 0:1 0x1000000 uImage-stamp;bootm 0x1000000;bootm 0x20100000");
	} else {
		printf("Booting from FLASH\n");
	}

	return 0;
}
#endif

#ifdef CONFIG_STAMP_CF

void cf_outb(unsigned char val, volatile unsigned char *addr)
{
	/*
	 * Set PF1 PF0 respectively to 0 1 to divert address
	 * to the expansion memory banks
	 */
	*pFIO_FLAG_S = CF_PF0;
	*pFIO_FLAG_C = CF_PF1;
	SSYNC();

	*(addr) = val;
	SSYNC();

	/* Setback PF1 PF0 to 0 0 to address external
	 * memory banks  */
	*(volatile unsigned short *)pFIO_FLAG_C = CF_PF1_PF0;
	SSYNC();
}

unsigned char cf_inb(volatile unsigned char *addr)
{
	volatile unsigned char c;

	*pFIO_FLAG_S = CF_PF0;
	*pFIO_FLAG_C = CF_PF1;
	SSYNC();

	c = *(addr);
	SSYNC();

	*pFIO_FLAG_C = CF_PF1_PF0;
	SSYNC();

	return c;
}

void cf_insw(unsigned short *sect_buf, unsigned short *addr, int words)
{
	int i;

	*pFIO_FLAG_S = CF_PF0;
	*pFIO_FLAG_C = CF_PF1;
	SSYNC();

	for (i = 0; i < words; i++) {
		*(sect_buf + i) = *(addr);
		SSYNC();
	}

	*pFIO_FLAG_C = CF_PF1_PF0;
	SSYNC();
}

void cf_outsw(unsigned short *addr, unsigned short *sect_buf, int words)
{
	int i;

	*pFIO_FLAG_S = CF_PF0;
	*pFIO_FLAG_C = CF_PF1;
	SSYNC();

	for (i = 0; i < words; i++) {
		*(addr) = *(sect_buf + i);
		SSYNC();
	}

	*pFIO_FLAG_C = CF_PF1_PF0;
	SSYNC();
}
#endif

void stamp_led_set(int LED1, int LED2, int LED3)
{
	*pFIO_INEN &= ~(PF2 | PF3 | PF4);
	*pFIO_DIR |= (PF2 | PF3 | PF4);

	if (LED1 == STATUS_LED_OFF)
		*pFIO_FLAG_S = PF2;
	else
		*pFIO_FLAG_C = PF2;
	if (LED2 == STATUS_LED_OFF)
		*pFIO_FLAG_S = PF3;
	else
		*pFIO_FLAG_C = PF3;
	if (LED3 == STATUS_LED_OFF)
		*pFIO_FLAG_S = PF4;
	else
		*pFIO_FLAG_C = PF4;
	SSYNC();
}

void show_boot_progress(int status)
{
	switch (status) {
	case 1:
		stamp_led_set(STATUS_LED_OFF, STATUS_LED_OFF, STATUS_LED_ON);
		break;
	case 2:
		stamp_led_set(STATUS_LED_OFF, STATUS_LED_ON, STATUS_LED_OFF);
		break;
	case 3:
		stamp_led_set(STATUS_LED_OFF, STATUS_LED_ON, STATUS_LED_ON);
		break;
	case 4:
		stamp_led_set(STATUS_LED_ON, STATUS_LED_OFF, STATUS_LED_OFF);
		break;
	case 5:
	case 6:
		stamp_led_set(STATUS_LED_ON, STATUS_LED_OFF, STATUS_LED_ON);
		break;
	case 7:
	case 8:
		stamp_led_set(STATUS_LED_ON, STATUS_LED_ON, STATUS_LED_OFF);
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		stamp_led_set(STATUS_LED_OFF, STATUS_LED_OFF, STATUS_LED_OFF);
		break;
	default:
		stamp_led_set(STATUS_LED_ON, STATUS_LED_ON, STATUS_LED_ON);
		break;
	}
}
