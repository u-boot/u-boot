/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* CPU specific code */
#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <asm/cache.h>

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

	/* turn off I/D-cache */

	return 0;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	panic("nx25-ae250 wdt not support yet.\n");
}
