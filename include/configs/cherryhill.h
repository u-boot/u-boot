/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=usbkbd,serial\0" \
					"stdout=vidconsole,serial\0" \
					"stderr=vidconsole,serial\0"

/* Environment configuration */

#endif	/* __CONFIG_H */
