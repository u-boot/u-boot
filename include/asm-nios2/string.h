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
#ifndef __ASM_NIOS2_STRING_H_
#define __ASM_NIOS2_STRING_H_

#undef __HAVE_ARCH_STRRCHR
extern char * strrchr(const char * s, int c);

#undef __HAVE_ARCH_STRCHR
extern char * strchr(const char * s, int c);

#undef __HAVE_ARCH_MEMCPY
extern void * memcpy(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMMOVE
extern void * memmove(void *, const void *, __kernel_size_t);

#undef __HAVE_ARCH_MEMCHR
extern void * memchr(const void *, int, __kernel_size_t);

#undef __HAVE_ARCH_MEMSET
extern void * memset(void *, int, __kernel_size_t);

#undef __HAVE_ARCH_MEMZERO
extern void memzero(void *ptr, __kernel_size_t n);

#endif /* __ASM_NIOS2_STRING_H_ */
