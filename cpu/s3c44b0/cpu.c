/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
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
