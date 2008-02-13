/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>
#include "qe.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * If a QE firmware has been uploaded, then add the 'firmware' node under
 * the 'qe' node.
 */
void fdt_fixup_qe_firmware(void *blob)
{
	struct qe_firmware_info *qe_fw_info;
	int node, ret;

	qe_fw_info = qe_get_firmware_info();
	if (!qe_fw_info)
		return;

	node = fdt_path_offset(blob, "/qe");
	if (node < 0)
		return;

	/* We assume the node doesn't exist yet */
	node = fdt_add_subnode(blob, node, "firmware");
	if (node < 0)
		return;

	ret = fdt_setprop(blob, node, "extended-modes",
		&qe_fw_info->extended_modes, sizeof(u64));
	if (ret < 0)
		goto error;

	ret = fdt_setprop_string(blob, node, "id", qe_fw_info->id);
	if (ret < 0)
		goto error;

	ret = fdt_setprop(blob, node, "virtual-traps", qe_fw_info->vtraps,
		sizeof(qe_fw_info->vtraps));
	if (ret < 0)
		goto error;

	return;

error:
	fdt_del_node(blob, node);
}

void ft_qe_setup(void *blob)
{
#ifdef CONFIG_QE
	do_fixup_by_prop_u32(blob, "device_type", "qe", 4,
		"bus-frequency", gd->qe_clk, 1);
	do_fixup_by_prop_u32(blob, "device_type", "qe", 4,
		"brg-frequency", gd->brg_clk, 1);
	do_fixup_by_compat_u32(blob, "fsl,qe",
		"clock-frequency", gd->qe_clk, 1);
	do_fixup_by_compat_u32(blob, "fsl,qe",
		"bus-frequency", gd->qe_clk, 1);
	do_fixup_by_compat_u32(blob, "fsl,qe",
		"brg-frequency", gd->brg_clk, 1);
	fdt_fixup_qe_firmware(blob);
#endif
}
