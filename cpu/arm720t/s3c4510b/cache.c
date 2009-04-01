/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
#include <asm/hardware.h>

void icache_enable (void)
{
	s32 i;

	/* disable all cache bits */
	CLR_REG( REG_SYSCFG, 0x3F);

	/* 8KB cache, write enable */
	SET_REG( REG_SYSCFG, CACHE_WRITE_BUFF | CACHE_MODE_01);

	/* clear TAG RAM bits */
	for ( i = 0; i < 256; i++)
	  PUT_REG( CACHE_TAG_RAM + 4*i, 0x00000000);

	/* clear SET0 RAM */
	for(i=0; i < 1024; i++)
	  PUT_REG( CACHE_SET0_RAM + 4*i, 0x00000000);

	/* clear SET1 RAM */
	for(i=0; i < 1024; i++)
	  PUT_REG( CACHE_SET1_RAM + 4*i, 0x00000000);

	/* enable cache */
	SET_REG( REG_SYSCFG, CACHE_ENABLE);

}

void icache_disable (void)
{
	/* disable all cache bits */
	CLR_REG( REG_SYSCFG, 0x3F);
}

int icache_status (void)
{
	return GET_REG( REG_SYSCFG) & CACHE_ENABLE;
}

void dcache_enable (void)
{
	/* we don't have seperate instruction/data caches */
	icache_enable();
}

void dcache_disable (void)
{
	/* we don't have seperate instruction/data caches */
	icache_disable();
}

int dcache_status (void)
{
	/* we don't have seperate instruction/data caches */
	return icache_status();
}
