/*
 * Copyright 2012 Joe Hershberger <joe.hershberger@ni.com>
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

#ifndef __ASM_ARCH_MMC_H_
#define __ASM_ARCH_MMC_H_

#include <config.h>

int zynq_sdhci_init(u32 regbase, u32 max_clk, u32 min_clk);

static inline int zynq_mmc_init(bd_t *bd)
{
	u32 regbase = (u32) SD_BASEADDR;

	return zynq_sdhci_init(regbase, 52000000, 52000000 >> 9);
}

#endif /* __ASM_ARCH_MMC_H_ */
