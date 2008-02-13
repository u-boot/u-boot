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
}
