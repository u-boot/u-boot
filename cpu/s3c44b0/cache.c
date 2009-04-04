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

#include <common.h>
#include <command.h>
#include <asm/hardware.h>

static void s3c44b0_flush_cache(void)
{
	volatile int i;
	/* flush cycle */
	for(i=0x10002000;i<0x10004800;i+=16)
	{
		*((int *)i)=0x0;
	}
}

void icache_enable (void)
{
	ulong reg;

	s3c44b0_flush_cache();

	/*
		Init cache
		Non-cacheable area (everything outside RAM)
		0x0000:0000 - 0x0C00:0000
	 */
	NCACHBE0 = 0xC0000000;
	NCACHBE1 = 0x00000000;

	/*
		Enable chache
	*/
	reg = SYSCFG;
	reg |= 0x00000006; /* 8kB */
	SYSCFG = reg;
}

void icache_disable (void)
{
	ulong reg;

	reg = SYSCFG;
	reg &= ~0x00000006; /* 8kB */
	SYSCFG = reg;
}

int icache_status (void)
{
	return 0;
}

void dcache_enable (void)
{
	icache_enable();
}

void dcache_disable (void)
{
	icache_disable();
}

int dcache_status (void)
{
	return dcache_status();
}
