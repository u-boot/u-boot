/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <asm/processor.h>

extern void board_pll_init_f(void);

/* Some specific Acadia Defines */
#define CPLD_BASE	0x80000000

void liveoak_gpio_init(void)
{
	/*
	 * GPIO0 setup (select GPIO or alternate function)
	 */
       	out32(GPIO0_OSRL, CFG_GPIO0_OSRL);
       	out32(GPIO0_OSRH, CFG_GPIO0_OSRH);	/* output select */
       	out32(GPIO0_ISR1L, CFG_GPIO0_ISR1L);
       	out32(GPIO0_ISR1H, CFG_GPIO0_ISR1H);	/* input select */
       	out32(GPIO0_TSRL, CFG_GPIO0_TSRL);
       	out32(GPIO0_TSRH, CFG_GPIO0_TSRH);	/* three-state select */
       	out32(GPIO0_TCR, CFG_GPIO0_TCR);  /* enable output driver for outputs */

	/*
	 * Ultra (405EZ) was nice enough to add another GPIO controller
	 */
	out32(GPIO1_OSRH, CFG_GPIO1_OSRH);	/* output select */
	out32(GPIO1_OSRL, CFG_GPIO1_OSRL);
	out32(GPIO1_ISR1H, CFG_GPIO1_ISR1H);	/* input select */
	out32(GPIO1_ISR1L, CFG_GPIO1_ISR1L);
	out32(GPIO1_TSRH, CFG_GPIO1_TSRH);	/* three-state select */
	out32(GPIO1_TSRL, CFG_GPIO1_TSRL);
	out32(GPIO1_TCR, CFG_GPIO1_TCR);  /* enable output driver for outputs */
}

#if 0 /* test-only: not called at all??? */
void ext_bus_cntlr_init(void)
{
#if (defined(EBC_PB4AP) && defined(EBC_PB4CR) && !(CFG_INIT_DCACHE_CS == 4))
       	mtebc(pb4ap, EBC_PB4AP);
       	mtebc(pb4cr, EBC_PB4CR);
#endif
}
#endif

int board_early_init_f(void)
{
	unsigned int reg;

#if 0 /* test-only */
	/*
	 * If CRAM memory and SPI/NAND boot, and if the CRAM memory is
	 * already initialized by the pre-loader then we can't reinitialize
	 * CPR registers, GPIO registers and EBC registers as this will
	 * have the effect of un-initializing CRAM.
	 */
	spr_reg = (volatile unsigned long) mfspr(SPRG7);
	if (spr_reg != LOAK_CRAM) { /* != CRAM */
		board_pll_init_f();
		liveoak_gpio_init();
		ext_bus_cntlr_init();

		mtebc(pb1ap, CFG_EBC_PB1AP);
		mtebc(pb1cr, CFG_EBC_PB1CR);

		mtebc(pb2ap, CFG_EBC_PB2AP);
		mtebc(pb2cr, CFG_EBC_PB2CR);
	}
#else
	board_pll_init_f();
	liveoak_gpio_init();
/*	ext_bus_cntlr_init(); */
#endif

#if 0 /* test-only (orig) */
	/*
	 * If we boot from NAND Flash, we are running in
	 * RAM, so disable the EBC_CS0 so that it goes back
	 * to the NOR Flash.  It will be enabled later
	 * for the NAND Flash on EBC_CS1
	 */
	mfsdr(sdrultra0, reg);
	mtsdr(sdrultra0, reg & ~SDR_ULTRA0_CSNSEL0);
#endif
#if 0 /* test-only */
	/* configure for NAND */
	mfsdr(sdrultra0, reg);
	reg &= ~SDR_ULTRA0_CSN_MASK;
	reg |= SDR_ULTRA0_CSNSEL0 >> CFG_NAND_CS;
	mtsdr(sdrultra0, reg & ~SDR_ULTRA0_CSNSEL0);
#endif

	/* USB Host core needs this bit set */
	mfsdr(sdrultra1, reg);
	mtsdr(sdrultra1, reg | SDR_ULTRA1_LEDNENABLE);

	mtdcr(uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(uicer, 0x00000000);	/* disable all ints */
	mtdcr(uiccr, 0x00000010);
	mtdcr(uicpr, 0xFE7FFFF0);	/* set int polarities */
	mtdcr(uictr, 0x00000010);	/* set int trigger levels */
	mtdcr(uicsr, 0xFFFFFFFF);	/* clear all ints */

	return 0;
}

int misc_init_f(void)
{
	/* Set EPLD to take PHY out of reset */
	out8(CPLD_BASE + 0x05, 0x00);
	udelay(100000);

	return 0;
}

/*
 * Check Board Identity:
 */
int checkboard(void)
{
	char *s = getenv("serial#");

	printf("Board: Acadia - AMCC PPC405EZ Evaluation Board");
	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}
