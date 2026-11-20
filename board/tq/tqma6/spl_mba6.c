// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Max Merchel
 */

#include <fsl_esdhc_imx.h>
#include <i2c.h>
#include <init.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>

#include "tqma6.h"
#include "../common/tq_bb.h"

#define UART_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
			PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define GPIO_OUT_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define GPIO_IN_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

static const iomux_v3_cfg_t mba6_uart2_pads[] = {
	MX6_PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void mba6_setup_iomuxc_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(mba6_uart2_pads,
					 ARRAY_SIZE(mba6_uart2_pads));
}

static const iomux_v3_cfg_t mba6_usdhc2_pads[] = {
	MX6_PAD_SD2_CLK__SD2_CLK |	MUX_PAD_CTRL(USDHC_CLK_PAD_CTRL),
	MX6_PAD_SD2_CMD__SD2_CMD |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT0__SD2_DATA0 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT1__SD2_DATA1 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT2__SD2_DATA2 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT3__SD2_DATA3 |	MUX_PAD_CTRL(USDHC_PAD_CTRL),
	/* CD */
	MX6_PAD_GPIO_4__GPIO1_IO04 |	MUX_PAD_CTRL(GPIO_IN_PAD_CTRL),
	/* WP */
	MX6_PAD_GPIO_2__GPIO1_IO02 |	MUX_PAD_CTRL(GPIO_IN_PAD_CTRL),
};

#define USDHC2_CD_GPIO	IMX_GPIO_NR(1, 4)
#define USDHC2_WP_GPIO	IMX_GPIO_NR(1, 2)

static struct fsl_esdhc_cfg mba6_usdhc_cfg = {
	.esdhc_base = USDHC2_BASE_ADDR,
	.max_bus_width = 4,
};

int tq_bb_board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = !gpio_get_value(USDHC2_CD_GPIO);

	return ret;
}

int tq_bb_board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC2_BASE_ADDR)
		ret = gpio_get_value(USDHC2_WP_GPIO);

	return ret;
}

int tq_bb_board_mmc_init(struct bd_info *bis)
{
	imx_iomux_v3_setup_multiple_pads(mba6_usdhc2_pads,
					 ARRAY_SIZE(mba6_usdhc2_pads));
	gpio_request(USDHC2_CD_GPIO, "usdhc2-cd");
	gpio_request(USDHC2_WP_GPIO, "usdhc2-wp");
	gpio_direction_input(USDHC2_CD_GPIO);
	gpio_direction_input(USDHC2_WP_GPIO);

	mba6_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	if (fsl_esdhc_initialize(bis, &mba6_usdhc_cfg))
		puts("Warning: failed to initialize SD\n");

	return 0;
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void tq_bb_board_init_f(ulong dummy)
{
	mba6_setup_iomuxc_uart();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
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
		puts("USB\n");
		spl_boot_list[0] = BOOT_DEVICE_BOARD;
		return;
	}

	switch (imx6_bmode) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
		/* SD/eSD - BOOT_DEVICE_MMC2 */
		puts("SD\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC2;
		break;
	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		/* MMC/eMMC - BOOT_DEVICE_MMC1 */
		puts("eMMC\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	case IMX6_BMODE_SERIAL_ROM:
		/* SERIAL_ROM - BOOT_DEVICE_BOARD */
		puts("Serial ROM\n");
		spl_boot_list[0] = BOOT_DEVICE_BOARD;
		switch ((imx6_bmode & IMX6_BMODE_SERIAL_ROM_MASK) >>
			IMX6_BMODE_SERIAL_ROM_SHIFT) {
		case IMX6_BMODE_ECSPI1:
		case IMX6_BMODE_ECSPI2:
		case IMX6_BMODE_ECSPI3:
		case IMX6_BMODE_ECSPI4:
		case IMX6_BMODE_ECSPI5:
			spl_boot_list[0] = BOOT_DEVICE_SPI;
			break;
		default:
			/* Default - BOOT_DEVICE_MMC1 */
			puts("Wrong SPI device\n");
			break;
		}
		break;
	default:
		/* Default - BOOT_DEVICE_MMC1 */
		puts("Wrong board boot order\n");
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	}
}

int board_fit_config_name_match(const char *name)
{
	const char *config = tqma6_get_fdt_configuration();

	if (config && name && !strcmp(config, name)) {
		printf("Device tree: %s\n", name);
		return 0;
	}

	return -EINVAL;
}
