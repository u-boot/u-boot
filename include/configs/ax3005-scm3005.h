/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021-2026 Axiado Corporation (or its affiliates).
 */

#ifndef __AX3005_SCM3005_H
#define __AX3005_SCM3005_H

#include <linux/sizes.h>

#define GICD_BASE		0x40400000
#define GICR_BASE		0x40500000

#define SYS_TIMER_CTRL		0x48016000
#define SYS_TIMER_ENABLE	0x1
#define SYS_TIMER_DISABLE	0x0

/* DRAM: 2 GB at 0x80000000 */
#define CFG_SYS_SDRAM_BASE	0x80000000
#define CFG_SYS_SDRAM_SIZE	SZ_2G
#define CFG_SYS_INIT_SP_ADDR	(CFG_SYS_SDRAM_BASE + SZ_1M)

#define CFG_SYS_MAXARGS		64
#define CFG_SYS_BARGSIZE	CFG_SYS_CBSIZE

#define CFG_SYS_BAUDRATE_TABLE	\
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

#endif /* __AX3005_SCM3005_H */
