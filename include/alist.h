/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Handles a contiguous list of pointers which be allocated and freed
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __ALIST_H
#define __ALIST_H

#include <stdbool.h>
#include <linux/bitops.h>
#include <linux/types.h>

/**
 * struct alist - object list that can be allocated and freed
 *
 * Holds a list of objects, each of the same size. The object is typically a
 * C struct. The array is alloced in memory can change in size.
 *
 * The list rememebers the size of the list, but has a separate count of how
 * much space is allocated, This allows it increase in size in steps as more
 * elements are added, which is more efficient that reallocating the list every
 * time a single item is added
 *
 * Two types of access are provided:
 *
 * alist_get...(index)
 *	gets an existing element, if its index is less that size
 *
 * alist_ensure(index)
 *	address an existing element, or creates a new one if not present
 *
 * @data: object data of size `@obj_size * @alloc`. The list can grow as
 * needed but never shrinks
 * @obj_size: Size of each object in bytes
 * @count: number of objects in array
 * @alloc: allocated length of array, to which @count can grow
 * @flags: flags for the alist (ALISTF_...)
 */
struct alist {
	void *data;
	u16 obj_size;
	u16 count;
	u16 alloc;
	u16 flags;
};

/**
 * enum alist_flags - Flags for the alist
 *
 * @ALIST_FAIL: true if any allocation has failed. Once this has happened, the
 * alist is dead and cannot grow further
 */
enum alist_flags {
	ALISTF_FAIL	= BIT(0),
};

/**
 * alist_has() - Check if an index is within the list range
 *
 * Checks if index is within the current alist count
 *
 * @lst: alist to check
 * @index: Index to check
 * Returns: true if value, else false
 */
static inline bool alist_has(struct alist *lst, uint index)
{
	return index < lst->count;
}

/**
 * alist_err() - Check if the alist is still valid
 *
 * @lst: List to check
 * Return: false if OK, true if any previous allocation failed
 */
static inline bool alist_err(struct alist *lst)
{
	return lst->flags & ALISTF_FAIL;
}

/**
 * alist_full() - Check if the alist is full
 *
 * @lst: List to check
 * Return: true if full, false otherwise
 */
static inline bool alist_full(struct alist *lst)
{
	return lst->count == lst->alloc;
}

/**
 * alist_get_ptr() - Get the value of a pointer
 *
 * @lst: alist to check
 * @index: Index to read from
 * Returns: pointer, if present, else NULL
 */
const void *alist_get_ptr(const struct alist *lst, uint index);

/**
 * alist_getd() - Get the value of a pointer directly, with no checking
 *
 * This must only be called on indexes for which alist_has() returns true
 *
 * @lst: alist to check
 * @index: Index to read from
 * Returns: pointer value (may be NULL)
 */
static inline const void *alist_getd(struct alist *lst, uint index)
{
	return lst->data + index * lst->obj_size;
}

/**
 * alist_get() - get an entry as a constant
 *
 * Use as (to obtain element 2 of the list):
 *	const struct my_struct *ptr = alist_get(lst, 2, struct my_struct)
 */
#define alist_get(_lst, _index, _struct)	\
	((const _struct *)alist_get_ptr(_lst, _index))

/** get an entry which can be written to */
#define alist_getw(_lst, _index, _struct)	\
	((_struct *)alist_get_ptr(_lst, _index))

/**
 * alist_ensure_ptr() - Ensure an object exists at a given index
 *
 * This provides read/write access to an array element. If it does not exist,
 * it is allocated, reading for the caller to store the object into
 *
 * Allocates a object at the given index if needed
 *
 * @lst: alist to check
 * @index: Index to address
 * Returns: pointer where struct can be read/written, or NULL if out of memory
 */
void *alist_ensure_ptr(struct alist *lst, uint index);

/**
 * alist_ensure() - Address a struct, the correct object type
 *
 * Use as:
 *	struct my_struct *ptr = alist_ensure(&lst, 4, struct my_struct);
 */
#define alist_ensure(_lst, _index, _struct)	\
	((_struct *)alist_ensure_ptr(_lst, _index))

/**
 * alist_add_placeholder() - Add a new item to the end of the list
 *
 * @lst: alist to add to
 * Return: Pointer to the newly added position, or NULL if out of memory. Note
 * that this is not inited so the caller must copy the requested struct to the
 * returned pointer
 */
void *alist_add_placeholder(struct alist *lst);

/**
 * alist_add_ptr() - Ad a new object to the list
 *
 * @lst: alist to add to
 * @obj: Pointer to object to copy in
 * Returns: pointer to where the object was copied, or NULL if out of memory
 */
void *alist_add_ptr(struct alist *lst, void *obj);

/**
 * alist_expand_by() - Expand a list by the given amount
 *
 * @lst: alist to expand
 * @inc_by: Amount to expand by
 * Return: true if OK, false if out of memory
 */
bool alist_expand_by(struct alist *lst, uint inc_by);

/**
 * alist_add() - Used to add an object type with the correct type
 *
 * Use as:
 *	struct my_struct obj;
 *	struct my_struct *ptr = alist_add(&lst, &obj);
 */
#define alist_add(_lst, _obj)	\
	((typeof(_obj) *)alist_add_ptr(_lst, &(_obj)))

/**
 * alist_init() - Set up a new object list
 *
 * Sets up a list of objects, initially empty
 *
 * @lst: alist to set up
 * @obj_size: Size of each element in bytes
 * @alloc_size: Number of items to allowed to start, before reallocation is
 * needed (0 to start with no space)
 * Return: true if OK, false if out of memory
 */
bool alist_init(struct alist *lst, uint obj_size, uint alloc_size);

/**
 * alist_init_struct() - Typed version of alist_init()
 *
 * Use as:
 *	alist_init(&lst, struct my_struct);
 */
#define alist_init_struct(_lst, _struct)	\
	alist_init(_lst, sizeof(_struct), 0)

/**
 * alist_uninit_move_ptr() - Return the allocated contents and uninit the alist
 *
 * This returns the alist data to the caller, so that the caller receives data
 * that it can be sure will hang around. The caller is responsible for freeing
 * the data.
 *
 * If the alist size is 0, this returns NULL
 *
 * The alist is uninited as part of this.
 *
 * The alist must be inited before this can be called.
 *
 * @alist: alist to uninit
 * @countp: if non-NULL, returns the number of objects in the returned data
 * (which is @alist->size)
 * Return: data contents, allocated with malloc(), or NULL if the data could not
 *	be allocated, or the data size is 0
 */
void *alist_uninit_move_ptr(struct alist *alist, size_t *countp);

/**
 * alist_uninit_move() - Typed version of alist_uninit_move_ptr()
 */
#define alist_uninit_move(_lst, _countp, _struct)	\
	(_struct *)alist_uninit_move_ptr(_lst, _countp)

/**
 * alist_uninit() - Free any memory used by an alist
 *
 * The alist must be inited before this can be called.
 *
 * @alist: alist to uninit
 */
void alist_uninit(struct alist *alist);

#endif /* __ALIST_H */
