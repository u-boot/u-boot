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

/* Get a virtual address associated with a BAR region */
void *pci_map_bar(pci_dev_t pdev, int bar, int flags)
{
	pci_addr_t pci_bus_addr;
	u32 bar_response;

	/* read BAR address */
	pci_read_config_dword(pdev, bar, &bar_response);
	pci_bus_addr = (pci_addr_t)(bar_response & ~0xf);

	/*
	 * Pass "0" as the length argument to pci_bus_to_virt.  The arg
	 * isn't actualy used on any platform because u-boot assumes a static
	 * linear mapping.  In the future, this could read the BAR size
	 * and pass that as the size if needed.
	 */
	return pci_bus_to_virt(pdev, pci_bus_addr, flags, 0, MAP_NOCACHE);
}

/*
 *
 */

static struct pci_controller* hose_head;

void pci_register_hose(struct pci_controller* hose)
{
	struct pci_controller **phose = &hose_head;

	while(*phose)
		phose = &(*phose)->next;

	hose->next = NULL;

	*phose = hose;
}

struct pci_controller *pci_bus_to_hose(int bus)
{
	struct pci_controller *hose;

	for (hose = hose_head; hose; hose = hose->next) {
		if (bus >= hose->first_busno && bus <= hose->last_busno)
			return hose;
	}

	printf("pci_bus_to_hose() failed\n");
	return NULL;
}

struct pci_controller *find_hose_by_cfg_addr(void *cfg_addr)
{
	struct pci_controller *hose;

	for (hose = hose_head; hose; hose = hose->next) {
		if (hose->cfg_addr == cfg_addr)
			return hose;
	}

	return NULL;
}

int pci_last_busno(void)
{
	struct pci_controller *hose = hose_head;

	if (!hose)
		return -1;

	while (hose->next)
		hose = hose->next;

	return hose->last_busno;
}

pci_dev_t pci_find_devices(struct pci_device_id *ids, int index)
{
	struct pci_controller * hose;
	u16 vendor, device;
	u8 header_type;
	pci_dev_t bdf;
	int i, bus, found_multi = 0;

	for (hose = hose_head; hose; hose = hose->next) {
#ifdef CONFIG_SYS_SCSI_SCAN_BUS_REVERSE
		for (bus = hose->last_busno; bus >= hose->first_busno; bus--)
#else
		for (bus = hose->first_busno; bus <= hose->last_busno; bus++)
#endif
			for (bdf = PCI_BDF(bus, 0, 0);
#if defined(CONFIG_ELPPC) || defined(CONFIG_PPMC7XX)
			     bdf < PCI_BDF(bus, PCI_MAX_PCI_DEVICES - 1,
				PCI_MAX_PCI_FUNCTIONS - 1);
#else
			     bdf < PCI_BDF(bus + 1, 0, 0);
#endif
			     bdf += PCI_BDF(0, 0, 1)) {
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

				for (i = 0; ids[i].vendor != 0; i++) {
					if (vendor == ids[i].vendor &&
					    device == ids[i].device) {
						if (index <= 0)
							return bdf;

						index--;
					}
				}
			}
	}

	return -1;
}

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

int __pci_hose_phys_to_bus(struct pci_controller *hose,
				phys_addr_t phys_addr,
				unsigned long flags,
				unsigned long skip_mask,
				pci_addr_t *ba)
{
	struct pci_region *res;
	pci_addr_t bus_addr;
	int i;

	for (i = 0; i < hose->region_count; i++) {
		res = &hose->regions[i];

		if (((res->flags ^ flags) & PCI_REGION_TYPE) != 0)
			continue;

		if (res->flags & skip_mask)
			continue;

		bus_addr = phys_addr - res->phys_start + res->bus_start;

		if (bus_addr >= res->bus_start &&
			bus_addr < res->bus_start + res->size) {
			*ba = bus_addr;
			return 0;
		}
	}

	return 1;
}

pci_addr_t pci_hose_phys_to_bus (struct pci_controller *hose,
				    phys_addr_t phys_addr,
				    unsigned long flags)
{
	pci_addr_t bus_addr = 0;
	int ret;

	if (!hose) {
		puts("pci_hose_phys_to_bus: invalid hose\n");
		return bus_addr;
	}

	/*
	 * if PCI_REGION_MEM is set we do a two pass search with preference
	 * on matches that don't have PCI_REGION_SYS_MEMORY set
	 */
	if ((flags & PCI_REGION_MEM) == PCI_REGION_MEM) {
		ret = __pci_hose_phys_to_bus(hose, phys_addr,
				flags, PCI_REGION_SYS_MEMORY, &bus_addr);
		if (!ret)
			return bus_addr;
	}

	ret = __pci_hose_phys_to_bus(hose, phys_addr, flags, 0, &bus_addr);

	if (ret)
		puts("pci_hose_phys_to_bus: invalid physical address\n");

	return bus_addr;
}

int __pci_hose_bus_to_phys(struct pci_controller *hose,
				pci_addr_t bus_addr,
				unsigned long flags,
				unsigned long skip_mask,
				phys_addr_t *pa)
{
	struct pci_region *res;
	int i;

	for (i = 0; i < hose->region_count; i++) {
		res = &hose->regions[i];

		if (((res->flags ^ flags) & PCI_REGION_TYPE) != 0)
			continue;

		if (res->flags & skip_mask)
			continue;

		if (bus_addr >= res->bus_start &&
			bus_addr < res->bus_start + res->size) {
			*pa = (bus_addr - res->bus_start + res->phys_start);
			return 0;
		}
	}

	return 1;
}

phys_addr_t pci_hose_bus_to_phys(struct pci_controller* hose,
				 pci_addr_t bus_addr,
				 unsigned long flags)
{
	phys_addr_t phys_addr = 0;
	int ret;

	if (!hose) {
		puts("pci_hose_bus_to_phys: invalid hose\n");
		return phys_addr;
	}

	/*
	 * if PCI_REGION_MEM is set we do a two pass search with preference
	 * on matches that don't have PCI_REGION_SYS_MEMORY set
	 */
	if ((flags & PCI_REGION_MEM) == PCI_REGION_MEM) {
		ret = __pci_hose_bus_to_phys(hose, bus_addr,
				flags, PCI_REGION_SYS_MEMORY, &phys_addr);
		if (!ret)
			return phys_addr;
	}

	ret = __pci_hose_bus_to_phys(hose, bus_addr, flags, 0, &phys_addr);

	if (ret)
		puts("pci_hose_bus_to_phys: invalid physical address\n");

	return phys_addr;
}

/*
 *
 */

int pci_hose_config_device(struct pci_controller *hose,
			   pci_dev_t dev,
			   unsigned long io,
			   pci_addr_t mem,
			   unsigned long command)
{
	u32 bar_response;
	unsigned int old_command;
	pci_addr_t bar_value;
	pci_size_t bar_size;
	unsigned char pin;
	int bar, found_mem64;

	debug("PCI Config: I/O=0x%lx, Memory=0x%llx, Command=0x%lx\n", io,
		(u64)mem, command);

	pci_hose_write_config_dword(hose, dev, PCI_COMMAND, 0);

	for (bar = PCI_BASE_ADDRESS_0; bar <= PCI_BASE_ADDRESS_5; bar += 4) {
		pci_hose_write_config_dword(hose, dev, bar, 0xffffffff);
		pci_hose_read_config_dword(hose, dev, bar, &bar_response);

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
				PCI_BASE_ADDRESS_MEM_TYPE_64) {
				u32 bar_response_upper;
				u64 bar64;
				pci_hose_write_config_dword(hose, dev, bar + 4,
					0xffffffff);
				pci_hose_read_config_dword(hose, dev, bar + 4,
					&bar_response_upper);

				bar64 = ((u64)bar_response_upper << 32) | bar_response;

				bar_size = ~(bar64 & PCI_BASE_ADDRESS_MEM_MASK) + 1;
				found_mem64 = 1;
			} else {
				bar_size = (u32)(~(bar_response & PCI_BASE_ADDRESS_MEM_MASK) + 1);
			}

			/* round up region base address to multiple of size */
			mem = ((mem - 1) | (bar_size - 1)) + 1;
			bar_value = mem;
			/* compute new region base address */
			mem = mem + bar_size;
		}

		/* Write it out and update our limit */
		pci_hose_write_config_dword (hose, dev, bar, (u32)bar_value);

		if (found_mem64) {
			bar += 4;
#ifdef CONFIG_SYS_PCI_64BIT
			pci_hose_write_config_dword(hose, dev, bar,
				(u32)(bar_value >> 32));
#else
			pci_hose_write_config_dword(hose, dev, bar, 0x00000000);
#endif
		}
	}

	/* Configure Cache Line Size Register */
	pci_hose_write_config_byte(hose, dev, PCI_CACHE_LINE_SIZE, 0x08);

	/* Configure Latency Timer */
	pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);

	/* Disable interrupt line, if device says it wants to use interrupts */
	pci_hose_read_config_byte(hose, dev, PCI_INTERRUPT_PIN, &pin);
	if (pin != 0) {
		pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, 0xff);
	}

	pci_hose_read_config_dword(hose, dev, PCI_COMMAND, &old_command);
	pci_hose_write_config_dword(hose, dev, PCI_COMMAND,
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
	pci_hose_config_device(hose, dev, entry->priv[0], entry->priv[1],
		entry->priv[2]);
}

void pci_cfgfunc_do_nothing(struct pci_controller *hose,
			    pci_dev_t dev, struct pci_config_table *entry)
{
}

/*
 * HJF: Changed this to return int. I think this is required
 * to get the correct result when scanning bridges
 */
extern int pciauto_config_device(struct pci_controller *hose, pci_dev_t dev);

#if defined(CONFIG_CMD_PCI) || defined(CONFIG_PCI_SCAN_SHOW)
const char * pci_class_str(u8 class)
{
	switch (class) {
	case PCI_CLASS_NOT_DEFINED:
		return "Build before PCI Rev2.0";
		break;
	case PCI_BASE_CLASS_STORAGE:
		return "Mass storage controller";
		break;
	case PCI_BASE_CLASS_NETWORK:
		return "Network controller";
		break;
	case PCI_BASE_CLASS_DISPLAY:
		return "Display controller";
		break;
	case PCI_BASE_CLASS_MULTIMEDIA:
		return "Multimedia device";
		break;
	case PCI_BASE_CLASS_MEMORY:
		return "Memory controller";
		break;
	case PCI_BASE_CLASS_BRIDGE:
		return "Bridge device";
		break;
	case PCI_BASE_CLASS_COMMUNICATION:
		return "Simple comm. controller";
		break;
	case PCI_BASE_CLASS_SYSTEM:
		return "Base system peripheral";
		break;
	case PCI_BASE_CLASS_INPUT:
		return "Input device";
		break;
	case PCI_BASE_CLASS_DOCKING:
		return "Docking station";
		break;
	case PCI_BASE_CLASS_PROCESSOR:
		return "Processor";
		break;
	case PCI_BASE_CLASS_SERIAL:
		return "Serial bus controller";
		break;
	case PCI_BASE_CLASS_INTELLIGENT:
		return "Intelligent controller";
		break;
	case PCI_BASE_CLASS_SATELLITE:
		return "Satellite controller";
		break;
	case PCI_BASE_CLASS_CRYPT:
		return "Cryptographic device";
		break;
	case PCI_BASE_CLASS_SIGNAL_PROCESSING:
		return "DSP";
		break;
	case PCI_CLASS_OTHERS:
		return "Does not fit any class";
		break;
	default:
	return  "???";
		break;
	};
}
#endif /* CONFIG_CMD_PCI || CONFIG_PCI_SCAN_SHOW */

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
	unsigned int sub_bus, found_multi = 0;
	unsigned short vendor, device, class;
	unsigned char header_type;
#ifndef CONFIG_PCI_PNP
	struct pci_config_table *cfg;
#endif
	pci_dev_t dev;
#ifdef CONFIG_PCI_SCAN_SHOW
	static int indent = 0;
#endif

	sub_bus = bus;

	for (dev =  PCI_BDF(bus,0,0);
	     dev <  PCI_BDF(bus, PCI_MAX_PCI_DEVICES - 1,
				PCI_MAX_PCI_FUNCTIONS - 1);
	     dev += PCI_BDF(0, 0, 1)) {

		if (pci_skip_dev(hose, dev))
			continue;

		if (PCI_FUNC(dev) && !found_multi)
			continue;

		pci_hose_read_config_byte(hose, dev, PCI_HEADER_TYPE, &header_type);

		pci_hose_read_config_word(hose, dev, PCI_VENDOR_ID, &vendor);

		if (vendor == 0xffff || vendor == 0x0000)
			continue;

		if (!PCI_FUNC(dev))
			found_multi = header_type & 0x80;

		debug("PCI Scan: Found Bus %d, Device %d, Function %d\n",
			PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));

		pci_hose_read_config_word(hose, dev, PCI_DEVICE_ID, &device);
		pci_hose_read_config_word(hose, dev, PCI_CLASS_DEVICE, &class);

#ifdef CONFIG_PCI_SCAN_SHOW
		indent++;

		/* Print leading space, including bus indentation */
		printf("%*c", indent + 1, ' ');

		if (pci_print_dev(hose, dev)) {
			printf("%02x:%02x.%-*x - %04x:%04x - %s\n",
			       PCI_BUS(dev), PCI_DEV(dev), 6 - indent, PCI_FUNC(dev),
			       vendor, device, pci_class_str(class >> 8));
		}
#endif

#ifdef CONFIG_PCI_PNP
		sub_bus = max(pciauto_config_device(hose, dev), sub_bus);
#else
		cfg = pci_find_config(hose, class, vendor, device,
				      PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));
		if (cfg) {
			cfg->config_device(hose, dev, cfg);
			sub_bus = max(sub_bus, hose->current_busno);
		}
#endif

#ifdef CONFIG_PCI_SCAN_SHOW
		indent--;
#endif

		if (hose->fixup_irq)
			hose->fixup_irq(hose, dev);
	}

	return sub_bus;
}

int pci_hose_scan(struct pci_controller *hose)
{
#if defined(CONFIG_PCI_BOOTDELAY)
	static int pcidelay_done;
	char *s;
	int i;

	if (!pcidelay_done) {
		/* wait "pcidelay" ms (if defined)... */
		s = getenv("pcidelay");
		if (s) {
			int val = simple_strtoul(s, NULL, 10);
			for (i = 0; i < val; i++)
				udelay(1000);
		}
		pcidelay_done = 1;
	}
#endif /* CONFIG_PCI_BOOTDELAY */

	/*
	 * Start scan at current_busno.
	 * PCIe will start scan at first_busno+1.
	 */
	/* For legacy support, ensure current >= first */
	if (hose->first_busno > hose->current_busno)
		hose->current_busno = hose->first_busno;
#ifdef CONFIG_PCI_PNP
	pciauto_config_init(hose);
#endif
	return pci_hose_scan_bus(hose, hose->current_busno);
}

void pci_init(void)
{
	hose_head = NULL;

	/* now call board specific pci_init()... */
	pci_init_board();
}
