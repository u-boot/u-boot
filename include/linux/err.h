#ifndef _LINUX_ERR_H
#define _LINUX_ERR_H

#include <linux/compiler.h>
#include <linux/compat.h>

#include <linux/errno.h>

/*
 * Kernel pointers have redundant information, so we can use a
 * scheme where we can return either an error code or a dentry
 * pointer with the same return value.
 *
 * This should be a per-architecture thing, to allow different
 * error and pointer decisions.
 */
#define MAX_ERRNO	4095

#ifndef __ASSEMBLY__

#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline void *ERR_PTR(long error)
{
	return (void *)(CONFIG_ERR_PTR_OFFSET + error);
}

static inline long PTR_ERR(const void *ptr)
{
	return ((long)ptr - CONFIG_ERR_PTR_OFFSET);
}

static inline long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)PTR_ERR(ptr));
}

static inline bool IS_ERR_OR_NULL(const void *ptr)
{
	return !ptr || IS_ERR_VALUE((unsigned long)PTR_ERR(ptr));
}

/**
 * ERR_CAST - Explicitly cast an error-valued pointer to another pointer type
 * @ptr: The pointer to cast.
 *
 * Explicitly cast an error-valued pointer to another pointer type in such a
 * way as to make it clear that's what's going on.
 */
static inline void * __must_check ERR_CAST(__force const void *ptr)
{
	/* cast away the const */
	return (void *) ptr;
}

/**
 * PTR_ERR_OR_ZERO - Extract the error code from a pointer if it has one.
 * @ptr: A potential error pointer.
 *
 * Convenience function that can be used inside a function that returns
 * an error code to propagate errors received as error pointers.
 * For example, ``return PTR_ERR_OR_ZERO(ptr);`` replaces:
 *
 * .. code-block:: c
 *
 *	if (IS_ERR(ptr))
 *		return PTR_ERR(ptr);
 *	else
 *		return 0;
 *
 * Return: The error code within @ptr if it is an error pointer; 0 otherwise.
 */
static inline int __must_check PTR_ERR_OR_ZERO(__force const void *ptr)
{
	if (IS_ERR(ptr))
		return PTR_ERR(ptr);
	else
		return 0;
}

#endif

#endif /* _LINUX_ERR_H */
