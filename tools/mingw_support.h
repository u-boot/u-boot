/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this software; if not, write to the
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __MINGW_SUPPORT_H_
#define __WINGW_SUPPORT_H_	1

/* Defining __INSIDE_MSYS__ helps to prevent u-boot/mingw overlap */
#define __INSIDE_MSYS__	1

#include <windows.h>

/* mmap protections */
#define PROT_READ	0x1		/* Page can be read */
#define PROT_WRITE	0x2		/* Page can be written */
#define PROT_EXEC	0x4		/* Page can be executed */
#define PROT_NONE	0x0		/* Page can not be accessed */

/* Sharing types (must choose one and only one of these) */
#define MAP_SHARED	0x01		/* Share changes */
#define MAP_PRIVATE	0x02		/* Changes are private */

/* Windows 64-bit access macros */
#define LODWORD(x) ((DWORD)((DWORDLONG)(x)))
#define HIDWORD(x) ((DWORD)(((DWORDLONG)(x) >> 32) & 0xffffffff))

typedef	UINT	uint;
typedef	ULONG	ulong;

int fsync(int fd);
void *mmap(void *, size_t, int, int, int, int);
int munmap(void *, size_t);
char *strtok_r(char *s, const char *delim, char **save_ptr);
#include "getline.h"

#endif /* __MINGW_SUPPORT_H_ */
