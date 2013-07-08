/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <nios2.h>
#include <nios2-io.h>
#include <asm/cache.h>

#if defined (CONFIG_SYS_NIOS_SYSID_BASE)
extern void display_sysid (void);
#endif /* CONFIG_SYS_NIOS_SYSID_BASE */

int checkcpu (void)
{
	printf ("CPU   : Nios-II\n");
#if !defined(CONFIG_SYS_NIOS_SYSID_BASE)
	printf ("SYSID : <unknown>\n");
#else
	display_sysid ();
#endif
	return (0);
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	/* indirect call to go beyond 256MB limitation of toolchain */
	nios2_callr(CONFIG_SYS_RESET_ADDR);
	return 0;
}

int dcache_status(void)
{
	return 1;
}

void dcache_enable(void)
{
	flush_dcache(CONFIG_SYS_DCACHE_SIZE, CONFIG_SYS_DCACHELINE_SIZE);
}

void dcache_disable(void)
{
	flush_dcache(CONFIG_SYS_DCACHE_SIZE, CONFIG_SYS_DCACHELINE_SIZE);
}
