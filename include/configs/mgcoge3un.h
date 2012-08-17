/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2010-2011
 * Holger Brunck, Keymile GmbH Hannover, holger.brunck@keymile.com
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

#ifndef _CONFIG_MGCOGE3UN_H
#define _CONFIG_MGCOGE3UN_H

/* include common defines/options for all arm based Keymile boards */
#include "km/km_arm.h"

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nKeymile MGCOGE3UN"
#define CONFIG_HOSTNAME		mgcoge3un
#define CONFIG_MGCOGE3UN

#define KM_IVM_BUS	"pca9547:70:9" /* I2C2 (Mux-Port 1)*/
#define KM_ENV_BUS	"pca9547:70:d" /* I2C2 (Mux-Port 5)*/

/* we use a new RAM type on mgcoge3un board */
#undef CONFIG_SYS_KWD_CONFIG
#define CONFIG_SYS_KWD_CONFIG $(SRCTREE)/$(CONFIG_BOARDDIR)/kwbimage-memphis.cfg

/*
 * mgcoge3un has a fixed link to the marvell switch
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

#define CONFIG_KM_BOARD_EXTRA_ENV	"waitforne=true\0"

/*
 * PCIe port not used on mgcoge3un
 */
#undef  CONFIG_KIRKWOOD_PCIE_INIT

#endif /* _CONFIG_MGCOGE3UN_H */
