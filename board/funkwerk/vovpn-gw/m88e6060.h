/*
 * (C) Copyright 2004
 * Elmeg Communications Systems GmbH, Juergen Selent (j.selent@elmeg.de)
 *
 * Support for the Elmeg VoVPN Gateway Module
 * ------------------------------------------
 * Initialize Marvell M88E6060 Switch
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

#ifndef _INC_m88e6060_h_
#define _INC_m88e6060_h_

/* ************************************************************************** */
/* *** DEFINES ************************************************************** */

/* switch hw */
#define M88X_PRT_CNT			6
#define M88X_PHY_CNT			5

/* phy register offsets */
#define M88X_PHY_CNTL			0x00
#define M88X_PHY_STAT			0x00
#define M88X_PHY_ID0			0x02
#define M88X_PHY_ID1			0x03
#define M88X_PHY_ANEG_ADV		0x04
#define M88X_PHY_LPA			0x05
#define M88X_PHY_ANEG_EXP		0x06
#define M88X_PHY_NPT			0x07
#define M88X_PHY_LPNP			0x08

/* port register offsets */
#define M88X_PRT_STAT			0x00
#define M88X_PRT_ID			0x03
#define M88X_PRT_CNTL			0x04
#define M88X_PRT_VLAN			0x06
#define M88X_PRT_PAV			0x0b
#define M88X_PRT_RX			0x10
#define M88X_PRT_TX			0x11

/* global/atu register offsets */
#define M88X_GLB_STAT			0x00
#define M88X_GLB_MAC01			0x01
#define M88X_GLB_MAC23			0x02
#define M88X_GLB_MAC45			0x03
#define M88X_GLB_CNTL			0x04
#define M88X_ATU_CNTL			0x0a
#define M88X_ATU_OP			0x0b

/* id0 register - 0x02 */
#define M88X_PHY_ID0_VALUE		0x0141

/* id1 register - 0x03 */
#define M88X_PHY_ID1_VALUE		0x0c80		/* without revision ! */


/* misc */
#define M88E6060_ID		((M88X_PHY_ID0_VALUE<<16) | M88X_PHY_ID1_VALUE)

/* ************************************************************************** */
/* *** TYPEDEFS ************************************************************* */

typedef struct {
	int		reg;
	unsigned short	msk;
	unsigned short	val;
} m88x_regCfg_t;

/* ************************************************************************** */
/* *** PROTOTYPES *********************************************************** */

extern int		m88e6060_initialize( int );

#endif	/* _INC_m88e6060_h_ */
