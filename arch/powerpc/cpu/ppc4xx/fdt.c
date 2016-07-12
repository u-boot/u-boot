/*
 * (C) Copyright 2007-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>
#include <asm/ppc4xx.h>

#ifdef CONFIG_OF_BOARD_SETUP
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/4xx_pcie.h>

DECLARE_GLOBAL_DATA_PTR;

int __ft_board_setup(void *blob, bd_t *bd)
{
	int rc;
	int i;
	u32 bxcr;
	u32 ranges[EBC_NUM_BANKS * 4];
	u32 *p = ranges;
	char ebc_path[] = "/plb/opb/ebc";

	ft_cpu_setup(blob, bd);

	/*
	 * Read 4xx EBC bus bridge registers to get mappings of the
	 * peripheral banks into the OPB/PLB address space
	 */
	for (i = 0; i < EBC_NUM_BANKS; i++) {
		mtdcr(EBC0_CFGADDR, EBC_BXCR(i));
		bxcr = mfdcr(EBC0_CFGDATA);

		if ((bxcr & EBC_BXCR_BU_MASK) != EBC_BXCR_BU_NONE) {
			*p++ = i;
			*p++ = 0;
			*p++ = bxcr & EBC_BXCR_BAS_MASK;
			*p++ = EBC_BXCR_BANK_SIZE(bxcr);
		}
	}


#ifdef CONFIG_FDT_FIXUP_NOR_FLASH_SIZE
	/* Update reg property in all nor flash nodes too */
	fdt_fixup_nor_flash_size(blob);
#endif

	/* Some 405 PPC's have EBC as direct PLB child in the dts */
	if (fdt_path_offset(blob, ebc_path) < 0)
		strcpy(ebc_path, "/plb/ebc");
	rc = fdt_find_and_setprop(blob, ebc_path, "ranges", ranges,
				  (p - ranges) * sizeof(u32), 1);
	if (rc) {
		printf("Unable to update property EBC mappings, err=%s\n",
		       fdt_strerror(rc));
	}

	return 0;
}
int ft_board_setup(void *blob, bd_t *bd)
		__attribute__((weak, alias("__ft_board_setup")));

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
	int off, ndepth = 0;

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
	 * Fixup all UART clocks for CPU internal UARTs
	 * (only these UARTs are definitely clocked by gd->arch.uart_clk)
	 *
	 * These UARTs are direct childs of /plb/opb. This code
	 * does not touch any UARTs that are connected to the ebc.
	 */
	off = fdt_path_offset(blob, "/plb/opb");
	while ((off = fdt_next_node(blob, off, &ndepth)) >= 0) {
		/*
		 * process all sub nodes and stop when we are back
		 * at the starting depth
		 */
		if (ndepth <= 0)
			break;

		/* only update direct childs */
		if ((ndepth == 1) &&
		    (fdt_node_check_compatible(blob, off, "ns16550") == 0))
			fdt_setprop(blob, off,
				    "clock-frequency",
				    (void *)&gd->arch.uart_clk, 4);
	}

	/*
	 * Fixup all ethernet nodes
	 * Note: aliases in the dts are required for this
	 */
	fdt_fixup_ethernet(blob);

	/*
	 * Fixup all available PCIe nodes by setting the device_type property
	 */
	fdt_pcie_setup(blob);
}
#endif /* CONFIG_OF_BOARD_SETUP */
