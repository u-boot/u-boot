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

#if defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
#include "cadmus.h"

extern void ft_cpu_setup(void *blob, bd_t *bd);

static void cds_pci_fixup(void *blob)
{
	int len;
	u32 *map;
	int slot;
	int i;

	map = ft_get_prop(blob, "/" OF_SOC "/pci@8000/interrupt-map", &len);

	if (!map)
		map = ft_get_prop(blob, "/" OF_PCI "/interrupt-map", &len);

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
	} else {
		printf("*** Warning - No PCI node found\n");
	}
}
#endif

#if defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	u32 *p;
	int len;

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
	ft_cpu_setup(blob, bd);

	p = ft_get_prop(blob, "/memory/reg", &len);
	if (p != NULL) {
		*p++ = cpu_to_be32(bd->bi_memstart);
		*p = cpu_to_be32(bd->bi_memsize);
	}

	cds_pci_fixup(blob);
}
#endif
