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
 * alist_calc_index() - Calculate the index of an item in the list
 *
 * The returned element number will be -1 if the list is empty or the pointer
 * pointers to before the list starts.
 *
 * If the pointer points to after the last item, the calculated element-number
 * will be returned, even though it is greater than lst->count
 *
 * @lst: alist to check
 * @ptr: pointer to check
 * Return: element number of the pointer
 */
int alist_calc_index(const struct alist *lst, const void *ptr);

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

/** get next entry as a constant */
#define alist_next(_lst, _objp)	\
	((const typeof(_objp))alist_next_ptrd(_lst, _objp))

/** get next entry, which can be written to */
#define alist_nextw(_lst, _objp)	\
	((typeof(_objp))alist_next_ptrd(_lst, _objp))

/**
 * alist_next_ptrd() - Get a pointer to the next list element
 *
 * This returns NULL if the requested element is beyond lst->count
 *
 * @lst: List to check
 * @ptr: Pointer to current element (must be valid)
 * Return: Pointer to next element, or NULL if @ptr is the last
 */
const void *alist_next_ptrd(const struct alist *lst, const void *ptr);

/**
 * alist_chk_ptr() - Check whether a pointer is within a list
 *
 * Checks if the pointer points to an existing element of the list. The pointer
 * must point to the start of an element, either in the list, or just outside of
 * it. This function is only useful for handling for() loops
 *
 * Return: true if @ptr is within the list (0..count-1), else false
 */
bool alist_chk_ptr(const struct alist *lst, const void *ptr);

/**
 * alist_start() - Get the start of the list (first element)
 *
 * Note that this will always return ->data even if it is not NULL
 *
 * Usage:
 *	const struct my_struct *obj;    # 'const' is optional
 *
 *	alist_start(&lst, struct my_struct)
 */
#define alist_start(_lst, _struct) \
	((_struct *)(_lst)->data)

/**
 * alist_end() - Get the end of the list (just after last element)
 *
 * Usage:
 *	const struct my_struct *obj;    # 'const' is optional
 *
 *	alist_end(&lst, struct my_struct)
 */
#define alist_end(_lst, _struct) \
	((_struct *)(_lst)->data + (_lst)->count)

/**
 * alist_for_each() - Iterate over an alist (with constant pointer)
 *
 * Use as:
 *	const struct my_struct *obj;    # 'const' is optional
 *
 *	alist_for_each(obj, &lst) {
 *		obj->...
 *	}
 */
#define alist_for_each(_pos, _lst) \
	for (_pos = alist_start(_lst, typeof(*(_pos))); \
	     _pos < alist_end(_lst, typeof(*(_pos))); \
	     _pos++)

/**
 * alist_for_each_filter() - version which sets up a 'from' pointer too
 *
 * This is used for filtering out information in the list. It works by iterating
 * through the list, copying elements down over the top of elements to be
 * deleted.
 *
 * In this example, 'from' iterates through the list from start to end,, 'to'
 * also begins at the start, but only increments if the element at 'from' should
 * be kept. This provides an O(n) filtering operation. Note that
 * alist_update_end() must be called after the loop, to update the count.
 *
 *	alist_for_each_filter(from, to, &lst) {
 *		if (from->val != 2)
 *			*to++ = *from;
 *	}
 *	alist_update_end(&lst, to);
 */
#define alist_for_each_filter(_pos, _from, _lst) \
	for (_pos = _from = alist_start(_lst, typeof(*(_pos))); \
	     _pos < alist_end(_lst, typeof(*(_pos))); \
	     _pos++)

/**
 * alist_update_end() - Set the element count based on a given pointer
 *
 * Set the given element as the final one
 */
void alist_update_end(struct alist *lst, const void *end);

/**
 * alist_empty() - Empty an alist
 *
 * This removes all entries from the list, without changing the allocated size
 */
void alist_empty(struct alist *lst);

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
