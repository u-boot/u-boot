/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef _TEST_BURST_H
#define _TEST_BURST_H

/* Cache line size */
#define CACHE_LINE_SIZE		16
/* Binary logarithm of the cache line size */
#define LG_CACHE_LINE_SIZE	4

#ifndef __ASSEMBLY__
extern void mmu_init(void);
extern void caches_init(void);
extern void flush_dcache_range(unsigned long start, unsigned long stop);
#endif

#endif /* _TEST_BURST_H */
