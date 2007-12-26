/*
 * Copyright 2004 Freescale Semiconductor.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>
#include "cadmus.h"

#if defined(CONFIG_OF_BOARD_SETUP)
static void cds_pci_fixup(void *blob)
{
	int node, tmp[2];
	const char *path;
	int len, slot, i;
	u32 *map = NULL;

	node = fdt_path_offset(blob, "/aliases");
	tmp[0] = 0;
	if (node >= 0) {
		path = fdt_getprop(blob, node, "pci0", NULL);
		if (path) {
			node = fdt_path_offset(blob, path);
			if (node >= 0) {
				map = fdt_getprop_w(blob, node, "interrupt-map", &len);
			}
		}
	}

	if (map) {
		len /= sizeof(u32);

		slot = get_pci_slot();

		for (i=0;i<len;i+=7) {
			/* We rotate the interrupt pins so that the mapping
			 * changes depending on the slot the carrier card is in.
			 */
			map[3] = ((map[3] + slot - 2) % 4) + 1;
			map+=7;
		}
	}
}

void
ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
	cds_pci_fixup(blob);
#endif
}
#endif
