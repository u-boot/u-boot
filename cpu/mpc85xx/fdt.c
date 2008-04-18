/*
 * Copyright 2007 Freescale Semiconductor, Inc.
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

extern void ft_qe_setup(void *blob);
#ifdef CONFIG_MP
#include "mp.h"
DECLARE_GLOBAL_DATA_PTR;

void ft_fixup_cpu(void *blob, u64 memory_limit)
{
	int off;
	ulong spin_tbl_addr = get_spin_addr();
	u32 bootpg, id = get_my_id();

	/* if we have 4G or more of memory, put the boot page at 4Gb-4k */
	if ((u64)gd->ram_size > 0xfffff000)
		bootpg = 0xfffff000;
	else
		bootpg = gd->ram_size - 4096;

	off = fdt_node_offset_by_prop_value(blob, -1, "device_type", "cpu", 4);
	while (off != -FDT_ERR_NOTFOUND) {
		u32 *reg = (u32 *)fdt_getprop(blob, off, "reg", 0);

		if (reg) {
			if (*reg == id) {
				fdt_setprop_string(blob, off, "status", "okay");
			} else {
				u64 val = *reg * SIZE_BOOT_ENTRY + spin_tbl_addr;
				val = cpu_to_fdt32(val);
				fdt_setprop_string(blob, off, "status",
								"disabled");
				fdt_setprop_string(blob, off, "enable-method",
								"spin-table");
				fdt_setprop(blob, off, "cpu-release-addr",
						&val, sizeof(val));
			}
		} else {
			printf ("cpu NULL\n");
		}
		off = fdt_node_offset_by_prop_value(blob, off,
				"device_type", "cpu", 4);
	}

	/* Reserve the boot page so OSes dont use it */
	if ((u64)bootpg < memory_limit) {
		off = fdt_add_mem_rsv(blob, bootpg, (u64)4096);
		if (off < 0)
			printf("%s: %s\n", __FUNCTION__, fdt_strerror(off));
	}
}
#endif

void ft_cpu_setup(void *blob, bd_t *bd)
{
#if defined(CONFIG_HAS_ETH0) || defined(CONFIG_HAS_ETH1) ||\
    defined(CONFIG_HAS_ETH2) || defined(CONFIG_HAS_ETH3)
	fdt_fixup_ethernet(blob, bd);
#endif

	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
		"timebase-frequency", bd->bi_busfreq / 8, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
		"bus-frequency", bd->bi_busfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
		"clock-frequency", bd->bi_intfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "soc", 4,
		"bus-frequency", bd->bi_busfreq, 1);
#ifdef CONFIG_QE
	ft_qe_setup(blob);
#endif

#ifdef CFG_NS16550
	do_fixup_by_compat_u32(blob, "ns16550",
		"clock-frequency", bd->bi_busfreq, 1);
#endif

#ifdef CONFIG_CPM2
	do_fixup_by_compat_u32(blob, "fsl,cpm2-scc-uart",
		"current-speed", bd->bi_baudrate, 1);

	do_fixup_by_compat_u32(blob, "fsl,cpm2-brg",
		"clock-frequency", bd->bi_brgfreq, 1);
#endif

	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);

#ifdef CONFIG_MP
	ft_fixup_cpu(blob, (u64)bd->bi_memstart + (u64)bd->bi_memsize);
#endif
}
