// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Logic PD, Inc.
 *
 * Author: Adam Ford <aford173@gmail.com>
 *
 * Based on SabreSD by Fabio Estevam <fabio.estevam@nxp.com>
 * and updates by Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <miiphy.h>
#include <input.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/sizes.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |            \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define NAND_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |            \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED   |             \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_SD3_DAT7__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD3_DAT6__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT5__UART2_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT6__UART2_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const uart3_pads[] = {
	MX6_PAD_EIM_D23__UART3_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D24__UART3_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D25__UART3_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_EB3__UART3_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void fixup_enet_clock(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	struct gpio_desc nint;
	struct gpio_desc reset;
	int ret;

	/* Set Ref Clock to 50 MHz */
	enable_fec_anatop_clock(0, ENET_50MHZ);

	/* Set GPIO_16 as ENET_REF_CLK_OUT */
	setbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	/* Request GPIO Pins to reset Ethernet with new clock */
	ret = dm_gpio_lookup_name("GPIO4_7", &nint);
	if (ret) {
		printf("Unable to lookup GPIO4_7\n");
		return;
	}

	ret = dm_gpio_request(&nint, "eth0_nInt");
	if (ret) {
		printf("Unable to request eth0_nInt\n");
		return;
	}

	/* Ensure nINT is input or PHY won't startup */
	dm_gpio_set_dir_flags(&nint, GPIOD_IS_IN);

	ret = dm_gpio_lookup_name("GPIO4_9", &reset);
	if (ret) {
		printf("Unable to lookup GPIO4_9\n");
		return;
	}

	ret = dm_gpio_request(&reset, "eth0_reset");
	if (ret) {
		printf("Unable to request eth0_reset\n");
		return;
	}

	/* Reset LAN8710A PHY */
	dm_gpio_set_dir_flags(&reset, GPIOD_IS_OUT);
	dm_gpio_set_value(&reset, 0);
	udelay(150);
	dm_gpio_set_value(&reset, 1);
	mdelay(50);
}

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
	imx_iomux_v3_setup_multiple_pads(uart3_pads, ARRAY_SIZE(uart3_pads));
}

static iomux_v3_cfg_t const nand_pads[] = {
	MX6_PAD_NANDF_CS0__NAND_CE0_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_ALE__NAND_ALE  | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_CLE__NAND_CLE  | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_WP_B__NAND_WP_B  | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_RB0__NAND_READY_B   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D0__NAND_DATA00    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D1__NAND_DATA01    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D2__NAND_DATA02    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D3__NAND_DATA03    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D4__NAND_DATA04    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D5__NAND_DATA05    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D6__NAND_DATA06    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D7__NAND_DATA07    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_SD4_CLK__NAND_WE_B    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_SD4_CMD__NAND_RE_B    | MUX_PAD_CTRL(NAND_PAD_CTRL),
};

static void setup_nand_pins(void)
{
	imx_iomux_v3_setup_multiple_pads(nand_pads, ARRAY_SIZE(nand_pads));
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_early_init_f(void)
{
	fixup_enet_clock();
	setup_iomux_uart();
	setup_nand_pins();
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
	return 0;
}

int board_late_init(void)
{
	env_set("board_name", "imx6logic");

	if (is_mx6dq()) {
		env_set("board_rev", "MX6DQ");
		env_set("fdt_file", "imx6q-logicpd.dtb");
	}

	return 0;
}
