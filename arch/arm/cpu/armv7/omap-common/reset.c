/*
 *
 * Common layer for reset related functionality of OMAP based socs.
 *
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 *
 * Sricharan R <r.sricharan@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <config.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <linux/compiler.h>

void __weak reset_cpu(unsigned long ignored)
{
	writel(PRM_RSTCTRL_RESET, PRM_RSTCTRL);
}

u32 __weak warm_reset(void)
{
	return (readl(PRM_RSTST) & PRM_RSTST_WARM_RESET_MASK);
}
