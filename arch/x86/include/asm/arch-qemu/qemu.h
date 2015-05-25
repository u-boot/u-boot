/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ARCH_QEMU_H_
#define _ARCH_QEMU_H_

/* Programmable Attribute Map (PAM) Registers */
#define I440FX_PAM		0x59
#define Q35_PAM			0x90
#define PAM_NUM			7
#define PAM_RW			0x33

/* IDE Timing Register */
#define IDE0_TIM		0x40
#define IDE1_TIM		0x42
#define IDE_DECODE_EN		0x8000

/* I/O Ports */
#define CMOS_ADDR_PORT		0x70
#define CMOS_DATA_PORT		0x71

#define LOW_RAM_ADDR		0x34
#define HIGH_RAM_ADDR		0x35

#endif /* _ARCH_QEMU_H_ */
