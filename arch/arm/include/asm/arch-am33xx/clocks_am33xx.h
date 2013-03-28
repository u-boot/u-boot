/*
 * clocks_am33xx.h
 *
 * AM33xx clock define
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _CLOCKS_AM33XX_H_
#define _CLOCKS_AM33XX_H_

/* MAIN PLL Fdll = 550 MHz, by default */
#ifndef CONFIG_SYS_MPUCLK
#define CONFIG_SYS_MPUCLK	550
#endif

extern void pll_init(void);
extern void enable_emif_clocks(void);
extern void enable_dmm_clocks(void);

#endif	/* endif _CLOCKS_AM33XX_H_ */
