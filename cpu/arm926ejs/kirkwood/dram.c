/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <config.h>
#include <asm/arch/kirkwood.h>

#define KW_REG_CPUCS_WIN_BAR(x)		(KW_REGISTER(0x1500) + (x * 0x08))
#define KW_REG_CPUCS_WIN_SZ(x)		(KW_REGISTER(0x1504) + (x * 0x08))
/*
 * kw_sdram_bar - reads SDRAM Base Address Register
 */
u32 kw_sdram_bar(enum memory_bank bank)
{
	u32 result = 0;
	u32 enable = 0x01 & readl(KW_REG_CPUCS_WIN_SZ(bank));

	if ((!enable) || (bank > BANK3))
		return 0;

	result = readl(KW_REG_CPUCS_WIN_BAR(bank));
	return result;
}

/*
 * kw_sdram_bs - reads SDRAM Bank size
 */
u32 kw_sdram_bs(enum memory_bank bank)
{
	u32 result = 0;
	u32 enable = 0x01 & readl(KW_REG_CPUCS_WIN_SZ(bank));

	if ((!enable) || (bank > BANK3))
		return 0;
	result = 0xff000000 & readl(KW_REG_CPUCS_WIN_SZ(bank));
	result += 0x01000000;
	return result;
}
