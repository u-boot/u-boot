/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2002, 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * PCI routines
 */

#include <common.h>

#ifdef CONFIG_PCI

#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>

#define PCI_HOSE_OP(rw, size, type)					\
int pci_hose_##rw##_config_##size(struct pci_controller *hose,		\
				  pci_dev_t dev,			\
				  int offset, type value)		\
{									\
	return hose->rw##_##size(hose, dev, offset, value);		\
}

PCI_HOSE_OP(read, byte, u8 *)
PCI_HOSE_OP(read, word, u16 *)
PCI_HOSE_OP(read, dword, u32 *)
PCI_HOSE_OP(write, byte, u8)
PCI_HOSE_OP(write, word, u16)
PCI_HOSE_OP(write, dword, u32)

#ifndef CONFIG_IXP425
#define PCI_OP(rw, size, type, error_code)				\
int pci_##rw##_config_##size(pci_dev_t dev, int offset, type value)	\
{									\
	struct pci_controller *hose = pci_bus_to_hose(PCI_BUS(dev));	\
									\
	if (!hose)							\
	{								\
		error_code;						\
		return -1;						\
	}								\
									\
	return pci_hose_##rw##_config_##size(hose, dev, offset, value);	\
}

PCI_OP(read, byte, u8 *, *value = 0xff)
PCI_OP(read, word, u16 *, *value = 0xffff)
PCI_OP(read, dword, u32 *, *value = 0xffffffff)
PCI_OP(write, byte, u8, )
PCI_OP(write, word, u16, )
PCI_OP(write, dword, u32, )
#endif	/* CONFIG_IXP425 */

#define PCI_READ_VIA_DWORD_OP(size, type, off_mask)			\
int pci_hose_read_config_##size##_via_dword(struct pci_controller *hose,\
					pci_dev_t dev,			\
					int offset, type val)		\
{									\
	u32 val32;							\
									\
	if (pci_hose_read_config_dword(hose, dev, offset & 0xfc, &val32) < 0) {	\
		*val = -1;						\
		return -1;						\
	}								\
									\
	*val = (val32 >> ((offset & (int)off_mask) * 8));		\
									\
	return 0;							\
}

#define PCI_WRITE_VIA_DWORD_OP(size, type, off_mask, val_mask)		\
int pci_hose_write_config_##size##_via_dword(struct pci_controller *hose,\
					     pci_dev_t dev,		\
					     int offset, type val)	\
{									\
	u32 val32, mask, ldata, shift;					\
									\
	if (pci_hose_read_config_dword(hose, dev, offset & 0xfc, &val32) < 0)\
		return -1;						\
									\
	shift = ((offset & (int)off_mask) * 8);				\
	ldata = (((unsigned long)val) & val_mask) << shift;		\
	mask = val_mask << shift;					\
	val32 = (val32 & ~mask) | ldata;				\
									\
	if (pci_hose_write_config_dword(hose, dev, offset & 0xfc, val32) < 0)\
		return -1;						\
									\
	return 0;							\
}

PCI_READ_VIA_DWORD_OP(byte, u8 *, 0x03)
PCI_READ_VIA_DWORD_OP(word, u16 *, 0x02)
PCI_WRITE_VIA_DWORD_OP(byte, u8, 0x03, 0x000000ff)
PCI_WRITE_VIA_DWORD_OP(word, u16, 0x02, 0x0000ffff)

/*
 *
 */

static struct pci_controller* hose_head = NULL;

void pci_register_hose(struct pci_controller* hose)
{
	struct pci_controller **phose = &hose_head;

	while(*phose)
		phose = &(*phose)->next;

	hose->next = NULL;

	*phose = hose;
}

struct pci_controller *pci_bus_to_hose (int bus)
{
	struct pci_controller *hose;

	for (hose = hose_head; hose; hose = hose->next)
		if (bus >= hose->first_busno && bus <= hose->last_busno)
			return hose;

	printf("pci_bus_to_hose() failed\n");
	return NULL;
}

#ifndef CONFIG_IXP425
pci_dev_t pci_find_devices(struct pci_device_id *ids, int index)
{
	struct pci_controller * hose;
	u16 vendor, device;
	u8 header_type;
	pci_dev_t bdf;
	int i, bus, found_multi = 0;

	for (hose = hose_head; hose; hose = hose->next)
	{
#ifdef CFG_SCSI_SCAN_BUS_REVERSE
		for (bus = hose->last_busno; bus >= hose->first_busno; bus--)
#else
		for (bus = hose->first_busno; bus <= hose->last_busno; bus++)
#endif
			for (bdf = PCI_BDF(bus,0,0);
#if defined(CONFIG_ELPPC) || defined(CONFIG_PPMC7XX)
			     bdf < PCI_BDF(bus,PCI_MAX_PCI_DEVICES-1,PCI_MAX_PCI_FUNCTIONS-1);
#else
			     bdf < PCI_BDF(bus+1,0,0);
#endif
			     bdf += PCI_BDF(0,0,1))
			{
				if (!PCI_FUNC(bdf)) {
					pci_read_config_byte(bdf,
							     PCI_HEADER_TYPE,
							     &header_type);

					found_multi = header_type & 0x80;
				} else {
					if (!found_multi)
						continue;
				}

				pci_read_config_word(bdf,
						     PCI_VENDOR_ID,
						     &vendor);
				pci_read_config_word(bdf,
						     PCI_DEVICE_ID,
						     &device);

				for (i=0; ids[i].vendor != 0; i++)
					if (vendor == ids[i].vendor &&
					    device == ids[i].device)
					{
						if (index <= 0)
							return bdf;

						index--;
					}
			}
	}

	return (-1);
}
#endif	/* CONFIG_IXP425 */

pci_dev_t pci_find_device(unsigned int vendor, unsigned int device, int index)
{
	static struct pci_device_id ids[2] = {{}, {0, 0}};

	ids[0].vendor = vendor;
	ids[0].device = device;

	return pci_find_devices(ids, index);
}

/*
 *
 */

unsigned long pci_hose_phys_to_bus (struct pci_controller *hose,
				    phys_addr_t phys_addr,
				    unsigned long flags)
{
	struct pci_region *res;
	unsigned long bus_addr;
	int i;

	if (!hose) {
		printf ("pci_hose_phys_to_bus: %s\n", "invalid hose");
		goto Done;
	}

	for (i = 0; i < hose->region_count; i++) {
		res = &hose->regions[i];

		if (((res->flags ^ flags) & PCI_REGION_TYPE) != 0)
			continue;

		bus_addr = phys_addr - res->phys_start + res->bus_start;

		if (bus_addr >= res->bus_start &&
			bus_addr < res->bus_start + res->size) {
			return bus_addr;
		}
	}

	printf ("pci_hose_phys_to_bus: %s\n", "invalid physical address");

Done:
	return 0;
}

phys_addr_t pci_hose_bus_to_phys(struct pci_controller* hose,
				 unsigned long bus_addr,
				 unsigned long flags)
{
	struct pci_region *res;
	int i;

	if (!hose) {
		printf ("pci_hose_bus_to_phys: %s\n", "invalid hose");
		goto Done;
	}

	for (i = 0; i < hose->region_count; i++) {
		res = &hose->regions[i];

		if (((res->flags ^ flags) & PCI_REGION_TYPE) != 0)
			continue;

		if (bus_addr >= res->bus_start &&
			bus_addr < res->bus_start + res->size) {
			return bus_addr - res->bus_start + res->phys_start;
		}
	}

	printf ("pci_hose_bus_to_phys: %s\n", "invalid physical address");

Done:
	return 0;
}

/*
 *
 */

int pci_hose_config_device(struct pci_controller *hose,
			   pci_dev_t dev,
			   unsigned long io,
			   unsigned long mem,
			   unsigned long command)
{
	unsigned int bar_response, bar_size, bar_value, old_command;
	unsigned char pin;
	int bar, found_mem64;

	debug ("PCI Config: I/O=0x%lx, Memory=0x%lx, Command=0x%lx\n",
		io, mem, command);

	pci_hose_write_config_dword (hose, dev, PCI_COMMAND, 0);

	for (bar = PCI_BASE_ADDRESS_0; bar < PCI_BASE_ADDRESS_5; bar += 4) {
		pci_hose_write_config_dword (hose, dev, bar, 0xffffffff);
		pci_hose_read_config_dword (hose, dev, bar, &bar_response);

		if (!bar_response)
			continue;

		found_mem64 = 0;

		/* Check the BAR type and set our address mask */
		if (bar_response & PCI_BASE_ADDRESS_SPACE) {
			bar_size = ~(bar_response & PCI_BASE_ADDRESS_IO_MASK) + 1;
			/* round up region base address to a multiple of size */
			io = ((io - 1) | (bar_size - 1)) + 1;
			bar_value = io;
			/* compute new region base address */
			io = io + bar_size;
		} else {
			if ((bar_response & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
				PCI_BASE_ADDRESS_MEM_TYPE_64)
				found_mem64 = 1;

			bar_size = ~(bar_response & PCI_BASE_ADDRESS_MEM_MASK) + 1;

			/* round up region base address to multiple of size */
			mem = ((mem - 1) | (bar_size - 1)) + 1;
			bar_value = mem;
			/* compute new region base address */
			mem = mem + bar_size;
		}

		/* Write it out and update our limit */
		pci_hose_write_config_dword (hose, dev, bar, bar_value);

		if (found_mem64) {
			bar += 4;
			pci_hose_write_config_dword (hose, dev, bar, 0x00000000);
		}
	}

	/* Configure Cache Line Size Register */
	pci_hose_write_config_byte (hose, dev, PCI_CACHE_LINE_SIZE, 0x08);

	/* Configure Latency Timer */
	pci_hose_write_config_byte (hose, dev, PCI_LATENCY_TIMER, 0x80);

	/* Disable interrupt line, if device says it wants to use interrupts */
	pci_hose_read_config_byte (hose, dev, PCI_INTERRUPT_PIN, &pin);
	if (pin != 0) {
		pci_hose_write_config_byte (hose, dev, PCI_INTERRUPT_LINE, 0xff);
	}

	pci_hose_read_config_dword (hose, dev, PCI_COMMAND, &old_command);
	pci_hose_write_config_dword (hose, dev, PCI_COMMAND,
				     (old_command & 0xffff0000) | command);

	return 0;
}

/*
 *
 */

struct pci_config_table *pci_find_config(struct pci_controller *hose,
					 unsigned short class,
					 unsigned int vendor,
					 unsigned int device,
					 unsigned int bus,
					 unsigned int dev,
					 unsigned int func)
{
	struct pci_config_table *table;

	for (table = hose->config_table; table && table->vendor; table++) {
		if ((table->vendor == PCI_ANY_ID || table->vendor == vendor) &&
		    (table->device == PCI_ANY_ID || table->device == device) &&
		    (table->class  == PCI_ANY_ID || table->class  == class)  &&
		    (table->bus    == PCI_ANY_ID || table->bus    == bus)    &&
		    (table->dev    == PCI_ANY_ID || table->dev    == dev)    &&
		    (table->func   == PCI_ANY_ID || table->func   == func)) {
			return table;
		}
	}

	return NULL;
}

void pci_cfgfunc_config_device(struct pci_controller *hose,
			       pci_dev_t dev,
			       struct pci_config_table *entry)
{
	pci_hose_config_device(hose, dev, entry->priv[0], entry->priv[1], entry->priv[2]);
}

void pci_cfgfunc_do_nothing(struct pci_controller *hose,
			    pci_dev_t dev, struct pci_config_table *entry)
{
}

/*
 *
 */

/* HJF: Changed this to return int. I think this is required
 * to get the correct result when scanning bridges
 */
extern int pciauto_config_device(struct pci_controller *hose, pci_dev_t dev);
extern void pciauto_config_init(struct pci_controller *hose);

int __pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
{
	/*
	 * Check if pci device should be skipped in configuration
	 */
	if (dev == PCI_BDF(hose->first_busno, 0, 0)) {
#if defined(CONFIG_PCI_CONFIG_HOST_BRIDGE) /* don't skip host bridge */
		/*
		 * Only skip configuration if "pciconfighost" is not set
		 */
		if (getenv("pciconfighost") == NULL)
			return 1;
#else
		return 1;
#endif
	}

	return 0;
}
int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
	__attribute__((weak, alias("__pci_skip_dev")));

#ifdef CONFIG_PCI_SCAN_SHOW
int __pci_print_dev(struct pci_controller *hose, pci_dev_t dev)
{
	if (dev == PCI_BDF(hose->first_busno, 0, 0))
		return 0;

	return 1;
}
int pci_print_dev(struct pci_controller *hose, pci_dev_t dev)
	__attribute__((weak, alias("__pci_print_dev")));
#endif /* CONFIG_PCI_SCAN_SHOW */

int pci_hose_scan_bus(struct pci_controller *hose, int bus)
{
	unsigned int sub_bus, found_multi=0;
	unsigned short vendor, device, class;
	unsigned char header_type;
	struct pci_config_table *cfg;
	pci_dev_t dev;

	sub_bus = bus;

	for (dev =  PCI_BDF(bus,0,0);
	     dev <  PCI_BDF(bus,PCI_MAX_PCI_DEVICES-1,PCI_MAX_PCI_FUNCTIONS-1);
	     dev += PCI_BDF(0,0,1)) {

		if (pci_skip_dev(hose, dev))
			continue;

		if (PCI_FUNC(dev) && !found_multi)
			continue;

		pci_hose_read_config_byte(hose, dev, PCI_HEADER_TYPE, &header_type);

		pci_hose_read_config_word(hose, dev, PCI_VENDOR_ID, &vendor);

		if (vendor != 0xffff && vendor != 0x0000) {

			if (!PCI_FUNC(dev))
				found_multi = header_type & 0x80;

			debug ("PCI Scan: Found Bus %d, Device %d, Function %d\n",
				PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev) );

			pci_hose_read_config_word(hose, dev, PCI_DEVICE_ID, &device);
			pci_hose_read_config_word(hose, dev, PCI_CLASS_DEVICE, &class);

			cfg = pci_find_config(hose, class, vendor, device,
					      PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));
			if (cfg) {
				cfg->config_device(hose, dev, cfg);
				sub_bus = max(sub_bus, hose->current_busno);
#ifdef CONFIG_PCI_PNP
			} else {
				int n = pciauto_config_device(hose, dev);

				sub_bus = max(sub_bus, n);
#endif
			}
			if (hose->fixup_irq)
				hose->fixup_irq(hose, dev);

#ifdef CONFIG_PCI_SCAN_SHOW
			if (pci_print_dev(hose, dev)) {
				unsigned char int_line;

				pci_hose_read_config_byte(hose, dev, PCI_INTERRUPT_LINE,
							  &int_line);
				printf("        %02x  %02x  %04x  %04x  %04x  %02x\n",
				       PCI_BUS(dev), PCI_DEV(dev), vendor, device, class,
				       int_line);
			}
#endif
		}
	}

	return sub_bus;
}

int pci_hose_scan(struct pci_controller *hose)
{
	/* Start scan at current_busno.
	 * PCIe will start scan at first_busno+1.
	 */
	/* For legacy support, ensure current>=first */
	if (hose->first_busno > hose->current_busno)
		hose->current_busno = hose->first_busno;
#ifdef CONFIG_PCI_PNP
	pciauto_config_init(hose);
#endif
	return pci_hose_scan_bus(hose, hose->current_busno);
}

void pci_init(void)
{
#if defined(CONFIG_PCI_BOOTDELAY)
	char *s;
	int i;

	/* wait "pcidelay" ms (if defined)... */
	s = getenv ("pcidelay");
	if (s) {
		int val = simple_strtoul (s, NULL, 10);
		for (i=0; i<val; i++)
			udelay (1000);
	}
#endif /* CONFIG_PCI_BOOTDELAY */

	/* now call board specific pci_init()... */
	pci_init_board();
}

#endif /* CONFIG_PCI */
