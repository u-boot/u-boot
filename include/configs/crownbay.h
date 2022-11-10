/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SMSC_LPC47M

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial,i8042-kbd,usbkbd\0" \
					"stdout=serial,vidconsole\0" \
					"stderr=serial,vidconsole\0"

/* Environment configuration */

#endif	/* __CONFIG_H */
