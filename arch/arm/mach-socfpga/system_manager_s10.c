// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/system_manager.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Configure all the pin muxes
 */
void sysmgr_pinmux_init(void)
{
	populate_sysmgr_pinmux();
	populate_sysmgr_fpgaintf_module();
}

/*
 * Populate the value for SYSMGR.FPGAINTF.MODULE based on pinmux setting.
 * The value is not wrote to SYSMGR.FPGAINTF.MODULE but
 * CONFIG_SYSMGR_ISWGRP_HANDOFF.
 */
void populate_sysmgr_fpgaintf_module(void)
{
	u32 handoff_val = 0;

	/* Enable the signal for those HPS peripherals that use FPGA. */
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_NAND_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_NAND;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_SDMMC_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_SDMMC;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_SPIM0_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_SPIM0;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_SPIM1_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_SPIM1;
	writel(handoff_val,
	       socfpga_get_sysmgr_addr() + SYSMGR_SOC64_FPGAINTF_EN2);

	handoff_val = 0;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_EMAC0_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_EMAC0;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_EMAC1_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_EMAC1;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_EMAC2_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_EMAC2;
	writel(handoff_val,
	       socfpga_get_sysmgr_addr() + SYSMGR_SOC64_FPGAINTF_EN3);
}

/*
 * Configure all the pin muxes
 */
void populate_sysmgr_pinmux(void)
{
	const u32 *sys_mgr_table_u32;
	unsigned int len, i;

	/* setup the pin sel */
	sysmgr_pinmux_table_sel(&sys_mgr_table_u32, &len);
	for (i = 0; i < len; i = i + 2) {
		writel(sys_mgr_table_u32[i + 1],
		       sys_mgr_table_u32[i] +
		       (u8 *)socfpga_get_sysmgr_addr() + SYSMGR_SOC64_PINSEL0);
	}

	/* setup the pin ctrl */
	sysmgr_pinmux_table_ctrl(&sys_mgr_table_u32, &len);
	for (i = 0; i < len; i = i + 2) {
		writel(sys_mgr_table_u32[i + 1],
		       sys_mgr_table_u32[i] +
		       (u8 *)socfpga_get_sysmgr_addr() + SYSMGR_SOC64_IOCTRL0);
	}

	/* setup the fpga use */
	sysmgr_pinmux_table_fpga(&sys_mgr_table_u32, &len);
	for (i = 0; i < len; i = i + 2) {
		writel(sys_mgr_table_u32[i + 1],
		       sys_mgr_table_u32[i] +
		       (u8 *)socfpga_get_sysmgr_addr() +
		       SYSMGR_SOC64_EMAC0_USEFPGA);
	}

	/* setup the IO delay */
	sysmgr_pinmux_table_delay(&sys_mgr_table_u32, &len);
	for (i = 0; i < len; i = i + 2) {
		writel(sys_mgr_table_u32[i + 1],
		       sys_mgr_table_u32[i] +
		       (u8 *)socfpga_get_sysmgr_addr() + SYSMGR_SOC64_IODELAY0);
	}
}
