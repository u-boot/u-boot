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
#include <ssi.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/ic/sc520.h>

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
	write_mmcr_byte(SC520_SSIMAP, SC520_IRQ6);              /* Set Synchronius serial INT to IRQ6*/
	write_mmcr_byte(SC520_WDTMAP, SC520_IRQ_DISABLED);      /* disable Watchdog INT */
	write_mmcr_byte(SC520_RTCMAP, SC520_IRQ8);              /* Set RTC int to 8 */
	write_mmcr_byte(SC520_WPVMAP, SC520_IRQ_DISABLED);      /* disable write protect INT */
	write_mmcr_byte(SC520_ICEMAP, SC520_IRQ1);              /* Set ICE Debug Serielport INT to IRQ1 */
	write_mmcr_byte(SC520_FERRMAP,SC520_IRQ13);             /* Set FP error INT to IRQ13 */

	write_mmcr_byte(SC520_UART1MAP, SC520_IRQ4);            /* Set internal UART2 INT to IRQ4 */
	write_mmcr_byte(SC520_UART2MAP, SC520_IRQ3);            /* Set internal UART2 INT to IRQ3 */

	write_mmcr_byte(SC520_GP0IMAP, SC520_IRQ7);             /* Set GPIRQ0 (PC-Card AUX IRQ) to IRQ7 */
	write_mmcr_byte(SC520_GP1IMAP, SC520_IRQ14);            /* Set GPIRQ1 (CF IRQ) to IRQ14 */
	write_mmcr_byte(SC520_GP3IMAP, SC520_IRQ5);             /* Set GPIRQ3 ( CAN IRQ ) ti IRQ5 */
	write_mmcr_byte(SC520_GP4IMAP, SC520_IRQ_DISABLED);     /* disbale GIRQ4 ( IRR IRQ ) */
	write_mmcr_byte(SC520_GP5IMAP, SC520_IRQ_DISABLED);     /* disable GPIRQ5 */
	write_mmcr_byte(SC520_GP6IMAP, SC520_IRQ_DISABLED);     /* disable GPIRQ6 */
	write_mmcr_byte(SC520_GP7IMAP, SC520_IRQ_DISABLED);     /* disable GPIRQ7 */
	write_mmcr_byte(SC520_GP8IMAP, SC520_IRQ_DISABLED);     /* disable GPIRQ8 */
	write_mmcr_byte(SC520_GP9IMAP, SC520_IRQ_DISABLED);     /* disable GPIRQ9 */
	write_mmcr_byte(SC520_GP2IMAP, SC520_IRQ_DISABLED);     /* disable GPIRQ2 */
	write_mmcr_byte(SC520_GP10IMAP,SC520_IRQ_DISABLED);     /* disable GPIRQ10 */

	write_mmcr_word(SC520_PCIHOSTMAP, 0x11f);               /* Map PCI hostbridge INT to NMI */
	write_mmcr_word(SC520_ECCMAP, 0x100);                   /* Map SDRAM ECC failure INT to NMI */

}


/* PCI stuff */
static void pci_sc520_spunk_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	int version = read_mmcr_byte(SC520_SYSINFO);

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
	case 6:  /* ETH0 */
		pin+=SC520_PCI_INTA;
		break;

	case 7:  /* ETH1 */
		pin+=SC520_PCI_INTB;
		break;

	case 8:  /* Crypto */
		pin+=SC520_PCI_INTC;
		break;

	case 9: /* PMC slot */
		pin+=SC520_PCI_INTD;
		break;

	case 10: /* PC-Card */

		if (version < 10) {
			pin+=SC520_PCI_INTD;
		} else {
			pin+=SC520_PCI_INTC;
		}
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
#if 0
	printf("fixup_irq: device %d pin %c irq %d\n",
	       PCI_DEV(dev), 'A' + pin, sc520_pci_ints[pin]);
#endif
}


static void pci_sc520_spunk_configure_cardbus(struct pci_controller *hose,
					      pci_dev_t dev, struct pci_config_table *te)
{
	u32 io_base;
	u32 temp;

	pciauto_config_device(hose, dev);

	pci_hose_write_config_word(hose, dev, PCI_COMMAND, 0x07);  /* enable device */
	pci_hose_write_config_byte(hose, dev, 0x0c, 0x10);         /* cacheline size */
	pci_hose_write_config_byte(hose, dev, 0x0d, 0x40);         /* latency timer */
	pci_hose_write_config_byte(hose, dev, 0x1b, 0x40);         /* cardbus latency timer */
	pci_hose_write_config_word(hose, dev, PCI_BRIDGE_CONTROL, 0x0040);  /* reset cardbus */
	pci_hose_write_config_word(hose, dev, PCI_BRIDGE_CONTROL, 0x0080);  /* route interrupts though ExCA */
	pci_hose_write_config_word(hose, dev,  0x44, 0x3e0); /* map legacy I/O port to 0x3e0 */

	pci_hose_read_config_dword(hose, dev,  0x80, &temp); /* System control */
	pci_hose_write_config_dword(hose, dev,  0x80, temp | 0x60); /* System control: disable clockrun */
	/* route MF0 to ~INT and MF3 to IRQ7
	 * reserve all others */
	pci_hose_write_config_dword(hose, dev, 0x8c, 0x00007002);
	pci_hose_write_config_byte(hose, dev, 0x91, 0x00);    /* card control */
	pci_hose_write_config_byte(hose, dev, 0x92, 0x62);    /* device control */

	if (te->device != 0xac56) {
		pci_hose_write_config_byte(hose, dev, 0x93, 0x21);    /* async interrupt enable */
		pci_hose_write_config_word(hose, dev, 0xa8, 0x0000);  /* reset GPIO */
		pci_hose_write_config_word(hose, dev, 0xac, 0x0000);  /* reset GPIO */
		pci_hose_write_config_word(hose, dev, 0xaa, 0x0000);  /* reset GPIO */
		pci_hose_write_config_word(hose, dev, 0xae, 0x0000);  /* reset GPIO */
	} else {
		pci_hose_write_config_byte(hose, dev, 0x93, 0x20);    /*  */
	}
	pci_hose_write_config_word(hose, dev, 0xa4, 0x8000);  /* reset power management */


	pci_hose_read_config_dword(hose, dev, PCI_BASE_ADDRESS_0, &io_base);
	io_base &= ~0xfL;

	writeb(0x07, io_base+0x803); /* route CSC irq though ExCA and enable IRQ7 */
	writel(0, io_base+0x10);     /* CLKRUN default */
	writel(0, io_base+0x20);     /* CLKRUN default */

}


static struct pci_config_table pci_sc520_spunk_config_table[] = {
	{ 0x104c, 0xac50, PCI_ANY_ID, 0, 0x0a, 0, pci_sc520_spunk_configure_cardbus, { 0, 0, 0} },
	{ 0x104c, 0xac56, PCI_ANY_ID, 0, 0x0a, 0, pci_sc520_spunk_configure_cardbus, { 0, 0, 0} },
	{ 0, 0, 0, 0, 0, 0, NULL, {0,0,0}}
};

static struct pci_controller sc520_spunk_hose = {
	fixup_irq: pci_sc520_spunk_fixup_irq,
	config_table: pci_sc520_spunk_config_table,
	first_busno: 0x00,
	last_busno: 0xff,
};

void pci_init_board(void)
{
	pci_sc520_init(&sc520_spunk_hose);
}


/* set up the ISA bus timing and system address mappings */
static void bus_init(void)
{
	/* versions
	 * 0   Hyglo versions 0.95 and 0.96 (large baords)
	 * ??  Hyglo version 0.97 (small board)
	 * 10  Spunk board
	 */
	int version = read_mmcr_byte(SC520_SYSINFO);

	if (version) {
		/* set up the GP IO pins (for the Spunk board) */
		write_mmcr_word(SC520_PIOPFS31_16, 0xfff0); 	/* set the GPIO pin function 31-16 reg */
		write_mmcr_word(SC520_PIOPFS15_0,  0x000f);  	/* set the GPIO pin function 15-0 reg */
		write_mmcr_word(SC520_PIODIR31_16, 0x000f); 	/* set the GPIO direction 31-16 reg */
		write_mmcr_word(SC520_PIODIR15_0,  0x1ff0);  	/* set the GPIO direction 15-0 reg */
		write_mmcr_byte(SC520_CSPFS, 0xc0);  		/* set the CS pin function reg */
		write_mmcr_byte(SC520_CLKSEL, 0x70);

		write_mmcr_word(SC520_PIOCLR31_16, 0x0003);     /* reset SSI chip-selects */
		write_mmcr_word(SC520_PIOSET31_16, 0x000c);

	} else {
		/* set up the GP IO pins (for the Hyglo board) */
		write_mmcr_word(SC520_PIOPFS31_16, 0xffc0); 	/* set the GPIO pin function 31-16 reg */
		write_mmcr_word(SC520_PIOPFS15_0, 0x1e7f);  	/* set the GPIO pin function 15-0 reg */
		write_mmcr_word(SC520_PIODIR31_16, 0x003f); 	/* set the GPIO direction 31-16 reg */
		write_mmcr_word(SC520_PIODIR15_0, 0xe180);  	/* set the GPIO direction 15-0 reg */
		write_mmcr_byte(SC520_CSPFS, 0x00);  		/* set the CS pin function reg */
		write_mmcr_byte(SC520_CLKSEL, 0x70);

		write_mmcr_word(SC520_PIOCLR15_0, 0x0180);      /* reset SSI chip-selects */
	}

	write_mmcr_byte(SC520_GPCSRT, 1);   /* set the GP CS offset */
	write_mmcr_byte(SC520_GPCSPW, 3);   /* set the GP CS pulse width */
	write_mmcr_byte(SC520_GPCSOFF, 1);  /* set the GP CS offset */
	write_mmcr_byte(SC520_GPRDW, 3);    /* set the RD pulse width */
	write_mmcr_byte(SC520_GPRDOFF, 1);  /* set the GP RD offset */
	write_mmcr_byte(SC520_GPWRW, 3);    /* set the GP WR pulse width */
	write_mmcr_byte(SC520_GPWROFF, 1);  /* set the GP WR offset */

	write_mmcr_word(SC520_BOOTCSCTL, 0x0407);		/* set up timing of BOOTCS */

	/* adjust the memory map:
	 * by default the first 256MB (0x00000000 - 0x0fffffff) is mapped to SDRAM
	 * and 256MB to 1G-128k  (0x1000000 - 0x37ffffff) is mapped to PCI mmio
	 * we need to map 1G-128k - 1G (0x38000000 - 0x3fffffff) to CS1 */


	/* bootcs */
	write_mmcr_long(SC520_PAR12, 0x8bffe800);

	/* IDE0 = GPCS6 1f0-1f7 */
	write_mmcr_long(SC520_PAR3,  0x380801f0);

	/* IDE1 = GPCS7 3f6 */
	write_mmcr_long(SC520_PAR4,  0x3c0003f6);

	asm ("wbinvd\n"); /* Flush cache, req. after setting the unchached attribute ona PAR */

	write_mmcr_byte(SC520_ADDDECCTL, read_mmcr_byte(SC520_ADDDECCTL) & ~(UART2_DIS|UART1_DIS));

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

	printf("isa_map_rom asked to map %d bytes at %x\n",
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

	printf ("setting PAR11 to %x\n", par);

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
	printf("isa_unmap_rom asked to unmap %x", addr);
	if ((addr>>12) == (read_mmcr_long(SC520_PAR11)&0x3ffff)) {
		write_mmcr_long(SC520_PAR11, 0);
		printf(" done\n");
		return;
	}
	printf(" not ours\n");
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

	printf ("setting PAR1 to %x\n", par);

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
	printf("pci_remove_rom_window: %x", addr);
	if (addr == PCI_ROM_TEMP_SPACE) {
		write_mmcr_long(SC520_PAR1, 0);
		printf(" done\n");
		return;
	}
	printf(" not ours\n");

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

	/* bring 0x100 - 0x2f7 back to ISA using PAR5 */
	write_mmcr_long(SC520_PAR5, 0x31f70100);

	/* com2 use 2f8-2ff */

	/* bring 0x300 - 0x3af back to ISA using PAR7 */
	write_mmcr_long(SC520_PAR7, 0x30af0300);

	/* vga use 3b0-3bb */

	/* bring 0x3bc - 0x3bf back to ISA using PAR8 */
	write_mmcr_long(SC520_PAR8, 0x300303bc);

	/* vga use 3c0-3df */

	/* bring 0x3e0 - 0x3f7 back to ISA using PAR9 */
	write_mmcr_long(SC520_PAR9, 0x301703e0);

	/* com1 use 3f8-3ff */

	return 0;
}
#endif

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init(void)
{
	init_sc520();
	bus_init();
	irq_init();

	/* max drive current on SDRAM */
	write_mmcr_word(SC520_DSCTL, 0x0100);

	/* enter debug mode after next reset (only if jumper is also set) */
	write_mmcr_byte(SC520_RESCFG, 0x08);
	/* configure the software timer to 33.000MHz */
	write_mmcr_byte(SC520_SWTMRCFG, 1);
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
	int version = read_mmcr_byte(SC520_SYSINFO);

	if (version == 0) {
		/* PIO31-PIO16 Data */
		write_mmcr_word(SC520_PIODATA31_16,
				(read_mmcr_word(SC520_PIODATA31_16) & 0xffc0)| ((val&0x7e)>>1)); /* 0x1f8 >> 3 */

		/* PIO0-PIO15 Data */
		write_mmcr_word(SC520_PIODATA15_0,
				(read_mmcr_word(SC520_PIODATA15_0) & 0x1fff)| ((val&0x7)<<13));
	} else {
		/* newer boards use PIO4-PIO12 */
		/* PIO0-PIO15 Data */
#if 0
		val = (val & 0x007) | ((val & 0x038) << 3) | ((val & 0x1c0) >> 3);
#else
		val = (val & 0x007) | ((val & 0x07e) << 2);
#endif
		write_mmcr_word(SC520_PIODATA15_0,
				(read_mmcr_word(SC520_PIODATA15_0) & 0xe00f) | ((val&0x01ff)<<4));
	}
}


int last_stage_init(void)
{

	int version = read_mmcr_byte(SC520_SYSINFO);

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
	int version = read_mmcr_byte(SC520_SYSINFO);

	if (version) {
		/* Spunk board: EEPROM and CAN are actove-low, TEMP and AUX are active high */
		switch (dev) {
		case 1: /* EEPROM */
			write_mmcr_word(SC520_PIOCLR31_16, 0x0004);
			break;

		case 2: /* Temp Probe */
			write_mmcr_word(SC520_PIOSET31_16, 0x0002);
			break;

		case 3: /* CAN */
			write_mmcr_word(SC520_PIOCLR31_16, 0x0008);
			break;

		case 4: /* AUX */
			write_mmcr_word(SC520_PIOSET31_16, 0x0001);
			break;

		case 0:
			write_mmcr_word(SC520_PIOCLR31_16, 0x0003);
			write_mmcr_word(SC520_PIOSET31_16, 0x000c);
			break;

		default:
			printf("Illegal SSI device requested: %d\n", dev);
		}
	} else {

		/* Globox board: Both EEPROM and TEMP are active-high */

		switch (dev) {
		case 1: /* EEPROM */
			write_mmcr_word(SC520_PIOSET15_0, 0x0100);
			break;

		case 2: /* Temp Probe */
			write_mmcr_word(SC520_PIOSET15_0, 0x0080);
			break;

		case 0:
			write_mmcr_word(SC520_PIOCLR15_0, 0x0180);
			break;

		default:
			printf("Illegal SSI device requested: %d\n", dev);
		}
	}
}


void spi_init_f(void)
{
	read_mmcr_byte(SC520_SYSINFO) ?
		spi_eeprom_probe(1) :
	mw_eeprom_probe(1);

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

	return 	read_mmcr_byte(SC520_SYSINFO) ?
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

	return 	read_mmcr_byte(SC520_SYSINFO) ?
		spi_eeprom_write(1, offset, buffer, len) :
	mw_eeprom_write(1, offset, buffer, len);
}
