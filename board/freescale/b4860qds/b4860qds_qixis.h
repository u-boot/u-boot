/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __B4860QDS_QIXIS_H__
#define __B4860QDS_QIXIS_H__

/* Definitions of QIXIS Registers for B4860QDS */

/* BRDCFG4[4:7]] select EC1 and EC2 as a pair */
#define BRDCFG4_EMISEL_MASK		0xE0
#define BRDCFG4_EMISEL_SHIFT		5

/* CLK */
#define QIXIS_CLK_66		0x0
#define QIXIS_CLK_100		0x1
#define QIXIS_CLK_125		0x2
#define QIXIS_CLK_133		0x3

#define QIXIS_SRDS1CLK_122		0x5a
#define QIXIS_SRDS1CLK_125		0x5e
#endif
