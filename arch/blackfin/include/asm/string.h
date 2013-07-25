/*
 * U-boot - string.h String functions
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Changed by Lineo Inc. May 2001 */

#ifndef _BLACKFINNOMMU_STRING_H_
#define _BLACKFINNOMMU_STRING_H_

#ifdef __KERNEL__		/* only set these up for kernel code */

#define __HAVE_ARCH_STRCPY
#define __HAVE_ARCH_STRNCPY
#define __HAVE_ARCH_STRCMP
#define __HAVE_ARCH_STRNCMP
#define __HAVE_ARCH_MEMCPY
#define __HAVE_ARCH_MEMCMP
#define __HAVE_ARCH_MEMSET
#define __HAVE_ARCH_MEMMOVE

extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern int strcmp(const char *cs, const char *ct);
extern int strncmp(const char *cs, const char *ct, size_t count);
extern void *memcpy(void *dest, const void *src, size_t count);
extern void *memset(void *s, int c, size_t count);
extern int memcmp(const void *, const void *, size_t);
extern void *memmove(void *dest, const void *src, size_t count);

#else				/* KERNEL */

/*
 * let user libraries deal with these,
 * IMHO the kernel has no place defining these functions for user apps
 */

#define __HAVE_ARCH_STRCPY	1
#define __HAVE_ARCH_STRNCPY	1
#define __HAVE_ARCH_STRCAT	1
#define __HAVE_ARCH_STRNCAT	1
#define __HAVE_ARCH_STRCMP	1
#define __HAVE_ARCH_STRNCMP	1
#define __HAVE_ARCH_STRNICMP	1
#define __HAVE_ARCH_STRCHR	1
#define __HAVE_ARCH_STRRCHR	1
#define __HAVE_ARCH_STRSTR	1
#define __HAVE_ARCH_STRLEN	1
#define __HAVE_ARCH_STRNLEN	1
#define __HAVE_ARCH_MEMSET	1
#define __HAVE_ARCH_MEMCPY	1
#define __HAVE_ARCH_MEMMOVE	1
#define __HAVE_ARCH_MEMSCAN	1
#define __HAVE_ARCH_MEMCMP	1
#define __HAVE_ARCH_MEMCHR	1
#define __HAVE_ARCH_STRTOK	1

#endif				/* KERNEL */

#endif				/* _BLACKFIN_STRING_H_ */
