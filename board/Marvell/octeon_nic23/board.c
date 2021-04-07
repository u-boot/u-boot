// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Stefan Roese <sr@denx.de>
 */

#include <dm.h>
#include <ram.h>

#include <mach/octeon_ddr.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/octeon_fdt.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-bgxx-defs.h>

#include "board_ddr.h"

#define NIC23_DEF_DRAM_FREQ		800

static u8 octeon_nic23_cfg0_spd_values[512] = {
	OCTEON_NIC23_CFG0_SPD_VALUES
};

static struct ddr_conf board_ddr_conf[] = {
	 OCTEON_NIC23_DDR_CONFIGURATION
};

struct ddr_conf *octeon_ddr_conf_table_get(int *count, int *def_ddr_freq)
{
	*count = ARRAY_SIZE(board_ddr_conf);
	*def_ddr_freq = NIC23_DEF_DRAM_FREQ;

	return board_ddr_conf;
}

int board_fix_fdt(void *fdt)
{
	u32 range_data[5 * 8];
	bool rev4;
	int node;
	int rc;

	/*
	 * ToDo:
	 * Read rev4 info from EEPROM or where the original U-Boot does
	 * and don't hard-code it here.
	 */
	rev4 = true;

	debug("%s() rev4: %s\n", __func__, rev4 ? "true" : "false");
	/* Patch the PHY configuration based on board revision */
	rc = octeon_fdt_patch_rename(fdt,
				     rev4 ? "4,nor-flash" : "4,no-nor-flash",
				     "cavium,board-trim", false, NULL, NULL);
	if (!rev4) {
		/* Modify the ranges for CS 0 */
		node = fdt_node_offset_by_compatible(fdt, -1,
						     "cavium,octeon-3860-bootbus");
		if (node < 0) {
			printf("%s: Error: cannot find boot bus in device tree!\n",
			       __func__);
			return -1;
		}

		rc = fdtdec_get_int_array(fdt, node, "ranges",
					  range_data, 5 * 8);
		if (rc) {
			printf("%s: Error reading ranges from boot bus FDT\n",
			       __func__);
			return -1;
		}
		range_data[2] = cpu_to_fdt32(0x10000);
		range_data[3] = 0;
		range_data[4] = 0;
		rc = fdt_setprop(fdt, node, "ranges", range_data,
				 sizeof(range_data));
		if (rc) {
			printf("%s: Error updating boot bus ranges in fdt\n",
			       __func__);
		}
	}
	return rc;
}

void board_configure_qlms(void)
{
	octeon_configure_qlm(4, 3000, CVMX_QLM_MODE_SATA_2X1, 0, 0, 0, 0);
	octeon_configure_qlm(5, 103125, CVMX_QLM_MODE_XFI_1X2, 0, 0, 2, 0);
	/* Apply amplitude tuning to 10G interface */
	octeon_qlm_tune_v3(0, 4, 3000, -1, -1, 7, -1);
	octeon_qlm_tune_v3(0, 5, 103125, 0x19, 0x0, -1, -1);
	octeon_qlm_set_channel_v3(0, 5, 0);
	octeon_qlm_dfe_disable(0, 5, -1, 103125, CVMX_QLM_MODE_XFI_1X2);
	debug("QLM 4 reference clock: %d\n"
	      "DLM 5 reference clock: %d\n",
	      cvmx_qlm_measure_clock(4), cvmx_qlm_measure_clock(5));
}

int board_late_init(void)
{
	board_configure_qlms();

	return 0;
}
