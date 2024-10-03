/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2024 - Analog Devices, Inc.
 */

#ifndef __CONFIG_SC594_SOM_H
#define __CONFIG_SC594_SOM_H

/*
 * Memory Settings
 */
#define MEM_IS43TR16512BL
#define MEM_ISSI_8Gb_DDR3_800MHZ
#define MEM_DMC0

#define CFG_SYS_SDRAM_BASE	0xA0000000
#define CFG_SYS_SDRAM_SIZE	0x20000000

#endif
