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
	writeb(0x01, &sc520_mmcr->cpuctl);

	/* wait at least one millisecond */
	asm("movl	$0x2000,%%ecx\n"
	    "0:	pushl %%ecx\n"
	    "popl	%%ecx\n"
	    "loop 0b\n": : : "ecx");

	/* turn on the SDRAM write buffer */
	writeb(0x11, &sc520_mmcr->dbctl);

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

	writeb(0x01, &sc520_mmcr->gpcsrt);		/* GP Chip Select Recovery Time */
	writeb(0x07, &sc520_mmcr->gpcspw);		/* GP Chip Select Pulse Width */
	writeb(0x00, &sc520_mmcr->gpcsoff);		/* GP Chip Select Offset */
	writeb(0x05, &sc520_mmcr->gprdw);		/* GP Read pulse width */
	writeb(0x01, &sc520_mmcr->gprdoff);		/* GP Read offset */
	writeb(0x05, &sc520_mmcr->gpwrw);		/* GP Write pulse width */
	writeb(0x01, &sc520_mmcr->gpwroff);		/* GP Write offset */

	writew(0x0630, &sc520_mmcr->piodata15_0);	/* PIO15_PIO0 Data */
	writew(0x2000, &sc520_mmcr->piodata31_16);	/* PIO31_PIO16 Data */
	writew(0x2000, &sc520_mmcr->piodir31_16);	/* GPIO Direction */
	writew(0x87b5, &sc520_mmcr->piodir15_0);	/* GPIO Direction */
	writew(0x0dfe, &sc520_mmcr->piopfs31_16);	/* GPIO pin function 31-16 reg */
	writew(0x200a, &sc520_mmcr->piopfs15_0);	/* GPIO pin function 15-0 reg */
	writeb(0xf8, &sc520_mmcr->cspfs);		/* Chip Select Pin Function Select */

	writel(0x200713f8, &sc520_mmcr->par[2]);	/* Uart A (GPCS0, 0x013f8, 8 Bytes) */
	writel(0x2c0712f8, &sc520_mmcr->par[3]);	/* Uart B (GPCS3, 0x012f8, 8 Bytes) */
	writel(0x300711f8, &sc520_mmcr->par[4]);	/* Uart C (GPCS4, 0x011f8, 8 Bytes) */
	writel(0x340710f8, &sc520_mmcr->par[5]);	/* Uart D (GPCS5, 0x010f8, 8 Bytes) */
	writel(0xe3ffc000, &sc520_mmcr->par[6]);	/* SDRAM (0x00000000, 128MB) */
	writel(0xaa3fd000, &sc520_mmcr->par[7]);	/* StrataFlash (ROMCS1, 0x10000000, 16MB) */
	writel(0xca3fd100, &sc520_mmcr->par[8]);	/* StrataFlash (ROMCS2, 0x11000000, 16MB) */
	writel(0x4203d900, &sc520_mmcr->par[9]);	/* SRAM (GPCS0, 0x19000000, 1MB) */
	writel(0x4e03d910, &sc520_mmcr->par[10]);	/* SRAM (GPCS3, 0x19100000, 1MB) */
	writel(0x50018100, &sc520_mmcr->par[11]);	/* DP-RAM (GPCS4, 0x18100000, 4kB) */
	writel(0x54020000, &sc520_mmcr->par[12]);	/* CFLASH1 (0x200000000, 4kB) */
	writel(0x5c020001, &sc520_mmcr->par[13]);	/* CFLASH2 (0x200010000, 4kB) */
/*	writel(0x8bfff800, &sc520_mmcr->par14); */	/* BOOTCS at  0x18000000 */
/*	writel(0x38201000, &sc520_mmcr->par15); */	/* LEDs etc (GPCS6, 0x1000, 20 Bytes */

	/* Disable Watchdog */
	writew(0x3333, &sc520_mmcr->wdtmrctl);
	writew(0xcccc, &sc520_mmcr->wdtmrctl);
	writew(0x0000, &sc520_mmcr->wdtmrctl);

	/* Chip Select Configuration */
	writew(0x0033, &sc520_mmcr->bootcsctl);
	writew(0x0615, &sc520_mmcr->romcs1ctl);
	writew(0x0615, &sc520_mmcr->romcs2ctl);

	writeb(0x00, &sc520_mmcr->adddecctl);
	writeb(0x07, &sc520_mmcr->uart1ctl);
	writeb(0x07, &sc520_mmcr->uart2ctl);
	writeb(0x06, &sc520_mmcr->sysarbctl);
	writew(0x0003, &sc520_mmcr->sysarbmenb);

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
