/*
 * Copyright 2011 Calxeda, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <asm/io.h>

#ifdef CONFIG_BOOTCOUNT_LIMIT
void bootcount_store(ulong a)
{
	writel((BOOTCOUNT_MAGIC & 0xffff0000) | a, CONFIG_SYS_BOOTCOUNT_ADDR);
}

ulong bootcount_load(void)
{
	u32 tmp = readl(CONFIG_SYS_BOOTCOUNT_ADDR);

	if ((tmp & 0xffff0000) != (BOOTCOUNT_MAGIC & 0xffff0000))
		return 0;
	else
		return tmp & 0x0000ffff;
}
#endif
