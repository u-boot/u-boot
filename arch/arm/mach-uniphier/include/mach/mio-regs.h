/*
 * UniPhier MIO (Media I/O) registers
 *
 * Copyright (C) 2014 Panasonic Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_MIO_REGS_H
#define ARCH_MIO_REGS_H

#define MIO_BASE		0x59810000

#define MIO_CLKCTRL(i)		(MIO_BASE + 0x200 * (i) + 0x0020)
#define MIO_RSTCTRL(i)		(MIO_BASE + 0x200 * (i) + 0x0110)
#define MIO_USB_RSTCTRL(i)	(MIO_BASE + 0x200 * (i) + 0x0114)

#define MIO_USB_RSTCTRL_XRST	(0x1 << 0)

#endif /* ARCH_MIO_REGS_H */
