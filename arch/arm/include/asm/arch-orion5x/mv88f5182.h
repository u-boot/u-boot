/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.aribaud@free.fr>
 *
 * Based on original Kirkwood 88F6182 support which is
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Header file for Feroceon CPU core 88F5182 SOC.
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _CONFIG_88F5182_H
#define _CONFIG_88F5182_H

/* SOC specific definitions */
#define F88F5182_REGS_PHYS_BASE		0xf1000000
#define ORION5X_REGS_PHY_BASE		F88F5182_REGS_PHYS_BASE

/* TCLK Core Clock defination */
#define CONFIG_SYS_TCLK			166000000 /* 166MHz */

#endif /* _CONFIG_88F5182_H */
