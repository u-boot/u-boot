// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2022 Toradex
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm-generic/gpio.h>
#include <asm/global_data.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <errno.h>
#include <env.h>
#include <init.h>
#include <linux/delay.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>

#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

/* Verdin UART_3, Console/Debug UART */
static const iomux_v3_cfg_t uart_pads[] = {
	MX8MP_PAD_UART3_RXD__UART3_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_UART3_TXD__UART3_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	init_uart_clk(2);

	return 0;
}

static void setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Enable RGMII TX clk output */
	setbits_le32(&gpr->gpr[1], BIT(22));
}

static int setup_eqos(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* set INTF as RGMII, enable RGMII TXC clock */
	clrsetbits_le32(&gpr->gpr[1],
			IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MASK, BIT(16));
	setbits_le32(&gpr->gpr[1], BIT(19) | BIT(21));

	return set_clk_eqos(ENET_125MHZ);
}

#if IS_ENABLED(CONFIG_NET)
int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

int board_init(void)
{
	int ret = 0;

	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	if (IS_ENABLED(CONFIG_DWC_ETH_QOS))
		ret = setup_eqos();

	return ret;
}

static void select_dt_from_module_version(void)
{
	char variant[32];
	char *env_variant = env_get("variant");
	int is_wifi = 0;

	if (IS_ENABLED(CONFIG_TDX_CFG_BLOCK)) {
		/*
		 * If we have a valid config block and it says we are a module with
		 * Wi-Fi/Bluetooth make sure we use the -wifi device tree.
		 */
		is_wifi = (tdx_hw_tag.prodid == VERDIN_IMX8MPQ_WIFI_BT_IT) ||
			  (tdx_hw_tag.prodid == VERDIN_IMX8MPQ_2GB_WIFI_BT_IT) ||
			  (tdx_hw_tag.prodid == VERDIN_IMX8MPQ_8GB_WIFI_BT);
	}

	if (is_wifi)
		strlcpy(&variant[0], "wifi", sizeof(variant));
	else
		strlcpy(&variant[0], "nonwifi", sizeof(variant));

	if (strcmp(variant, env_variant)) {
		printf("Setting variant to %s\n", variant);
		env_set("variant", variant);

		if (IS_ENABLED(CONFIG_ENV_IS_NOWHERE))
			env_save();
	}
}

int board_late_init(void)
{
	select_dt_from_module_version();

	return 0;
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif
