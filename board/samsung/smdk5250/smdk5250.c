/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cros_ec.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <errno.h>
#include <i2c.h>
#include <lcd.h>
#include <netdev.h>
#include <spi.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/sromc.h>
#include <asm/arch/dp_info.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_USB_EHCI_EXYNOS
static int board_usb_vbus_init(void)
{
	struct exynos5_gpio_part1 *gpio1 = (struct exynos5_gpio_part1 *)
						samsung_get_base_gpio_part1();

	/* Enable VBUS power switch */
	s5p_gpio_direction_output(&gpio1->x2, 6, 1);

	/* VBUS turn ON time */
	mdelay(3);

	return 0;
}
#endif

#ifdef CONFIG_SOUND_MAX98095
static void  board_enable_audio_codec(void)
{
	struct exynos5_gpio_part1 *gpio1 = (struct exynos5_gpio_part1 *)
						samsung_get_base_gpio_part1();

	/* Enable MAX98095 Codec */
	s5p_gpio_direction_output(&gpio1->x1, 7, 1);
	s5p_gpio_set_pull(&gpio1->x1, 7, GPIO_PULL_NONE);
}
#endif

int exynos_init(void)
{
#ifdef CONFIG_USB_EHCI_EXYNOS
	board_usb_vbus_init();
#endif
#ifdef CONFIG_SOUND_MAX98095
	board_enable_audio_codec();
#endif
	return 0;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_SMC911X
	u32 smc_bw_conf, smc_bc_conf;
	struct fdt_sromc config;
	fdt_addr_t base_addr;

	/* Non-FDT configuration - bank number and timing parameters*/
	config.bank = CONFIG_ENV_SROM_BANK;
	config.width = 2;

	config.timing[FDT_SROM_TACS] = 0x01;
	config.timing[FDT_SROM_TCOS] = 0x01;
	config.timing[FDT_SROM_TACC] = 0x06;
	config.timing[FDT_SROM_TCOH] = 0x01;
	config.timing[FDT_SROM_TAH] = 0x0C;
	config.timing[FDT_SROM_TACP] = 0x09;
	config.timing[FDT_SROM_PMC] = 0x01;
	base_addr = CONFIG_SMC911X_BASE;

	/* Ethernet needs data bus width of 16 bits */
	if (config.width != 2) {
		debug("%s: Unsupported bus width %d\n", __func__,
			config.width);
		return -1;
	}
	smc_bw_conf = SROMC_DATA16_WIDTH(config.bank)
			| SROMC_BYTE_ENABLE(config.bank);

	smc_bc_conf = SROMC_BC_TACS(config.timing[FDT_SROM_TACS])   |\
			SROMC_BC_TCOS(config.timing[FDT_SROM_TCOS]) |\
			SROMC_BC_TACC(config.timing[FDT_SROM_TACC]) |\
			SROMC_BC_TCOH(config.timing[FDT_SROM_TCOH]) |\
			SROMC_BC_TAH(config.timing[FDT_SROM_TAH])   |\
			SROMC_BC_TACP(config.timing[FDT_SROM_TACP]) |\
			SROMC_BC_PMC(config.timing[FDT_SROM_PMC]);

	/* Select and configure the SROMC bank */
	exynos_pinmux_config(PERIPH_ID_SROMC, config.bank);
	s5p_config_sromc(config.bank, smc_bw_conf, smc_bc_conf);
	return smc911x_initialize(0, base_addr);
#endif
	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("\nBoard: SMDK5250\n");
	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int err, ret = 0, index, bus_width;
	u32 base;

	err = exynos_pinmux_config(PERIPH_ID_SDMMC0, PINMUX_FLAG_8BIT_MODE);
	if (err)
		debug("SDMMC0 not configured\n");
	ret |= err;

	/*EMMC: dwmmc Channel-0 with 8 bit bus width */
	index = 0;
	base =  samsung_get_base_mmc() + (0x10000 * index);
	bus_width = 8;
	err = exynos_dwmci_add_port(index, base, bus_width, (u32)NULL);
	if (err)
		debug("dwmmc Channel-0 init failed\n");
	ret |= err;

	err = exynos_pinmux_config(PERIPH_ID_SDMMC2, PINMUX_FLAG_NONE);
	if (err)
		debug("SDMMC2 not configured\n");
	ret |= err;

	/*SD: dwmmc Channel-2 with 4 bit bus width */
	index = 2;
	base = samsung_get_base_mmc() + (0x10000 * index);
	bus_width = 4;
	err = exynos_dwmci_add_port(index, base, bus_width, (u32)NULL);
	if (err)
		debug("dwmmc Channel-2 init failed\n");
	ret |= err;

	return ret;
}
#endif

void board_i2c_init(const void *blob)
{
	int i;

	for (i = 0; i < CONFIG_MAX_I2C_NUM; i++) {
		exynos_pinmux_config((PERIPH_ID_I2C0 + i),
				     PINMUX_FLAG_NONE);
	}
}

#ifdef CONFIG_LCD
void exynos_cfg_lcd_gpio(void)
{
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();

	/* For Backlight */
	s5p_gpio_cfg_pin(&gpio1->b2, 0, GPIO_OUTPUT);
	s5p_gpio_set_value(&gpio1->b2, 0, 1);

	/* LCD power on */
	s5p_gpio_cfg_pin(&gpio1->x1, 5, GPIO_OUTPUT);
	s5p_gpio_set_value(&gpio1->x1, 5, 1);

	/* Set Hotplug detect for DP */
	s5p_gpio_cfg_pin(&gpio1->x0, 7, GPIO_FUNC(0x3));
}

void exynos_set_dp_phy(unsigned int onoff)
{
	set_dp_phy_ctrl(onoff);
}

vidinfo_t panel_info = {
	.vl_freq	= 60,
	.vl_col		= 2560,
	.vl_row		= 1600,
	.vl_width	= 2560,
	.vl_height	= 1600,
	.vl_clkp	= CONFIG_SYS_LOW,
	.vl_hsp		= CONFIG_SYS_LOW,
	.vl_vsp		= CONFIG_SYS_LOW,
	.vl_dp		= CONFIG_SYS_LOW,
	.vl_bpix	= 4,	/* LCD_BPP = 2^4, for output conosle on LCD */

	/* wDP panel timing infomation */
	.vl_hspw	= 32,
	.vl_hbpd	= 80,
	.vl_hfpd	= 48,

	.vl_vspw	= 6,
	.vl_vbpd	= 37,
	.vl_vfpd	= 3,
	.vl_cmd_allow_len = 0xf,

	.win_id		= 3,
	.dual_lcd_enabled = 0,

	.init_delay	= 0,
	.power_on_delay = 0,
	.reset_delay	= 0,
	.interface_mode = FIMD_RGB_INTERFACE,
	.dp_enabled	= 1,
};

static struct edp_device_info edp_info = {
	.disp_info = {
		.h_res = 2560,
		.h_sync_width = 32,
		.h_back_porch = 80,
		.h_front_porch = 48,
		.v_res = 1600,
		.v_sync_width  = 6,
		.v_back_porch = 37,
		.v_front_porch = 3,
		.v_sync_rate = 60,
	},
	.lt_info = {
		.lt_status = DP_LT_NONE,
	},
	.video_info = {
		.master_mode = 0,
		.bist_mode = DP_DISABLE,
		.bist_pattern = NO_PATTERN,
		.h_sync_polarity = 0,
		.v_sync_polarity = 0,
		.interlaced = 0,
		.color_space = COLOR_RGB,
		.dynamic_range = VESA,
		.ycbcr_coeff = COLOR_YCBCR601,
		.color_depth = COLOR_8,
	},
};

static struct exynos_dp_platform_data dp_platform_data = {
	.edp_dev_info	= &edp_info,
};

void init_panel_info(vidinfo_t *vid)
{
	vid->rgb_mode   = MODE_RGB_P;
	exynos_set_dp_platform_data(&dp_platform_data);
}
#endif
