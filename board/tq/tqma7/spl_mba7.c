// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Alexander Feilke
 */

#include <fsl_esdhc_imx.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch-mx7/mx7d_pins.h>

#include "../common/tq_bb.h"

#define UART_RX_PAD_CTRL       (PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_PUS_PU100KOHM | \
	PAD_CTL_PUE | PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

#define UART_TX_PAD_CTRL       (PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_PUS_PU100KOHM | \
	PAD_CTL_PUE | PAD_CTL_SRE_SLOW)

#define USDHC_DATA_PAD_CTRL	(PAD_CTL_DSE_3P3V_98OHM | PAD_CTL_SRE_FAST | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

#define USDHC_CMD_PAD_CTRL	(PAD_CTL_DSE_3P3V_98OHM | PAD_CTL_SRE_FAST | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

#define USDHC_CLK_PAD_CTRL	(PAD_CTL_DSE_3P3V_49OHM | PAD_CTL_SRE_FAST | \
	PAD_CTL_PUE | PAD_CTL_PUS_PU47KOHM)

#define USDHC_STROBE_PAD_CTRL	(PAD_CTL_DSE_3P3V_98OHM | PAD_CTL_SRE_FAST | \
	PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PUS_PD100KOHM)

#define GPIO_IN_PAD_CTRL	(PAD_CTL_PUS_PU100KOHM | \
	PAD_CTL_DSE_3P3V_196OHM | PAD_CTL_HYS | PAD_CTL_SRE_SLOW)
#define GPIO_OUT_PAD_CTRL	(PAD_CTL_PUS_PU100KOHM | \
	PAD_CTL_DSE_3P3V_98OHM | PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

static const iomux_v3_cfg_t mba7_uart6_pads[] = {
	NEW_PAD_CTRL(MX7D_PAD_EPDC_DATA08__UART6_DCE_RX, UART_RX_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_EPDC_DATA09__UART6_DCE_TX, UART_TX_PAD_CTRL),
};

static void mba7_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(mba7_uart6_pads, ARRAY_SIZE(mba7_uart6_pads));
}

static const iomux_v3_cfg_t mba7_usdhc1_pads[] = {
	NEW_PAD_CTRL(MX7D_PAD_SD1_CLK__SD1_CLK,		USDHC_CLK_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_CMD__SD1_CMD,		USDHC_CMD_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_DATA0__SD1_DATA0,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_DATA1__SD1_DATA1,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_DATA2__SD1_DATA2,	USDHC_DATA_PAD_CTRL),
	NEW_PAD_CTRL(MX7D_PAD_SD1_DATA3__SD1_DATA3,	USDHC_DATA_PAD_CTRL),
	/* CD */
	NEW_PAD_CTRL(MX7D_PAD_SD1_CD_B__GPIO5_IO0,	GPIO_IN_PAD_CTRL),
	/* WP */
	NEW_PAD_CTRL(MX7D_PAD_SD1_WP__GPIO5_IO1,	GPIO_IN_PAD_CTRL),
};

#define USDHC1_CD_GPIO	IMX_GPIO_NR(5, 0)
#define USDHC1_WP_GPIO	IMX_GPIO_NR(5, 1)

int tq_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC1_BASE_ADDR)
		ret = !gpio_get_value(USDHC1_CD_GPIO);

	return ret;
}

int tq_bb_board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC1_BASE_ADDR)
		ret = gpio_get_value(USDHC1_WP_GPIO);

	return ret;
}

static struct fsl_esdhc_cfg mba7_usdhc_cfg = {
	.esdhc_base = USDHC1_BASE_ADDR,
	.max_bus_width = 4,
};

int tq_bb_board_mmc_init(struct bd_info *bis)
{
	imx_iomux_v3_setup_multiple_pads(mba7_usdhc1_pads,
					 ARRAY_SIZE(mba7_usdhc1_pads));
	gpio_request(USDHC1_CD_GPIO, "usdhc1-cd");
	gpio_request(USDHC1_WP_GPIO, "usdhc1-wp");
	gpio_direction_input(USDHC1_CD_GPIO);
	gpio_direction_input(USDHC1_WP_GPIO);

	mba7_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	if (fsl_esdhc_initialize(bis, &mba7_usdhc_cfg))
		puts("Warning: failed to initialize SD\n");

	return 0;
}

int tq_bb_board_early_init_f(void)
{
	/* iomux and setup of uart */
	mba7_setup_iomuxc_uart();

	return 0;
}

/*
 * This is done per baseboard to allow different implementations
 */
void board_boot_order(u32 *spl_boot_list)
{
	enum boot_device bd;
	/*
	 * try to get sd card slots in order:
	 * eMMC: on Module
	 * -> therefore index 0 for bootloader
	 * index n in kernel (controller instance 3) -> patches needed for
	 * alias indexing
	 * SD1: on Mainboard
	 * index n in kernel (controller instance 1) -> patches needed for
	 * alias indexing
	 * we assume to have a kernel patch that will present mmcblk dev
	 * indexed like controller devs
	 */
	puts("Boot: ");

	bd = get_boot_device();
	switch (bd) {
	case MMC3_BOOT:
		puts("USDHC3(eMMC)\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	case SD1_BOOT:
		puts("USDHC1(SD)\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC2;
		break;
	case QSPI_BOOT:
		puts("QSPI\n");
		spl_boot_list[0] = BOOT_DEVICE_NOR;
		break;
	case USB_BOOT:
		puts("USB\n");
		spl_boot_list[0] = BOOT_DEVICE_BOARD;
		break;
	default:
		/* Default - BOOT_DEVICE_MMC1 */
		puts("WARN: unknown boot device, fallback to eMMC\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	}
}

int board_fit_config_name_match(const char *name)
{
	char *config = NULL;

	if (is_cpu_type(MXC_CPU_MX7S))
		config = "imx7s-mba7";
	else if (is_cpu_type(MXC_CPU_MX7D))
		config = "imx7d-mba7";

	if (strcmp(config, name))
		return -EINVAL;

	printf("Device tree: %s\n", name);

	return 0;
}
