/*
 * (C) Copyright 2007
 * Stefano Babic, DENX Gmbh, sbabic@denx.de
 *
 * (C) Copyright 2004
 * Robert Whaley, Applied Data Systems, Inc. rwhaley@applieddata.net
 *
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <asm/arch/pxa-regs.h>
#include <asm/arch/pxa.h>
#include <netdev.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define		RH_A_PSM	(1 << 8)	/* power switching mode */
#define		RH_A_NPS	(1 << 9)	/* no power switching */

extern struct serial_device serial_ffuart_device;
extern struct serial_device serial_btuart_device;
extern struct serial_device serial_stuart_device;

#if CONFIG_MK_POLARIS
#define BOOT_CONSOLE	"serial_stuart"
#else
#define BOOT_CONSOLE	"serial_ffuart"
#endif
/* ------------------------------------------------------------------------- */

/*
 * Miscelaneous platform dependent initialisations
 */

int usb_board_init(void)
{
	writel((readl(UHCHR) | UHCHR_PCPL | UHCHR_PSPL) &
		~(UHCHR_SSEP0 | UHCHR_SSEP1 | UHCHR_SSEP2 | UHCHR_SSE),
		UHCHR);

	writel(readl(UHCHR) | UHCHR_FSBIR, UHCHR);

	while (readl(UHCHR) & UHCHR_FSBIR)
		;

	writel(readl(UHCHR) & ~UHCHR_SSE, UHCHR);
	writel((UHCHIE_UPRIE | UHCHIE_RWIE), UHCHIE);

	/* Clear any OTG Pin Hold */
	if (readl(PSSR) & PSSR_OTGPH)
		writel(readl(PSSR) | PSSR_OTGPH, PSSR);

	writel(readl(UHCRHDA) & ~(RH_A_NPS), UHCRHDA);
	writel(readl(UHCRHDA) | RH_A_PSM, UHCRHDA);

	/* Set port power control mask bits, only 3 ports. */
	writel(readl(UHCRHDB) | (0x7<<17), UHCRHDB);

	return 0;
}

void usb_board_init_fail(void)
{
	return;
}

void usb_board_stop(void)
{
	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	udelay(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCCOMS) | 1, UHCCOMS);
	udelay(10);

	writel(readl(CKEN) & ~CKEN10_USBHOST, CKEN);

	return;
}

int board_init (void)
{
	/* We have RAM, disable cache */
	dcache_disable();
	icache_disable();

	/* arch number of ConXS Board */
	gd->bd->bi_arch_number = 776;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa000003c;

	return 0;
}

int board_late_init(void)
{
#if defined(CONFIG_SERIAL_MULTI)
	char *console=getenv("boot_console");

	if ((console == NULL) || (strcmp(console,"serial_btuart") &&
		strcmp(console,"serial_stuart") &&
		strcmp(console,"serial_ffuart"))) {
			console = BOOT_CONSOLE;
	}
	setenv("stdout",console);
	setenv("stdin", console);
	setenv("stderr",console);
#endif
	return 0;
}

struct serial_device *default_serial_console (void)
{
	return &serial_ffuart_device;
}

int dram_init(void)
{
	pxa2xx_dram_init();
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}

#ifdef CONFIG_DRIVER_DM9000
int board_eth_init(bd_t *bis)
{
	return dm9000_initialize(bis);
}
#endif
