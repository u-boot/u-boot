/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the Renesas GRPEACH board
 *
 * Copyright (C) 2017-2019 Renesas Electronics
 */

#ifndef __GRPEACH_H
#define __GRPEACH_H

/* Internal RAM Size (RZ/A1=3M, RZ/A1M=5M, RZ/A1H=10M) */
#define CFG_SYS_SDRAM_BASE		0x20000000
#define CFG_SYS_SDRAM_SIZE		(10 * 1024 * 1024)

#endif	/* __GRPEACH_H */
