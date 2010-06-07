/*
 * TNETV107X: Architecture initialization
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <asm/io.h>

void chip_configuration_unlock(void)
{
       __raw_writel(TNETV107X_KICK0_MAGIC, TNETV107X_KICK0);
       __raw_writel(TNETV107X_KICK1_MAGIC, TNETV107X_KICK1);
}

int arch_cpu_init(void)
{
       icache_enable();
       chip_configuration_unlock();

       return 0;
}
