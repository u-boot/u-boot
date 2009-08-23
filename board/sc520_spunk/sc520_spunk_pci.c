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
#include <ds1722.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/ic/sc520.h>
#include <asm/ic/pci.h>

DECLARE_GLOBAL_DATA_PTR;

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
