/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>
#include <configs/x86-chromebook.h>

#define CONFIG_RTL8169
/* Avoid a warning in the Realtek Ethernet driver */
#define CONFIG_SYS_CACHELINE_SIZE 16

#define CONFIG_VGA_AS_SINGLE_DEVICE

#endif	/* __CONFIG_H */
