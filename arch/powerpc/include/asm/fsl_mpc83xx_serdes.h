/*
 * Copyright 2010 Freescale Semiconductor, Inc.
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

#ifndef __FSL_MPC83XX_SERDES_H
#define __FSL_MPC83XX_SERDES_H

#include <config.h>

#define FSL_SERDES_CLK_100		(0 << 28)
#define FSL_SERDES_CLK_125		(1 << 28)
#define FSL_SERDES_CLK_150		(3 << 28)
#define FSL_SERDES_PROTO_SATA		0
#define FSL_SERDES_PROTO_PEX		1
#define FSL_SERDES_PROTO_PEX_X2		2
#define FSL_SERDES_PROTO_SGMII		3
#define FSL_SERDES_VDD_1V		1

extern void fsl_setup_serdes(u32 offset, char proto, u32 rfcks, char vdd);

#endif /* __FSL_MPC83XX_SERDES_H */
