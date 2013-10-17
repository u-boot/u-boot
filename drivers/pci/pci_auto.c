/*
 * arch/powerpc/kernel/pci_auto.c
 *
 * PCI autoconfiguration library
 *
 * Author: Matt Porter <mporter@mvista.com>
 *
 * Copyright 2000 MontaVista Software Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <pci.h>

#undef DEBUG
#ifdef DEBUG
#define DEBUGF(x...) printf(x)
#else
#define DEBUGF(x...)
#endif /* DEBUG */

#define	PCIAUTO_IDE_MODE_MASK		0x05

/* the user can define CONFIG_SYS_PCI_CACHE_LINE_SIZE to avoid problems */
#ifndef CONFIG_SYS_PCI_CACHE_LINE_SIZE
#define CONFIG_SYS_PCI_CACHE_LINE_SIZE	8
#endif

/*
 *
 */

void pciauto_region_init(struct pci_region *res)
{
	/*
	 * Avoid allocating PCI resources from address 0 -- this is illegal
	 * according to PCI 2.1 and moreover, this is known to cause Linux IDE
	 * drivers to fail. Use a reasonable starting value of 0x1000 instead.
	 */
	res->bus_lower = res->bus_start ? res->bus_start : 0x1000;
}

void pciauto_region_align(struct pci_region *res, pci_size_t size)
{
	res->bus_lower = ((res->bus_lower - 1) | (size - 1)) + 1;
}

int pciauto_region_allocate(struct pci_region *res, pci_size_t size,
	pci_addr_t *bar)
{
	pci_addr_t addr;

	if (!res) {
		DEBUGF("No resource");
		goto error;
	}

	addr = ((res->bus_lower - 1) | (size - 1)) + 1;

	if (addr - res->bus_start + size > res->size) {
		DEBUGF("No room in resource");
		goto error;
	}

	res->bus_lower = addr + size;

	DEBUGF("address=0x%llx bus_lower=0x%llx", (u64)addr, (u64)res->bus_lower);

	*bar = addr;
	return 0;

 error:
	*bar = (pci_addr_t)-1;
	return -1;
}

/*
 *
 */

void pciauto_setup_device(struct pci_controller *hose,
			  pci_dev_t dev, int bars_num,
			  struct pci_region *mem,
			  struct pci_region *prefetch,
			  struct pci_region *io)
{
	u32 bar_response;
	pci_size_t bar_size;
	u16 cmdstat = 0;
	int bar, bar_nr = 0;
#ifndef CONFIG_PCI_ENUM_ONLY
	pci_addr_t bar_value;
	struct pci_region *bar_res;
	int found_mem64 = 0;
#endif

	pci_hose_read_config_word(hose, dev, PCI_COMMAND, &cmdstat);
	cmdstat = (cmdstat & ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY)) | PCI_COMMAND_MASTER;

	for (bar = PCI_BASE_ADDRESS_0;
		bar < PCI_BASE_ADDRESS_0 + (bars_num * 4); bar += 4) {
		/* Tickle the BAR and get the response */
#ifndef CONFIG_PCI_ENUM_ONLY
		pci_hose_write_config_dword(hose, dev, bar, 0xffffffff);
#endif
		pci_hose_read_config_dword(hose, dev, bar, &bar_response);

		/* If BAR is not implemented go to the next BAR */
		if (!bar_response)
			continue;

#ifndef CONFIG_PCI_ENUM_ONLY
		found_mem64 = 0;
#endif

		/* Check the BAR type and set our address mask */
		if (bar_response & PCI_BASE_ADDRESS_SPACE) {
			bar_size = ((~(bar_response & PCI_BASE_ADDRESS_IO_MASK))
				   & 0xffff) + 1;
#ifndef CONFIG_PCI_ENUM_ONLY
			bar_res = io;
#endif

			DEBUGF("PCI Autoconfig: BAR %d, I/O, size=0x%llx, ", bar_nr, (u64)bar_size);
		} else {
			if ((bar_response & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
			     PCI_BASE_ADDRESS_MEM_TYPE_64) {
				u32 bar_response_upper;
				u64 bar64;

#ifndef CONFIG_PCI_ENUM_ONLY
				pci_hose_write_config_dword(hose, dev, bar + 4,
					0xffffffff);
#endif
				pci_hose_read_config_dword(hose, dev, bar + 4,
					&bar_response_upper);

				bar64 = ((u64)bar_response_upper << 32) | bar_response;

				bar_size = ~(bar64 & PCI_BASE_ADDRESS_MEM_MASK) + 1;
#ifndef CONFIG_PCI_ENUM_ONLY
				found_mem64 = 1;
#endif
			} else {
				bar_size = (u32)(~(bar_response & PCI_BASE_ADDRESS_MEM_MASK) + 1);
			}
#ifndef CONFIG_PCI_ENUM_ONLY
			if (prefetch && (bar_response & PCI_BASE_ADDRESS_MEM_PREFETCH))
				bar_res = prefetch;
			else
				bar_res = mem;
#endif

			DEBUGF("PCI Autoconfig: BAR %d, Mem, size=0x%llx, ", bar_nr, (u64)bar_size);
		}

#ifndef CONFIG_PCI_ENUM_ONLY
		if (pciauto_region_allocate(bar_res, bar_size, &bar_value) == 0) {
			/* Write it out and update our limit */
			pci_hose_write_config_dword(hose, dev, bar, (u32)bar_value);

			if (found_mem64) {
				bar += 4;
#ifdef CONFIG_SYS_PCI_64BIT
				pci_hose_write_config_dword(hose, dev, bar, (u32)(bar_value>>32));
#else
				/*
				 * If we are a 64-bit decoder then increment to the
				 * upper 32 bits of the bar and force it to locate
				 * in the lower 4GB of memory.
				 */
				pci_hose_write_config_dword(hose, dev, bar, 0x00000000);
#endif
			}

		}
#endif
		cmdstat |= (bar_response & PCI_BASE_ADDRESS_SPACE) ?
			PCI_COMMAND_IO : PCI_COMMAND_MEMORY;

		DEBUGF("\n");

		bar_nr++;
	}

	pci_hose_write_config_word(hose, dev, PCI_COMMAND, cmdstat);
	pci_hose_write_config_byte(hose, dev, PCI_CACHE_LINE_SIZE,
		CONFIG_SYS_PCI_CACHE_LINE_SIZE);
	pci_hose_write_config_byte(hose, dev, PCI_LATENCY_TIMER, 0x80);
}

void pciauto_prescan_setup_bridge(struct pci_controller *hose,
					 pci_dev_t dev, int sub_bus)
{
	struct pci_region *pci_mem = hose->pci_mem;
	struct pci_region *pci_prefetch = hose->pci_prefetch;
	struct pci_region *pci_io = hose->pci_io;
	u16 cmdstat;

	pci_hose_read_config_word(hose, dev, PCI_COMMAND, &cmdstat);

	/* Configure bus number registers */
	pci_hose_write_config_byte(hose, dev, PCI_PRIMARY_BUS,
				   PCI_BUS(dev) - hose->first_busno);
	pci_hose_write_config_byte(hose, dev, PCI_SECONDARY_BUS,
				   sub_bus - hose->first_busno);
	pci_hose_write_config_byte(hose, dev, PCI_SUBORDINATE_BUS, 0xff);

	if (pci_mem) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_mem, 0x100000);

		/* Set up memory and I/O filter limits, assume 32-bit I/O space */
		pci_hose_write_config_word(hose, dev, PCI_MEMORY_BASE,
					(pci_mem->bus_lower & 0xfff00000) >> 16);

		cmdstat |= PCI_COMMAND_MEMORY;
	}

	if (pci_prefetch) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_prefetch, 0x100000);

		/* Set up memory and I/O filter limits, assume 32-bit I/O space */
		pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_BASE,
					(pci_prefetch->bus_lower & 0xfff00000) >> 16);

		cmdstat |= PCI_COMMAND_MEMORY;
	} else {
		/* We don't support prefetchable memory for now, so disable */
		pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_BASE, 0x1000);
		pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_LIMIT, 0x0);
	}

	if (pci_io) {
		/* Round I/O allocator to 4KB boundary */
		pciauto_region_align(pci_io, 0x1000);

		pci_hose_write_config_byte(hose, dev, PCI_IO_BASE,
					(pci_io->bus_lower & 0x0000f000) >> 8);
		pci_hose_write_config_word(hose, dev, PCI_IO_BASE_UPPER16,
					(pci_io->bus_lower & 0xffff0000) >> 16);

		cmdstat |= PCI_COMMAND_IO;
	}

	/* Enable memory and I/O accesses, enable bus master */
	pci_hose_write_config_word(hose, dev, PCI_COMMAND,
					cmdstat | PCI_COMMAND_MASTER);
}

void pciauto_postscan_setup_bridge(struct pci_controller *hose,
					  pci_dev_t dev, int sub_bus)
{
	struct pci_region *pci_mem = hose->pci_mem;
	struct pci_region *pci_prefetch = hose->pci_prefetch;
	struct pci_region *pci_io = hose->pci_io;

	/* Configure bus number registers */
	pci_hose_write_config_byte(hose, dev, PCI_SUBORDINATE_BUS,
				   sub_bus - hose->first_busno);

	if (pci_mem) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_mem, 0x100000);

		pci_hose_write_config_word(hose, dev, PCI_MEMORY_LIMIT,
				(pci_mem->bus_lower - 1) >> 16);
	}

	if (pci_prefetch) {
		/* Round memory allocator to 1MB boundary */
		pciauto_region_align(pci_prefetch, 0x100000);

		pci_hose_write_config_word(hose, dev, PCI_PREF_MEMORY_LIMIT,
				(pci_prefetch->bus_lower - 1) >> 16);
	}

	if (pci_io) {
		/* Round I/O allocator to 4KB boundary */
		pciauto_region_align(pci_io, 0x1000);

		pci_hose_write_config_byte(hose, dev, PCI_IO_LIMIT,
				((pci_io->bus_lower - 1) & 0x0000f000) >> 8);
		pci_hose_write_config_word(hose, dev, PCI_IO_LIMIT_UPPER16,
				((pci_io->bus_lower - 1) & 0xffff0000) >> 16);
	}
}

/*
 *
 */

void pciauto_config_init(struct pci_controller *hose)
{
	int i;

	hose->pci_io = hose->pci_mem = hose->pci_prefetch = NULL;

	for (i = 0; i < hose->region_count; i++) {
		switch(hose->regions[i].flags) {
		case PCI_REGION_IO:
			if (!hose->pci_io ||
			    hose->pci_io->size < hose->regions[i].size)
				hose->pci_io = hose->regions + i;
			break;
		case PCI_REGION_MEM:
			if (!hose->pci_mem ||
			    hose->pci_mem->size < hose->regions[i].size)
				hose->pci_mem = hose->regions + i;
			break;
		case (PCI_REGION_MEM | PCI_REGION_PREFETCH):
			if (!hose->pci_prefetch ||
			    hose->pci_prefetch->size < hose->regions[i].size)
				hose->pci_prefetch = hose->regions + i;
			break;
		}
	}


	if (hose->pci_mem) {
		pciauto_region_init(hose->pci_mem);

		DEBUGF("PCI Autoconfig: Bus Memory region: [0x%llx-0x%llx],\n"
		       "\t\tPhysical Memory [%llx-%llxx]\n",
		    (u64)hose->pci_mem->bus_start,
		    (u64)(hose->pci_mem->bus_start + hose->pci_mem->size - 1),
		    (u64)hose->pci_mem->phys_start,
		    (u64)(hose->pci_mem->phys_start + hose->pci_mem->size - 1));
	}

	if (hose->pci_prefetch) {
		pciauto_region_init(hose->pci_prefetch);

		DEBUGF("PCI Autoconfig: Bus Prefetchable Mem: [0x%llx-0x%llx],\n"
		       "\t\tPhysical Memory [%llx-%llx]\n",
		    (u64)hose->pci_prefetch->bus_start,
		    (u64)(hose->pci_prefetch->bus_start +
			    hose->pci_prefetch->size - 1),
		    (u64)hose->pci_prefetch->phys_start,
		    (u64)(hose->pci_prefetch->phys_start +
			    hose->pci_prefetch->size - 1));
	}

	if (hose->pci_io) {
		pciauto_region_init(hose->pci_io);

		DEBUGF("PCI Autoconfig: Bus I/O region: [0x%llx-0x%llx],\n"
		       "\t\tPhysical Memory: [%llx-%llx]\n",
		    (u64)hose->pci_io->bus_start,
		    (u64)(hose->pci_io->bus_start + hose->pci_io->size - 1),
		    (u64)hose->pci_io->phys_start,
		    (u64)(hose->pci_io->phys_start + hose->pci_io->size - 1));

	}
}

/*
 * HJF: Changed this to return int. I think this is required
 * to get the correct result when scanning bridges
 */
int pciauto_config_device(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned int sub_bus = PCI_BUS(dev);
	unsigned short class;
	unsigned char prg_iface;
	int n;

	pci_hose_read_config_word(hose, dev, PCI_CLASS_DEVICE, &class);

	switch (class) {
	case PCI_CLASS_BRIDGE_PCI:
		hose->current_busno++;
		pciauto_setup_device(hose, dev, 2, hose->pci_mem,
			hose->pci_prefetch, hose->pci_io);

		DEBUGF("PCI Autoconfig: Found P2P bridge, device %d\n", PCI_DEV(dev));

		/* Passing in current_busno allows for sibling P2P bridges */
		pciauto_prescan_setup_bridge(hose, dev, hose->current_busno);
		/*
		 * need to figure out if this is a subordinate bridge on the bus
		 * to be able to properly set the pri/sec/sub bridge registers.
		 */
		n = pci_hose_scan_bus(hose, hose->current_busno);

		/* figure out the deepest we've gone for this leg */
		sub_bus = max(n, sub_bus);
		pciauto_postscan_setup_bridge(hose, dev, sub_bus);

		sub_bus = hose->current_busno;
		break;

	case PCI_CLASS_STORAGE_IDE:
		pci_hose_read_config_byte(hose, dev, PCI_CLASS_PROG, &prg_iface);
		if (!(prg_iface & PCIAUTO_IDE_MODE_MASK)) {
			DEBUGF("PCI Autoconfig: Skipping legacy mode IDE controller\n");
			return sub_bus;
		}

		pciauto_setup_device(hose, dev, 6, hose->pci_mem,
			hose->pci_prefetch, hose->pci_io);
		break;

	case PCI_CLASS_BRIDGE_CARDBUS:
		/*
		 * just do a minimal setup of the bridge,
		 * let the OS take care of the rest
		 */
		pciauto_setup_device(hose, dev, 0, hose->pci_mem,
			hose->pci_prefetch, hose->pci_io);

		DEBUGF("PCI Autoconfig: Found P2CardBus bridge, device %d\n",
			PCI_DEV(dev));

		hose->current_busno++;
		break;

#if defined(CONFIG_PCIAUTO_SKIP_HOST_BRIDGE)
	case PCI_CLASS_BRIDGE_OTHER:
		DEBUGF("PCI Autoconfig: Skipping bridge device %d\n",
		       PCI_DEV(dev));
		break;
#endif
#if defined(CONFIG_MPC834x) && !defined(CONFIG_VME8349)
	case PCI_CLASS_BRIDGE_OTHER:
		/*
		 * The host/PCI bridge 1 seems broken in 8349 - it presents
		 * itself as 'PCI_CLASS_BRIDGE_OTHER' and appears as an _agent_
		 * device claiming resources io/mem/irq.. we only allow for
		 * the PIMMR window to be allocated (BAR0 - 1MB size)
		 */
		DEBUGF("PCI Autoconfig: Broken bridge found, only minimal config\n");
		pciauto_setup_device(hose, dev, 0, hose->pci_mem,
			hose->pci_prefetch, hose->pci_io);
		break;
#endif

	case PCI_CLASS_PROCESSOR_POWERPC: /* an agent or end-point */
		DEBUGF("PCI AutoConfig: Found PowerPC device\n");

	default:
		pciauto_setup_device(hose, dev, 6, hose->pci_mem,
			hose->pci_prefetch, hose->pci_io);
		break;
	}

	return sub_bus;
}
