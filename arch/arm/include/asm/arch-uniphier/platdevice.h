/*
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_PLATDEVICE_H
#define ARCH_PLATDEVICE_H

#include <dm/platdata.h>
#include <dm/platform_data/serial-uniphier.h>

#define SERIAL_DEVICE(n, ba, clk)					\
static struct uniphier_serial_platform_data serial_device##n = {	\
	.base = ba,							\
	.uartclk = clk							\
};									\
U_BOOT_DEVICE(serial##n) = {						\
	.name = DRIVER_NAME,						\
	.platdata = &serial_device##n					\
};

#include <asm/arch/ehci-uniphier.h>

#endif /* ARCH_PLATDEVICE_H */
