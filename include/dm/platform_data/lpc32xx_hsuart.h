/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef _LPC32XX_HSUART_PLAT_H
#define _LPC32XX_HSUART_PLAT_H

/**
 * struct lpc32xx_hsuart_plat - NXP LPC32xx HSUART platform data
 *
 * @base:               Base register address
 */
struct lpc32xx_hsuart_plat {
	unsigned long base;
};

#endif
