/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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

#ifndef __ASM_NIOS2_BITOPS_H_
#define __ASM_NIOS2_BITOPS_H_


extern void set_bit(int nr, volatile void * a);
extern void clear_bit(int nr, volatile void * a);
extern int test_and_clear_bit(int nr, volatile void * a);
extern void change_bit(unsigned long nr, volatile void *addr);
extern int test_and_set_bit(int nr, volatile void * a);
extern int test_and_change_bit(int nr, volatile void * addr);
extern int test_bit(int nr, volatile void * a);
extern int ffs(int i);
#define PLATFORM_FFS

#endif /* __ASM_NIOS2_BITOPS_H */
