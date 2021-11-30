/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>
#include <configs/x86-chromebook.h>

/* We can rely on running natively, and this saves code size */
#undef CONFIG_BIOSEMU

#undef CONFIG_STD_DEVICES_SETTINGS
#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,i8042-kbd,serial\0" \
					"stdout=vidconsole,serial\0" \
					"stderr=vidconsole,serial\0"

#define CONFIG_TPL_TEXT_BASE		0xfffd8000

#endif	/* __CONFIG_H */
