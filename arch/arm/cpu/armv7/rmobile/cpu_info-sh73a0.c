/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012 Renesas Solutions Corp.
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
#include <asm/io.h>

u32 rmobile_get_cpu_type(void)
{
	u32 id;
	u32 type;
	struct sh73a0_hpb *hpb = (struct sh73a0_hpb *)HPB_BASE;

	id = readl(&hpb->cccr);
	type = (id >> 8) & 0xFF;

	return type;
}

u32 rmobile_get_cpu_rev_integer(void)
{
	u32 id;
	u32 rev;
	struct sh73a0_hpb *hpb = (struct sh73a0_hpb *)HPB_BASE;

	id = readl(&hpb->cccr);
	rev = ((id >> 4) & 0xF) + 1;

	return rev;
}

u32 rmobile_get_cpu_rev_fraction(void)
{
	u32 id;
	u32 rev;
	struct sh73a0_hpb *hpb = (struct sh73a0_hpb *)HPB_BASE;

	id = readl(&hpb->cccr);
	rev = id & 0xF;

	return rev;
}
