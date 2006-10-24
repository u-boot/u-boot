/*
 * Copyright (C) 2006 Atmel Corporation
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
#ifndef __ASM_AVR32_SDRAM_H
#define __ASM_AVR32_SDRAM_H

struct sdram_info {
	unsigned long phys_addr;
	unsigned int row_bits, col_bits, bank_bits;
	unsigned int cas, twr, trc, trp, trcd, tras, txsr;
};

extern unsigned long sdram_init(const struct sdram_info *info);

#endif /* __ASM_AVR32_SDRAM_H */
