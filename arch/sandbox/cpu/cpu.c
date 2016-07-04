/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define DEBUG
#include <common.h>
#include <errno.h>
#include <libfdt.h>
#include <os.h>
#include <asm/io.h>
#include <asm/state.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

/* Enable access to PCI memory with map_sysmem() */
static bool enable_pci_map;

#ifdef CONFIG_PCI
/* Last device that was mapped into memory, and length of mapping */
static struct udevice *map_dev;
unsigned long map_len;
#endif

void sandbox_exit(void)
{
	/* Do this here while it still has an effect */
	os_fd_restore();
	if (state_uninit())
		os_exit(2);

	if (dm_uninit())
		os_exit(2);

	/* This is considered normal termination for now */
	os_exit(0);
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	struct sandbox_state *state = state_get_current();

	if (!state->skip_delays)
		os_usleep(usec);
}

int cleanup_before_linux(void)
{
	return 0;
}

int cleanup_before_linux_select(int flags)
{
	return 0;
}

void *map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags)
{
#if defined(CONFIG_PCI) && !defined(CONFIG_SPL_BUILD)
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

int sandbox_read_fdt_from_file(void)
{
	struct sandbox_state *state = state_get_current();
	const char *fname = state->fdt_fname;
	void *blob;
	loff_t size;
	int err;
	int fd;

	blob = map_sysmem(CONFIG_SYS_FDT_LOAD_ADDR, 0);
	if (!state->fdt_fname) {
		err = fdt_create_empty_tree(blob, 256);
		if (!err)
			goto done;
		printf("Unable to create empty FDT: %s\n", fdt_strerror(err));
		return -EINVAL;
	}

	err = os_get_filesize(fname, &size);
	if (err < 0) {
		printf("Failed to file FDT file '%s'\n", fname);
		return err;
	}
	fd = os_open(fname, OS_O_RDONLY);
	if (fd < 0) {
		printf("Failed to open FDT file '%s'\n", fname);
		return -EACCES;
	}
	if (os_read(fd, blob, size) != size) {
		os_close(fd);
		return -EIO;
	}
	os_close(fd);

done:
	gd->fdt_blob = blob;

	return 0;
}
