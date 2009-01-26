/*
 * (C) Copyright 2008
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * Based on board/amcc/yosemite/yosemite.c
 * (C) Copyright 2006-2007
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ppc4xx.h>
#include <asm/processor.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* info for FLASH chips */
extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

int board_early_init_f(void)
{
	register uint reg;

	/*
	 * Setup the external bus controller/chip selects
	 */
	mfebc(xbcfg, reg);
	mtebc(xbcfg, reg | 0x04000000);		/* Set ATC */

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
	mtdcr(uic0sr, 0xffffffff);	/* clear all */
	mtdcr(uic0er, 0x00000000);	/* disable all */
	mtdcr(uic0cr, 0x00000009);	/* ATI & UIC1 crit are critical */
	mtdcr(uic0pr, 0xfffffe13);	/* per ref-board manual */
	mtdcr(uic0tr, 0x01c00008);	/* per ref-board manual */
	mtdcr(uic0vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic0sr, 0xffffffff);	/* clear all */

	mtdcr(uic1sr, 0xffffffff);	/* clear all */
	mtdcr(uic1er, 0x00000000);	/* disable all */
	mtdcr(uic1cr, 0x00000000);	/* all non-critical */
	mtdcr(uic1pr, 0xffffe0ff);	/* per ref-board manual */
	mtdcr(uic1tr, 0x00ffc000);	/* per ref-board manual */
	mtdcr(uic1vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic1sr, 0xffffffff);	/* clear all */

	/*
	 * Setup other serial configuration
	 */
	mfsdr(sdr_pci0, reg);
	mtsdr(sdr_pci0, 0x80000000 | reg);	/* PCI arbiter enabled */
	mtsdr(sdr_pfc0, 0x00003e00);	/* Pin function */
	mtsdr(sdr_pfc1, 0x00048000);	/* Pin function: UART0 has 4 pins */

	return 0;
}

int misc_init_r(void)
{
	uint pbcr;
	int size_val;
	uint sz;

	/* Re-do sizing to get full correct info */
	mfebc(pb0cr, pbcr);

	if (gd->bd->bi_flashsize > 0x08000000)
		panic("Max. flash banksize is 128 MB!\n");

	for (sz = gd->bd->bi_flashsize, size_val = 7;
	    ((sz & 0x08000000) == 0) && (size_val > 0); --size_val)
		sz <<= 1;

	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
	mtebc(pb0cr, pbcr);

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
	char *s = getenv("serial#");

	printf("Board: GDPPC440ETX - G&D PPC440EP/GR ETX-module");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return 0;
}

/*
 * pci_pre_init
 *
 * This routine is called just prior to registering the hose and gives
 * the board the opportunity to check things. Returning a value of zero
 * indicates that things are bad & PCI initialization should be aborted.
 *
 *	Different boards may wish to customize the pci controller structure
 *	(add regions, override default access routines, etc) or perform
 *	certain pre-initialization actions.
 *
 */
#if defined(CONFIG_PCI)
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long addr;

	/*
	 * Set priority for all PLB3 devices to 0.
	 * Set PLB3 arbiter to fair mode.
	 */
	mfsdr(sdr_amp1, addr);
	mtsdr(sdr_amp1, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb3_acr);
	mtdcr(plb3_acr, addr | 0x80000000);

	/*
	 * Set priority for all PLB4 devices to 0.
	 */
	mfsdr(sdr_amp0, addr);
	mtsdr(sdr_amp0, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb4_acr) | 0xa0000000;	/* Was 0x8---- */
	mtdcr(plb4_acr, addr);

	/*
	 * Set Nebula PLB4 arbiter to fair mode.
	 */
	/* Segment0 */
	addr = (mfdcr(plb0_acr) & ~plb0_acr_ppm_mask) | plb0_acr_ppm_fair;
	addr = (addr & ~plb0_acr_hbu_mask) | plb0_acr_hbu_enabled;
	addr = (addr & ~plb0_acr_rdp_mask) | plb0_acr_rdp_4deep;
	addr = (addr & ~plb0_acr_wrp_mask) | plb0_acr_wrp_2deep;
	mtdcr(plb0_acr, addr);

	/* Segment1 */
	addr = (mfdcr(plb1_acr) & ~plb1_acr_ppm_mask) | plb1_acr_ppm_fair;
	addr = (addr & ~plb1_acr_hbu_mask) | plb1_acr_hbu_enabled;
	addr = (addr & ~plb1_acr_rdp_mask) | plb1_acr_rdp_4deep;
	addr = (addr & ~plb1_acr_wrp_mask) | plb1_acr_wrp_2deep;
	mtdcr(plb1_acr, addr);

	/* enable 66 MHz ext. Clock */
	out32(GPIO1_TCR, in32(GPIO1_TCR) | 0x00008000);
	out32(GPIO1_OR, in32(GPIO1_OR) | 0x00008000);

	return 1;
}
#endif	/* defined(CONFIG_PCI) */

/*
 * pci_target_init
 *
 * The bootstrap configuration provides default settings for the pci
 * inbound map (PIM). But the bootstrap config choices are limited and
 * may not be sufficient for a given board.
 *
 */
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/*
	 * Set up Direct MMIO registers
	 */

	/*
	 * PowerPC440 EP PCI Master configuration.
	 * Map one 1Gig range of PLB/processor addresses to PCI memory space.
	 *       PLB address 0xA0000000-0xDFFFFFFF
	 *   ==> PCI address 0xA0000000-0xDFFFFFFF
	 *   Use byte reversed out routines to handle endianess.
	 * Make this region non-prefetchable.
	 */
	out32r(PCIX0_PMM0MA, 0x00000000); 	/* disabled b4 setting */
	out32r(PCIX0_PMM0LA, CONFIG_SYS_PCI_MEMBASE);
	out32r(PCIX0_PMM0PCILA, CONFIG_SYS_PCI_MEMBASE);
	out32r(PCIX0_PMM0PCIHA, 0x00000000);
	out32r(PCIX0_PMM0MA, 0xE0000001); /* 512M, no prefetch, enable region */

	out32r(PCIX0_PMM1MA, 0x00000000);	/* disabled b4 setting */
	out32r(PCIX0_PMM1LA, CONFIG_SYS_PCI_MEMBASE2);
	out32r(PCIX0_PMM1PCILA, CONFIG_SYS_PCI_MEMBASE2);
	out32r(PCIX0_PMM1PCIHA, 0x00000000);
	out32r(PCIX0_PMM1MA, 0xE0000001); /* 512M, no prefetch, enable region */

	out32r(PCIX0_PTM1MS, 0x00000001);
	out32r(PCIX0_PTM1LA, 0);
	out32r(PCIX0_PTM2MS, 0);
	out32r(PCIX0_PTM2LA, 0);

	/*
	 * Set up Configuration registers
	 */

	/* Program the board's subsystem id/vendor id */
	pci_write_config_word(0, PCI_SUBSYSTEM_VENDOR_ID,
			      CONFIG_SYS_PCI_SUBSYS_VENDORID);
	pci_write_config_word(0, PCI_SUBSYSTEM_ID, CONFIG_SYS_PCI_SUBSYS_ID);

	/* Configure command register as bus master */
	pci_write_config_word(0, PCI_COMMAND, PCI_COMMAND_MASTER);

	/* 240nS PCI clock */
	pci_write_config_word(0, PCI_LATENCY_TIMER, 1);

	/* No error reporting */
	pci_write_config_word(0, PCI_ERREN, 0);

	pci_write_config_dword(0, PCI_BRDGOPT2, 0x00000101);

}
#endif	/* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT) */

/*
 *  pci_master_init
 *
 */
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	unsigned short temp_short;

	/*
	 * Write the PowerPC440 EP PCI Configuration regs.
	 *   Enable PowerPC440 EP to be a master on the PCI bus (PMM).
	 *   Enable PowerPC440 EP to act as a PCI memory target (PTM).
	 */
	pci_read_config_word(0, PCI_COMMAND, &temp_short);
	pci_write_config_word(0, PCI_COMMAND,
			      temp_short | PCI_COMMAND_MASTER |
			      PCI_COMMAND_MEMORY);
}
#endif	/* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT) */

/*
 *  is_pci_host
 *
 *	This routine is called to determine if a pci scan should be
 *	performed. With various hardware environments (especially cPCI and
 *	PPMC) it's insufficient to depend on the state of the arbiter enable
 *	bit in the strap register, or generic host/adapter assumptions.
 *
 *	Rather than hard-code a bad assumption in the general 440 code, the
 *	440 pci code requires the board to decide at runtime.
 *
 *	Return 0 for adapter mode, non-zero for host (monarch) mode.
 *
 *
 */
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
	return 1;
}
#endif	/* defined(CONFIG_PCI) */
