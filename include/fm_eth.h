/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
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

#ifndef __FM_ETH_H__
#define __FM_ETH_H__

#include <common.h>
#include <asm/types.h>
#include <asm/fsl_enet.h>

enum fm_port {
	FM1_DTSEC1,
	FM1_DTSEC2,
	FM1_DTSEC3,
	FM1_DTSEC4,
	FM1_DTSEC5,
	FM1_10GEC1,
	FM2_DTSEC1,
	FM2_DTSEC2,
	FM2_DTSEC3,
	FM2_DTSEC4,
	FM2_DTSEC5,
	FM2_10GEC1,
	NUM_FM_PORTS,
};

enum fm_eth_type {
	FM_ETH_1G_E,
	FM_ETH_10G_E,
};

#define CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR	(CONFIG_SYS_FSL_FM1_ADDR + 0xe1120)
#define CONFIG_SYS_FM1_TGEC_MDIO_ADDR	(CONFIG_SYS_FSL_FM1_ADDR + 0xf1000)

#define DEFAULT_FM_MDIO_NAME "FSL_MDIO0"
#define DEFAULT_FM_TGEC_MDIO_NAME "FM_TGEC_MDIO"

/* Fman ethernet info struct */
#define FM_ETH_INFO_INITIALIZER(idx, pregs) \
	.fm		= idx,						\
	.phy_regs	= (void *)pregs,				\
	.enet_if	= PHY_INTERFACE_MODE_NONE,			\

#define FM_DTSEC_INFO_INITIALIZER(idx, n) \
{									\
	FM_ETH_INFO_INITIALIZER(idx, CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR)	\
	.index		= idx,						\
	.num		= n - 1,					\
	.type		= FM_ETH_1G_E,					\
	.port		= FM##idx##_DTSEC##n,				\
	.rx_port_id	= RX_PORT_1G_BASE + n - 1,			\
	.tx_port_id	= TX_PORT_1G_BASE + n - 1,			\
	.compat_offset	= CONFIG_SYS_FSL_FM##idx##_OFFSET +		\
				offsetof(struct ccsr_fman, mac_1g[n-1]),\
}

#define FM_TGEC_INFO_INITIALIZER(idx, n) \
{									\
	FM_ETH_INFO_INITIALIZER(idx, CONFIG_SYS_FM1_TGEC_MDIO_ADDR)	\
	.index		= idx,						\
	.num		= n - 1,					\
	.type		= FM_ETH_10G_E,					\
	.port		= FM##idx##_10GEC##n,				\
	.rx_port_id	= RX_PORT_10G_BASE + n - 1,			\
	.tx_port_id	= TX_PORT_10G_BASE + n - 1,			\
	.compat_offset	= CONFIG_SYS_FSL_FM##idx##_OFFSET +		\
				offsetof(struct ccsr_fman, mac_10g[n-1]),\
}

struct fm_eth_info {
	u8 enabled;
	u8 fm;
	u8 num;
	u8 phy_addr;
	int index;
	u16 rx_port_id;
	u16 tx_port_id;
	enum fm_port port;
	enum fm_eth_type type;
	void *phy_regs;
	phy_interface_t enet_if;
	u32 compat_offset;
	struct mii_dev *bus;
};

struct tgec_mdio_info {
	struct tgec_mdio_controller *regs;
	char *name;
};

int fm_tgec_mdio_init(bd_t *bis, struct tgec_mdio_info *info);
int fm_standard_init(bd_t *bis);
void fman_enet_init(void);
void fdt_fixup_fman_ethernet(void *fdt);
phy_interface_t fm_info_get_enet_if(enum fm_port port);
void fm_info_set_phy_address(enum fm_port port, int address);
int fm_info_get_phy_address(enum fm_port port);
void fm_info_set_mdio(enum fm_port port, struct mii_dev *bus);
void fm_disable_port(enum fm_port port);

#endif
