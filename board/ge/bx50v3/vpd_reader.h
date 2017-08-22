/*
 * Copyright 2016 General Electric Company
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "common.h"

/*
 * Read VPD from given data, verify content, and call callback
 * for each vital product data block.
 *
 * Returns Non-zero on error.  Negative numbers encode errno.
 */
int vpd_reader(
	size_t size,
	uint8_t * data,
	void * userdata,
	int (*fn)(
	    void * userdata,
	    uint8_t id,
	    uint8_t version,
	    uint8_t type,
	    size_t size,
	    uint8_t const * data));
