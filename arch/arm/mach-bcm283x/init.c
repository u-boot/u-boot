// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <dm/device.h>
#include <fdt_support.h>
#include <asm/global_data.h>

#define BCM2711_RPI4_PCIE_XHCI_MMIO_PHYS	0x600000000UL
#define BCM2711_RPI4_PCIE_XHCI_MMIO_SIZE	0x400000UL

#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>

#define MEM_MAP_MAX_ENTRIES (4)

static struct mm_region bcm283x_mem_map[MEM_MAP_MAX_ENTRIES] = {
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x3f000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x3f000000UL,
		.phys = 0x3f000000UL,
		.size = 0x01000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

static struct mm_region bcm2711_mem_map[MEM_MAP_MAX_ENTRIES] = {
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0xfc000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xfc000000UL,
		.phys = 0xfc000000UL,
		.size = 0x03800000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = BCM2711_RPI4_PCIE_XHCI_MMIO_PHYS,
		.phys = BCM2711_RPI4_PCIE_XHCI_MMIO_PHYS,
		.size = BCM2711_RPI4_PCIE_XHCI_MMIO_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = bcm283x_mem_map;

/*
 * I/O address space varies on different chip versions.
 * We set the base address by inspecting the DTB.
 */
static const struct udevice_id board_ids[] = {
	{ .compatible = "brcm,bcm2837", .data = (ulong)&bcm283x_mem_map},
	{ .compatible = "brcm,bcm2838", .data = (ulong)&bcm2711_mem_map},
	{ .compatible = "brcm,bcm2711", .data = (ulong)&bcm2711_mem_map},
	{ },
};

static void _rpi_update_mem_map(struct mm_region *pd)
{
	int i;

	for (i = 0; i < MEM_MAP_MAX_ENTRIES; i++) {
		mem_map[i].virt = pd[i].virt;
		mem_map[i].phys = pd[i].phys;
		mem_map[i].size = pd[i].size;
		mem_map[i].attrs = pd[i].attrs;
	}
}

static void rpi_update_mem_map(void)
{
	int ret;
	struct mm_region *mm;
	const struct udevice_id *of_match = board_ids;

	while (of_match->compatible) {
		ret = fdt_node_check_compatible(gd->fdt_blob, 0,
						of_match->compatible);
		if (!ret) {
			mm = (struct mm_region *)of_match->data;
			_rpi_update_mem_map(mm);
			break;
		}

		of_match++;
	}
}
#else
static void rpi_update_mem_map(void) {}
#endif

unsigned long rpi_bcm283x_base = 0x3f000000;

int arch_cpu_init(void)
{
	icache_enable();

	return 0;
}

int mach_cpu_init(void)
{
	int ret, soc_offset;
	u64 io_base, size;

	rpi_update_mem_map();

	/* Get IO base from device tree */
	soc_offset = fdt_path_offset(gd->fdt_blob, "/soc");
	if (soc_offset < 0)
		return soc_offset;

	ret = fdt_read_range((void *)gd->fdt_blob, soc_offset, 0, NULL,
				&io_base, &size);
	if (ret)
		return ret;

	rpi_bcm283x_base = io_base;

	return 0;
}

#ifdef CONFIG_ARMV7_LPAE
#ifdef CONFIG_TARGET_RPI_4_32B
#define BCM2711_RPI4_PCIE_XHCI_MMIO_VIRT	0xffc00000UL
#include <addr_map.h>
#include <asm/system.h>

void init_addr_map(void)
{
	mmu_set_region_dcache_behaviour_phys(BCM2711_RPI4_PCIE_XHCI_MMIO_VIRT,
					     BCM2711_RPI4_PCIE_XHCI_MMIO_PHYS,
					     BCM2711_RPI4_PCIE_XHCI_MMIO_SIZE,
					     DCACHE_OFF);

	/* identity mapping for 0..BCM2711_RPI4_PCIE_XHCI_MMIO_VIRT */
	addrmap_set_entry(0, 0, BCM2711_RPI4_PCIE_XHCI_MMIO_VIRT, 0);
	/* XHCI MMIO on PCIe at BCM2711_RPI4_PCIE_XHCI_MMIO_VIRT */
	addrmap_set_entry(BCM2711_RPI4_PCIE_XHCI_MMIO_VIRT,
			  BCM2711_RPI4_PCIE_XHCI_MMIO_PHYS,
			  BCM2711_RPI4_PCIE_XHCI_MMIO_SIZE, 1);
}
#endif

void enable_caches(void)
{
	dcache_enable();
}
#endif
