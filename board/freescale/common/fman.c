/*
 * Copyright 2011 Freescale Semiconductor, Inc.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <libfdt.h>
#include <libfdt_env.h>
#include <fdt_support.h>

#include <fm_eth.h>
#include <asm/fsl_serdes.h>

/*
 * Given the following ...
 *
 * 1) A pointer to an Fman Ethernet node (as identified by the 'compat'
 * compatible string and 'addr' physical address)
 *
 * 2) The name of an alias that points to the ethernet-phy node (usually inside
 * a virtual MDIO node)
 *
 * ... update that Ethernet node's phy-handle property to point to the
 * ethernet-phy node.  This is how we link an Ethernet node to its PHY, so each
 * PHY in a virtual MDIO node must have an alias.
 *
 * Returns 0 on success, or a negative FDT error code on error.
 */
int fdt_set_phy_handle(void *fdt, char *compat, phys_addr_t addr,
			const char *alias)
{
	int offset;
	unsigned int ph;
	const char *path;

	/* Get a path to the node that 'alias' points to */
	path = fdt_get_alias(fdt, alias);
	if (!path)
		return -FDT_ERR_BADPATH;

	/* Get the offset of that node */
	offset = fdt_path_offset(fdt, path);
	if (offset < 0)
		return offset;

	ph = fdt_create_phandle(fdt, offset);
	if (!ph)
		return -FDT_ERR_BADPHANDLE;

	offset = fdt_node_offset_by_compat_reg(fdt, compat, addr);
	if (offset < 0)
		return offset;

	return fdt_setprop(fdt, offset, "phy-handle", &ph, sizeof(ph));
}

/*
 * Return the SerDes device enum for a given Fman port
 *
 * This function just maps the fm_port namespace to the srds_prtcl namespace.
 */
enum srds_prtcl serdes_device_from_fm_port(enum fm_port port)
{
	static const enum srds_prtcl srds_table[] = {
		[FM1_DTSEC1] = SGMII_FM1_DTSEC1,
		[FM1_DTSEC2] = SGMII_FM1_DTSEC2,
		[FM1_DTSEC3] = SGMII_FM1_DTSEC3,
		[FM1_DTSEC4] = SGMII_FM1_DTSEC4,
		[FM1_DTSEC5] = SGMII_FM1_DTSEC5,
		[FM1_10GEC1] = XAUI_FM1,
		[FM2_DTSEC1] = SGMII_FM2_DTSEC1,
		[FM2_DTSEC2] = SGMII_FM2_DTSEC2,
		[FM2_DTSEC3] = SGMII_FM2_DTSEC3,
		[FM2_DTSEC4] = SGMII_FM2_DTSEC4,
		[FM2_DTSEC5] = SGMII_FM2_DTSEC5,
		[FM2_10GEC1] = XAUI_FM2,
	};

	if ((port < FM1_DTSEC1) || (port > FM2_10GEC1))
		return NONE;
	else
		return srds_table[port];
}
