/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 *
 * Based on Kirkwood support:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef NET2BIG_V2_H
#define NET2BIG_V2_H

/* GPIO configuration */
#define NET2BIG_V2_OE_LOW		0x0600E000
#define NET2BIG_V2_OE_HIGH		0x00000134
#define NET2BIG_V2_OE_VAL_LOW		0x10030000
#define NET2BIG_V2_OE_VAL_HIGH		0x00000000

/* Buttons */
#define NET2BIG_V2_GPIO_PUSH_BUTTON	34

#endif /* NET2BIG_V2_H */
