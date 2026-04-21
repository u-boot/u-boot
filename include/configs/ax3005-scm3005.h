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

#define RAMDISK_BASE			0x80B00000
#define RAMDISK_SIZE			0x6400000

#define CONFIG_BOOTARGS			"console=ttyPS3,115200 " \
					"maxcpus=4 nr_cpus=4 earlycon " \
					"hugepages=16 root=/dev/ram rw " \
					"phram.phram=ramrofs," \
					__stringify(RAMDISK_BASE) "," __stringify(RAMDISK_SIZE)

#define CFG_SYS_BAUDRATE_TABLE	\
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

#endif /* __AX3005_SCM3005_H */
