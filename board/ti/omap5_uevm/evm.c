/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated, <www.ti.com>
 * Aneesh V       <aneesh@ti.com>
 * Steve Sakoman  <steve@sakoman.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <palmas.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <tca642x.h>

#include "mux_data.h"

#ifdef CONFIG_USB_EHCI
#include <usb.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/ehci.h>
#include <asm/ehci-omap.h>

#define DIE_ID_REG_BASE     (OMAP54XX_L4_CORE_BASE + 0x2000)
#define DIE_ID_REG_OFFSET	0x200

#endif

DECLARE_GLOBAL_DATA_PTR;

const struct omap_sysinfo sysinfo = {
	"Board: OMAP5432 uEVM\n"
};

/**
 * @brief tca642x_init - uEVM default values for the GPIO expander
 * input reg, output reg, polarity reg, configuration reg
 */
struct tca642x_bank_info tca642x_init[] = {
	{ .input_reg = 0x00,
	  .output_reg = 0x04,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0x80 },
	{ .input_reg = 0x00,
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0xff },
	{ .input_reg = 0x00,
	  .output_reg = 0x00,
	  .polarity_reg = 0x00,
	  .configuration_reg = 0x40 },
};

/**
 * @brief board_init
 *
 * @return 0
 */
int board_init(void)
{
	gpmc_init();
	gd->bd->bi_arch_number = MACH_TYPE_OMAP5_SEVM;
	gd->bd->bi_boot_params = (0x80000000 + 0x100); /* boot param addr */

	tca642x_set_inital_state(CONFIG_SYS_I2C_TCA642X_ADDR, tca642x_init);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return 0;
}

/**
 * @brief misc_init_r - Configure EVM board specific configurations
 * such as power configurations, ethernet initialization as phase2 of
 * boot sequence
 *
 * @return 0
 */
int misc_init_r(void)
{
#ifdef CONFIG_PALMAS_POWER
	palmas_init_settings();
#endif
	return 0;
}

void set_muxconf_regs_essential(void)
{
	do_set_mux((*ctrl)->control_padconf_core_base,
		   core_padconf_array_essential,
		   sizeof(core_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));

	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkup_padconf_array_essential,
		   sizeof(wkup_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));
}

void set_muxconf_regs_non_essential(void)
{
	do_set_mux((*ctrl)->control_padconf_core_base,
		   core_padconf_array_non_essential,
		   sizeof(core_padconf_array_non_essential) /
		   sizeof(struct pad_conf_entry));

	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkup_padconf_array_non_essential,
		   sizeof(wkup_padconf_array_non_essential) /
		   sizeof(struct pad_conf_entry));
}

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_GENERIC_MMC)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);
	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}
#endif

#ifdef CONFIG_USB_EHCI
static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_HSIC,
	.port_mode[2] = OMAP_EHCI_PORT_MODE_HSIC,
};

static void enable_host_clocks(void)
{
	int hs_clk_ctrl_val = (OPTFCLKEN_HSIC60M_P3_CLK |
				OPTFCLKEN_HSIC480M_P3_CLK |
				OPTFCLKEN_HSIC60M_P2_CLK |
				OPTFCLKEN_HSIC480M_P2_CLK |
				OPTFCLKEN_UTMI_P3_CLK | OPTFCLKEN_UTMI_P2_CLK);

	/* Enable port 2 and 3 clocks*/
	setbits_le32((*prcm)->cm_l3init_hsusbhost_clkctrl, hs_clk_ctrl_val);

	/* Enable port 2 and 3 usb host ports tll clocks*/
	setbits_le32((*prcm)->cm_l3init_hsusbtll_clkctrl,
			(OPTFCLKEN_USB_CH1_CLK_ENABLE | OPTFCLKEN_USB_CH2_CLK_ENABLE));
}

int ehci_hcd_init(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	int ret;
	int auxclk;
	int reg;
	uint8_t device_mac[6];

	enable_host_clocks();

	if (!getenv("usbethaddr")) {
		reg = DIE_ID_REG_BASE + DIE_ID_REG_OFFSET;

		/*
		 * create a fake MAC address from the processor ID code.
		 * first byte is 0x02 to signify locally administered.
		 */
		device_mac[0] = 0x02;
		device_mac[1] = readl(reg + 0x10) & 0xff;
		device_mac[2] = readl(reg + 0xC) & 0xff;
		device_mac[3] = readl(reg + 0x8) & 0xff;
		device_mac[4] = readl(reg) & 0xff;
		device_mac[5] = (readl(reg) >> 8) & 0xff;

		eth_setenv_enetaddr("usbethaddr", device_mac);
	}

	auxclk = readl((*prcm)->scrm_auxclk1);
	/* Request auxilary clock */
	auxclk |= AUXCLK_ENABLE_MASK;
	writel(auxclk, (*prcm)->scrm_auxclk1);

	ret = omap_ehci_hcd_init(&usbhs_bdata, hccr, hcor);
	if (ret < 0) {
		puts("Failed to initialize ehci\n");
		return ret;
	}

	return 0;
}

int ehci_hcd_stop(void)
{
	int ret;

	ret = omap_ehci_hcd_stop();
	return ret;
}

void usb_hub_reset_devices(int port)
{
	/* The LAN9730 needs to be reset after the port power has been set. */
	if (port == 3) {
		gpio_direction_output(CONFIG_OMAP_EHCI_PHY3_RESET_GPIO, 0);
		udelay(10);
		gpio_direction_output(CONFIG_OMAP_EHCI_PHY3_RESET_GPIO, 1);
	}
}
#endif
