/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
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

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <bouncebuf.h>

static int addr_aligned(void *data, size_t len)
{
	const ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	if ((ulong)data & align_mask) {
		debug("Unaligned start address %p\n", data);
		return 0;
	}

	data += len;

	/* Check if end is aligned */
	if ((ulong)data & align_mask) {
		debug("Unaligned end address %p\n", data);
		return 0;
	}

	/* Aligned */
	return 1;
}

int bounce_buffer_start(void **data, size_t len, void **backup, uint8_t flags)
{
	void *tmp;
	size_t alen;

	if (addr_aligned(*data, len)) {
		*backup = NULL;
		return 0;
	}

	alen = roundup(len, ARCH_DMA_MINALIGN);
	tmp = memalign(ARCH_DMA_MINALIGN, alen);

	if (!tmp)
		return -ENOMEM;

	if (flags & GEN_BB_READ)
		memcpy(tmp, *data, len);

	*backup = *data;
	*data = tmp;

	return 0;
}

int bounce_buffer_stop(void **data, size_t len, void **backup, uint8_t flags)
{
	void *tmp = *data;

	/* The buffer was already aligned, since "backup" is NULL. */
	if (!*backup)
		return 0;

	if (flags & GEN_BB_WRITE)
		memcpy(*backup, *data, len);

	*data = *backup;
	free(tmp);

	return 0;
}
