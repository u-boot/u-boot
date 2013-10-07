/*
 * (C) Copyright 2008
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * Based on board/amcc/yosemite/yosemite.c
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/ppc4xx.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/4xx_pci.h>

DECLARE_GLOBAL_DATA_PTR;

/* info for FLASH chips */
extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

int board_early_init_f(void)
{
	register uint reg;

	/*
	 * Setup the external bus controller/chip selects
	 */
	mfebc(EBC0_CFG, reg);
	mtebc(EBC0_CFG, reg | 0x04000000);		/* Set ATC */

	/*
	 * Setup the GPIO pins
	 */

	/* setup Address lines for flash size 64Meg. */
	out32(GPIO0_OSRL, in32(GPIO0_OSRL) | 0x54000000);
	out32(GPIO0_TSRL, in32(GPIO0_TSRL) | 0x54000000);
	out32(GPIO0_ISR1L, in32(GPIO0_ISR1L) | 0x54000000);

	/* setup emac */
	out32(GPIO0_TCR, in32(GPIO0_TCR) | 0xC080);
	out32(GPIO0_TSRL, in32(GPIO0_TSRL) | 0x40);
	out32(GPIO0_ISR1L, in32(GPIO0_ISR1L) | 0x55);
	out32(GPIO0_OSRH, in32(GPIO0_OSRH) | 0x50004000);
	out32(GPIO0_ISR1H, in32(GPIO0_ISR1H) | 0x00440000);

	/* UART0 and UART1*/
	out32(GPIO1_TCR, in32(GPIO1_TCR)     | 0x16000000);
	out32(GPIO1_OSRL, in32(GPIO1_OSRL)   | 0x02180000);
	out32(GPIO1_ISR1L, in32(GPIO1_ISR1L) | 0x00400000);
	out32(GPIO1_ISR2L, in32(GPIO1_ISR2L) | 0x04010000);

	/* disable boot-eeprom WP */
	out32(GPIO0_OSRL, in32(GPIO0_OSRL) & ~0x00C00000);
	out32(GPIO0_TSRL, in32(GPIO0_TSRL) & ~0x00C00000);
	out32(GPIO0_ISR1L, in32(GPIO0_ISR1L) & ~0x00C00000);
	out32(GPIO0_TCR, in32(GPIO0_TCR) | 0x08000000);
	out32(GPIO0_OR, in32(GPIO0_OR) & ~0x08000000);

	/* external interrupts IRQ0...3 */
	out32(GPIO1_TCR, in32(GPIO1_TCR) & ~0x00f00000);
	out32(GPIO1_TSRL, in32(GPIO1_TSRL) & ~0x00005500);
	out32(GPIO1_ISR1L, in32(GPIO1_ISR1L) | 0x00005500);


	/*
	 * Setup the interrupt controller polarities, triggers, etc.
	 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */
	mtdcr(UIC0ER, 0x00000000);	/* disable all */
	mtdcr(UIC0CR, 0x00000009);	/* ATI & UIC1 crit are critical */
	mtdcr(UIC0PR, 0xfffffe13);	/* per ref-board manual */
	mtdcr(UIC0TR, 0x01c00008);	/* per ref-board manual */
	mtdcr(UIC0VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */

	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xffffe0ff);	/* per ref-board manual */
	mtdcr(UIC1TR, 0x00ffc000);	/* per ref-board manual */
	mtdcr(UIC1VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	/*
	 * Setup other serial configuration
	 */
	mfsdr(SDR0_PCI0, reg);
	mtsdr(SDR0_PCI0, 0x80000000 | reg);	/* PCI arbiter enabled */
	mtsdr(SDR0_PFC0, 0x00003e00);	/* Pin function */
	mtsdr(SDR0_PFC1, 0x00048000);	/* Pin function: UART0 has 4 pins */

	return 0;
}

int misc_init_r(void)
{
	uint pbcr;
	int size_val;
	uint sz;

	/* Re-do sizing to get full correct info */
	mfebc(PB0CR, pbcr);

	if (gd->bd->bi_flashsize > 0x08000000)
		panic("Max. flash banksize is 128 MB!\n");

	for (sz = gd->bd->bi_flashsize, size_val = 7;
	    ((sz & 0x08000000) == 0) && (size_val > 0); --size_val)
		sz <<= 1;

	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtebc(PB0CR, pbcr);

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    -CONFIG_SYS_MONITOR_LEN,
			    0xffffffff,
			    &flash_info[0]);

	return 0;
}

int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: GDPPC440ETX - G&D PPC440EP/GR ETX-module");

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return 0;
}

/*
 * Override weak pci_pre_init()
 */
#if defined(CONFIG_PCI)
int pci_pre_init(struct pci_controller *hose)
{
	/* First call common code */
	__pci_pre_init(hose);

	/* enable 66 MHz ext. Clock */
	out32(GPIO1_TCR, in32(GPIO1_TCR) | 0x00008000);
	out32(GPIO1_OR, in32(GPIO1_OR) | 0x00008000);

	return 1;
}
#endif	/* defined(CONFIG_PCI) */
