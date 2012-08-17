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

#ifndef __SANDBOX_CACHE_H__
#define __SANDBOX_CACHE_H__

/*
 * For native compilation of the sandbox we should still align
 * the contents of stack buffers to something reasonable.  The
 * GCC macro __BIGGEST_ALIGNMENT__ is defined to be the maximum
 * required alignment for any basic type.  This seems reasonable.
 */
#define ARCH_DMA_MINALIGN	__BIGGEST_ALIGNMENT__

#endif /* __SANDBOX_CACHE_H__ */
