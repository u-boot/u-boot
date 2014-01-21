/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <i2c.h>
#include <lcd.h>
#include <spi.h>
#include <asm/arch/board.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/dp_info.h>

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

int exynos_init(void)
{
#ifdef CONFIG_USB_EHCI_EXYNOS
	board_usb_vbus_init();
#endif
	return 0;
}

#ifdef CONFIG_LCD
void cfg_lcd_gpio(void)
{
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *)samsung_get_base_gpio_part1();

	/* For Backlight */
	s5p_gpio_cfg_pin(&gpio1->b2, 0, GPIO_OUTPUT);
	s5p_gpio_set_value(&gpio1->b2, 0, 1);

	/* LCD power on */
	s5p_gpio_cfg_pin(&gpio1->x1, 5, GPIO_OUTPUT);
	s5p_gpio_set_value(&gpio1->x1, 5, 1);

	/* Set Hotplug detect for DP */
	s5p_gpio_cfg_pin(&gpio1->x0, 7, GPIO_FUNC(0x3));
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
	.cfg_gpio	= cfg_lcd_gpio,
	.backlight_on	= NULL,
	.lcd_power_on	= NULL,
	.reset_lcd	= NULL,
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
	.phy_enable	= set_dp_phy_ctrl,
	.edp_dev_info	= &edp_info,
};

void init_panel_info(vidinfo_t *vid)
{
	vid->rgb_mode   = MODE_RGB_P;

	exynos_set_dp_platform_data(&dp_platform_data);
}
#endif

int board_get_revision(void)
{
	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	const char *board_name;

	board_name = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	if (board_name == NULL)
		printf("\nUnknown Board\n");
	else
		printf("\nBoard: %s\n", board_name);

	return 0;
}
#endif
