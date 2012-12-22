/*
 * Copyright (C) 2011 Samsung Electronics
 * Heungjun Kim <riverful.kim@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <lcd.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk.h>
#include <asm/arch/mipi_dsim.h>
#include <asm/arch/watchdog.h>
#include <asm/arch/power.h>
#include <power/pmic.h>
#include <usb/s3c_udc.h>
#include <power/max8997_pmic.h>
#include <libtizen.h>
#include <power/max8997_muic.h>
#include <power/battery.h>
#include <power/max17042_fg.h>

#include "setup.h"

DECLARE_GLOBAL_DATA_PTR;

unsigned int board_rev;

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void)
{
	return board_rev;
}
#endif

static void check_hw_revision(void);

static int hwrevision(int rev)
{
	return (board_rev & 0xf) == rev;
}

struct s3c_plat_otg_data s5pc210_otg_data;

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	check_hw_revision();
	printf("HW Revision:\t0x%x\n", board_rev);

	return 0;
}

void i2c_init_board(void)
{
	struct exynos4_gpio_part1 *gpio1 =
		(struct exynos4_gpio_part1 *)samsung_get_base_gpio_part1();
	struct exynos4_gpio_part2 *gpio2 =
		(struct exynos4_gpio_part2 *)samsung_get_base_gpio_part2();

	/* I2C_5 -> PMIC */
	s5p_gpio_direction_output(&gpio1->b, 7, 1);
	s5p_gpio_direction_output(&gpio1->b, 6, 1);
	/* I2C_9 -> FG */
	s5p_gpio_direction_output(&gpio2->y4, 0, 1);
	s5p_gpio_direction_output(&gpio2->y4, 1, 1);
}

static void trats_low_power_mode(void)
{
	struct exynos4_clock *clk =
	    (struct exynos4_clock *)samsung_get_base_clock();
	struct exynos4_power *pwr =
	    (struct exynos4_power *)samsung_get_base_power();

	/* Power down CORE1 */
	/* LOCAL_PWR_CFG [1:0] 0x3 EN, 0x0 DIS */
	writel(0x0, &pwr->arm_core1_configuration);

	/* Change the APLL frequency */
	/* ENABLE (1 enable) | LOCKED (1 locked)  */
	/* [31]              | [29]               */
	/* FSEL      | MDIV          | PDIV            | SDIV */
	/* [27]      | [25:16]       | [13:8]          | [2:0]      */
	writel(0xa0c80604, &clk->apll_con0);

	/* Change CPU0 clock divider */
	/* CORE2_RATIO  | APLL_RATIO   | PCLK_DBG_RATIO | ATB_RATIO  */
	/* [30:28]      | [26:24]      | [22:20]        | [18:16]    */
	/* PERIPH_RATIO | COREM1_RATIO | COREM0_RATIO   | CORE_RATIO */
	/* [14:12]      | [10:8]       | [6:4]          | [2:0]      */
	writel(0x00000100, &clk->div_cpu0);

	/* CLK_DIV_STAT_CPU0 - wait until clock gets stable (0 = stable) */
	while (readl(&clk->div_stat_cpu0) & 0x1111111)
		continue;

	/* Change clock divider ratio for DMC */
	/* DMCP_RATIO                  | DMCD_RATIO  */
	/* [22:20]                     | [18:16]     */
	/* DMC_RATIO | DPHY_RATIO | ACP_PCLK_RATIO   | ACP_RATIO */
	/* [14:12]   | [10:8]     | [6:4]            | [2:0]     */
	writel(0x13113117, &clk->div_dmc0);

	/* CLK_DIV_STAT_DMC0 - wait until clock gets stable (0 = stable) */
	while (readl(&clk->div_stat_dmc0) & 0x11111111)
		continue;

	/* Turn off unnecessary power domains */
	writel(0x0, &pwr->xxti_configuration);	/* XXTI */
	writel(0x0, &pwr->cam_configuration);	/* CAM */
	writel(0x0, &pwr->tv_configuration);    /* TV */
	writel(0x0, &pwr->mfc_configuration);   /* MFC */
	writel(0x0, &pwr->g3d_configuration);   /* G3D */
	writel(0x0, &pwr->gps_configuration);   /* GPS */
	writel(0x0, &pwr->gps_alive_configuration);	/* GPS_ALIVE */

	/* Turn off unnecessary clocks */
	writel(0x0, &clk->gate_ip_cam);	/* CAM */
	writel(0x0, &clk->gate_ip_tv);          /* TV */
	writel(0x0, &clk->gate_ip_mfc);	/* MFC */
	writel(0x0, &clk->gate_ip_g3d);	/* G3D */
	writel(0x0, &clk->gate_ip_image);	/* IMAGE */
	writel(0x0, &clk->gate_ip_gps);	/* GPS */
}

static int pmic_init_max8997(void)
{
	struct pmic *p = pmic_get("MAX8997_PMIC");
	int i = 0, ret = 0;
	u32 val;

	if (pmic_probe(p))
		return -1;

	/* BUCK1 VARM: 1.2V */
	val = (1200000 - 650000) / 25000;
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK1DVS1, val);
	val = ENBUCK | ACTIVE_DISCHARGE;		/* DVS OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK1CTRL, val);

	/* BUCK2 VINT: 1.1V */
	val = (1100000 - 650000) / 25000;
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK2DVS1, val);
	val = ENBUCK | ACTIVE_DISCHARGE;		/* DVS OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK2CTRL, val);


	/* BUCK3 G3D: 1.1V - OFF */
	ret |= pmic_reg_read(p, MAX8997_REG_BUCK3CTRL, &val);
	val &= ~ENBUCK;
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK3CTRL, val);

	val = (1100000 - 750000) / 50000;
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK3DVS, val);

	/* BUCK4 CAMISP: 1.2V - OFF */
	ret |= pmic_reg_read(p, MAX8997_REG_BUCK4CTRL, &val);
	val &= ~ENBUCK;
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK4CTRL, val);

	val = (1200000 - 650000) / 25000;
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK4DVS, val);

	/* BUCK5 VMEM: 1.2V */
	val = (1200000 - 650000) / 25000;
	for (i = 0; i < 8; i++)
		ret |= pmic_reg_write(p, MAX8997_REG_BUCK5DVS1 + i, val);

	val = ENBUCK | ACTIVE_DISCHARGE;		/* DVS OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK5CTRL, val);

	/* BUCK6 CAM AF: 2.8V */
	/* No Voltage Setting Register */
	/* GNSLCT 3.0X */
	val = GNSLCT;
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK6CTRL, val);

	/* BUCK7 VCC_SUB: 2.0V */
	val = (2000000 - 750000) / 50000;
	ret |= pmic_reg_write(p, MAX8997_REG_BUCK7DVS, val);

	/* LDO1 VADC: 3.3V */
	val = max8997_reg_ldo(3300000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO1CTRL, val);

	/* LDO1 Disable active discharging */
	ret |= pmic_reg_read(p, MAX8997_REG_LDO1CONFIG, &val);
	val &= ~LDO_ADE;
	ret |= pmic_reg_write(p, MAX8997_REG_LDO1CONFIG, val);

	/* LDO2 VALIVE: 1.1V */
	val = max8997_reg_ldo(1100000) | EN_LDO;
	ret |= pmic_reg_write(p, MAX8997_REG_LDO2CTRL, val);

	/* LDO3 VUSB/MIPI: 1.1V */
	val = max8997_reg_ldo(1100000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO3CTRL, val);

	/* LDO4 VMIPI: 1.8V */
	val = max8997_reg_ldo(1800000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO4CTRL, val);

	/* LDO5 VHSIC: 1.2V */
	val = max8997_reg_ldo(1200000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO5CTRL, val);

	/* LDO6 VCC_1.8V_PDA: 1.8V */
	val = max8997_reg_ldo(1800000) | EN_LDO;
	ret |= pmic_reg_write(p, MAX8997_REG_LDO6CTRL, val);

	/* LDO7 CAM_ISP: 1.8V */
	val = max8997_reg_ldo(1800000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO7CTRL, val);

	/* LDO8 VDAC/VUSB: 3.3V */
	val = max8997_reg_ldo(3300000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO8CTRL, val);

	/* LDO9 VCC_2.8V_PDA: 2.8V */
	val = max8997_reg_ldo(2800000) | EN_LDO;
	ret |= pmic_reg_write(p, MAX8997_REG_LDO9CTRL, val);

	/* LDO10 VPLL: 1.1V */
	val = max8997_reg_ldo(1100000) | EN_LDO;
	ret |= pmic_reg_write(p, MAX8997_REG_LDO10CTRL, val);

	/* LDO11 TOUCH: 2.8V */
	val = max8997_reg_ldo(2800000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO11CTRL, val);

	/* LDO12 VTCAM: 1.8V */
	val = max8997_reg_ldo(1800000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO12CTRL, val);

	/* LDO13 VCC_3.0_LCD: 3.0V */
	val = max8997_reg_ldo(3000000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO13CTRL, val);

	/* LDO14 MOTOR: 3.0V */
	val = max8997_reg_ldo(3000000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO14CTRL, val);

	/* LDO15 LED_A: 2.8V */
	val = max8997_reg_ldo(2800000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO15CTRL, val);

	/* LDO16 CAM_SENSOR: 1.8V */
	val = max8997_reg_ldo(1800000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO16CTRL, val);

	/* LDO17 VTF: 2.8V */
	val = max8997_reg_ldo(2800000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO17CTRL, val);

	/* LDO18 TOUCH_LED 3.3V */
	val = max8997_reg_ldo(3300000) | DIS_LDO;	/* OFF */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO18CTRL, val);

	/* LDO21 VDDQ: 1.2V */
	val = max8997_reg_ldo(1200000) | EN_LDO;
	ret |= pmic_reg_write(p, MAX8997_REG_LDO21CTRL, val);

	/* SAFEOUT for both 1 and 2: 4.9V, Active discharge, Enable */
	val = (SAFEOUT_4_90V << 0) | (SAFEOUT_4_90V << 2) |
		ACTDISSAFEO1 | ACTDISSAFEO2 | ENSAFEOUT1 | ENSAFEOUT2;
	ret |= pmic_reg_write(p, MAX8997_REG_SAFEOUTCTRL, val);

	if (ret) {
		puts("MAX8997 PMIC setting error!\n");
		return -1;
	}
	return 0;
}

int power_init_board(void)
{
	int chrg, ret;
	struct power_battery *pb;
	struct pmic *p_fg, *p_chrg, *p_muic, *p_bat;

	ret = pmic_init(I2C_5);
	ret |= pmic_init_max8997();
	ret |= power_fg_init(I2C_9);
	ret |= power_muic_init(I2C_5);
	ret |= power_bat_init(0);
	if (ret)
		return ret;

	p_fg = pmic_get("MAX17042_FG");
	if (!p_fg) {
		puts("MAX17042_FG: Not found\n");
		return -ENODEV;
	}

	p_chrg = pmic_get("MAX8997_PMIC");
	if (!p_chrg) {
		puts("MAX8997_PMIC: Not found\n");
		return -ENODEV;
	}

	p_muic = pmic_get("MAX8997_MUIC");
	if (!p_muic) {
		puts("MAX8997_MUIC: Not found\n");
		return -ENODEV;
	}

	p_bat = pmic_get("BAT_TRATS");
	if (!p_bat) {
		puts("BAT_TRATS: Not found\n");
		return -ENODEV;
	}

	p_fg->parent =  p_bat;
	p_chrg->parent = p_bat;
	p_muic->parent = p_bat;

	p_bat->low_power_mode = trats_low_power_mode;
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
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE);

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

static unsigned int get_hw_revision(void)
{
	struct exynos4_gpio_part1 *gpio =
		(struct exynos4_gpio_part1 *)samsung_get_base_gpio_part1();
	int hwrev = 0;
	int i;

	/* hw_rev[3:0] == GPE1[3:0] */
	for (i = 0; i < 4; i++) {
		s5p_gpio_cfg_pin(&gpio->e1, i, GPIO_INPUT);
		s5p_gpio_set_pull(&gpio->e1, i, GPIO_PULL_NONE);
	}

	udelay(1);

	for (i = 0; i < 4; i++)
		hwrev |= (s5p_gpio_get_value(&gpio->e1, i) << i);

	debug("hwrev 0x%x\n", hwrev);

	return hwrev;
}

static void check_hw_revision(void)
{
	int hwrev;

	hwrev = get_hw_revision();

	board_rev |= hwrev;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board:\tTRATS\n");
	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	struct exynos4_gpio_part2 *gpio =
		(struct exynos4_gpio_part2 *)samsung_get_base_gpio_part2();
	int err;

	/* eMMC_EN: SD_0_CDn: GPK0[2] Output High */
	s5p_gpio_direction_output(&gpio->k0, 2, 1);
	s5p_gpio_set_pull(&gpio->k0, 2, GPIO_PULL_NONE);

	/*
	 * MMC device init
	 * mmc0	 : eMMC (8-bit buswidth)
	 * mmc2	 : SD card (4-bit buswidth)
	 */
	err = exynos_pinmux_config(PERIPH_ID_SDMMC0, PINMUX_FLAG_8BIT_MODE);
	if (err)
		debug("SDMMC0 not configured\n");
	else
		err = s5p_mmc_init(0, 8);

	/* T-flash detect */
	s5p_gpio_cfg_pin(&gpio->x3, 4, 0xf);
	s5p_gpio_set_pull(&gpio->x3, 4, GPIO_PULL_UP);

	/*
	 * Check the T-flash  detect pin
	 * GPX3[4] T-flash detect pin
	 */
	if (!s5p_gpio_get_value(&gpio->x3, 4)) {
		err = exynos_pinmux_config(PERIPH_ID_SDMMC2, PINMUX_FLAG_NONE);
		if (err)
			debug("SDMMC2 not configured\n");
		else
			err = s5p_mmc_init(2, 4);
	}

	return err;
}
#endif

#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
	int ret = 0;
	u32 val = 0;
	struct pmic *p = pmic_get("MAX8997_PMIC");
	if (!p)
		return -ENODEV;

	if (pmic_probe(p))
		return -1;

	if (on) {
		ret |= pmic_set_output(p, MAX8997_REG_SAFEOUTCTRL,
				      ENSAFEOUT1, LDO_ON);
		ret |= pmic_reg_read(p, MAX8997_REG_LDO3CTRL, &val);
		ret |= pmic_reg_write(p, MAX8997_REG_LDO3CTRL, EN_LDO | val);

		ret |= pmic_reg_read(p, MAX8997_REG_LDO8CTRL, &val);
		ret |= pmic_reg_write(p, MAX8997_REG_LDO8CTRL, EN_LDO | val);
	} else {
		ret |= pmic_reg_read(p, MAX8997_REG_LDO8CTRL, &val);
		ret |= pmic_reg_write(p, MAX8997_REG_LDO8CTRL, DIS_LDO | val);

		ret |= pmic_reg_read(p, MAX8997_REG_LDO3CTRL, &val);
		ret |= pmic_reg_write(p, MAX8997_REG_LDO3CTRL, DIS_LDO | val);
		ret |= pmic_set_output(p, MAX8997_REG_SAFEOUTCTRL,
				      ENSAFEOUT1, LDO_OFF);
	}

	if (ret) {
		puts("MAX8997 LDO setting error!\n");
		return -1;
	}

	return 0;
}

struct s3c_plat_otg_data s5pc210_otg_data = {
	.phy_control	= s5pc210_phy_control,
	.regs_phy	= EXYNOS4_USBPHY_BASE,
	.regs_otg	= EXYNOS4_USBOTG_BASE,
	.usb_phy_ctrl	= EXYNOS4_USBPHY_CONTROL,
	.usb_flags	= PHY0_SLEEP,
};

void board_usb_init(void)
{
	debug("USB_udc_probe\n");
	s3c_udc_probe(&s5pc210_otg_data);
}
#endif

static void pmic_reset(void)
{
	struct exynos4_gpio_part2 *gpio =
		(struct exynos4_gpio_part2 *)samsung_get_base_gpio_part2();

	s5p_gpio_direction_output(&gpio->x0, 7, 1);
	s5p_gpio_set_pull(&gpio->x2, 7, GPIO_PULL_NONE);
}

static void board_clock_init(void)
{
	struct exynos4_clock *clk =
		(struct exynos4_clock *)samsung_get_base_clock();

	writel(CLK_SRC_CPU_VAL, (unsigned int)&clk->src_cpu);
	writel(CLK_SRC_TOP0_VAL, (unsigned int)&clk->src_top0);
	writel(CLK_SRC_FSYS_VAL, (unsigned int)&clk->src_fsys);
	writel(CLK_SRC_PERIL0_VAL, (unsigned int)&clk->src_peril0);

	writel(CLK_DIV_CPU0_VAL, (unsigned int)&clk->div_cpu0);
	writel(CLK_DIV_CPU1_VAL, (unsigned int)&clk->div_cpu1);
	writel(CLK_DIV_DMC0_VAL, (unsigned int)&clk->div_dmc0);
	writel(CLK_DIV_DMC1_VAL, (unsigned int)&clk->div_dmc1);
	writel(CLK_DIV_LEFTBUS_VAL, (unsigned int)&clk->div_leftbus);
	writel(CLK_DIV_RIGHTBUS_VAL, (unsigned int)&clk->div_rightbus);
	writel(CLK_DIV_TOP_VAL, (unsigned int)&clk->div_top);
	writel(CLK_DIV_FSYS1_VAL, (unsigned int)&clk->div_fsys1);
	writel(CLK_DIV_FSYS2_VAL, (unsigned int)&clk->div_fsys2);
	writel(CLK_DIV_FSYS3_VAL, (unsigned int)&clk->div_fsys3);
	writel(CLK_DIV_PERIL0_VAL, (unsigned int)&clk->div_peril0);
	writel(CLK_DIV_PERIL3_VAL, (unsigned int)&clk->div_peril3);

	writel(PLL_LOCKTIME, (unsigned int)&clk->apll_lock);
	writel(PLL_LOCKTIME, (unsigned int)&clk->mpll_lock);
	writel(PLL_LOCKTIME, (unsigned int)&clk->epll_lock);
	writel(PLL_LOCKTIME, (unsigned int)&clk->vpll_lock);
	writel(APLL_CON1_VAL, (unsigned int)&clk->apll_con1);
	writel(APLL_CON0_VAL, (unsigned int)&clk->apll_con0);
	writel(MPLL_CON1_VAL, (unsigned int)&clk->mpll_con1);
	writel(MPLL_CON0_VAL, (unsigned int)&clk->mpll_con0);
	writel(EPLL_CON1_VAL, (unsigned int)&clk->epll_con1);
	writel(EPLL_CON0_VAL, (unsigned int)&clk->epll_con0);
	writel(VPLL_CON1_VAL, (unsigned int)&clk->vpll_con1);
	writel(VPLL_CON0_VAL, (unsigned int)&clk->vpll_con0);

	writel(CLK_GATE_IP_CAM_VAL, (unsigned int)&clk->gate_ip_cam);
	writel(CLK_GATE_IP_VP_VAL, (unsigned int)&clk->gate_ip_tv);
	writel(CLK_GATE_IP_MFC_VAL, (unsigned int)&clk->gate_ip_mfc);
	writel(CLK_GATE_IP_G3D_VAL, (unsigned int)&clk->gate_ip_g3d);
	writel(CLK_GATE_IP_IMAGE_VAL, (unsigned int)&clk->gate_ip_image);
	writel(CLK_GATE_IP_LCD0_VAL, (unsigned int)&clk->gate_ip_lcd0);
	writel(CLK_GATE_IP_LCD1_VAL, (unsigned int)&clk->gate_ip_lcd1);
	writel(CLK_GATE_IP_FSYS_VAL, (unsigned int)&clk->gate_ip_fsys);
	writel(CLK_GATE_IP_GPS_VAL, (unsigned int)&clk->gate_ip_gps);
	writel(CLK_GATE_IP_PERIL_VAL, (unsigned int)&clk->gate_ip_peril);
	writel(CLK_GATE_IP_PERIR_VAL, (unsigned int)&clk->gate_ip_perir);
	writel(CLK_GATE_BLOCK_VAL, (unsigned int)&clk->gate_block);
}

static void board_power_init(void)
{
	struct exynos4_power *pwr =
		(struct exynos4_power *)samsung_get_base_power();

	/* PS HOLD */
	writel(EXYNOS4_PS_HOLD_CON_VAL, (unsigned int)&pwr->ps_hold_control);

	/* Set power down */
	writel(0, (unsigned int)&pwr->cam_configuration);
	writel(0, (unsigned int)&pwr->tv_configuration);
	writel(0, (unsigned int)&pwr->mfc_configuration);
	writel(0, (unsigned int)&pwr->g3d_configuration);
	writel(0, (unsigned int)&pwr->lcd1_configuration);
	writel(0, (unsigned int)&pwr->gps_configuration);
	writel(0, (unsigned int)&pwr->gps_alive_configuration);

	/* It is necessary to power down core 1 */
	/* to successfully boot CPU1 in kernel */
	writel(0, (unsigned int)&pwr->arm_core1_configuration);
}

static void board_uart_init(void)
{
	struct exynos4_gpio_part1 *gpio1 =
		(struct exynos4_gpio_part1 *)samsung_get_base_gpio_part1();
	struct exynos4_gpio_part2 *gpio2 =
		(struct exynos4_gpio_part2 *)samsung_get_base_gpio_part2();
	int i;

	/*
	 * UART2 GPIOs
	 * GPA1CON[0] = UART_2_RXD(2)
	 * GPA1CON[1] = UART_2_TXD(2)
	 * GPA1CON[2] = I2C_3_SDA (3)
	 * GPA1CON[3] = I2C_3_SCL (3)
	 */

	for (i = 0; i < 4; i++) {
		s5p_gpio_set_pull(&gpio1->a1, i, GPIO_PULL_NONE);
		s5p_gpio_cfg_pin(&gpio1->a1, i, GPIO_FUNC((i > 1) ? 0x3 : 0x2));
	}

	/* UART_SEL GPY4[7] (part2) at EXYNOS4 */
	s5p_gpio_set_pull(&gpio2->y4, 7, GPIO_PULL_UP);
	s5p_gpio_direction_output(&gpio2->y4, 7, 1);
}

int board_early_init_f(void)
{
	wdt_stop();
	pmic_reset();
	board_clock_init();
	board_uart_init();
	board_power_init();

	return 0;
}

static void lcd_reset(void)
{
	struct exynos4_gpio_part2 *gpio2 =
		(struct exynos4_gpio_part2 *)samsung_get_base_gpio_part2();

	s5p_gpio_direction_output(&gpio2->y4, 5, 1);
	udelay(10000);
	s5p_gpio_direction_output(&gpio2->y4, 5, 0);
	udelay(10000);
	s5p_gpio_direction_output(&gpio2->y4, 5, 1);
}

static int lcd_power(void)
{
	int ret = 0;
	struct pmic *p = pmic_get("MAX8997_PMIC");
	if (!p)
		return -ENODEV;

	if (pmic_probe(p))
		return 0;

	/* LDO15 voltage: 2.2v */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO15CTRL, 0x1c | EN_LDO);
	/* LDO13 voltage: 3.0v */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO13CTRL, 0x2c | EN_LDO);

	if (ret) {
		puts("MAX8997 LDO setting error!\n");
		return -1;
	}

	return 0;
}

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

static struct exynos_platform_mipi_dsim s6e8ax0_platform_data = {
	.lcd_panel_info = NULL,
	.dsim_config = &dsim_config,
};

static struct mipi_dsim_lcd_device mipi_lcd_device = {
	.name	= "s6e8ax0",
	.id	= -1,
	.bus_id	= 0,
	.platform_data	= (void *)&s6e8ax0_platform_data,
};

static int mipi_power(void)
{
	int ret = 0;
	struct pmic *p = pmic_get("MAX8997_PMIC");
	if (!p)
		return -ENODEV;

	if (pmic_probe(p))
		return 0;

	/* LDO3 voltage: 1.1v */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO3CTRL, 0x6 | EN_LDO);
	/* LDO4 voltage: 1.8v */
	ret |= pmic_reg_write(p, MAX8997_REG_LDO4CTRL, 0x14 | EN_LDO);

	if (ret) {
		puts("MAX8997 LDO setting error!\n");
		return -1;
	}

	return 0;
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

	.win_id		= 3,
	.cfg_gpio	= NULL,
	.backlight_on	= NULL,
	.lcd_power_on	= NULL,	/* lcd_power_on in mipi dsi driver */
	.reset_lcd	= lcd_reset,
	.dual_lcd_enabled = 0,

	.init_delay	= 0,
	.power_on_delay = 0,
	.reset_delay	= 0,
	.interface_mode = FIMD_RGB_INTERFACE,
	.mipi_enabled	= 1,
};

void init_panel_info(vidinfo_t *vid)
{
	vid->logo_on	= 1,
	vid->resolution	= HD_RESOLUTION,
	vid->rgb_mode	= MODE_RGB_P,

#ifdef CONFIG_TIZEN
	get_tizen_logo_info(vid);
#endif

	if (hwrevision(2))
		mipi_lcd_device.reverse_panel = 1;

	strcpy(s6e8ax0_platform_data.lcd_panel_name, mipi_lcd_device.name);
	s6e8ax0_platform_data.lcd_power = lcd_power;
	s6e8ax0_platform_data.mipi_power = mipi_power;
	s6e8ax0_platform_data.phy_enable = set_mipi_phy_ctrl;
	s6e8ax0_platform_data.lcd_panel_info = (void *)vid;
	exynos_mipi_dsi_register_lcd_device(&mipi_lcd_device);
	s6e8ax0_init();
	exynos_set_dsim_platform_data(&s6e8ax0_platform_data);

	setenv("lcdinfo", "lcd=s6e8ax0");
}
