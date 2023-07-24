/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for IOT2050
 * Copyright (c) Siemens AG, 2018-2021
 *
 * Authors:
 *   Le Jin <le.jin@siemens.com>
 *   Jan Kiszka <jan.kiszka@siemens.com>
 */

#ifndef __CONFIG_IOT2050_H
#define __CONFIG_IOT2050_H

#include <linux/sizes.h>

#include <configs/ti_armv7_common.h>

#ifdef CONFIG_ENV_WRITEABLE_LIST
#define CFG_ENV_FLAGS_LIST_STATIC					\
	"board_uuid:sw,board_name:sw,board_serial:sw,board_a5e:sw,"	\
	"mlfb:sw,fw_version:sw,seboot_version:sw,"			\
	"m2_manuel_config:sw,"						\
	"eth1addr:mw,eth2addr:mw,watchdog_timeout_ms:dw,boot_targets:sw"
#endif

#endif /* __CONFIG_IOT2050_H */
