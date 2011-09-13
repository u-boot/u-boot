/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2010-2011
 * Holger Brunck, Keymile GmbH Hannover, holger.brunck@keymile.com.
 * Valentin Longchamp, Keymile AG Bern, valentin.longchamp@keymile.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

/* for linking errors see
 * http://lists.denx.de/pipermail/u-boot/2009-July/057350.html */

#ifndef _CONFIG_PORTL2_H
#define _CONFIG_PORTL2_H

/* include common defines/options for all arm based Keymile boards */
#include "km/km_arm.h"

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nKeymile Port-L2"
#define CONFIG_HOSTNAME			portl2
#define CONFIG_PORTL2

#define KM_IVM_BUS	"pca9544a:70:9" /* I2C2 (Mux-Port 1)*/
/*
 * Note: This is only valid for HW > P1A if you got an outdated P1A
 *       use KM_ENV_BUS  "pca9544a:70:a"
 */
#define KM_ENV_BUS	"pca9544a:70:d"	/* I2C2 (Mux-Port 5)*/

/*
 * portl2 has a fixed link to the XMPP backplane
 * with 100MB full duplex and autoneg off, for this
 * reason we have to change the default settings
 */
#define PORT_SERIAL_CONTROL_VALUE		( \
	MVGBE_FORCE_LINK_PASS			| \
	MVGBE_DIS_AUTO_NEG_FOR_DUPLX		| \
	MVGBE_DIS_AUTO_NEG_FOR_FLOW_CTRL	| \
	MVGBE_ADV_NO_FLOW_CTRL			| \
	MVGBE_FORCE_FC_MODE_NO_PAUSE_DIS_TX	| \
	MVGBE_FORCE_BP_MODE_NO_JAM		| \
	(1 << 9) /* Reserved bit has to be 1 */	| \
	MVGBE_DO_NOT_FORCE_LINK_FAIL		| \
	MVGBE_DIS_AUTO_NEG_SPEED_GMII		| \
	MVGBE_DTE_ADV_0				| \
	MVGBE_MIIPHY_MAC_MODE			| \
	MVGBE_AUTO_NEG_NO_CHANGE		| \
	MVGBE_MAX_RX_PACKET_1552BYTE		| \
	MVGBE_CLR_EXT_LOOPBACK			| \
	MVGBE_SET_FULL_DUPLEX_MODE		| \
	MVGBE_DIS_FLOW_CTRL_TX_RX_IN_FULL_DUPLEX	|\
	MVGBE_SET_GMII_SPEED_TO_10_100	|\
	MVGBE_SET_MII_SPEED_TO_100)

/*
 * portl2 does use the PCIe Port0
 */
#define  CONFIG_KIRKWOOD_PCIE_INIT

#endif /* _CONFIG_PORTL2_H */
