/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SYS_MONITOR_LEN		(1 << 20)

#define CONFIG_PREBOOT

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

#define VIDEO_IO_OFFSET				0
#define CONFIG_X86EMU_RAW_IO
#define CONFIG_CMD_BMP
#define CONFIG_BMP_16BPP

/* Environment settings */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x006ec000
#define CONFIG_ENV_OFFSET_REDUND	\
	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)

#endif	/* __CONFIG_H */
