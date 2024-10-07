/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2024 - Analog Devices, Inc.
 */

#ifndef __CONFIG_SC598_SOM_H
#define __CONFIG_SC598_SOM_H

/*
 * Memory Settings
 */
#define MEM_IS43TR16512BL
#define MEM_ISSI_4Gb_DDR3_800MHZ
#define MEM_DMC0

#define CFG_SYS_SDRAM_BASE	0x90000000
#define CFG_SYS_SDRAM_SIZE	0x0e000000

/* GIC */
#define GICD_BASE 0x31200000
#define GICR_BASE 0x31240000

#endif
