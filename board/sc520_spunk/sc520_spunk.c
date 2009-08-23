/*
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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
#include <netdev.h>
#include <ds1722.h>
#include <asm/io.h>
#include <asm/ic/sc520.h>
#include <asm/ic/ssi.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Theory:
 * We first set up all IRQs to be non-pci, edge triggered,
 * when we later enumerate the pci bus and pci_sc520_fixup_irq() gets
 * called we reallocate irqs to the pci bus with sc520_pci_set_irq()
 * as needed. Whe choose the irqs to gram from a configurable list
 * inside pci_sc520_fixup_irq() (If this list contains stupid irq's
 * such as 0 thngas will not work)
 */

static void irq_init(void)
{
	/* disable global interrupt mode */
	sc520_mmcr->picicr = 0x40;

	/* set all irqs to edge */
	sc520_mmcr->pic_mode[0] = 0x00;
	sc520_mmcr->pic_mode[1] = 0x00;
	sc520_mmcr->pic_mode[2] = 0x00;

	/* active low polarity on PIC interrupt pins,
	 *  active high polarity on all other irq pins */
	sc520_mmcr->intpinpol = 0x0000;

	/* set irq number mapping */
	sc520_mmcr->gp_tmr_int_map[0] = SC520_IRQ_DISABLED;	/* disable GP timer 0 INT */
	sc520_mmcr->gp_tmr_int_map[1] = SC520_IRQ_DISABLED;	/* disable GP timer 1 INT */
	sc520_mmcr->gp_tmr_int_map[2] = SC520_IRQ_DISABLED;	/* disable GP timer 2 INT */
	sc520_mmcr->pit_int_map[0] = SC520_IRQ0;		/* Set PIT timer 0 INT to IRQ0 */
	sc520_mmcr->pit_int_map[1] = SC520_IRQ_DISABLED;	/* disable PIT timer 1 INT */
	sc520_mmcr->pit_int_map[2] = SC520_IRQ_DISABLED;	/* disable PIT timer 2 INT */
	sc520_mmcr->pci_int_map[0] = SC520_IRQ_DISABLED;	/* disable PCI INT A */
	sc520_mmcr->pci_int_map[1] = SC520_IRQ_DISABLED;	/* disable PCI INT B */
	sc520_mmcr->pci_int_map[2] = SC520_IRQ_DISABLED;	/* disable PCI INT C */
	sc520_mmcr->pci_int_map[3] = SC520_IRQ_DISABLED;	/* disable PCI INT D */
	sc520_mmcr->dmabcintmap = SC520_IRQ_DISABLED;		/* disable DMA INT */
	sc520_mmcr->ssimap = SC520_IRQ6;			/* Set Synchronius serial INT to IRQ6*/
	sc520_mmcr->wdtmap = SC520_IRQ_DISABLED;		/* disable Watchdog INT */
	sc520_mmcr->rtcmap = SC520_IRQ8;			/* Set RTC int to 8 */
	sc520_mmcr->wpvmap = SC520_IRQ_DISABLED;		/* disable write protect INT */
	sc520_mmcr->icemap = SC520_IRQ1;			/* Set ICE Debug Serielport INT to IRQ1 */
	sc520_mmcr->ferrmap = SC520_IRQ13; 			/* Set FP error INT to IRQ13 */


	sc520_mmcr->uart_int_map[0] = SC520_IRQ4;		/* Set internal UART1 INT to IRQ4 */
	sc520_mmcr->uart_int_map[1] = SC520_IRQ3;		/* Set internal UART2 INT to IRQ3 */

	sc520_mmcr->gp_int_map[0] = SC520_IRQ7;			/* Set GPIRQ0 (PC-Card AUX IRQ) to IRQ7 */
	sc520_mmcr->gp_int_map[1] = SC520_IRQ14;		/* Set GPIRQ1 (CF IRQ) to IRQ14 */
	sc520_mmcr->gp_int_map[3] = SC520_IRQ5;			/* Set GPIRQ3 ( CAN IRQ ) ti IRQ5 */
	sc520_mmcr->gp_int_map[4] = SC520_IRQ_DISABLED;		/* disbale GIRQ4 ( IRR IRQ ) */
	sc520_mmcr->gp_int_map[5] = SC520_IRQ_DISABLED;		/* disable GPIRQ5 */
	sc520_mmcr->gp_int_map[6] = SC520_IRQ_DISABLED;		/* disable GPIRQ6 */
	sc520_mmcr->gp_int_map[7] = SC520_IRQ_DISABLED;		/* disable GPIRQ7 */
	sc520_mmcr->gp_int_map[8] = SC520_IRQ_DISABLED;		/* disable GPIRQ8 */
	sc520_mmcr->gp_int_map[9] = SC520_IRQ_DISABLED;		/* disable GPIRQ9 */
	sc520_mmcr->gp_int_map[2] = SC520_IRQ_DISABLED;		/* disable GPIRQ2 */
	sc520_mmcr->gp_int_map[10] = SC520_IRQ_DISABLED;	/* disable GPIRQ10 */

	sc520_mmcr->pcihostmap = 0x11f;				/* Map PCI hostbridge INT to NMI */
	sc520_mmcr->eccmap = 0x100;				/* Map SDRAM ECC failure INT to NMI */

}

/* set up the ISA bus timing and system address mappings */
static void bus_init(void)
{
	/* versions
	 * 0   Hyglo versions 0.95 and 0.96 (large baords)
	 * ??  Hyglo version 0.97 (small board)
	 * 10  Spunk board
	 */
	int version = sc520_mmcr->sysinfo;

	if (version) {
		/* set up the GP IO pins (for the Spunk board) */
		sc520_mmcr->piopfs31_16 = 0xfff0;	/* set the GPIO pin function 31-16 reg */
		sc520_mmcr->piopfs15_0 = 0x000f;	/* set the GPIO pin function 15-0 reg */
		sc520_mmcr->piodir31_16 = 0x000f;	/* set the GPIO direction 31-16 reg */
		sc520_mmcr->piodir15_0 = 0x1ff0;	/* set the GPIO direction 15-0 reg */
		sc520_mmcr->cspfs = 0xc0;		/* set the CS pin function reg */
		sc520_mmcr->clksel = 0x70;

		sc520_mmcr->pioclr31_16 = 0x0003;	/* reset SSI chip-selects */
		sc520_mmcr->pioset31_16 = 0x000c;

	} else {
		/* set up the GP IO pins (for the Hyglo board) */
		sc520_mmcr->piopfs31_16 = 0xffc0;	/* set the GPIO pin function 31-16 reg */
		sc520_mmcr->piopfs15_0 = 0x1e7f;	/* set the GPIO pin function 15-0 reg */
		sc520_mmcr->piodir31_16 = 0x003f;	/* set the GPIO direction 31-16 reg */
		sc520_mmcr->piodir15_0 = 0xe180;	/* set the GPIO direction 15-0 reg */
		sc520_mmcr->cspfs = 0x00;		/* set the CS pin function reg */
		sc520_mmcr->clksel = 0x70;

		sc520_mmcr->pioclr15_0 = 0x0180;	/* reset SSI chip-selects */
	}

	sc520_mmcr->gpcsrt = 1;		/* set the GP CS offset */
	sc520_mmcr->gpcspw = 3;		/* set the GP CS pulse width */
	sc520_mmcr->gpcsoff = 1;	/* set the GP CS offset */
	sc520_mmcr->gprdw = 3;		/* set the RD pulse width */
	sc520_mmcr->gprdoff = 1;	/* set the GP RD offset */
	sc520_mmcr->gpwrw = 3;		/* set the GP WR pulse width */
	sc520_mmcr->gpwroff = 1;	/* set the GP WR offset */

	sc520_mmcr->bootcsctl = 0x0407;	/* set up timing of BOOTCS */

	/* adjust the memory map:
	 * by default the first 256MB (0x00000000 - 0x0fffffff) is mapped to SDRAM
	 * and 256MB to 1G-128k  (0x1000000 - 0x37ffffff) is mapped to PCI mmio
	 * we need to map 1G-128k - 1G (0x38000000 - 0x3fffffff) to CS1 */


	/* bootcs */
	sc520_mmcr->par[12] = 0x8bffe800;

	/* IDE0 = GPCS6 1f0-1f7 */
	sc520_mmcr->par[3] = 0x380801f0;

	/* IDE1 = GPCS7 3f6 */
	sc520_mmcr->par[4] = 0x3c0003f6;

	asm ("wbinvd\n"); /* Flush cache, req. after setting the unchached attribute ona PAR */

	sc520_mmcr->adddecctl = sc520_mmcr->adddecctl & ~(UART2_DIS|UART1_DIS);

}


/* par usage:
 * PAR0   (legacy_video)
 * PAR1   (PCI ROM mapping)
 * PAR2
 * PAR3   IDE
 * PAR4   IDE
 * PAR5   (legacy_video)
 * PAR6
 * PAR7   (legacy_video)
 * PAR8   (legacy_video)
 * PAR9   (legacy_video)
 * PAR10
 * PAR11  (ISAROM)
 * PAR12  BOOTCS
 * PAR13
 * PAR14
 * PAR15
 */

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	init_sc520();
	bus_init();
	irq_init();

	/* max drive current on SDRAM */
	sc520_mmcr->dsctl = 0x0100;

	/* enter debug mode after next reset (only if jumper is also set) */
	sc520_mmcr->rescfg = 0x08;
	/* configure the software timer to 33.000MHz */
	sc520_mmcr->swtmrcfg = 1;
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
	int version = sc520_mmcr->sysinfo;

	if (val < -32) val = -1;  /* let things compatible */
	if (version == 0) {
		/* PIO31-PIO16 Data */
		sc520_mmcr->piodata31_16 = (sc520_mmcr->piodata31_16 & 0xffc0) | ((val&0x7e)>>1); /* 0x1f8 >> 3 */

		/* PIO0-PIO15 Data */
		sc520_mmcr->piodata15_0 = (sc520_mmcr->piodata15_0 & 0x1fff)| ((val&0x7)<<13);
	} else {
		/* newer boards use PIO4-PIO12 */
		/* PIO0-PIO15 Data */
#if 0
		val = (val & 0x007) | ((val & 0x038) << 3) | ((val & 0x1c0) >> 3);
#else
		val = (val & 0x007) | ((val & 0x07e) << 2);
#endif
		sc520_mmcr->piodata15_0 = (sc520_mmcr->piodata15_0 & 0xe00f) | ((val&0x01ff)<<4);
	}
}


int last_stage_init(void)
{

	int version = sc520_mmcr->sysinfo;

	printf("Omicron Ceti SC520 Spunk revision %x\n", version);

#if 0
	if (version) {
		int x, y;

		printf("eeprom probe %d\n", spi_eeprom_probe(1));

		spi_eeprom_read(1, 0, (u8*)&x, 2);
		spi_eeprom_read(1, 1, (u8*)&y, 2);
		printf("eeprom bytes %04x%04x\n", x, y);
		x ^= 0xffff;
		y ^= 0xffff;
		spi_eeprom_write(1, 0, (u8*)&x, 2);
		spi_eeprom_write(1, 1, (u8*)&y, 2);

		spi_eeprom_read(1, 0, (u8*)&x, 2);
		spi_eeprom_read(1, 1, (u8*)&y, 2);
		printf("eeprom bytes %04x%04x\n", x, y);

	} else {
		int x, y;

		printf("eeprom probe %d\n", mw_eeprom_probe(1));

		mw_eeprom_read(1, 0, (u8*)&x, 2);
		mw_eeprom_read(1, 1, (u8*)&y, 2);
		printf("eeprom bytes %04x%04x\n", x, y);

		x ^= 0xffff;
		y ^= 0xffff;
		mw_eeprom_write(1, 0, (u8*)&x, 2);
		mw_eeprom_write(1, 1, (u8*)&y, 2);

		mw_eeprom_read(1, 0, (u8*)&x, 2);
		mw_eeprom_read(1, 1, (u8*)&y, 2);
		printf("eeprom bytes %04x%04x\n", x, y);


	}
#endif

	ds1722_probe(2);

	return 0;
}

void ssi_chip_select(int dev)
{
	int version = sc520_mmcr->sysinfo;

	if (version) {
		/* Spunk board: EEPROM and CAN are actove-low, TEMP and AUX are active high */
		switch (dev) {
		case 1: /* EEPROM */
			sc520_mmcr->pioclr31_16 = 0x0004;
			break;

		case 2: /* Temp Probe */
			sc520_mmcr->pioset31_16 = 0x0002;
			break;

		case 3: /* CAN */
			sc520_mmcr->pioclr31_16 = 0x0008;
			break;

		case 4: /* AUX */
			sc520_mmcr->pioset31_16 = 0x0001;
			break;

		case 0:
			sc520_mmcr->pioclr31_16 = 0x0003;
			sc520_mmcr->pioset31_16 = 0x000c;
			break;

		default:
			printf("Illegal SSI device requested: %d\n", dev);
		}
	} else {

		/* Globox board: Both EEPROM and TEMP are active-high */

		switch (dev) {
		case 1: /* EEPROM */
			sc520_mmcr->pioset15_0 = 0x0100;
			break;

		case 2: /* Temp Probe */
			sc520_mmcr->pioset15_0 = 0x0080;
			break;

		case 0:
			sc520_mmcr->pioclr15_0 = 0x0180;
			break;

		default:
			printf("Illegal SSI device requested: %d\n", dev);
		}
	}
}

void spi_eeprom_probe(int x)
{
}

int spi_eeprom_read(int x, int offset, uchar *buffer, int len)
{
       return 0;
}

int spi_eeprom_write(int x, int offset, uchar *buffer, int len)
{
       return 0;
}

void mw_eeprom_probe(int x)
{
}

int mw_eeprom_read(int x, int offset, uchar *buffer, int len)
{
       return 0;
}

int mw_eeprom_write(int x, int offset, uchar *buffer, int len)
{
       return 0;
}

void spi_init_f(void)
{
	sc520_mmcr->sysinfo ? spi_eeprom_probe(1) : mw_eeprom_probe(1);

}

ssize_t spi_read(uchar *addr, int alen, uchar *buffer, int len)
{
	int offset;
	int i;

	offset = 0;
	for (i=0;i<alen;i++) {
		offset <<= 8;
		offset |= addr[i];
	}

	return	sc520_mmcr->sysinfo ?
		spi_eeprom_read(1, offset, buffer, len) :
	mw_eeprom_read(1, offset, buffer, len);
}

ssize_t spi_write(uchar *addr, int alen, uchar *buffer, int len)
{
	int offset;
	int i;

	offset = 0;
	for (i=0;i<alen;i++) {
		offset <<= 8;
		offset |= addr[i];
	}

	return	sc520_mmcr->sysinfo ?
		spi_eeprom_write(1, offset, buffer, len) :
	mw_eeprom_write(1, offset, buffer, len);
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
