/*
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PLAT_UNIPHIER_SERIAL_H
#define __PLAT_UNIPHIER_SERIAL_H

#define DRIVER_NAME	"uniphier-uart"

struct uniphier_serial_platform_data {
	unsigned long base;
	unsigned int uartclk;
};

#endif /* __PLAT_UNIPHIER_SERIAL_H */
