/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Based on:
 *
 * ----------------------------------------------------------------------------
 *
 * dm644x_emac.h
 *
 * TI DaVinci (DM644X) EMAC peripheral driver header for DV-EVM
 *
 * Copyright (C) 2005 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------

 * Modifications:
 * ver. 1.0: Sep 2005, TI PSP Team - Created EMAC version for uBoot.
 *
 */

#ifndef _AM3517_EMAC_H_
#define _AM3517_EMAC_H_

#define EMAC_BASE_ADDR                 0x5C010000
#define EMAC_WRAPPER_BASE_ADDR         0x5C000000
#define EMAC_WRAPPER_RAM_ADDR          0x5C020000
#define EMAC_MDIO_BASE_ADDR            0x5C030000
#define EMAC_HW_RAM_ADDR               0x01E20000

#define EMAC_MDIO_BUS_FREQ             166000000       /* 166 MHZ check */
#define EMAC_MDIO_CLOCK_FREQ           1000000         /* 2.0 MHz */

/* SOFTRESET macro definition interferes with emac_regs structure definition */
#undef SOFTRESET

typedef volatile unsigned int	dv_reg;
typedef volatile unsigned int	*dv_reg_p;

#define DAVINCI_EMAC_VERSION2

#endif  /* _AM3517_EMAC_H_ */
