/*
 * Copyright (C) ST-Ericsson SA 2009
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __U8500_H
#define __U8500_H

/*
 * base register values for U8500
 */
#define CFG_PRCMU_BASE		0x80157000	/* Power, reset and clock
						   Management Unit */
#define CFG_SDRAMC_BASE		0x903CF000	/* SDRAMC cnf registers */
#define CFG_FSMC_BASE		0x80000000	/* FSMC Controller */

/*
 * U8500 GPIO register base for 9 banks
 */
#define U8500_GPIO_0_BASE			0x8012E000
#define U8500_GPIO_1_BASE			0x8012E080
#define U8500_GPIO_2_BASE			0x8000E000
#define U8500_GPIO_3_BASE			0x8000E080
#define U8500_GPIO_4_BASE			0x8000E100
#define U8500_GPIO_5_BASE			0x8000E180
#define U8500_GPIO_6_BASE			0x8011E000
#define U8500_GPIO_7_BASE			0x8011E080
#define U8500_GPIO_8_BASE			0xA03FE000

#endif	/* __U8500_H */
