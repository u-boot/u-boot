/*
 * Copyright (C) 2014-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <mach/platdevice.h>

#define UART_MASTER_CLK		80000000

SERIAL_DEVICE(0, 0x54006800, UART_MASTER_CLK)
SERIAL_DEVICE(1, 0x54006900, UART_MASTER_CLK)
SERIAL_DEVICE(2, 0x54006a00, UART_MASTER_CLK)
SERIAL_DEVICE(3, 0x54006b00, UART_MASTER_CLK)
