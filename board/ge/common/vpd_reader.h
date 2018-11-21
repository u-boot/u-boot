/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 General Electric Company
 */

#include "common.h"

/*
 * Read VPD from given data, verify content, and call callback
 * for each vital product data block.
 *
 * Returns Non-zero on error.  Negative numbers encode errno.
 */
int vpd_reader(size_t size, u8 *data, void *userdata,
	       int (*fn)(void *userdata, u8 id, u8 version, u8 type,
			 size_t size, u8 const *data));
