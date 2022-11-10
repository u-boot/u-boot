/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

/* ns16550 UART is memory-mapped in Quark SoC */
#undef  CONFIG_SYS_NS16550_PORT_MAPPED

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

/* Environment configuration */

#endif	/* __CONFIG_H */
