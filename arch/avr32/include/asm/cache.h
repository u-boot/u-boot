/*
 * Copyright (c) 2011 The Chromium OS Authors.
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

#ifndef __AVR32_CACHE_H__
#define __AVR32_CACHE_H__

/*
 * Since the AVR32 architecture has a queryable cacheline size with a maximum
 * value of 256 we set the DMA buffer alignemnt requirement to this maximum
 * value.  The board config can override this if it knows that the cacheline
 * size is a smaller value.  AVR32 boards use the CONFIG_SYS_DCACHE_LINESZ
 * macro to specify cache line size, so if it is set we use it instead.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#elif defined(CONFIG_SYS_DCACHE_LINESZ)
#define ARCH_DMA_MINALIGN	CONFIG_SYS_DCACHE_LINESZ
#else
#define ARCH_DMA_MINALIGN	256
#endif

#endif /* __AVR32_CACHE_H__ */
