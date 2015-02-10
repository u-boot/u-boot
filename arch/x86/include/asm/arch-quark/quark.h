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

/* Port 0x00: Memory Arbiter Message Port Registers */

/* Enhanced Configuration Space */
#define AEC_CTRL		0x00

/* Port 0x03: Host Bridge Message Port Registers */

/* Host Memory I/O Boundary */
#define HM_BOUND		0x08

/* Extended Configuration Space */
#define HEC_REG			0x09

/* Port 0x04: Remote Management Unit Message Port Registers */

/* ACPI PBLK Base Address Register */
#define PBLK_BA			0x70

/* SPI DMA Base Address Register */
#define SPI_DMA_BA		0x7a

/* Port 0x05: Memory Manager Message Port Registers */

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

/* Legacy Bridge PCI Configuration Registers */
#define LB_GBA			0x44
#define LB_PM1BLK		0x48
#define LB_GPE0BLK		0x4c
#define LB_ACTL			0x58
#define LB_PABCDRC		0x60
#define LB_PEFGHRC		0x64
#define LB_WDTBA		0x84
#define LB_BCE			0xd4
#define LB_BC			0xd8
#define LB_RCBA			0xf0

#endif /* _QUARK_H_ */
