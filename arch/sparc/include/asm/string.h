/*
 * (C) Copyright 2000 - 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef _SPARC_STRING_H_
#define _SPARC_STRING_H_

/*
#define __HAVE_ARCH_STRCPY
#define __HAVE_ARCH_STRNCPY
#define __HAVE_ARCH_STRLEN
#define __HAVE_ARCH_STRCMP
#define __HAVE_ARCH_STRCAT
#define __HAVE_ARCH_MEMSET
#define __HAVE_ARCH_BCOPY
#define __HAVE_ARCH_MEMCPY
#define __HAVE_ARCH_MEMMOVE
#define __HAVE_ARCH_MEMCMP
#define __HAVE_ARCH_MEMCHR
*/

extern int strcasecmp(const char *, const char *);
extern int strncasecmp(const char *, const char *, int);
extern char *strcpy(char *, const char *);
extern char *strncpy(char *, const char *, __kernel_size_t);
extern __kernel_size_t strlen(const char *);
extern int strcmp(const char *, const char *);
extern char *strcat(char *, const char *);
extern void *memset(void *, int, __kernel_size_t);
extern void *memcpy(void *, const void *, __kernel_size_t);
extern void *memmove(void *, const void *, __kernel_size_t);
extern int memcmp(const void *, const void *, __kernel_size_t);
extern void *memchr(const void *, int, __kernel_size_t);

#endif
