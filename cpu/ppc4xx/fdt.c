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

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>
#include <ppc4xx.h>

#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <libfdt_env.h>
#include <fdt_support.h>
#include <asm/4xx_pcie.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Fixup all PCIe nodes by setting the device_type property
 * to "pci-endpoint" instead is "pci" for endpoint ports.
 * This property will get checked later by the Linux driver
 * to properly configure the PCIe port in Linux (again).
 */
void fdt_pcie_setup(void *blob)
{
	const char *compat = "ibm,plb-pciex";
	const char *prop = "device_type";
	const char *prop_val = "pci-endpoint";
	const u32 *port;
	int no;
	int rc;

	/* Search first PCIe node */
	no = fdt_node_offset_by_compatible(blob, -1, compat);
	while (no != -FDT_ERR_NOTFOUND) {
		port = fdt_getprop(blob, no, "port", NULL);
		if (port == NULL) {
			printf("WARNING: could not find port property\n");
		} else {
			if (is_end_point(*port)) {
				rc = fdt_setprop(blob, no, prop, prop_val,
						 strlen(prop_val) + 1);
				if (rc < 0)
					printf("WARNING: could not set %s for %s: %s.\n",
					       prop, compat, fdt_strerror(rc));
			}
		}

		/* Jump to next PCIe node */
		no = fdt_node_offset_by_compatible(blob, no, compat);
	}
}

void ft_cpu_setup(void *blob, bd_t *bd)
{
	sys_info_t sys_info;

	get_sys_info(&sys_info);

	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4, "timebase-frequency",
			     bd->bi_intfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4, "clock-frequency",
			     bd->bi_intfreq, 1);
	do_fixup_by_path_u32(blob, "/plb", "clock-frequency", sys_info.freqPLB, 1);
	do_fixup_by_path_u32(blob, "/plb/opb", "clock-frequency", sys_info.freqOPB, 1);

	if (fdt_path_offset(blob, "/plb/opb/ebc") >= 0)
		do_fixup_by_path_u32(blob, "/plb/opb/ebc", "clock-frequency",
			sys_info.freqEBC, 1);
	else
		do_fixup_by_path_u32(blob, "/plb/ebc", "clock-frequency",
			sys_info.freqEBC, 1);

	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);

	/*
	 * Setup all baudrates for the UARTs
	 */
	do_fixup_by_compat_u32(blob, "ns16550", "clock-frequency", gd->uart_clk, 1);

	/*
	 * Fixup all ethernet nodes
	 * Note: aliases in the dts are required for this
	 */
	fdt_fixup_ethernet(blob, bd);

	/*
	 * Fixup all available PCIe nodes by setting the device_type property
	 */
	fdt_pcie_setup(blob);
}
#endif /* CONFIG_OF_LIBFDT */
