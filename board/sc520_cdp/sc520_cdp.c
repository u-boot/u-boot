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
#include <pci.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/ic/sc520.h>
#include <asm/ic/ali512x.h>
#include <spi.h>

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
	write_mmcr_byte(SC520_PICICR, 0x40);

	/* set all irqs to edge */
	write_mmcr_byte(SC520_MPICMODE, 0x00);
	write_mmcr_byte(SC520_SL1PICMODE, 0x00);
	write_mmcr_byte(SC520_SL2PICMODE, 0x00);

	/* active low polarity on PIC interrupt pins,
	 *  active high polarity on all other irq pins */
	write_mmcr_word(SC520_INTPINPOL, 0x0000);

	/* set irq number mapping */
	write_mmcr_byte(SC520_GPTMR0MAP, SC520_IRQ_DISABLED);   /* disable GP timer 0 INT */
	write_mmcr_byte(SC520_GPTMR1MAP, SC520_IRQ_DISABLED);   /* disable GP timer 1 INT */
	write_mmcr_byte(SC520_GPTMR2MAP, SC520_IRQ_DISABLED);   /* disable GP timer 2 INT */
	write_mmcr_byte(SC520_PIT0MAP, SC520_IRQ0);             /* Set PIT timer 0 INT to IRQ0 */
	write_mmcr_byte(SC520_PIT1MAP, SC520_IRQ_DISABLED);     /* disable PIT timer 1 INT */
	write_mmcr_byte(SC520_PIT2MAP, SC520_IRQ_DISABLED);     /* disable PIT timer 2 INT */
	write_mmcr_byte(SC520_PCIINTAMAP, SC520_IRQ_DISABLED);  /* disable PCI INT A */
	write_mmcr_byte(SC520_PCIINTBMAP, SC520_IRQ_DISABLED);  /* disable PCI INT B */
	write_mmcr_byte(SC520_PCIINTCMAP, SC520_IRQ_DISABLED);  /* disable PCI INT C */
	write_mmcr_byte(SC520_PCIINTDMAP, SC520_IRQ_DISABLED);  /* disable PCI INT D */
	write_mmcr_byte(SC520_DMABCINTMAP, SC520_IRQ_DISABLED); /* disable DMA INT */
	write_mmcr_byte(SC520_SSIMAP, SC520_IRQ_DISABLED);      /* disable Synchronius serial INT */
	write_mmcr_byte(SC520_WDTMAP, SC520_IRQ_DISABLED);      /* disable Watchdog INT */
	write_mmcr_byte(SC520_RTCMAP, SC520_IRQ8);              /* Set RTC int to 8 */
	write_mmcr_byte(SC520_WPVMAP, SC520_IRQ_DISABLED);      /* disable write protect INT */
	write_mmcr_byte(SC520_ICEMAP, SC520_IRQ1);              /* Set ICE Debug Serielport INT to IRQ1 */
	write_mmcr_byte(SC520_FERRMAP,SC520_IRQ13);             /* Set FP error INT to IRQ13 */

	if (CFG_USE_SIO_UART) {
		write_mmcr_byte(SC520_UART1MAP, SC520_IRQ_DISABLED); /* disable internal UART1 INT */
		write_mmcr_byte(SC520_UART2MAP, SC520_IRQ_DISABLED); /* disable internal UART2 INT */
		write_mmcr_byte(SC520_GP3IMAP, SC520_IRQ3);          /* Set GPIRQ3 (ISA IRQ3) to IRQ3 */
		write_mmcr_byte(SC520_GP4IMAP, SC520_IRQ4);          /* Set GPIRQ4 (ISA IRQ4) to IRQ4 */
	} else {
		write_mmcr_byte(SC520_UART1MAP, SC520_IRQ4);         /* Set internal UART2 INT to IRQ4 */
		write_mmcr_byte(SC520_UART2MAP, SC520_IRQ3);         /* Set internal UART2 INT to IRQ3 */
		write_mmcr_byte(SC520_GP3IMAP, SC520_IRQ_DISABLED);  /* disable GPIRQ3 (ISA IRQ3) */
		write_mmcr_byte(SC520_GP4IMAP, SC520_IRQ_DISABLED);  /* disable GPIRQ4 (ISA IRQ4) */
	}

	write_mmcr_byte(SC520_GP1IMAP, SC520_IRQ1);             /* Set GPIRQ1 (SIO IRQ1) to IRQ1 */
	write_mmcr_byte(SC520_GP5IMAP, SC520_IRQ5);             /* Set GPIRQ5 (ISA IRQ5) to IRQ5 */
	write_mmcr_byte(SC520_GP6IMAP, SC520_IRQ6);             /* Set GPIRQ6 (ISA IRQ6) to IRQ6 */
	write_mmcr_byte(SC520_GP7IMAP, SC520_IRQ7);             /* Set GPIRQ7 (ISA IRQ7) to IRQ7 */
	write_mmcr_byte(SC520_GP8IMAP, SC520_IRQ8);             /* Set GPIRQ8 (SIO IRQ8) to IRQ8 */
	write_mmcr_byte(SC520_GP9IMAP, SC520_IRQ9);             /* Set GPIRQ9 (ISA IRQ2) to IRQ9 */
	write_mmcr_byte(SC520_GP0IMAP, SC520_IRQ11);            /* Set GPIRQ0 (ISA IRQ11) to IRQ10 */
	write_mmcr_byte(SC520_GP2IMAP, SC520_IRQ12);            /* Set GPIRQ2 (ISA IRQ12) to IRQ12 */
	write_mmcr_byte(SC520_GP10IMAP,SC520_IRQ14);            /* Set GPIRQ10 (ISA IRQ14) to IRQ14 */

	write_mmcr_word(SC520_PCIHOSTMAP, 0x11f);                /* Map PCI hostbridge INT to NMI */
	write_mmcr_word(SC520_ECCMAP, 0x100);                    /* Map SDRAM ECC failure INT to NMI */

}


/* PCI stuff */
static void pci_sc520_cdp_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	/* a configurable lists of irqs to steal
	 * when we need one (a board with more pci interrupt pins
	 * would use a larger table */
	static int irq_list[] = {
		CFG_FIRST_PCI_IRQ,
		CFG_SECOND_PCI_IRQ,
		CFG_THIRD_PCI_IRQ,
		CFG_FORTH_PCI_IRQ
	};
	static int next_irq_index=0;

	char tmp_pin;
	int pin;

	pci_hose_read_config_byte(hose, dev, PCI_INTERRUPT_PIN, &tmp_pin);
	pin = tmp_pin;

	pin-=1; /* pci config space use 1-based numbering */
	if (-1 == pin) {
		return; /* device use no irq */
	}


	/* map device number +  pin to a pin on the sc520 */
	switch (PCI_DEV(dev)) {
	case 20:
		pin+=SC520_PCI_INTA;
		break;

	case 19:
		pin+=SC520_PCI_INTB;
		break;

	case 18:
		pin+=SC520_PCI_INTC;
		break;

	case 17:
		pin+=SC520_PCI_INTD;
		break;

	default:
		return;
	}

	pin&=3; /* wrap around */

	if (sc520_pci_ints[pin] == -1) {
		/* re-route one interrupt for us */
		if (next_irq_index > 3) {
			return;
		}
		if (pci_sc520_set_irq(pin, irq_list[next_irq_index])) {
			return;
		}
		next_irq_index++;
	}


	if (-1 != sc520_pci_ints[pin]) {
		pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE,
					   sc520_pci_ints[pin]);
	}
	PRINTF("fixup_irq: device %d pin %c irq %d\n",
	       PCI_DEV(dev), 'A' + pin, sc520_pci_ints[pin]);
}

static struct pci_controller sc520_cdp_hose = {
	fixup_irq: pci_sc520_cdp_fixup_irq,
};

void pci_init_board(void)
{
	pci_sc520_init(&sc520_cdp_hose);
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
	write_mmcr_word(SC520_PIOPFS31_16, 0xf7ff); 	/* set the GPIO pin function 31-16 reg */
	write_mmcr_word(SC520_PIOPFS15_0, 0xffff);  	/* set the GPIO pin function 15-0 reg */
	write_mmcr_byte(SC520_CSPFS, 0xf8);  		/* set the CS pin function  reg */
	write_mmcr_byte(SC520_CLKSEL, 0x70);


	write_mmcr_byte(SC520_GPCSRT, 1);   /* set the GP CS offset */
	write_mmcr_byte(SC520_GPCSPW, 3);   /* set the GP CS pulse width */
	write_mmcr_byte(SC520_GPCSOFF, 1);  /* set the GP CS offset */
	write_mmcr_byte(SC520_GPRDW, 3);    /* set the RD pulse width */
	write_mmcr_byte(SC520_GPRDOFF, 1);  /* set the GP RD offset */
	write_mmcr_byte(SC520_GPWRW, 3);    /* set the GP WR pulse width */
	write_mmcr_byte(SC520_GPWROFF, 1);  /* set the GP WR offset */

	write_mmcr_word(SC520_BOOTCSCTL, 0x1823);		/* set up timing of BOOTCS */
	write_mmcr_word(SC520_ROMCS1CTL, 0x1823);		/* set up timing of ROMCS1 */
	write_mmcr_word(SC520_ROMCS2CTL, 0x1823);		/* set up timing of ROMCS2 */

	/* adjust the memory map:
	 * by default the first 256MB (0x00000000 - 0x0fffffff) is mapped to SDRAM
	 * and 256MB to 1G-128k  (0x1000000 - 0x37ffffff) is mapped to PCI mmio
	 * we need to map 1G-128k - 1G (0x38000000 - 0x3fffffff) to CS1 */


	/* SRAM = GPCS3 128k @ d0000-effff*/
	write_mmcr_long(SC520_PAR2,  0x4e00400d);

	/* IDE0 = GPCS6 1f0-1f7 */
	write_mmcr_long(SC520_PAR3,  0x380801f0);

	/* IDE1 = GPCS7 3f6 */
	write_mmcr_long(SC520_PAR4,  0x3c0003f6);
	/* bootcs */
	write_mmcr_long(SC520_PAR12, 0x8bffe800);
	/* romcs2 */
	write_mmcr_long(SC520_PAR13, 0xcbfff000);
	/* romcs1 */
	write_mmcr_long(SC520_PAR14, 0xabfff800);
	/* 680 LEDS */
	write_mmcr_long(SC520_PAR15, 0x30000640);

	write_mmcr_byte(SC520_ADDDECCTL, 0);

	asm ("wbinvd\n"); /* Flush cache, req. after setting the unchached attribute ona PAR */

	if (CFG_USE_SIO_UART) {
		write_mmcr_byte(SC520_ADDDECCTL, read_mmcr_byte(SC520_ADDDECCTL) | UART2_DIS|UART1_DIS);
		setup_ali_sio(1);
	} else {
		write_mmcr_byte(SC520_ADDDECCTL, read_mmcr_byte(SC520_ADDDECCTL) & ~(UART2_DIS|UART1_DIS));
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
 * This function should map a chunk of size bytes
 * of the system address space to the ISA bus
 *
 * The function will return the memory address
 * as seen by the host (which may very will be the
 * same as the bus address)
 */
u32 isa_map_rom(u32 bus_addr, int size)
{
	u32 par;

	PRINTF("isa_map_rom asked to map %d bytes at %x\n",
	       size, bus_addr);

	par = size;
	if (par < 0x80000) {
		par = 0x80000;
	}
	par >>= 12;
	par--;
	par&=0x7f;
	par <<= 18;
	par |= (bus_addr>>12);
	par |= 0x50000000;

	PRINTF ("setting PAR11 to %x\n", par);

	/* Map rom 0x10000 with PAR1 */
	write_mmcr_long(SC520_PAR11,  par);

	return bus_addr;
}

/*
 * this function removed any mapping created
 * with pci_get_rom_window()
 */
void isa_unmap_rom(u32 addr)
{
	PRINTF("isa_unmap_rom asked to unmap %x", addr);
	if ((addr>>12) == (read_mmcr_long(SC520_PAR11)&0x3ffff)) {
		write_mmcr_long(SC520_PAR11, 0);
		PRINTF(" done\n");
		return;
	}
	PRINTF(" not ours\n");
}

#ifdef CONFIG_PCI
#define PCI_ROM_TEMP_SPACE 0x10000
/*
 * This function should map a chunk of size bytes
 * of the system address space to the PCI bus,
 * suitable to map PCI ROMS (bus address < 16M)
 * the function will return the host memory address
 * which should be converted into a bus address
 * before used to configure the PCI rom address
 * decoder
 */
u32 pci_get_rom_window(struct pci_controller *hose, int size)
{
	u32 par;

	par = size;
	if (par < 0x80000) {
		par = 0x80000;
	}
	par >>= 16;
	par--;
	par&=0x7ff;
	par <<= 14;
	par |= (PCI_ROM_TEMP_SPACE>>16);
	par |= 0x72000000;

	PRINTF ("setting PAR1 to %x\n", par);

	/* Map rom 0x10000 with PAR1 */
	write_mmcr_long(SC520_PAR1,  par);

	return PCI_ROM_TEMP_SPACE;
}

/*
 * this function removed any mapping created
 * with pci_get_rom_window()
 */
void pci_remove_rom_window(struct pci_controller *hose, u32 addr)
{
	PRINTF("pci_remove_rom_window: %x", addr);
	if (addr == PCI_ROM_TEMP_SPACE) {
		write_mmcr_long(SC520_PAR1, 0);
		PRINTF(" done\n");
		return;
	}
	PRINTF(" not ours\n");

}

/*
 * This function is called in order to provide acces to the
 * legacy video I/O ports on the PCI bus.
 * After this function accesses to I/O ports 0x3b0-0x3bb and
 * 0x3c0-0x3df shuld result in transactions on the PCI bus.
 *
 */
int pci_enable_legacy_video_ports(struct pci_controller *hose)
{
	/* Map video memory to 0xa0000*/
	write_mmcr_long(SC520_PAR0,  0x7200400a);

	/* forward all I/O accesses to PCI */
	write_mmcr_byte(SC520_ADDDECCTL,
			read_mmcr_byte(SC520_ADDDECCTL) | IO_HOLE_DEST_PCI);


	/* so we map away all io ports to pci (only way to access pci io
	 * below 0x400. But then we have to map back the portions that we dont
	 * use so that the generate cycles on the GPIO bus where the sio and
	 * ISA slots are connected, this requre the use of several PAR registers
	 */

	/* bring 0x100 - 0x1ef back to ISA using PAR5 */
	write_mmcr_long(SC520_PAR5, 0x30ef0100);

	/* IDE use 1f0-1f7 */

	/* bring 0x1f8 - 0x2f7 back to ISA using PAR6 */
	write_mmcr_long(SC520_PAR6, 0x30ff01f8);

	/* com2 use 2f8-2ff */

	/* bring 0x300 - 0x3af back to ISA using PAR7 */
	write_mmcr_long(SC520_PAR7, 0x30af0300);

	/* vga use 3b0-3bb */

	/* bring 0x3bc - 0x3bf back to ISA using PAR8 */
	write_mmcr_long(SC520_PAR8, 0x300303bc);

	/* vga use 3c0-3df */

	/* bring 0x3e0 - 0x3f5 back to ISA using PAR9 */
	write_mmcr_long(SC520_PAR9, 0x301503e0);

	/* ide use 3f6 */

	/* bring 0x3f7  back to ISA using PAR10 */
	write_mmcr_long(SC520_PAR10, 0x300003f7);

	/* com1 use 3f8-3ff */

	return 0;
}
#endif

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	init_sc520();
	bus_init();
	irq_init();

	/* max drive current on SDRAM */
	write_mmcr_word(SC520_DSCTL, 0x0100);

	/* enter debug mode after next reset (only if jumper is also set) */
	write_mmcr_byte(SC520_RESCFG, 0x08);
	/* configure the software timer to 33.333MHz */
	write_mmcr_byte(SC520_SWTMRCFG, 0);
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

int spi_eeprom_read(int x, int offset, char *buffer, int len)
{
       return 0;
}

int spi_eeprom_write(int x, int offset, char *buffer, int len)
{
       return 0;
}

void spi_init_f(void)
{
#ifdef CONFIG_SC520_CDP_USE_SPI
	spi_eeprom_probe(1);
#endif
#ifdef CONFIG_SC520_CDP_USE_MW
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

#ifdef CONFIG_SC520_CDP_USE_SPI
	res = spi_eeprom_read(1, offset, buffer, len);
#endif
#ifdef CONFIG_SC520_CDP_USE_MW
	res = mw_eeprom_read(2, offset, buffer, len);
#endif
#if !defined(CONFIG_SC520_CDP_USE_SPI) && !defined(CONFIG_SC520_CDP_USE_MW)
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

#ifdef CONFIG_SC520_CDP_USE_SPI
	res = spi_eeprom_write(1, offset, buffer, len);
#endif
#ifdef CONFIG_SC520_CDP_USE_MW
	res = mw_eeprom_write(2, offset, buffer, len);
#endif
#if !defined(CONFIG_SC520_CDP_USE_SPI) && !defined(CONFIG_SC520_CDP_USE_MW)
	res = 0;
#endif
	return res;
}
