/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 *
 * mmap/munmap implementation derived from:
 * Clamav Native Windows Port : mmap win32 compatibility layer
 * Copyright (c) 2005-2006 Gianluigi Tiesi <sherpya@netfarm.it>
 * Parts by Kees Zeelenberg <kzlg@users.sourceforge.net> (LibGW32C)
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

#include "mingw_support.h"
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <io.h>

int fsync(int fd)
{
	return _commit(fd);
}

void *mmap(void *addr, size_t len, int prot, int flags, int fd, int offset)
{
	void *map = NULL;
	HANDLE handle = INVALID_HANDLE_VALUE;
	DWORD cfm_flags = 0, mvf_flags = 0;

	switch (prot) {
	case PROT_READ | PROT_WRITE:
		cfm_flags = PAGE_READWRITE;
		mvf_flags = FILE_MAP_ALL_ACCESS;
		break;
	case PROT_WRITE:
		cfm_flags = PAGE_READWRITE;
		mvf_flags = FILE_MAP_WRITE;
		break;
	case PROT_READ:
		cfm_flags = PAGE_READONLY;
		mvf_flags = FILE_MAP_READ;
		break;
	default:
		return MAP_FAILED;
	}

	handle = CreateFileMappingA((HANDLE) _get_osfhandle(fd), NULL,
				cfm_flags, HIDWORD(len), LODWORD(len), NULL);
	if (!handle)
		return MAP_FAILED;

	map = MapViewOfFile(handle, mvf_flags, HIDWORD(offset),
			LODWORD(offset), len);
	CloseHandle(handle);

	if (!map)
		return MAP_FAILED;

	return map;
}

int munmap(void *addr, size_t len)
{
	if (!UnmapViewOfFile(addr))
		return -1;

	return 0;
}
