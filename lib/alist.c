// SPDX-License-Identifier: GPL-2.0+
/*
 * Handles a contiguous list of pointers which be allocated and freed
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <alist.h>
#include <display_options.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

enum {
	ALIST_INITIAL_SIZE	= 4,	/* default size of unsized list */
};

bool alist_init(struct alist *lst, uint obj_size, uint start_size)
{
	/* Avoid realloc for the initial size to help malloc_simple */
	memset(lst, '\0', sizeof(struct alist));
	if (start_size) {
		lst->data = calloc(obj_size, start_size);
		if (!lst->data) {
			lst->flags = ALISTF_FAIL;
			return false;
		}
		lst->alloc = start_size;
	}
	lst->obj_size = obj_size;

	return true;
}

void alist_uninit(struct alist *lst)
{
	free(lst->data);

	/* Clear fields to avoid any confusion */
	memset(lst, '\0', sizeof(struct alist));
}

void alist_empty(struct alist *lst)
{
	lst->count = 0;
}

/**
 * alist_expand_to() - Expand a list to the given size
 *
 * @lst: List to modify
 * @inc_by: Amount to expand to
 * Return: true if OK, false if out of memory
 */
static bool alist_expand_to(struct alist *lst, uint new_alloc)
{
	void *new_data;

	if (lst->flags & ALISTF_FAIL)
		return false;

	/* avoid using realloc() since it increases code size */
	new_data = malloc(lst->obj_size * new_alloc);
	if (!new_data) {
		lst->flags |= ALISTF_FAIL;
		return false;
	}

	memcpy(new_data, lst->data, lst->obj_size * lst->alloc);
	free(lst->data);

	memset(new_data + lst->obj_size * lst->alloc, '\0',
	       lst->obj_size * (new_alloc - lst->alloc));
	lst->alloc = new_alloc;
	lst->data = new_data;

	return true;
}

bool alist_expand_by(struct alist *lst, uint inc_by)
{
	return alist_expand_to(lst, lst->alloc + inc_by);
}

/**
 * alist_expand_min() - Expand to at least the provided size
 *
 * Expands to the lowest power of two which can incorporate the new size
 *
 * @lst: alist to expand
 * @min_alloc: Minimum new allocated size; if 0 then ALIST_INITIAL_SIZE is used
 * Return: true if OK, false if out of memory
 */
static bool alist_expand_min(struct alist *lst, uint min_alloc)
{
	uint new_alloc;

	for (new_alloc = lst->alloc ?: ALIST_INITIAL_SIZE;
	     new_alloc < min_alloc;)
		new_alloc *= 2;

	return alist_expand_to(lst, new_alloc);
}

const void *alist_get_ptr(const struct alist *lst, uint index)
{
	if (index >= lst->count)
		return NULL;

	return lst->data + index * lst->obj_size;
}

int alist_calc_index(const struct alist *lst, const void *ptr)
{
	uint index;

	if (!lst->count || ptr < lst->data)
		return -1;

	index = (ptr - lst->data) / lst->obj_size;

	return index;
}

void alist_update_end(struct alist *lst, const void *ptr)
{
	int index;

	index = alist_calc_index(lst, ptr);
	lst->count = index == -1 ? 0 : index;
}

bool alist_chk_ptr(const struct alist *lst, const void *ptr)
{
	int index = alist_calc_index(lst, ptr);

	return index >= 0 && index < lst->count;
}

const void *alist_next_ptrd(const struct alist *lst, const void *ptr)
{
	int index = alist_calc_index(lst, ptr);

	assert(index != -1);

	return alist_get_ptr(lst, index + 1);
}

void *alist_ensure_ptr(struct alist *lst, uint index)
{
	uint minsize = index + 1;
	void *ptr;

	if (index >= lst->alloc && !alist_expand_min(lst, minsize))
		return NULL;

	ptr = lst->data + index * lst->obj_size;
	if (minsize >= lst->count)
		lst->count = minsize;

	return ptr;
}

void *alist_add_placeholder(struct alist *lst)
{
	return alist_ensure_ptr(lst, lst->count);
}

void *alist_add_ptr(struct alist *lst, void *obj)
{
	void *ptr;

	ptr = alist_add_placeholder(lst);
	if (!ptr)
		return NULL;
	memcpy(ptr, obj, lst->obj_size);

	return ptr;
}

void *alist_uninit_move_ptr(struct alist *alist, size_t *countp)
{
	void *ptr;

	if (countp)
		*countp = alist->count;
	if (!alist->count) {
		alist_uninit(alist);
		return NULL;
	}

	ptr = alist->data;

	/* Clear everything out so there is no record of the data */
	alist_init(alist, alist->obj_size, 0);

	return ptr;
}
