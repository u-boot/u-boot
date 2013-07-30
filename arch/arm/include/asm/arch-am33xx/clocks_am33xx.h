/*
 * clocks_am33xx.h
 *
 * AM33xx clock define
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CLOCKS_AM33XX_H_
#define _CLOCKS_AM33XX_H_

/* MAIN PLL Fdll = 550 MHz, by default */
#ifndef CONFIG_SYS_MPUCLK
#define CONFIG_SYS_MPUCLK	550
#endif

#define UART_RESET		(0x1 << 1)
#define UART_CLK_RUNNING_MASK	0x1
#define UART_SMART_IDLE_EN	(0x1 << 0x3)

extern void enable_dmm_clocks(void);

#endif	/* endif _CLOCKS_AM33XX_H_ */
