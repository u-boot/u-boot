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
#include <asm/io.h>
#include <asm/ic/sc520.h>
#include <ali512x.h>
#include <spi.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

#undef SC520_CDP_DEBUG

#ifdef	SC520_CDP_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/* ------------------------------------------------------------------------- */


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
	sc520_mmcr->ssimap = SC520_IRQ_DISABLED;		/* disable Synchronius serial INT */
	sc520_mmcr->wdtmap = SC520_IRQ_DISABLED;		/* disable Watchdog INT */
	sc520_mmcr->rtcmap = SC520_IRQ8;			/* Set RTC int to 8 */
	sc520_mmcr->wpvmap = SC520_IRQ_DISABLED;		/* disable write protect INT */
	sc520_mmcr->icemap = SC520_IRQ1;			/* Set ICE Debug Serielport INT to IRQ1 */
	sc520_mmcr->ferrmap = SC520_IRQ13; 			/* Set FP error INT to IRQ13 */

	if (CONFIG_SYS_USE_SIO_UART) {
		sc520_mmcr->uart_int_map[0] = SC520_IRQ_DISABLED;	/* disable internal UART1 INT */
		sc520_mmcr->uart_int_map[1] = SC520_IRQ_DISABLED;	/* disable internal UART2 INT */
		sc520_mmcr->gp_int_map[3] = SC520_IRQ3;		/* Set GPIRQ3 (ISA IRQ3) to IRQ3 */
		sc520_mmcr->gp_int_map[4] = SC520_IRQ4;		/* Set GPIRQ4 (ISA IRQ4) to IRQ4 */
	} else {
		sc520_mmcr->uart_int_map[0] = SC520_IRQ4;		/* Set internal UART2 INT to IRQ4 */
		sc520_mmcr->uart_int_map[1] = SC520_IRQ3;		/* Set internal UART2 INT to IRQ3 */
		sc520_mmcr->gp_int_map[3] = SC520_IRQ_DISABLED;	/* disable GPIRQ3 (ISA IRQ3) */
		sc520_mmcr->gp_int_map[4] = SC520_IRQ_DISABLED;	/* disable GPIRQ4 (ISA IRQ4) */
	}

	sc520_mmcr->gp_int_map[1] = SC520_IRQ1;			/* Set GPIRQ1 (SIO IRQ1) to IRQ1 */
	sc520_mmcr->gp_int_map[5] = SC520_IRQ5;			/* Set GPIRQ5 (ISA IRQ5) to IRQ5 */
	sc520_mmcr->gp_int_map[6] = SC520_IRQ6;			/* Set GPIRQ6 (ISA IRQ6) to IRQ6 */
	sc520_mmcr->gp_int_map[7] = SC520_IRQ7;			/* Set GPIRQ7 (ISA IRQ7) to IRQ7 */
	sc520_mmcr->gp_int_map[8] = SC520_IRQ8;			/* Set GPIRQ8 (SIO IRQ8) to IRQ8 */
	sc520_mmcr->gp_int_map[9] = SC520_IRQ9;			/* Set GPIRQ9 (ISA IRQ2) to IRQ9 */
	sc520_mmcr->gp_int_map[0] = SC520_IRQ11;		/* Set GPIRQ0 (ISA IRQ11) to IRQ10 */
	sc520_mmcr->gp_int_map[2] = SC520_IRQ12;		/* Set GPIRQ2 (ISA IRQ12) to IRQ12 */
	sc520_mmcr->gp_int_map[10] = SC520_IRQ14;		/* Set GPIRQ10 (ISA IRQ14) to IRQ14 */

	sc520_mmcr->pcihostmap = 0x11f;				/* Map PCI hostbridge INT to NMI */
	sc520_mmcr->eccmap = 0x100;				/* Map SDRAM ECC failure INT to NMI */
}

static void silence_uart(int port)
{
	outb(0, port+1);
}

void setup_ali_sio(int uart_primary)
{
	ali512x_init();

	ali512x_set_fdc(ALI_ENABLED, 0x3f2, 6, 0);
	ali512x_set_pp(ALI_ENABLED, 0x278, 7, 3);
	ali512x_set_uart(ALI_ENABLED, ALI_UART1, uart_primary?0x3f8:0x3e8, 4);
	ali512x_set_uart(ALI_ENABLED, ALI_UART2, uart_primary?0x2f8:0x2e8, 3);
	ali512x_set_rtc(ALI_DISABLED, 0, 0);
	ali512x_set_kbc(ALI_ENABLED, 1, 12);
	ali512x_set_cio(ALI_ENABLED);

	/* IrDa pins */
	ali512x_cio_function(12, 1, 0, 0);
	ali512x_cio_function(13, 1, 0, 0);

	/* SSI chip select pins */
	ali512x_cio_function(14, 0, 0, 0);  /* SSI_CS */
	ali512x_cio_function(15, 0, 0, 0);  /* SSI_MV */
	ali512x_cio_function(16, 0, 0, 0);  /* SSI_SPI# */

	/* Board REV pins */
	ali512x_cio_function(20, 0, 0, 1);
	ali512x_cio_function(21, 0, 0, 1);
	ali512x_cio_function(22, 0, 0, 1);
	ali512x_cio_function(23, 0, 0, 1);
}


/* set up the ISA bus timing and system address mappings */
static void bus_init(void)
{

	/* set up the GP IO pins */
	sc520_mmcr->piopfs31_16 = 0xf7ff;	/* set the GPIO pin function 31-16 reg */
	sc520_mmcr->piopfs15_0 = 0xffff;	/* set the GPIO pin function 15-0 reg */
	sc520_mmcr->cspfs = 0xf8;		/* set the CS pin function  reg */
	sc520_mmcr->clksel = 0x70;

	sc520_mmcr->gpcsrt = 1;   		/* set the GP CS offset */
	sc520_mmcr->gpcspw = 3;   		/* set the GP CS pulse width */
	sc520_mmcr->gpcsoff = 1;  		/* set the GP CS offset */
	sc520_mmcr->gprdw = 3;    		/* set the RD pulse width */
	sc520_mmcr->gprdoff = 1;  		/* set the GP RD offset */
	sc520_mmcr->gpwrw = 3;   		 /* set the GP WR pulse width */
	sc520_mmcr->gpwroff = 1; 		 /* set the GP WR offset */

	sc520_mmcr->bootcsctl = 0x1823;		/* set up timing of BOOTCS */
	sc520_mmcr->romcs1ctl = 0x1823;		/* set up timing of ROMCS1 */
	sc520_mmcr->romcs2ctl = 0x1823;		/* set up timing of ROMCS2 */

	/* adjust the memory map:
	 * by default the first 256MB (0x00000000 - 0x0fffffff) is mapped to SDRAM
	 * and 256MB to 1G-128k  (0x1000000 - 0x37ffffff) is mapped to PCI mmio
	 * we need to map 1G-128k - 1G (0x38000000 - 0x3fffffff) to CS1 */


	/* SRAM = GPCS3 128k @ d0000-effff*/
	sc520_mmcr->par[2] = 0x4e00400d;

	/* IDE0 = GPCS6 1f0-1f7 */
	sc520_mmcr->par[3] = 0x380801f0;

	/* IDE1 = GPCS7 3f6 */
	sc520_mmcr->par[4] = 0x3c0003f6;
	/* bootcs */
	sc520_mmcr->par[12] = 0x8bffe800;
	/* romcs2 */
	sc520_mmcr->par[13] = 0xcbfff000;
	/* romcs1 */
	sc520_mmcr->par[14] = 0xabfff800;
	/* 680 LEDS */
	sc520_mmcr->par[15] = 0x30000640;

	sc520_mmcr->adddecctl = 0;

	asm ("wbinvd\n"); /* Flush cache, req. after setting the unchached attribute ona PAR */

	if (CONFIG_SYS_USE_SIO_UART) {
		sc520_mmcr->adddecctl = sc520_mmcr->adddecctl | UART2_DIS | UART1_DIS;
		setup_ali_sio(1);
	} else {
		sc520_mmcr->adddecctl = sc520_mmcr->adddecctl & ~(UART2_DIS|UART1_DIS);
		setup_ali_sio(0);
		silence_uart(0x3e8);
		silence_uart(0x2e8);
	}

}

/* GPCS usage
 * GPCS0       PIO27 (NMI)
 * GPCS1       ROMCS1
 * GPCS2       ROMCS2
 * GPCS3       SRAMCS       PAR2
 * GPCS4       unused       PAR3
 * GPCS5       unused       PAR4
 * GPCS6       IDE
 * GPCS7       IDE
 */


/* par usage:
 * PAR0   legacy_video
 * PAR1   PCI ROM mapping
 * PAR2   SRAM
 * PAR3   IDE
 * PAR4   IDE
 * PAR5   legacy_video
 * PAR6   legacy_video
 * PAR7   legacy_video
 * PAR8   legacy_video
 * PAR9   legacy_video
 * PAR10  legacy_video
 * PAR11  ISAROM
 * PAR12  BOOTCS
 * PAR13  ROMCS1
 * PAR14  ROMCS2
 * PAR15  Port 0x680 LED display
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
	/* configure the software timer to 33.333MHz */
	sc520_mmcr->swtmrcfg = 0;
	gd->bus_clk = 33333000;

	return 0;
}

int dram_init(void)
{
	init_sc520_dram();
	return 0;
}

void show_boot_progress(int val)
{
	if (val < -32) val = -1;  /* let things compatible */
	outb(val&0xff, 0x80);
	outb((val&0xff00)>>8, 0x680);
}


int last_stage_init(void)
{
	int minor;
	int major;

	major = minor = 0;
	major |= ali512x_cio_in(23)?2:0;
	major |= ali512x_cio_in(22)?1:0;
	minor |= ali512x_cio_in(21)?2:0;
	minor |= ali512x_cio_in(20)?1:0;

	printf("AMD SC520 CDP revision %d.%d\n", major, minor);

	return 0;
}


void ssi_chip_select(int dev)
{

	/* Spunk board: SPI EEPROM is active-low, MW EEPROM and AUX are active high */
	switch (dev) {
	case 1: /* SPI EEPROM */
		ali512x_cio_out(16, 0);
		break;

	case 2: /* MW EEPROM */
		ali512x_cio_out(15, 1);
		break;

	case 3: /* AUX */
		ali512x_cio_out(14, 1);
		break;

	case 0:
		ali512x_cio_out(16, 1);
		ali512x_cio_out(15, 0);
		ali512x_cio_out(14, 0);
		break;

	default:
		printf("Illegal SSI device requested: %d\n", dev);
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

void spi_init_f(void)
{
#ifdef CONFIG_SYS_SC520_CDP_USE_SPI
	spi_eeprom_probe(1);
#endif
#ifdef CONFIG_SYS_SC520_CDP_USE_MW
	mw_eeprom_probe(2);
#endif
}

ssize_t spi_read(uchar *addr, int alen, uchar *buffer, int len)
{
	int offset;
	int i;
	ssize_t res;

	offset = 0;
	for (i=0;i<alen;i++) {
		offset <<= 8;
		offset |= addr[i];
	}

#ifdef CONFIG_SYS_SC520_CDP_USE_SPI
	res = spi_eeprom_read(1, offset, buffer, len);
#endif
#ifdef CONFIG_SYS_SC520_CDP_USE_MW
	res = mw_eeprom_read(2, offset, buffer, len);
#endif
#if !defined(CONFIG_SYS_SC520_CDP_USE_SPI) && !defined(CONFIG_SYS_SC520_CDP_USE_MW)
	res = 0;
#endif
	return res;
}

ssize_t spi_write(uchar *addr, int alen, uchar *buffer, int len)
{
	int offset;
	int i;
	ssize_t res;

	offset = 0;
	for (i=0;i<alen;i++) {
		offset <<= 8;
		offset |= addr[i];
	}

#ifdef CONFIG_SYS_SC520_CDP_USE_SPI
	res = spi_eeprom_write(1, offset, buffer, len);
#endif
#ifdef CONFIG_SYS_SC520_CDP_USE_MW
	res = mw_eeprom_write(2, offset, buffer, len);
#endif
#if !defined(CONFIG_SYS_SC520_CDP_USE_SPI) && !defined(CONFIG_SYS_SC520_CDP_USE_MW)
	res = 0;
#endif
	return res;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
