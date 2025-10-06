// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2021 Intel Corporation <www.intel.com>
 *
 */

#include <asm/arch/handoff_soc64.h>
#include <asm/arch/system_manager.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitfield.h>

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5)
/*
 * Setting RESET_PULSE_OVERRIDE bit for successful reset staggering pulse
 * generation and setting PORT_OVERCURRENT bit so that until we turn on the
 * Vbus, it doesn't give false information about Vbus to the HPS controller.
 */
static void sysmgr_config_usb3(void)
{
	u32 reg_val = 0;

	reg_val = readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_USB3_MISC_CTRL_REG0);
	reg_val |= FIELD_PREP(SYSMGR_SOC64_USB3_MISC_CTRL_REG0_RESET_PUL_OVR,
				SET_USB3_MISC_CTRL_REG0_PORT_RESET_PUL_OVR);
	reg_val |= FIELD_PREP(SYSMGR_SOC64_USB3_MISC_CTRL_REG0_PORT_OVR_CURR,
				SET_USB3_MISC_CTRL_REG0_PORT_OVR_CURR_BIT_1);
	writel(reg_val, socfpga_get_sysmgr_addr() + SYSMGR_SOC64_USB3_MISC_CTRL_REG0);
}
#endif

/*
 * Configure all the pin muxes
 */
void sysmgr_pinmux_init(void)
{
	populate_sysmgr_pinmux();
	populate_sysmgr_fpgaintf_module();

#if IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5)
	sysmgr_config_usb3();
#endif
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
	u32 len, i;
	u32 len_mux = socfpga_get_handoff_size((void *)SOC64_HANDOFF_MUX);
	u32 len_ioctl = socfpga_get_handoff_size((void *)SOC64_HANDOFF_IOCTL);
	u32 len_fpga = socfpga_get_handoff_size((void *)SOC64_HANDOFF_FPGA);
	u32 len_delay = socfpga_get_handoff_size((void *)SOC64_HANDOFF_DELAY);

	len = (len_mux > len_ioctl) ? len_mux : len_ioctl;
	len = (len > len_fpga) ? len : len_fpga;
	len = (len > len_delay) ? len : len_delay;

	u32 handoff_table[len];

	/* setup the pin sel */
	len = (len_mux < SOC64_HANDOFF_MUX_LEN) ? len_mux : SOC64_HANDOFF_MUX_LEN;
	socfpga_handoff_read((void *)SOC64_HANDOFF_MUX, handoff_table, len);
	for (i = 0; i < len; i = i + 2) {
		writel(handoff_table[i + 1],
		       handoff_table[i] +
		       (u8 *)socfpga_get_sysmgr_addr() +
		       SYSMGR_SOC64_PINSEL0);
	}

	/* setup the pin ctrl */
	len = (len_ioctl < SOC64_HANDOFF_IOCTL_LEN) ? len_ioctl : SOC64_HANDOFF_IOCTL_LEN;
	socfpga_handoff_read((void *)SOC64_HANDOFF_IOCTL, handoff_table, len);
	for (i = 0; i < len; i = i + 2) {
		writel(handoff_table[i + 1],
		       handoff_table[i] +
		       (u8 *)socfpga_get_sysmgr_addr() +
		       SYSMGR_SOC64_IOCTRL0);
	}

	/* setup the fpga use */
	len = (len_fpga < SOC64_HANDOFF_FPGA_LEN) ? len_fpga : SOC64_HANDOFF_FPGA_LEN;
	socfpga_handoff_read((void *)SOC64_HANDOFF_FPGA, handoff_table, len);
	for (i = 0; i < len; i = i + 2) {
		writel(handoff_table[i + 1],
		       handoff_table[i] +
		       (u8 *)socfpga_get_sysmgr_addr() +
		       SYSMGR_SOC64_EMAC0_USEFPGA);
	}

	/* setup the IO delay */
	len = (len_delay < SOC64_HANDOFF_DELAY_LEN) ? len_delay : SOC64_HANDOFF_DELAY_LEN;
	socfpga_handoff_read((void *)SOC64_HANDOFF_DELAY, handoff_table, len);
	for (i = 0; i < len; i = i + 2) {
		writel(handoff_table[i + 1],
		       handoff_table[i] +
		       (u8 *)socfpga_get_sysmgr_addr() +
		       SYSMGR_SOC64_IODELAY0);
	}
}
