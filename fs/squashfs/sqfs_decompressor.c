// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Bootlin
 *
 * Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "sqfs_decompressor.h"
#include "sqfs_filesystem.h"
#include "sqfs_utils.h"

int sqfs_decompress(u16 comp_type, void *dest, unsigned long *dest_len,
		    void *source, u32 lenp)
{
	int ret = 0;

	switch (comp_type) {
	default:
		printf("Error: unknown compression type.\n");
		return -EINVAL;
	}

	return ret;
}
