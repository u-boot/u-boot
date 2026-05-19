// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Max Merchel
 */

#include <errno.h>
#include <fsl_esdhc_imx.h>
#include <malloc.h>
#include <spl.h>
#include <spl_gpio.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6ul_pins.h>
#include <asm/mach-imx/sys_proto.h>

#include "../common/tq_bb.h"
#include "tqma6ul.h"

#define GPIO_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
		       PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
			    PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | \
			    PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | \
			PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW | \
			PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define UART_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | \
		       PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
		       PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

static const iomux_v3_cfg_t mba6ul_uart_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_UART1_TX_DATA__UART1_DCE_TX, UART_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_UART1_RX_DATA__UART1_DCE_RX, UART_PAD_CTRL),
};

static void mba6ul_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(mba6ul_uart_pads,
					 ARRAY_SIZE(mba6ul_uart_pads));
}

/* SD card on USDHC1 */
static const iomux_v3_cfg_t mba6ul_usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__USDHC1_CLK |	MUX_PAD_CTRL(USDHC_CLK_PAD_CTRL),
	MX6_PAD_SD1_CMD__USDHC1_CMD |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA0__USDHC1_DATA0 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA1__USDHC1_DATA1 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA2__USDHC1_DATA2 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA3__USDHC1_DATA3 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	/* WP */
	MX6_PAD_UART1_CTS_B__GPIO1_IO18 |	MUX_PAD_CTRL(GPIO_PAD_CTRL),
	/* CD */
	MX6_PAD_UART1_RTS_B__GPIO1_IO19 |	MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

#define USDHC1_CD_GPIO	IMX_GPIO_NR(1, 19)
#define USDHC1_WP_GPIO	IMX_GPIO_NR(1, 18)

static struct fsl_esdhc_cfg mba6ul_usdhc1_cfg = {
	.esdhc_base = USDHC1_BASE_ADDR,
	.max_bus_width = 4,
};

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

int tq_bb_board_mmc_init(struct bd_info *bis)
{
	imx_iomux_v3_setup_multiple_pads(mba6ul_usdhc1_pads,
					 ARRAY_SIZE(mba6ul_usdhc1_pads));
	gpio_request(USDHC1_CD_GPIO, "usdhc1-cd");
	gpio_request(USDHC1_WP_GPIO, "usdhc1-wp");
	gpio_direction_input(USDHC1_CD_GPIO);
	gpio_direction_input(USDHC1_WP_GPIO);

	mba6ul_usdhc1_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	if (fsl_esdhc_initialize(bis, &mba6ul_usdhc1_cfg))
		puts("Warning: failed to initialize SD card\n");

	return 0;
}

int board_early_init_f(void)
{
	tq_bb_board_early_init_f();

	mba6ul_setup_iomuxc_uart();

	return 0;
}

/*
 * This is done per baseboard to allow different implementations
 */
void board_boot_order(u32 *spl_boot_list)
{
	u32 bmode = imx6_src_get_boot_mode();
	u8 imx6_bmode = (bmode & IMX6_BMODE_MASK) >> IMX6_BMODE_SHIFT;

	/* USB boot */
	if (spl_boot_device() == BOOT_DEVICE_BOARD) {
		printf("USB\n");
		spl_boot_list[0] = BOOT_DEVICE_BOARD;
		return;
	}

	switch (imx6_bmode) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
		/* SD/eSD - BOOT_DEVICE_MMC2 */
		printf("SD\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC2;
		break;
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		/* MMC/eMMC - BOOT_DEVICE_MMC1 */
		printf("eMMC\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	case IMX6_BMODE_QSPI:
		/* QSPI - BOOT_DEVICE_SPI */
		printf("QSPI\n");
		spl_boot_list[0] = BOOT_DEVICE_NOR;
		break;
	case IMX6_BMODE_SERIAL_ROM:
		/* SERIAL_ROM - BOOT_DEVICE_BOARD */
		printf("Serial ROM\n");
		spl_boot_list[0] = BOOT_DEVICE_BOARD;
		break;
	default:
		printf("WARNING: unknown boot device, fallback to eMMC\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	}
}

int board_fit_config_name_match(const char *name)
{
	/* Longest FDT name */
	char dt[] = "imx6ull-tqma6ull2l-mba6ulx";
	enum tqma6ul_som_type somtype;

	somtype = set_tqma6ul_dt_name(dt, sizeof(dt), "mba6ulx");
	if (somtype == tqma6ul_som_type_unknown)
		return -EINVAL;

	if (!strcmp(name, dt))
		return -EINVAL;

	printf("Device tree: %s\n", name);
	return 0;
}
