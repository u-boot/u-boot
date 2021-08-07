/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef __ARCH_IMX8ULP_SYS_PROTO_H
#define __ARCH_NMX8ULP_SYS_PROTO_H

#include <asm/mach-imx/sys_proto.h>

extern unsigned long rom_pointer[];

enum bt_mode get_boot_mode(void);
#endif
