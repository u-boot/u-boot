/*
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/ic/sc520.h>

#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
#endif

#include "hardware.h"

DECLARE_GLOBAL_DATA_PTR;

#undef SC520_CDP_DEBUG

#ifdef	SC520_CDP_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

unsigned long monitor_flash_len = CONFIG_SYS_MONITOR_LEN;

void init_sc520_enet (void)
{
	/* Set CPU Speed to 100MHz */
	sc520_mmcr->cpuctl = 0x01;

	/* wait at least one millisecond */
	asm("movl	$0x2000,%%ecx\n"
	    "0:	pushl %%ecx\n"
	    "popl	%%ecx\n"
	    "loop 0b\n": : : "ecx");

	/* turn on the SDRAM write buffer */
	sc520_mmcr->dbctl = 0x11;

	/* turn on the cache and disable write through */
	asm("movl	%%cr0, %%eax\n"
	    "andl	$0x9fffffff, %%eax\n"
	    "movl	%%eax, %%cr0\n"  : : : "eax");
}

/*
 * Miscellaneous platform dependent initializations
 */
int board_early_init_f(void)
{
	init_sc520_enet();

	sc520_mmcr->gpcsrt = 0x01;		/* GP Chip Select Recovery Time */
	sc520_mmcr->gpcspw = 0x07;		/* GP Chip Select Pulse Width */
	sc520_mmcr->gpcsoff = 0x00;		/* GP Chip Select Offset */
	sc520_mmcr->gprdw = 0x05;		/* GP Read pulse width */
	sc520_mmcr->gprdoff = 0x01;		/* GP Read offset */
	sc520_mmcr->gpwrw = 0x05;		/* GP Write pulse width */
	sc520_mmcr->gpwroff = 0x01;		/* GP Write offset */

	sc520_mmcr->piodata15_0 = 0x0630;	/* PIO15_PIO0 Data */
	sc520_mmcr->piodata31_16 = 0x2000;	/* PIO31_PIO16 Data */
	sc520_mmcr->piodir31_16 = 0x2000;	/* GPIO Direction */
	sc520_mmcr->piodir15_0 = 0x87b5;	/* GPIO Direction */
	sc520_mmcr->piopfs31_16 = 0x0dfe;	/* GPIO pin function 31-16 reg */
	sc520_mmcr->piopfs15_0 = 0x200a;	/* GPIO pin function 15-0 reg */
	sc520_mmcr->cspfs = 0x00f8;		/* Chip Select Pin Function Select */

	sc520_mmcr->par[2] = 0x200713f8;	/* Uart A (GPCS0, 0x013f8, 8 Bytes) */
	sc520_mmcr->par[3] = 0x2c0712f8;	/* Uart B (GPCS3, 0x012f8, 8 Bytes) */
	sc520_mmcr->par[4] = 0x300711f8;	/* Uart C (GPCS4, 0x011f8, 8 Bytes) */
	sc520_mmcr->par[5] = 0x340710f8;	/* Uart D (GPCS5, 0x010f8, 8 Bytes) */
	sc520_mmcr->par[6] =  0xe3ffc000;	/* SDRAM (0x00000000, 128MB) */
	sc520_mmcr->par[7] = 0xaa3fd000;	/* StrataFlash (ROMCS1, 0x10000000, 16MB) */
	sc520_mmcr->par[8] = 0xca3fd100;	/* StrataFlash (ROMCS2, 0x11000000, 16MB) */
	sc520_mmcr->par[9] = 0x4203d900;	/* SRAM (GPCS0, 0x19000000, 1MB) */
	sc520_mmcr->par[10] = 0x4e03d910;	/* SRAM (GPCS3, 0x19100000, 1MB) */
	sc520_mmcr->par[11] = 0x50018100;	/* DP-RAM (GPCS4, 0x18100000, 4kB) */
	sc520_mmcr->par[12] = 0x54020000;	/* CFLASH1 (0x200000000, 4kB) */
	sc520_mmcr->par[13] = 0x5c020001;	/* CFLASH2 (0x200010000, 4kB) */
/*	sc520_mmcr->par14 = 0x8bfff800; */	/* BOOTCS at  0x18000000 */
/*	sc520_mmcr->par15 = 0x38201000; */	/* LEDs etc (GPCS6, 0x1000, 20 Bytes */

	/* Disable Watchdog */
	sc520_mmcr->wdtmrctl = 0x3333;
	sc520_mmcr->wdtmrctl = 0xcccc;
	sc520_mmcr->wdtmrctl = 0x0000;

	/* Chip Select Configuration */
	sc520_mmcr->bootcsctl = 0x0033;
	sc520_mmcr->romcs1ctl = 0x0615;
	sc520_mmcr->romcs2ctl = 0x0615;

	sc520_mmcr->adddecctl = 0x02;
	sc520_mmcr->uart1ctl = 0x07;
	sc520_mmcr->sysarbctl = 0x06;
	sc520_mmcr->sysarbmenb = 0x0003;

	return 0;
}

int board_early_init_r(void)
{
	/* CPU Speed to 100MHz */
	gd->cpu_clk = 100000000;

	/* Crystal is 33.000MHz */
	gd->bus_clk = 33000000;

	return 0;
}

int dram_init(void)
{
	init_sc520_dram();
	return 0;
}

void show_boot_progress(int val)
{
	uchar led_mask;

	led_mask = 0x00;

	if (val < 0)
		led_mask |= LED_ERR_BITMASK;

	led_mask |= (uchar)(val & 0x001f);
	outb(led_mask, LED_LATCH_ADDRESS);
}


int last_stage_init(void)
{
	int minor;
	int major;

	major = minor = 0;

	printf("Serck Controls eNET\n");

	return 0;
}

ulong board_flash_get_legacy (ulong base, int banknum, flash_info_t * info)
{
	if (banknum == 0) {	/* non-CFI boot flash */
		info->portwidth = FLASH_CFI_8BIT;
		info->chipwidth = FLASH_CFI_BY8;
		info->interface = FLASH_CFI_X8;
		return 1;
	} else
		return 0;
}
