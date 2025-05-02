/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Handles a buffer that can be allocated and freed
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __ABUF_H
#define __ABUF_H

#ifdef USE_HOSTCC
#include <sys/types.h>
#else
#include <linux/types.h>
#endif

/**
 * struct abuf - buffer that can be allocated and freed
 *
 * This is useful for a block of data which may be allocated with malloc(), or
 * not, so that it needs to be freed correctly when finished with.
 *
 * For now it has a very simple purpose.
 *
 * Using memset() to zero all fields is guaranteed to be equivalent to
 * abuf_init().
 *
 * @data: Pointer to data
 * @size: Size of data in bytes
 * @alloced: true if allocated with malloc(), so must be freed after use
 */
struct abuf {
	void *data;
	size_t size;
	bool alloced;
};

static inline void *abuf_data(const struct abuf *abuf)
{
	return abuf->data;
}

static inline size_t abuf_size(const struct abuf *abuf)
{
	return abuf->size;
}

/**
 * abuf_addr() - Get the address of a buffer's data
 *
 * @abuf: Buffer to check
 * Return: address of buffer
 */
ulong abuf_addr(const struct abuf *abuf);

/**
 * abuf_set() - set the (unallocated) data in a buffer
 *
 * This simply makes the abuf point to the supplied data, which must be live
 * for the lifetime of the abuf. It is not alloced.
 *
 * Any existing data in the abuf is freed and the alloced member is set to
 * false.
 *
 * @abuf: abuf to adjust
 * @data: New contents of abuf
 * @size: New size of abuf
 */
void abuf_set(struct abuf *abuf, void *data, size_t size);

/**
 * abuf_map_sysmem() - calls map_sysmem() to set up an abuf
 *
 * This is equivalent to abuf_set(abuf, map_sysmem(addr, size), size)
 *
 * Any existing data in the abuf is freed and the alloced member is set to
 * false.
 *
 * @abuf: abuf to adjust
 * @addr: Address to set the abuf to
 * @size: New size of abuf
 */
void abuf_map_sysmem(struct abuf *abuf, ulong addr, size_t size);

/**
 * abuf_realloc() - Change the size of a buffer
 *
 * This uses realloc() to change the size of the buffer, with the same semantics
 * as that function. If the abuf is not currently alloced, then it will alloc
 * it if the size needs to increase (i.e. set the alloced member to true)
 *
 * @abuf: abuf to adjust
 * @new_size: new size in bytes.
 *	if 0, the abuf is freed
 *	if greater than the current size, the abuf is extended and the new
 *	   space is not inited. The alloced member is set to true
 *	if less than the current size, the abuf is contracted and the data at
 *	   the end is lost. If @new_size is 0, this sets the alloced member to
 *	   false
 * Return: true if OK, false if out of memory
 */
bool abuf_realloc(struct abuf *abuf, size_t new_size);

/**
 * abuf_realloc_inc() - Increment abuf size by a given amount
 *
 * @abuf: abuf to adjust
 * @inc: Size incrmement to use (the buffer size will be increased by this much)
 * Return: true if OK, false if out of memory
 */
bool abuf_realloc_inc(struct abuf *abuf, size_t inc);

/**
 * abuf_copy() - Make a copy of an abuf
 *
 * Creates an allocated copy of @old in @new
 *
 * @old: abuf to copy
 * @new: new abuf to hold the copy (inited by this function)
 * Return: true if OK, false if out of memory
 */
bool abuf_copy(const struct abuf *old, struct abuf *new);

/**
 * abuf_printf() - Format a string and place it in an abuf
 *
 * @buf: The buffer to place the result into
 * @fmt: The format string to use
 * @...: Arguments for the format string
 * Return: the number of characters writtenwhich would be
 * generated for the given input, excluding the trailing null,
 * as per ISO C99.
 *
 * The abuf is expanded as necessary to fit the formated string
 *
 * See the vsprintf() documentation for format string extensions over C99.
 *
 * Returns: number of characters written (excluding trailing nul) on success,
 * -E2BIG if the size exceeds 4K, -ENOMEM if out of memory, -EFAULT if there is
 * an internal bug in the vsnprintf() implementation
 */
int abuf_printf(struct abuf *buf, const char *fmt, ...)
		__attribute__ ((format (__printf__, 2, 3)));

/**
 * abuf_uninit_move() - Return the allocated contents and uninit the abuf
 *
 * This returns the abuf data to the caller, allocating it if necessary, so that
 * the caller receives data that it can be sure will hang around. The caller is
 * responsible for freeing the data.
 *
 * If the abuf has allocated data, it is returned. If the abuf has data but it
 * is not allocated, then it is first allocated, then returned.
 *
 * If the abuf size is 0, this returns NULL
 *
 * The abuf is uninited as part of this, except if the allocation fails, in
 * which NULL is returned and the abuf remains untouched.
 *
 * The abuf must be inited before this can be called.
 *
 * @abuf: abuf to uninit
 * @sizep: if non-NULL, returns the size of the returned data
 * Return: data contents, allocated with malloc(), or NULL if the data could not
 *	be allocated, or the data size is 0
 */
void *abuf_uninit_move(struct abuf *abuf, size_t *sizep);

/**
 * abuf_init_move() - Make abuf take over the management of an allocated region
 *
 * After this, @data must not be used. All access must be via the abuf.
 *
 * @abuf: abuf to init
 * @data: Existing allocated buffer to place in the abuf
 * @size: Size of allocated buffer
 */
void abuf_init_move(struct abuf *abuf, void *data, size_t size);

/**
 * abuf_init_set() - Set up a new abuf
 *
 * Inits a new abuf and sets up its (unallocated) data
 *
 * @abuf: abuf to set up
 * @data: New contents of abuf
 * @size: New size of abuf
 */
void abuf_init_set(struct abuf *abuf, void *data, size_t size);

/**
 * abuf_init_const() - Set up a new const abuf
 *
 * Inits a new abuf and sets up its (unallocated) data. The only current
 * difference between this and abuf_init_set() is the 'data' parameter is a
 * const pointer. At some point a flag could be used to indicate const-ness.
 *
 * @abuf: abuf to set up
 * @data: New contents of abuf
 * @size: New size of abuf
 */
void abuf_init_const(struct abuf *abuf, const void *data, size_t size);

/**
 * abuf_init_size() - Set up an allocated abuf
 *
 * Init a new abuf and allocate its size.
 *
 * @abuf: abuf to set up
 * @data: New contents of abuf
 * @size: New size of abuf
 */
bool abuf_init_size(struct abuf *buf, size_t size);

/**
 * abuf_uninit() - Free any memory used by an abuf
 *
 * The buffer must be inited before this can be called.
 *
 * @abuf: abuf to uninit
 */
void abuf_uninit(struct abuf *abuf);

/**
 * abuf_init() - Set up a new abuf
 *
 * This initially has no data and alloced is set to false. This is equivalent to
 * setting all fields to 0, e.g. with memset(), so callers can do that instead
 * if desired.
 *
 * @abuf: abuf to set up
 */
void abuf_init(struct abuf *abuf);

#endif
