/*
 * Copyright (C) 2011 by Vladimir Zapolskiy <vz@mleia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#ifndef _LPC32XX_UART_H
#define _LPC32XX_UART_H

#include <asm/types.h>

/* UART Control Registers */
struct uart_ctrl_regs {
	u32 ctrl;		/* Control Register		*/
	u32 clkmode;		/* Clock Mode Register		*/
	u32 loop;		/* Loopback Control Register	*/
};

/* UART Control Register bits */
#define UART_CTRL_UART3_MD_CTRL		(1 << 11)
#define UART_CTRL_HDPX_INV		(1 << 10)
#define UART_CTRL_HDPX_EN		(1 << 9)
#define UART_CTRL_UART6_IRDA		(1 << 5)
#define UART_CTRL_IR_TX6_INV		(1 << 4)
#define UART_CTRL_IR_RX6_INV		(1 << 3)
#define UART_CTRL_IR_RX_LENGTH		(1 << 2)
#define UART_CTRL_IR_TX_LENGTH		(1 << 1)
#define UART_CTRL_UART5_USB_MODE	(1 << 0)

/* UART Clock Mode Register bits */
#define UART_CLKMODE_STATX(n)		(1 << ((n) + 16))
#define UART_CLKMODE_STAT		(1 << 14)
#define UART_CLKMODE_MASK(n)		(0x3 << (2 * (n) - 2))
#define UART_CLKMODE_AUTO(n)		(0x2 << (2 * (n) - 2))
#define UART_CLKMODE_ON(n)		(0x1 << (2 * (n) - 2))
#define UART_CLKMODE_OFF(n)		(0x0 << (2 * (n) - 2))

/* UART Loopback Control Register bits */
#define UART_LOOPBACK(n)		(1 << ((n) - 1))

#endif /* _LPC32XX_UART_H */
