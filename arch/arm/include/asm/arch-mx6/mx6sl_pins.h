/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __ASM_ARCH_MX6_MX6SL_PINS_H__
#define __ASM_ARCH_MX6_MX6SL_PINS_H__

#include <asm/imx-common/iomux-v3.h>

enum {
	MX6_PAD_SD2_CLK__USDHC2_CLK				= IOMUX_PAD(0x055C, 0x0254, 0, 0x0000, 0, 0),
	MX6_PAD_SD2_CMD__USDHC2_CMD				= IOMUX_PAD(0x0560, 0x0258, 0, 0x0000, 0, 0),
	MX6_PAD_SD2_DAT0__USDHC2_DAT0				= IOMUX_PAD(0x0564, 0x025C, 0, 0x0000, 0, 0),
	MX6_PAD_SD2_DAT1__USDHC2_DAT1				= IOMUX_PAD(0x0568, 0x0260, 0, 0x0000, 0, 0),
	MX6_PAD_SD2_DAT2__USDHC2_DAT2				= IOMUX_PAD(0x056C, 0x0264, 0, 0x0000, 0, 0),
	MX6_PAD_SD2_DAT3__USDHC2_DAT3				= IOMUX_PAD(0x0570, 0x0268, 0, 0x0000, 0, 0),
	MX6_PAD_UART1_RXD__UART1_RXD				= IOMUX_PAD(0x05A0, 0x0298, 0, 0x07FC, 0, 0),
	MX6_PAD_UART1_TXD__UART1_TXD				= IOMUX_PAD(0x05A4, 0x029C, 0, 0x0000, 0, 0),
};
#endif	/* __ASM_ARCH_MX6_MX6SL_PINS_H__ */
