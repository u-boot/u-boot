/*
 * Copyright 2011 Freescale Semiconductor, Inc.
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
#include <common.h>
#include <asm/io.h>
#include <asm/fsl_serdes.h>

#include "fm.h"

struct fm_eth_info fm_info[] = {
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 1)
	FM_DTSEC_INFO_INITIALIZER(1, 1),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 2)
	FM_DTSEC_INFO_INITIALIZER(1, 2),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 3)
	FM_DTSEC_INFO_INITIALIZER(1, 3),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 4)
	FM_DTSEC_INFO_INITIALIZER(1, 4),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 5)
	FM_DTSEC_INFO_INITIALIZER(1, 5),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 1)
	FM_DTSEC_INFO_INITIALIZER(2, 1),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 2)
	FM_DTSEC_INFO_INITIALIZER(2, 2),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 3)
	FM_DTSEC_INFO_INITIALIZER(2, 3),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 4)
	FM_DTSEC_INFO_INITIALIZER(2, 4),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 5)
	FM_DTSEC_INFO_INITIALIZER(2, 5),
#endif
#if (CONFIG_SYS_NUM_FM1_10GEC >= 1)
	FM_TGEC_INFO_INITIALIZER(1, 1),
#endif
#if (CONFIG_SYS_NUM_FM2_10GEC >= 1)
	FM_TGEC_INFO_INITIALIZER(2, 1),
#endif
};

int fm_standard_init(bd_t *bis)
{
	int i;
	struct ccsr_fman *reg;

	reg = (void *)CONFIG_SYS_FSL_FM1_ADDR;
	if (fm_init_common(0, reg))
		return 0;

	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		if ((fm_info[i].enabled) && (fm_info[i].index == 1))
			fm_eth_initialize(reg, &fm_info[i]);
	}

#if (CONFIG_SYS_NUM_FMAN == 2)
	reg = (void *)CONFIG_SYS_FSL_FM2_ADDR;
	if (fm_init_common(1, reg))
		return 0;

	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		if ((fm_info[i].enabled) && (fm_info[i].index == 2))
			fm_eth_initialize(reg, &fm_info[i]);
	}
#endif

	return 1;
}

/* simple linear search to map from port to array index */
static int fm_port_to_index(enum fm_port port)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		if (fm_info[i].port == port)
			return i;
	}

	return -1;
}

/*
 * Determine if an interface is actually active based on HW config
 * we expect fman_port_enet_if() to report PHY_INTERFACE_MODE_NONE if
 * the interface is not active based on HW cfg of the SoC
 */
void fman_enet_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		phy_interface_t enet_if;

		enet_if = fman_port_enet_if(fm_info[i].port);
		if (enet_if != PHY_INTERFACE_MODE_NONE) {
			fm_info[i].enabled = 1;
			fm_info[i].enet_if = enet_if;
		} else {
			fm_info[i].enabled = 0;
		}
	}

	return ;
}

void fm_disable_port(enum fm_port port)
{
	int i = fm_port_to_index(port);

	fm_info[i].enabled = 0;
	fman_disable_port(port);
}

void fm_info_set_mdio(enum fm_port port, struct mii_dev *bus)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return;

	fm_info[i].bus = bus;
}

void fm_info_set_phy_address(enum fm_port port, int address)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return;

	fm_info[i].phy_addr = address;
}

/*
 * Returns the PHY address for a given Fman port
 *
 * The port must be set via a prior call to fm_info_set_phy_address().
 * A negative error code is returned if the port is invalid.
 */
int fm_info_get_phy_address(enum fm_port port)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return -1;

	return fm_info[i].phy_addr;
}

/*
 * Returns the type of the data interface between the given MAC and its PHY.
 * This is typically determined by the RCW.
 */
phy_interface_t fm_info_get_enet_if(enum fm_port port)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return PHY_INTERFACE_MODE_NONE;

	if (fm_info[i].enabled)
		return fm_info[i].enet_if;

	return PHY_INTERFACE_MODE_NONE;
}

static void
__def_board_ft_fman_fixup_port(void *blob, char * prop, phys_addr_t pa,
				enum fm_port port, int offset)
{
	return ;
}

void board_ft_fman_fixup_port(void *blob, char * prop, phys_addr_t pa,
				enum fm_port port, int offset)
	 __attribute__((weak, alias("__def_board_ft_fman_fixup_port")));

static void ft_fixup_port(void *blob, struct fm_eth_info *info, char *prop)
{
	int off;
	uint32_t ph;
	phys_addr_t paddr = CONFIG_SYS_CCSRBAR_PHYS + info->compat_offset;
	u64 dtsec1_addr = (u64)CONFIG_SYS_CCSRBAR_PHYS +
				CONFIG_SYS_FSL_FM1_DTSEC1_OFFSET;

	off = fdt_node_offset_by_compat_reg(blob, prop, paddr);

	if (info->enabled) {
		fdt_fixup_phy_connection(blob, off, info->enet_if);
		board_ft_fman_fixup_port(blob, prop, paddr, info->port, off);
		return ;
	}

	/* board code might have caused offset to change */
	off = fdt_node_offset_by_compat_reg(blob, prop, paddr);

	/* Don't disable FM1-DTSEC1 MAC as its used for MDIO */
	if (paddr != dtsec1_addr)
		fdt_status_disabled(blob, off); /* disable the MAC node */

	/* disable the fsl,dpa-ethernet node that points to the MAC */
	ph = fdt_get_phandle(blob, off);
	do_fixup_by_prop(blob, "fsl,fman-mac", &ph, sizeof(ph),
		"status", "disabled", strlen("disabled") + 1, 1);
}

void fdt_fixup_fman_ethernet(void *blob)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		if (fm_info[i].type == FM_ETH_1G_E)
			ft_fixup_port(blob, &fm_info[i], "fsl,fman-1g-mac");
		else
			ft_fixup_port(blob, &fm_info[i], "fsl,fman-10g-mac");
	}
}
