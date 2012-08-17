/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
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

#ifndef __ASM_OPENRISC_CACHE_H_
#define __ASM_OPENRISC_CACHE_H_

/*
 * Valid L1 data cache line sizes for the OpenRISC architecture are
 * 16 and 32 bytes.
 * If the board configuration has not specified one we default to the
 * largest of these values for alignment of DMA buffers.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN       CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN       32
#endif

#endif /* __ASM_OPENRISC_CACHE_H_ */
