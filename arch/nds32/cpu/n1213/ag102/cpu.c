/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
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

/* CPU specific code */
#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <asm/cache.h>

#include <faraday/ftwdt010_wdt.h>

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

#ifdef CONFIG_MMU
	/* turn off I/D-cache */
	icache_disable();
	dcache_disable();

	/* flush I/D-cache */
	invalidate_icac();
	invalidate_dcac();
#endif

	return 0;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();

	/*
	 * reset to the base addr of andesboot.
	 * currently no ROM loader at addr 0.
	 * do not use reset_cpu(0);
	 */
#ifdef CONFIG_FTWDT010_WATCHDOG
	/*
	 * workaround: if we use CONFIG_HW_WATCHDOG with ftwdt010, will lead
	 * automatic hardware reset when booting Linux.
	 * Please do not use CONFIG_HW_WATCHDOG and WATCHDOG_RESET() here.
	 */
	ftwdt010_wdt_reset();
#endif /* CONFIG_FTWDT010_WATCHDOG */
	hang();

	/*NOTREACHED*/
}
