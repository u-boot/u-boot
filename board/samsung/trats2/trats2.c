/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 * Sanghee Kim <sh0130.kim@samsung.com>
 * Piotr Wilczek <p.wilczek@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/power.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include <asm/arch/mipi_dsim.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <power/battery.h>
#include <power/max77693_pmic.h>
#include <power/max77693_muic.h>
#include <power/max77693_fg.h>
#include <libtizen.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

static struct exynos4x12_gpio_part1 *gpio1;
static struct exynos4x12_gpio_part2 *gpio2;

static unsigned int board_rev = -1;

static inline u32 get_model_rev(void);

static void check_hw_revision(void)
{
	int modelrev = 0;
	int i;

	gpio2 = (struct exynos4x12_gpio_part2 *)EXYNOS4X12_GPIO_PART2_BASE;

	/*
	 * GPM1[1:0]: MODEL_REV[1:0]
	 * Don't set as pull-none for these N/C pin.
	 * TRM say that it may cause unexcepted state and leakage current.
	 * and pull-none is only for output function.
	 */
	for (i = 0; i < 2; i++)
		s5p_gpio_cfg_pin(&gpio2->m1, i, GPIO_INPUT);

	/* GPM1[5:2]: HW_REV[3:0] */
	for (i = 2; i < 6; i++) {
		s5p_gpio_cfg_pin(&gpio2->m1, i, GPIO_INPUT);
		s5p_gpio_set_pull(&gpio2->m1, i, GPIO_PULL_NONE);
	}

	/* GPM1[1:0]: MODEL_REV[1:0] */
	for (i = 0; i < 2; i++)
		modelrev |= (s5p_gpio_get_value(&gpio2->m1, i) << i);

	/* board_rev[15:8] = model */
	board_rev = modelrev << 8;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board:\tTRATS2\n");
	return 0;
}
#endif

static void show_hw_revision(void)
{
	printf("HW Revision:\t0x%04x\n", board_rev);
}

u32 get_board_rev(void)
{
	return board_rev;
}

static inline u32 get_model_rev(void)
{
	return (board_rev >> 8) & 0xff;
}

static void board_external_gpio_init(void)
{
	gpio2 = (struct exynos4x12_gpio_part2 *)EXYNOS4X12_GPIO_PART2_BASE;

	/*
	 * some pins which in alive block are connected with external pull-up
	 * but it's default setting is pull-down.
	 * if that pin set as input then that floated
	 */

	s5p_gpio_set_pull(&gpio2->x0, 2, GPIO_PULL_NONE);	/* PS_ALS_INT */
	s5p_gpio_set_pull(&gpio2->x0, 4, GPIO_PULL_NONE);	/* TSP_nINT */
	s5p_gpio_set_pull(&gpio2->x0, 7, GPIO_PULL_NONE);	/* AP_PMIC_IRQ*/
	s5p_gpio_set_pull(&gpio2->x1, 5, GPIO_PULL_NONE);	/* IF_PMIC_IRQ*/
	s5p_gpio_set_pull(&gpio2->x2, 0, GPIO_PULL_NONE);	/* VOL_UP */
	s5p_gpio_set_pull(&gpio2->x2, 1, GPIO_PULL_NONE);	/* VOL_DOWN */
	s5p_gpio_set_pull(&gpio2->x2, 3, GPIO_PULL_NONE);	/* FUEL_ALERT */
	s5p_gpio_set_pull(&gpio2->x2, 4, GPIO_PULL_NONE);	/* ADC_INT */
	s5p_gpio_set_pull(&gpio2->x2, 7, GPIO_PULL_NONE);	/* nPOWER */
	s5p_gpio_set_pull(&gpio2->x3, 0, GPIO_PULL_NONE);	/* WPC_INT */
	s5p_gpio_set_pull(&gpio2->x3, 5, GPIO_PULL_NONE);	/* OK_KEY */
	s5p_gpio_set_pull(&gpio2->x3, 7, GPIO_PULL_NONE);	/* HDMI_HPD */
}

#ifdef CONFIG_SYS_I2C_INIT_BOARD
static void board_init_i2c(void)
{
	gpio1 = (struct exynos4x12_gpio_part1 *)EXYNOS4X12_GPIO_PART1_BASE;
	gpio2 = (struct exynos4x12_gpio_part2 *)EXYNOS4X12_GPIO_PART2_BASE;

	/* I2C_7 */
	s5p_gpio_direction_output(&gpio1->d0, 2, 1);
	s5p_gpio_direction_output(&gpio1->d0, 3, 1);

	/* I2C_8 */
	s5p_gpio_direction_output(&gpio1->f1, 4, 1);
	s5p_gpio_direction_output(&gpio1->f1, 5, 1);

	/* I2C_9 */
	s5p_gpio_direction_output(&gpio2->m2, 1, 1);
	s5p_gpio_direction_output(&gpio2->m2, 0, 1);
}
#endif

int board_early_init_f(void)
{
	check_hw_revision();
	board_external_gpio_init();

	gd->flags |= GD_FLG_DISABLE_CONSOLE;

	return 0;
}

static int pmic_init_max77686(void);

int board_init(void)
{
	struct exynos4_power *pwr =
		(struct exynos4_power *)EXYNOS4X12_POWER_BASE;

	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	/* workaround: clear INFORM4..5 */
	writel(0, (unsigned int)&pwr->inform4);
	writel(0, (unsigned int)&pwr->inform5);

	return 0;
}

int power_init_board(void)
{
	int chrg;
	struct power_battery *pb;
	struct pmic *p_chrg, *p_muic, *p_fg, *p_bat;

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	board_init_i2c();
#endif
	pmic_init(I2C_0);		/* I2C adapter 0 - bus name I2C_5 */
	pmic_init_max77686();
	pmic_init_max77693(I2C_2);	/* I2C adapter 2 - bus name I2C_10 */
	power_muic_init(I2C_2);		/* I2C adapter 2 - bus name I2C_10 */
	power_fg_init(I2C_1);		/* I2C adapter 1 - bus name I2C_9 */
	power_bat_init(0);

	p_chrg = pmic_get("MAX77693_PMIC");
	if (!p_chrg) {
		puts("MAX77693_PMIC: Not found\n");
		return -ENODEV;
	}

	p_muic = pmic_get("MAX77693_MUIC");
	if (!p_muic) {
		puts("MAX77693_MUIC: Not found\n");
		return -ENODEV;
	}

	p_fg = pmic_get("MAX77693_FG");
	if (!p_fg) {
		puts("MAX17042_FG: Not found\n");
		return -ENODEV;
	}

	if (p_chrg->chrg->chrg_bat_present(p_chrg) == 0)
		puts("No battery detected\n");

	p_bat = pmic_get("BAT_TRATS2");
	if (!p_bat) {
		puts("BAT_TRATS2: Not found\n");
		return -ENODEV;
	}

	p_fg->parent =  p_bat;
	p_chrg->parent = p_bat;
	p_muic->parent = p_bat;

	p_bat->pbat->battery_init(p_bat, p_fg, p_chrg, p_muic);

	pb = p_bat->pbat;
	chrg = p_muic->chrg->chrg_type(p_muic);
	debug("CHARGER TYPE: %d\n", chrg);

	if (!p_chrg->chrg->chrg_bat_present(p_chrg)) {
		puts("No battery detected\n");
		return -1;
	}

	p_fg->fg->fg_battery_check(p_fg, p_bat);

	if (pb->bat->state == CHARGE && chrg == CHARGER_USB)
		puts("CHARGE Battery !\n");

	return 0;
}

int dram_init(void)
{
	u32 size_mb;

	size_mb = (get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE)) >> 20;

	gd->ram_size = size_mb << 20;

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = PHYS_SDRAM_4_SIZE;
}

int board_mmc_init(bd_t *bis)
{
	int err0, err2 = 0;

	gpio2 = (struct exynos4x12_gpio_part2 *)EXYNOS4X12_GPIO_PART2_BASE;

	/* eMMC_EN: SD_0_CDn: GPK0[2] Output High */
	s5p_gpio_direction_output(&gpio2->k0, 2, 1);
	s5p_gpio_set_pull(&gpio2->k0, 2, GPIO_PULL_NONE);

	/*
	 * eMMC GPIO:
	 * SDR 8-bit@48MHz at MMC0
	 * GPK0[0]      SD_0_CLK(2)
	 * GPK0[1]      SD_0_CMD(2)
	 * GPK0[2]      SD_0_CDn        -> Not used
	 * GPK0[3:6]    SD_0_DATA[0:3](2)
	 * GPK1[3:6]    SD_0_DATA[0:3](3)
	 *
	 * DDR 4-bit@26MHz at MMC4
	 * GPK0[0]      SD_4_CLK(3)
	 * GPK0[1]      SD_4_CMD(3)
	 * GPK0[2]      SD_4_CDn        -> Not used
	 * GPK0[3:6]    SD_4_DATA[0:3](3)
	 * GPK1[3:6]    SD_4_DATA[4:7](4)
	 */

	err0 = exynos_pinmux_config(PERIPH_ID_SDMMC0, PINMUX_FLAG_8BIT_MODE);

	/*
	 * MMC device init
	 * mmc0  : eMMC (8-bit buswidth)
	 * mmc2  : SD card (4-bit buswidth)
	 */
	if (err0)
		debug("SDMMC0 not configured\n");
	else
		err0 = s5p_mmc_init(0, 8);

	/* T-flash detect */
	s5p_gpio_cfg_pin(&gpio2->x3, 4, 0xf);
	s5p_gpio_set_pull(&gpio2->x3, 4, GPIO_PULL_UP);

	/*
	 * Check the T-flash  detect pin
	 * GPX3[4] T-flash detect pin
	 */
	if (!s5p_gpio_get_value(&gpio2->x3, 4)) {
		err2 = exynos_pinmux_config(PERIPH_ID_SDMMC2, PINMUX_FLAG_NONE);
		if (err2)
			debug("SDMMC2 not configured\n");
		else
			err2 = s5p_mmc_init(2, 4);
	}

	return err0 & err2;
}

static int pmic_init_max77686(void)
{
	struct pmic *p = pmic_get("MAX77686_PMIC");

	if (pmic_probe(p))
		return -1;

	/* BUCK/LDO Output Voltage */
	max77686_set_ldo_voltage(p, 21, 2800000);	/* LDO21 VTF_2.8V */
	max77686_set_ldo_voltage(p, 23, 3300000);	/* LDO23 TSP_AVDD_3.3V*/
	max77686_set_ldo_voltage(p, 24, 1800000);	/* LDO24 TSP_VDD_1.8V */

	/* BUCK/LDO Output Mode */
	max77686_set_buck_mode(p, 1, OPMODE_STANDBY);	/* BUCK1 VMIF_1.1V_AP */
	max77686_set_buck_mode(p, 2, OPMODE_ON);	/* BUCK2 VARM_1.0V_AP */
	max77686_set_buck_mode(p, 3, OPMODE_ON);	/* BUCK3 VINT_1.0V_AP */
	max77686_set_buck_mode(p, 4, OPMODE_ON);	/* BUCK4 VG3D_1.0V_AP */
	max77686_set_buck_mode(p, 5, OPMODE_ON);	/* BUCK5 VMEM_1.2V_AP */
	max77686_set_buck_mode(p, 6, OPMODE_ON);	/* BUCK6 VCC_SUB_1.35V*/
	max77686_set_buck_mode(p, 7, OPMODE_ON);	/* BUCK7 VCC_SUB_2.0V */
	max77686_set_buck_mode(p, 8, OPMODE_OFF);	/* VMEM_VDDF_2.85V */
	max77686_set_buck_mode(p, 9, OPMODE_OFF);	/* CAM_ISP_CORE_1.2V*/

	max77686_set_ldo_mode(p, 1, OPMODE_LPM);	/* LDO1 VALIVE_1.0V_AP*/
	max77686_set_ldo_mode(p, 2, OPMODE_STANDBY);	/* LDO2 VM1M2_1.2V_AP */
	max77686_set_ldo_mode(p, 3, OPMODE_LPM);	/* LDO3 VCC_1.8V_AP */
	max77686_set_ldo_mode(p, 4, OPMODE_LPM);	/* LDO4 VCC_2.8V_AP */
	max77686_set_ldo_mode(p, 5, OPMODE_OFF);	/* LDO5_VCC_1.8V_IO */
	max77686_set_ldo_mode(p, 6, OPMODE_STANDBY);	/* LDO6 VMPLL_1.0V_AP */
	max77686_set_ldo_mode(p, 7, OPMODE_STANDBY);	/* LDO7 VPLL_1.0V_AP */
	max77686_set_ldo_mode(p, 8, OPMODE_LPM);	/* LDO8 VMIPI_1.0V_AP */
	max77686_set_ldo_mode(p, 9, OPMODE_OFF);	/* CAM_ISP_MIPI_1.2*/
	max77686_set_ldo_mode(p, 10, OPMODE_LPM);	/* LDO10 VMIPI_1.8V_AP*/
	max77686_set_ldo_mode(p, 11, OPMODE_STANDBY);	/* LDO11 VABB1_1.8V_AP*/
	max77686_set_ldo_mode(p, 12, OPMODE_LPM);	/* LDO12 VUOTG_3.0V_AP*/
	max77686_set_ldo_mode(p, 13, OPMODE_OFF);	/* LDO13 VC2C_1.8V_AP */
	max77686_set_ldo_mode(p, 14, OPMODE_STANDBY);	/* VABB02_1.8V_AP */
	max77686_set_ldo_mode(p, 15, OPMODE_STANDBY);	/* LDO15 VHSIC_1.0V_AP*/
	max77686_set_ldo_mode(p, 16, OPMODE_STANDBY);	/* LDO16 VHSIC_1.8V_AP*/
	max77686_set_ldo_mode(p, 17, OPMODE_OFF);	/* CAM_SENSOR_CORE_1.2*/
	max77686_set_ldo_mode(p, 18, OPMODE_OFF);	/* CAM_ISP_SEN_IO_1.8V*/
	max77686_set_ldo_mode(p, 19, OPMODE_OFF);	/* LDO19 VT_CAM_1.8V */
	max77686_set_ldo_mode(p, 20, OPMODE_ON);	/* LDO20 VDDQ_PRE_1.8V*/
	max77686_set_ldo_mode(p, 21, OPMODE_OFF);	/* LDO21 VTF_2.8V */
	max77686_set_ldo_mode(p, 22, OPMODE_OFF);	/* LDO22 VMEM_VDD_2.8V*/
	max77686_set_ldo_mode(p, 23, OPMODE_OFF);	/* LDO23 TSP_AVDD_3.3V*/
	max77686_set_ldo_mode(p, 24, OPMODE_OFF);	/* LDO24 TSP_VDD_1.8V */
	max77686_set_ldo_mode(p, 25, OPMODE_OFF);	/* LDO25 VCC_3.3V_LCD */
	max77686_set_ldo_mode(p, 26, OPMODE_OFF);	/*LDO26 VCC_3.0V_MOTOR*/

	return 0;
}

/*
 * LCD
 */

#ifdef CONFIG_LCD
static struct mipi_dsim_config dsim_config = {
	.e_interface		= DSIM_VIDEO,
	.e_virtual_ch		= DSIM_VIRTUAL_CH_0,
	.e_pixel_format		= DSIM_24BPP_888,
	.e_burst_mode		= DSIM_BURST_SYNC_EVENT,
	.e_no_data_lane		= DSIM_DATA_LANE_4,
	.e_byte_clk		= DSIM_PLL_OUT_DIV8,
	.hfp			= 1,

	.p			= 3,
	.m			= 120,
	.s			= 1,

	/* D-PHY PLL stable time spec :min = 200usec ~ max 400usec */
	.pll_stable_time	= 500,

	/* escape clk : 10MHz */
	.esc_clk		= 20 * 1000000,

	/* stop state holding counter after bta change count 0 ~ 0xfff */
	.stop_holding_cnt	= 0x7ff,
	/* bta timeout 0 ~ 0xff */
	.bta_timeout		= 0xff,
	/* lp rx timeout 0 ~ 0xffff */
	.rx_timeout		= 0xffff,
};

static struct exynos_platform_mipi_dsim dsim_platform_data = {
	.lcd_panel_info = NULL,
	.dsim_config = &dsim_config,
};

static struct mipi_dsim_lcd_device mipi_lcd_device = {
	.name	= "s6e8ax0",
	.id	= -1,
	.bus_id	= 0,
	.platform_data	= (void *)&dsim_platform_data,
};

static int mipi_power(void)
{
	struct pmic *p = pmic_get("MAX77686_PMIC");

	/* LDO8 VMIPI_1.0V_AP */
	max77686_set_ldo_mode(p, 8, OPMODE_ON);
	/* LDO10 VMIPI_1.8V_AP */
	max77686_set_ldo_mode(p, 10, OPMODE_ON);

	return 0;
}

void exynos_lcd_power_on(void)
{
	struct pmic *p = pmic_get("MAX77686_PMIC");

	gpio1 = (struct exynos4x12_gpio_part1 *)EXYNOS4X12_GPIO_PART1_BASE;

	/* LCD_2.2V_EN: GPC0[1] */
	s5p_gpio_set_pull(&gpio1->c0, 1, GPIO_PULL_UP);
	s5p_gpio_direction_output(&gpio1->c0, 1, 1);

	/* LDO25 VCC_3.1V_LCD */
	pmic_probe(p);
	max77686_set_ldo_voltage(p, 25, 3100000);
	max77686_set_ldo_mode(p, 25, OPMODE_LPM);
}

void exynos_reset_lcd(void)
{
	gpio1 = (struct exynos4x12_gpio_part1 *)EXYNOS4X12_GPIO_PART1_BASE;

	/* reset lcd */
	s5p_gpio_direction_output(&gpio1->f2, 1, 0);
	udelay(10);
	s5p_gpio_set_value(&gpio1->f2, 1, 1);
}

vidinfo_t panel_info = {
	.vl_freq	= 60,
	.vl_col		= 720,
	.vl_row		= 1280,
	.vl_width	= 720,
	.vl_height	= 1280,
	.vl_clkp	= CONFIG_SYS_HIGH,
	.vl_hsp		= CONFIG_SYS_LOW,
	.vl_vsp		= CONFIG_SYS_LOW,
	.vl_dp		= CONFIG_SYS_LOW,
	.vl_bpix	= 5,	/* Bits per pixel, 2^5 = 32 */

	/* s6e8ax0 Panel infomation */
	.vl_hspw	= 5,
	.vl_hbpd	= 10,
	.vl_hfpd	= 10,

	.vl_vspw	= 2,
	.vl_vbpd	= 1,
	.vl_vfpd	= 13,
	.vl_cmd_allow_len = 0xf,
	.mipi_enabled = 1,

	.dual_lcd_enabled = 0,

	.init_delay	= 0,
	.power_on_delay = 25,
	.reset_delay	= 0,
	.interface_mode = FIMD_RGB_INTERFACE,
};

void init_panel_info(vidinfo_t *vid)
{
	vid->logo_on	= 1;
	vid->resolution	= HD_RESOLUTION;
	vid->rgb_mode	= MODE_RGB_P;

	vid->power_on_delay = 30;

	mipi_lcd_device.reverse_panel = 1;

#ifdef CONFIG_TIZEN
	get_tizen_logo_info(vid);
#endif

	strcpy(dsim_platform_data.lcd_panel_name, mipi_lcd_device.name);
	dsim_platform_data.mipi_power = mipi_power;
	dsim_platform_data.phy_enable = set_mipi_phy_ctrl;
	dsim_platform_data.lcd_panel_info = (void *)vid;
	exynos_mipi_dsi_register_lcd_device(&mipi_lcd_device);

	s6e8ax0_init();

	exynos_set_dsim_platform_data(&dsim_platform_data);
}
#endif /* LCD */

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	setenv("model", "GT-I8800");
	setenv("board", "TRATS2");

	show_hw_revision();

	return 0;
}
#endif
