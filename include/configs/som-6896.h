/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the SOM-6896
 *
 * Copyright (C) 2015 NovaTech LLC
 * George McCollister <george.mccollister@gmail.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SYS_MONITOR_LEN		(1 << 20)

#define CONFIG_MISC_INIT_R

#define VIDEO_IO_OFFSET			0
#define CONFIG_X86EMU_RAW_IO

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial,usbkbd\0" \
					"stdout=serial,vidconsole\0" \
					"stderr=serial,vidconsole\0"

#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x00ff0000

#endif	/* __CONFIG_H */
