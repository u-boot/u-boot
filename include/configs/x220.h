/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Allied Telesis Labs
 */

#ifndef _CONFIG_X220_H
#define _CONFIG_X220_H

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define CFG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#endif /* _CONFIG_X220_H */
