/**
 * linux-compat.h - DesignWare USB3 Linux Compatibiltiy Adapter  Header
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com
 *
 * Authors: Kishon Vijay Abraham I <kishon@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 *
 */

#ifndef __DWC3_LINUX_COMPAT__
#define __DWC3_LINUX_COMPAT__

#define WARN(val, format, arg...)	debug(format, ##arg)
#define dev_WARN(dev, format, arg...)	debug(format, ##arg)

static inline size_t strlcat(char *dest, const char *src, size_t n)
{
	strcat(dest, src);
	return strlen(dest) + strlen(src);
}

#endif
