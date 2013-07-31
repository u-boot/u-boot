/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * S3C44B0 CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/hardware.h>

int arch_cpu_init (void)
{
	icache_enable();

	return 0;
}

int cleanup_before_linux (void)
{
	/*
		cache memory should be enabled before calling
		Linux to make the kernel uncompression faster
	*/
	icache_enable();

	disable_interrupts ();

	return 0;
}

void reset_cpu (ulong addr)
{
	/*
		reset the cpu using watchdog
	*/

	/* Disable the watchdog.*/
	WTCON&=~(1<<5);

	/* set the timeout value to a short time... */
	WTCNT = 0x1;

	/* Enable the watchdog. */
	WTCON|=1;
	WTCON|=(1<<5);

	while(1) {
		/*NOP*/
	}
}
