/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <os.h>
#include <asm/state.h>

DECLARE_GLOBAL_DATA_PTR;

void reset_cpu(ulong ignored)
{
	if (state_uninit())
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

int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	if (flag & (BOOTM_STATE_OS_GO | BOOTM_STATE_OS_FAKE_GO)) {
		bootstage_mark(BOOTSTAGE_ID_RUN_OS);
		printf("## Transferring control to Linux (at address %08lx)...\n",
		       images->ep);
		reset_cpu(0);
	}

	return 0;
}

int cleanup_before_linux(void)
{
	return 0;
}

void *map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags)
{
	return (void *)(gd->arch.ram_buf + paddr);
}

phys_addr_t map_to_sysmem(void *ptr)
{
	return (u8 *)ptr - gd->arch.ram_buf;
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
}
