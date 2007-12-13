/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/* define DEBUG for debugging output (obviously ;-)) */
#if 0
#define DEBUG
#endif

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>
#include <ppc4xx.h>

#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <libfdt_env.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The aliases needed for this generic etherne MAC address
 * fixup function are not in place yet. So don't use this
 * approach for now. This will be enabled later.
 */
#undef USES_FDT_ALIASES

#ifndef USES_FDT_ALIASES
static void do_fixup_macaddr(void *fdt, int offset, const void *val, int i)
{
	int rc;

	debug("Updating node EMAC%d\n", i);

	rc = fdt_setprop(fdt, offset, "mac-address", val, 6);
	if (rc)
		printf("Unable to update property %s, err=%s\n",
		       "mac-address", fdt_strerror(rc));
	rc = fdt_setprop(fdt, offset, "local-mac-address", val, 6);
	if (rc)
		printf("Unable to update property %s, err=%s\n",
		       "local-mac-address", fdt_strerror(rc));
}
#endif /* USES_FDT_ALIASES */

void ft_cpu_setup(void *blob, bd_t *bd)
{
	char *cpu_path = "/cpus/" OF_CPU;
	sys_info_t sys_info;
	int offset;
	int i;

	get_sys_info(&sys_info);

	do_fixup_by_path_u32(blob, cpu_path, "timebase-frequency", bd->bi_intfreq, 1);
	do_fixup_by_path_u32(blob, cpu_path, "clock-frequency", bd->bi_intfreq, 1);
	do_fixup_by_path_u32(blob, "/plb", "clock-frequency", sys_info.freqPLB, 1);
	do_fixup_by_path_u32(blob, "/plb/opb", "clock-frequency", sys_info.freqOPB, 1);
	do_fixup_by_path_u32(blob, "/plb/opb/ebc", "clock-frequency",
			     sys_info.freqEBC, 1);
	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);

	/*
	 * Setup all baudrates for the UARTs
	 */
	do_fixup_by_compat_u32(blob, "ns16550", "clock-frequency", gd->uart_clk, 1);

#ifdef USES_FDT_ALIASES
	/*
	 * The aliases needed for this generic etherne MAC address
	 * fixup function are not in place yet. So don't use this
	 * approach for now. This will be enabled later.
	 */
	fdt_fixup_ethernet(blob, bd);
#else
	offset = -1;
	for (i = 0; i < 4; i++) {
		/*
		 * FIXME: This will cause problems with emac3 compatible
		 * devices, like on 405GP. But hopefully when we deal
		 * with those devices, the aliases stuff will be in
		 * place.
		 */
		offset = fdt_node_offset_by_compatible(blob, offset, "ibm,emac4");
		if (offset < 0)
			break;

		switch (i) {
		case 0:
			do_fixup_macaddr(blob, offset, bd->bi_enetaddr, 0);
			break;
#ifdef CONFIG_HAS_ETH1
		case 1:
			do_fixup_macaddr(blob, offset, bd->bi_enet1addr, 1);
			break;
#endif
#ifdef CONFIG_HAS_ETH2
		case 2:
			do_fixup_macaddr(blob, offset, bd->bi_enet2addr, 2);
			break;
#endif
#ifdef CONFIG_HAS_ETH3
		case 3:
			do_fixup_macaddr(blob, offset, bd->bi_enet3addr, 3);
			break;
#endif
		}
	}
#endif /* USES_FDT_ALIASES */
}
#endif /* CONFIG_OF_LIBFDT */
