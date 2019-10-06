/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */
#ifndef _ERRNO_H
#define _ERRNO_H

#include <linux/errno.h>

extern int errno;

#define __set_errno(val) do { errno = val; } while (0)

/**
 * errno_str() - get description for error number
 *
 * @errno:	error number (negative in case of error)
 * Return:	string describing the error. If CONFIG_ERRNO_STR is not
 *		defined an empty string is returned.
 */
#ifdef CONFIG_ERRNO_STR
const char *errno_str(int errno);
#else
static const char error_message[] = "";

static inline const char *errno_str(int errno)
{
	return error_message;
}
#endif
#endif /* _ERRNO_H */
