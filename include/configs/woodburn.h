/*
 * (C) Copyright 2011, Stefano Babic <sbabic@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * Configuration for the woodburn board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>
#include "woodburn_common.h"

/* Set TEXT at the beginning of the NOR flash */
#define CONFIG_SYS_TEXT_BASE	0xA0000000

#endif				/* __CONFIG_H */
