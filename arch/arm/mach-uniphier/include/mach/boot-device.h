/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_BOOT_DEVICE_H_
#define _ASM_BOOT_DEVICE_H_

int get_boot_mode_sel(void);

struct boot_device_info {
	u32 type;
	char *info;
};

extern struct boot_device_info boot_device_table[];

#endif /* _ASM_BOOT_DEVICE_H_ */
