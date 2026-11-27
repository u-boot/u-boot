/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * LoongArch QEMU always places a part of the RAM at 0x0. These options are only
 * intended to specify the initial stack pointer address, and can be replaced
 * with CUSTOM_SYS_INIT_SP_ADDR when it's supported in the future.
 */
#define CFG_SYS_INIT_RAM_ADDR	0x0
#define CFG_SYS_INIT_RAM_SIZE	0x80000

#endif
