/*
 * Copyright (C) 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_DBG_SERVER_H__
#define __FSL_DBG_SERVER_H__

#include <asm/io.h>
#include <common.h>

/*
 * Define Debug Server firmware version information
 */

/* Major version number: incremented on API compatibility changes */
#define DEBUG_SERVER_VER_MAJOR	0

/* Minor version number: incremented on API additions (backward
 * compatible); reset when major version is incremented.
 */
#define DEBUG_SERVER_VER_MINOR	1

#define DEBUG_SERVER_INIT_STATUS	(1 << 0)
#define DEBUG_SERVER_INIT_STATUS_MASK	(0x00000001)

int debug_server_init(void);
unsigned long debug_server_get_dram_block_size(void);

#endif /* __FSL_DBG_SERVER_H__ */

