/*
 * ICP DAS LP-8x4x Support
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 * adapted from Voipac PXA270 Support by
 * Copyright (C) 2013 Sergey Yanovich <ynvich@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/regs-mmc.h>
#include <asm/arch/pxa.h>
#include <netdev.h>
#include <serial.h>
#include <asm/io.h>
#include <usb.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */
int board_init(void)
{
	/* We have RAM, disable cache */
	dcache_disable();
	icache_disable();

	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	return 0;
}

int dram_init(void)
{
	pxa2xx_dram_init();
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}

#ifdef	CONFIG_CMD_MMC
int board_mmc_init(bd_t *bis)
{
	pxa_mmc_register(0);
	return 0;
}
#endif

#ifdef	CONFIG_CMD_USB
int board_usb_init(int index, enum usb_init_type init)
{
	if (index !=0 || init != USB_INIT_HOST)
		return -1;

	writel(readl(CKEN) | CKEN10_USBHOST, CKEN);

	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	udelay(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCHR) | UHCHR_FSBIR, UHCHR);

	while (readl(UHCHR) & UHCHR_FSBIR)
		continue; /* required by checkpath.pl */

	writel(readl(UHCHR) & ~UHCHR_SSEP0, UHCHR);
	writel(readl(UHCRHDA) & ~(0x1000), UHCRHDA);
	writel(readl(UHCRHDA) | 0x800, UHCRHDA);

	writel(readl(UHCHR) & ~UHCHR_SSE, UHCHR);
	writel((UHCHIE_UPRIE | UHCHIE_RWIE), UHCHIE);

	/* Clear any OTG Pin Hold */
	if (readl(PSSR) & PSSR_OTGPH)
		writel(readl(PSSR) | PSSR_OTGPH, PSSR);

	writel(readl(UHCRHDA) & ~(0x200), UHCRHDA);
	writel(readl(UHCRHDA) | 0x100, UHCRHDA);

	/* Set port power control mask bits, only 3 ports. */
	writel(readl(UHCRHDB) | (0x7<<17), UHCRHDB);

	return 0;
}

int usb_board_stop(void)
{
	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	udelay(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCCOMS) | 1, UHCCOMS);
	udelay(10);

	writel(readl(UHCHR) | UHCHR_SSEP0 | UHCHR_SSE, UHCHR);

	writel(readl(CKEN) & ~CKEN10_USBHOST, CKEN);

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	if (index !=0 || init != USB_INIT_HOST)
		return -1;

	return usb_board_stop();
}
#endif

#ifdef CONFIG_DRIVER_DM9000
int board_eth_init(bd_t *bis)
{
	return dm9000_initialize(bis);
}
#endif
