/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#include <asm/mach-imx/sys_proto.h>
#include <linux/types.h>

enum boot_device get_boot_device(void);
int print_bootinfo(void);
