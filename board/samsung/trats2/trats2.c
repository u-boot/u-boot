/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 * Sanghee Kim <sh0130.kim@samsung.com>
 * Piotr Wilczek <p.wilczek@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/mipi_dsim.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <power/battery.h>
#include <power/max77693_pmic.h>
#include <power/max77693_muic.h>
#include <power/max77693_fg.h>
#include <libtizen.h>
#include <errno.h>
#include <usb.h>
#include <usb/s3c_udc.h>
#include <usb_mass_storage.h>

DECLARE_GLOBAL_DATA_PTR;

static struct exynos4x12_gpio_part1 *gpio1;
static struct exynos4x12_gpio_part2 *gpio2;

static unsigned int board_rev = -1;

static inline u32 get_model_rev(void);

static void check_hw_revision(void)
{
	int modelrev = 0;
	int i;

	gpio2 = (struct exynos4x12_gpio_part2 *)samsung_get_base_gpio_part2();

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
	gpio2 = (struct exynos4x12_gpio_part2 *)samsung_get_base_gpio_part2();

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
	int err;

	gpio1 = (struct exynos4x12_gpio_part1 *)samsung_get_base_gpio_part1();
	gpio2 = (struct exynos4x12_gpio_part2 *)samsung_get_base_gpio_part2();

	/* I2C_7 */
	err = exynos_pinmux_config(PERIPH_ID_I2C7, PINMUX_FLAG_NONE);
	if (err) {
		debug("I2C%d not configured\n", (I2C_7));
		return;
	}

	/* I2C_8 */
	s5p_gpio_direction_output(&gpio1->f1, 4, 1);
	s5p_gpio_direction_output(&gpio1->f1, 5, 1);

	/* I2C_9 */
	s5p_gpio_direction_output(&gpio2->m2, 1, 1);
	s5p_gpio_direction_output(&gpio2->m2, 0, 1);
}
#endif

#ifdef CONFIG_SYS_I2C_SOFT
int get_soft_i2c_scl_pin(void)
{
	if (I2C_ADAP_HWNR)
		return exynos4x12_gpio_get(2, m2, 1); /* I2C9 */
	else
		return exynos4x12_gpio_get(1, f1, 4); /* I2C8 */
}

int get_soft_i2c_sda_pin(void)
{
	if (I2C_ADAP_HWNR)
		return exynos4x12_gpio_get(2, m2, 0); /* I2C9 */
	else
		return exynos4x12_gpio_get(1, f1, 5); /* I2C8 */
}
#endif

int exynos_early_init_f(void)
{
	board_external_gpio_init();

	return 0;
}

static int pmic_init_max77686(void);

int exynos_init(void)
{
	struct exynos4_power *pwr =
		(struct exynos4_power *)samsung_get_base_power();

	check_hw_revision();
	printf("HW Revision:\t0x%04x\n", board_rev);

	/*
	 * First bootloader on the TRATS2 platform uses
	 * INFORM4 and INFORM5 registers for recovery
	 *
	 * To indicate correct boot chain - those two
	 * registers must be cleared out
	 */
	writel(0, &pwr->inform4);
	writel(0, &pwr->inform5);

	return 0;
}

int exynos_power_init(void)
{
	int chrg;
	struct power_battery *pb;
	struct pmic *p_chrg, *p_muic, *p_fg, *p_bat;

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	board_init_i2c();
#endif
	pmic_init(I2C_7);		/* I2C adapter 7 - bus name s3c24x0_7 */
	pmic_init_max77686();
	pmic_init_max77693(I2C_10);	/* I2C adapter 10 - bus name soft1 */
	power_muic_init(I2C_10);	/* I2C adapter 10 - bus name soft1 */
	power_fg_init(I2C_9);		/* I2C adapter 9 - bus name soft0 */
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

#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
	int ret = 0;
	unsigned int val;
	struct pmic *p, *p_pmic, *p_muic;

	p_pmic = pmic_get("MAX77686_PMIC");
	if (!p_pmic)
		return -ENODEV;

	if (pmic_probe(p_pmic))
		return -1;

	p_muic = pmic_get("MAX77693_MUIC");
	if (!p_muic)
		return -ENODEV;

	if (pmic_probe(p_muic))
		return -1;

	if (on) {
		ret = max77686_set_ldo_mode(p_pmic, 12, OPMODE_ON);
		if (ret)
			return -1;

		p = pmic_get("MAX77693_PMIC");
		if (!p)
			return -ENODEV;

		if (pmic_probe(p))
			return -1;

		/* SAFEOUT */
		ret = pmic_reg_read(p, MAX77693_SAFEOUT, &val);
		if (ret)
			return -1;

		val |= MAX77693_ENSAFEOUT1;
		ret = pmic_reg_write(p, MAX77693_SAFEOUT, val);
		if (ret)
			return -1;

		/* PATH: USB */
		ret = pmic_reg_write(p_muic, MAX77693_MUIC_CONTROL1,
			MAX77693_MUIC_CTRL1_DN1DP2);

	} else {
		ret = max77686_set_ldo_mode(p_pmic, 12, OPMODE_LPM);
		if (ret)
			return -1;

		/* PATH: UART */
		ret = pmic_reg_write(p_muic, MAX77693_MUIC_CONTROL1,
			MAX77693_MUIC_CTRL1_UT1UR2);
	}

	if (ret)
		return -1;

	return 0;
}

struct s3c_plat_otg_data s5pc210_otg_data = {
	.phy_control	= s5pc210_phy_control,
	.regs_phy	= EXYNOS4X12_USBPHY_BASE,
	.regs_otg	= EXYNOS4X12_USBOTG_BASE,
	.usb_phy_ctrl	= EXYNOS4X12_USBPHY_CONTROL,
	.usb_flags	= PHY0_SLEEP,
};

int board_usb_init(int index, enum usb_init_type init)
{
	debug("USB_udc_probe\n");
	return s3c_udc_probe(&s5pc210_otg_data);
}

int g_dnl_board_usb_cable_connected(void)
{
	struct pmic *muic = pmic_get("MAX77693_MUIC");
	if (!muic)
		return 0;

	return !!muic->chrg->chrg_type(muic);
}
#endif

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
int mipi_power(void)
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

	gpio1 = (struct exynos4x12_gpio_part1 *)samsung_get_base_gpio_part1();

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
	gpio1 = (struct exynos4x12_gpio_part1 *)samsung_get_base_gpio_part1();

	/* reset lcd */
	s5p_gpio_direction_output(&gpio1->f2, 1, 0);
	udelay(10);
	s5p_gpio_set_value(&gpio1->f2, 1, 1);
}

void exynos_lcd_misc_init(vidinfo_t *vid)
{
#ifdef CONFIG_TIZEN
	get_tizen_logo_info(vid);
#endif
#ifdef CONFIG_S6E8AX0
	s6e8ax0_init();
#endif
}
#endif /* LCD */
