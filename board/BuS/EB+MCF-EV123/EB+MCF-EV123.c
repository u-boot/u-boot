/*
 * (C) Copyright 2005-2009
 * BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
 *
 * (C) Copyright 2000-2003
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
#include <command.h>
#include "asm/m5282.h"
#include <bmp_layout.h>
#include <status_led.h>
#include <bus_vcxk.h>

/*---------------------------------------------------------------------------*/

DECLARE_GLOBAL_DATA_PTR;

unsigned long display_width;
unsigned long display_height;

/*---------------------------------------------------------------------------*/

int checkboard (void)
{
	puts ("Board: MCF-EV1 + MCF-EV23 (BuS Elektronik GmbH & Co. KG)\n");
#if (TEXT_BASE ==  CONFIG_SYS_INT_FLASH_BASE)
	puts ("       Boot from Internal FLASH\n");
#endif

	return 0;
}

phys_size_t initdram (int board_type)
{
	int size, i;

	size = 0;
	MCFSDRAMC_DCR = MCFSDRAMC_DCR_RTIM_6
			| MCFSDRAMC_DCR_RC ((15 * CONFIG_SYS_CLK) >> 4);
#ifdef CONFIG_SYS_SDRAM_BASE0

	MCFSDRAMC_DACR0 = MCFSDRAMC_DACR_BASE (CONFIG_SYS_SDRAM_BASE0)
			| MCFSDRAMC_DACR_CASL (1)
			| MCFSDRAMC_DACR_CBM (3)
			| MCFSDRAMC_DACR_PS_16;

	MCFSDRAMC_DMR0 = MCFSDRAMC_DMR_BAM_16M | MCFSDRAMC_DMR_V;

	MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_IP;

	*(unsigned short *) (CONFIG_SYS_SDRAM_BASE0) = 0xA5A5;
	MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_RE;
	for (i = 0; i < 2000; i++)
		asm (" nop");
	mbar_writeLong (MCFSDRAMC_DACR0,
			mbar_readLong (MCFSDRAMC_DACR0) | MCFSDRAMC_DACR_IMRS);
	*(unsigned int *) (CONFIG_SYS_SDRAM_BASE0 + 0x220) = 0xA5A5;
	size += CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
#endif
#ifdef CONFIG_SYS_SDRAM_BASE1
	MCFSDRAMC_DACR1 = MCFSDRAMC_DACR_BASE (CONFIG_SYS_SDRAM_BASE1)
			| MCFSDRAMC_DACR_CASL (1)
			| MCFSDRAMC_DACR_CBM (3)
			| MCFSDRAMC_DACR_PS_16;

	MCFSDRAMC_DMR1 = MCFSDRAMC_DMR_BAM_16M | MCFSDRAMC_DMR_V;

	MCFSDRAMC_DACR1 |= MCFSDRAMC_DACR_IP;

	*(unsigned short *) (CONFIG_SYS_SDRAM_BASE1) = 0xA5A5;
	MCFSDRAMC_DACR1 |= MCFSDRAMC_DACR_RE;

	for (i = 0; i < 2000; i++)
		asm (" nop");

	MCFSDRAMC_DACR1 |= MCFSDRAMC_DACR_IMRS;
	*(unsigned int *) (CONFIG_SYS_SDRAM_BASE1 + 0x220) = 0xA5A5;
	size += CONFIG_SYS_SDRAM_SIZE1 * 1024 * 1024;
#endif
	return size;
}

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	printf("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test passed.\n");
	return 0;
}
#endif

int misc_init_r(void)
{
#ifdef	CONFIG_HW_WATCHDOG
	hw_watchdog_init();
#endif
#ifndef CONFIG_VIDEO
	vcxk_init(16, 16);
#endif
	return 1;
}

#if defined(CONFIG_VIDEO)

/*
 ****h* EB+CPU5282-T1/drv_video_init
 * FUNCTION
 ***
 */

int drv_video_init(void)
{
	char *s;
	unsigned long splash;

	printf("Init Video as ");

	if ((s = getenv("displaywidth")) != NULL)
		display_width = simple_strtoul(s, NULL, 10);
	else
		display_width = 256;

	if ((s = getenv("displayheight")) != NULL)
		display_height = simple_strtoul(s, NULL, 10);
	else
		display_height = 256;

	printf("%lu x %lu pixel matrix\n", display_width, display_height);

	MCFCCM_CCR &= ~MCFCCM_CCR_SZEN;
	MCFGPIO_PEPAR &= ~MCFGPIO_PEPAR_PEPA2;

	vcxk_init(display_width, display_height);

#ifdef CONFIG_SPLASH_SCREEN
	if ((s = getenv("splashimage")) != NULL) {
		debug("use splashimage: %s\n", s);
		splash = simple_strtoul(s, NULL, 16);
		debug("use splashimage: %x\n", splash);
		vcxk_acknowledge_wait();
		video_display_bitmap(splash, 0, 0);
	}
#endif
	return 0;
}
#endif

/*---------------------------------------------------------------------------*/

#ifdef CONFIG_VIDEO
int do_brightness(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rcode = 0;
	ulong side;
	ulong bright;

	switch (argc) {
	case 3:
		side = simple_strtoul(argv[1], NULL, 10);
		bright = simple_strtoul(argv[2], NULL, 10);
		if ((side >= 0) && (side <= 3) &&
			(bright >= 0) && (bright <= 1000)) {
			vcxk_setbrightness(side, bright);
			rcode = 0;
		} else {
			printf("parameters out of range\n");
			printf("Usage:\n%s\n", cmdtp->usage);
			rcode = 1;
		}
		break;
	default:
		printf("Usage:\n%s\n", cmdtp->usage);
		rcode = 1;
		break;
	}
	return rcode;
}

/*---------------------------------------------------------------------------*/

U_BOOT_CMD(
	bright,	3,	0,	do_brightness,
	"sets the display brightness\n",
	" <side> <0..1000>\n        side: 0/3=both; 1=first; 2=second\n"
);

#endif

/* EOF EB+MCF-EV123.c */
