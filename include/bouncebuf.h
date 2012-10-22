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

#ifndef __INCLUDE_BOUNCEBUF_H__
#define __INCLUDE_BOUNCEBUF_H__

/*
 * GEN_BB_READ -- Data are read from the buffer eg. by DMA hardware.
 * The source buffer is copied into the bounce buffer (if unaligned, otherwise
 * the source buffer is used directly) upon start() call, then the operation
 * requiring the aligned transfer happens, then the bounce buffer is lost upon
 * stop() call.
 */
#define GEN_BB_READ	(1 << 0)
/*
 * GEN_BB_WRITE -- Data are written into the buffer eg. by DMA hardware.
 * The source buffer starts in an undefined state upon start() call, then the
 * operation requiring the aligned transfer happens, then the bounce buffer is
 * copied into the destination buffer (if unaligned, otherwise destination
 * buffer is used directly) upon stop() call.
 */
#define GEN_BB_WRITE	(1 << 1)
/*
 * GEN_BB_RW -- Data are read and written into the buffer eg. by DMA hardware.
 * The source buffer is copied into the bounce buffer (if unaligned, otherwise
 * the source buffer is used directly) upon start() call, then the  operation
 * requiring the aligned transfer happens, then the bounce buffer is  copied
 * into the destination buffer (if unaligned, otherwise destination buffer is
 * used directly) upon stop() call.
 */
#define GEN_BB_RW	(GEN_BB_READ | GEN_BB_WRITE)

#ifdef CONFIG_BOUNCE_BUFFER
/**
 * bounce_buffer_start() -- Start the bounce buffer session
 * data:	pointer to buffer to be aligned
 * len:		length of the buffer
 * backup:	pointer to backup buffer (the original value is stored here if
 *              needed
 * flags:	flags describing the transaction, see above.
 */
int bounce_buffer_start(void **data, size_t len, void **backup, uint8_t flags);
/**
 * bounce_buffer_stop() -- Finish the bounce buffer session
 * data:	pointer to buffer that was aligned
 * len:		length of the buffer
 * backup:	pointer to backup buffer (the original value is stored here if
 *              needed
 * flags:	flags describing the transaction, see above.
 */
int bounce_buffer_stop(void **data, size_t len, void **backup, uint8_t flags);
#else
static inline int bounce_buffer_start(void **data, size_t len, void **backup,
					uint8_t flags)
{
	return 0;
}

static inline int bounce_buffer_stop(void **data, size_t len, void **backup,
					uint8_t flags)
{
	return 0;
}
#endif

#endif
