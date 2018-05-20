// SPDX-License-Identifier: GPL-2.0+
/*
 * Altera SoCFPGA common board code
 *
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/misc.h>
#include <asm/io.h>

#include <usb.h>
#include <usb/dwc2_udc.h>

DECLARE_GLOBAL_DATA_PTR;

void s_init(void) {}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	/* Address of boot parameters for ATAG (if ATAG is used) */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#if defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
	/* configuring the clock based on handoff */
	cm_basic_init(gd->fdt_blob);

	/* Add device descriptor to FPGA device table */
	socfpga_fpga_add();
#endif

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

#ifdef CONFIG_USB_GADGET
struct dwc2_plat_otg_data socfpga_otg_data = {
	.usb_gusbcfg	= 0x1417,
};

int board_usb_init(int index, enum usb_init_type init)
{
	int node[2], count;
	fdt_addr_t addr;

	count = fdtdec_find_aliases_for_id(gd->fdt_blob, "udc",
					   COMPAT_ALTERA_SOCFPGA_DWC2USB,
					   node, 2);
	if (count <= 0)	/* No controller found. */
		return 0;

	addr = fdtdec_get_addr(gd->fdt_blob, node[0], "reg");
	if (addr == FDT_ADDR_T_NONE) {
		printf("UDC Controller has no 'reg' property!\n");
		return -EINVAL;
	}

	/* Patch the address from OF into the controller pdata. */
	socfpga_otg_data.regs_otg = addr;

	return dwc2_udc_probe(&socfpga_otg_data);
}

int g_dnl_board_usb_cable_connected(void)
{
	return 1;
}
#endif
