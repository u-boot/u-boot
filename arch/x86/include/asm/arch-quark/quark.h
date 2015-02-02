/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _QUARK_H_
#define _QUARK_H_

/* Message Bus Ports */
#define MSG_PORT_MEM_ARBITER	0x00
#define MSG_PORT_HOST_BRIDGE	0x03
#define MSG_PORT_RMU		0x04
#define MSG_PORT_MEM_MGR	0x05
#define MSG_PORT_SOC_UNIT	0x31

/* Host Memory I/O Boundary */
#define HM_BOUND		0x08

/* eSRAM Block Page Control */
#define ESRAM_BLK_CTRL		0x82
#define ESRAM_BLOCK_MODE	0x10000000

/* DRAM */
#define DRAM_BASE		0x00000000
#define DRAM_MAX_SIZE		0x80000000

/* eSRAM */
#define ESRAM_SIZE		0x80000

/* Memory BAR Enable */
#define MEM_BAR_EN		0x00000001

/* I/O BAR Enable */
#define IO_BAR_EN		0x80000000

/* 64KiB of RMU binary in flash */
#define RMU_BINARY_SIZE		0x10000

#endif /* _QUARK_H_ */
