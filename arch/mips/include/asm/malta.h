/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef _MIPS_ASM_MALTA_H
#define _MIPS_ASM_MALTA_H

#define MALTA_IO_PORT_BASE	0x18000000

#define MALTA_UART_BASE		(MALTA_IO_PORT_BASE + 0x3f8)

#define MALTA_GT_BASE		0x1be00000

#define MALTA_RESET_BASE	0x1f000500
#define GORESET			0x42

#define MALTA_FLASH_BASE	0x1fc00000

#endif /* _MIPS_ASM_MALTA_H */
