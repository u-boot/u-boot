/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define DEBUG
#include <common.h>
#include <dm/root.h>
#include <os.h>
#include <asm/state.h>

DECLARE_GLOBAL_DATA_PTR;

/* Enable access to PCI memory with map_sysmem() */
static bool enable_pci_map;

#ifdef CONFIG_PCI
/* Last device that was mapped into memory, and length of mapping */
static struct udevice *map_dev;
unsigned long map_len;
#endif

void reset_cpu(ulong ignored)
{
	if (state_uninit())
		os_exit(2);

	if (dm_uninit())
		os_exit(2);

	/* This is considered normal termination for now */
	os_exit(0);
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	reset_cpu(0);

	return 0;
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	os_usleep(usec);
}

unsigned long __attribute__((no_instrument_function)) timer_get_us(void)
{
	return os_get_nsec() / 1000;
}

int cleanup_before_linux(void)
{
	return 0;
}

void *map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags)
{
#ifdef CONFIG_PCI
	unsigned long plen = len;
	void *ptr;

	map_dev = NULL;
	if (enable_pci_map && !pci_map_physmem(paddr, &len, &map_dev, &ptr)) {
		if (plen != len) {
			printf("%s: Warning: partial map at %x, wanted %lx, got %lx\n",
			       __func__, paddr, len, plen);
		}
		map_len = len;
		return ptr;
	}
#endif

	return (void *)(gd->arch.ram_buf + paddr);
}

void unmap_physmem(const void *vaddr, unsigned long flags)
{
#ifdef CONFIG_PCI
	if (map_dev) {
		pci_unmap_physmem(vaddr, map_len, map_dev);
		map_dev = NULL;
	}
#endif
}

void sandbox_set_enable_pci_map(int enable)
{
	enable_pci_map = enable;
}

phys_addr_t map_to_sysmem(const void *ptr)
{
	return (u8 *)ptr - gd->arch.ram_buf;
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
}
