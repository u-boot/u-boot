/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Total Compute platform. Parts were derived from other ARM
 * configurations.
 * (C) Copyright 2020-2023 Arm Limited
 * Usama Arif <usama.arif@arm.com>
 */

#ifndef __TOTAL_COMPUTE_H
#define __TOTAL_COMPUTE_H

/* Link Definitions */

#define CFG_EXTRA_ENV_SETTINGS	\
				"bootm_size=0x20000000\0"	\
				"load_addr=0xa0000000\0"	\
				"initrd_addr_r=0x88000000\0"
/*
 * If vbmeta partition is present, boot Android with verification using AVB.
 * Else if system partition is present (no vbmeta partition), boot Android
 * without verification (for development purposes).
 * Else boot FIT image.
 */

#define CFG_SYS_FLASH_BASE		0x0C000000

#endif /* __TOTAL_COMPUTE_H */
