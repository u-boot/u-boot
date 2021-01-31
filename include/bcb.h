// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Eugeniu Rosca <rosca.eugeniu@gmail.com>
 *
 * Android Bootloader Control Block Header
 */

#ifndef __BCB_H__
#define __BCB_H__

#if CONFIG_IS_ENABLED(CMD_BCB)
int bcb_write_reboot_reason(int devnum, char *partp, const char *reasonp);
#else
#include <linux/errno.h>
static inline int bcb_write_reboot_reason(int devnum, char *partp, const char *reasonp)
{
	return -EOPNOTSUPP;
}
#endif

#endif /* __BCB_H__ */
