/*
 * PXA common functions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
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

#ifndef	__PXA_H__
#define	__PXA_H__

#define PXA255_A0	0x00000106
#define PXA250_C0	0x00000105
#define PXA250_B2	0x00000104
#define PXA250_B1	0x00000103
#define PXA250_B0	0x00000102
#define PXA250_A1	0x00000101
#define PXA250_A0	0x00000100
#define PXA210_C0	0x00000125
#define PXA210_B2	0x00000124
#define PXA210_B1	0x00000123
#define PXA210_B0	0x00000122

int cpu_is_pxa25x(void);
int cpu_is_pxa27x(void);
uint32_t pxa_get_cpu_revision(void);
void pxa2xx_dram_init(void);

#endif	/* __PXA_H__ */
