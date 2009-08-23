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
#include <netdev.h>
#include <ds1722.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/ic/sc520.h>
#include <asm/ic/pci.h>
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


/* PCI stuff */
static void pci_sc520_spunk_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	int version = sc520_mmcr->sysinfo;

	/* a configurable lists of irqs to steal
	 * when we need one (a board with more pci interrupt pins
	 * would use a larger table */
	static int irq_list[] = {
		CONFIG_SYS_FIRST_PCI_IRQ,
		CONFIG_SYS_SECOND_PCI_IRQ,
		CONFIG_SYS_THIRD_PCI_IRQ,
		CONFIG_SYS_FORTH_PCI_IRQ
	};
	static int next_irq_index=0;

	uchar tmp_pin;
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
	sc520_mmcr->par[11] = par;

	return bus_addr;
}

/*
 * this function removed any mapping created
 * with pci_get_rom_window()
 */
void isa_unmap_rom(u32 addr)
{
	printf("isa_unmap_rom asked to unmap %x", addr);
	if ((addr>>12) == (sc520_mmcr->par[11] & 0x3ffff)) {
		sc520_mmcr->par[11] = 0;
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
	sc520_mmcr->par[1] = par;

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
		sc520_mmcr->par[1] = 0;
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
	sc520_mmcr->par[0] = 0x7200400a;

	/* forward all I/O accesses to PCI */
	sc520_mmcr->adddecctl = sc520_mmcr->adddecctl | IO_HOLE_DEST_PCI;


	/* so we map away all io ports to pci (only way to access pci io
	 * below 0x400. But then we have to map back the portions that we dont
	 * use so that the generate cycles on the GPIO bus where the sio and
	 * ISA slots are connected, this requre the use of several PAR registers
	 */

	/* bring 0x100 - 0x2f7 back to ISA using PAR5 */
	sc520_mmcr->par[5] = 0x31f70100;

	/* com2 use 2f8-2ff */

	/* bring 0x300 - 0x3af back to ISA using PAR7 */
	sc520_mmcr->par[7] = 0x30af0300;

	/* vga use 3b0-3bb */

	/* bring 0x3bc - 0x3bf back to ISA using PAR8 */
	sc520_mmcr->par[8] = 0x300303bc;

	/* vga use 3c0-3df */

	/* bring 0x3e0 - 0x3f7 back to ISA using PAR9 */
	sc520_mmcr->par[9] = 0x301703e0;

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
