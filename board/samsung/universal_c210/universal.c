/*
 *  Copyright (C) 2010 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>
#include <lcd.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/adc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/watchdog.h>
#include <ld9040.h>
#include <power/pmic.h>
#include <usb.h>
#include <usb/s3c_udc.h>
#include <asm/arch/cpu.h>
#include <power/max8998_pmic.h>
#include <libtizen.h>
#include <samsung/misc.h>
#include <usb_mass_storage.h>

DECLARE_GLOBAL_DATA_PTR;

struct exynos4_gpio_part1 *gpio1;
struct exynos4_gpio_part2 *gpio2;
unsigned int board_rev;

u32 get_board_rev(void)
{
	return board_rev;
}

static int get_hwrev(void)
{
	return board_rev & 0xFF;
}

static void init_pmic_lcd(void);

int exynos_power_init(void)
{
	int ret;

	/*
	 * For PMIC the I2C bus is named as I2C5, but it is connected
	 * to logical I2C adapter 0
	 */
	ret = pmic_init(I2C_0);
	if (ret)
		return ret;

	init_pmic_lcd();

	return 0;
}

static unsigned short get_adc_value(int channel)
{
	struct s5p_adc *adc = (struct s5p_adc *)samsung_get_base_adc();
	unsigned short ret = 0;
	unsigned int reg;
	unsigned int loop = 0;

	writel(channel & 0xF, &adc->adcmux);
	writel((1 << 14) | (49 << 6), &adc->adccon);
	writel(1000 & 0xffff, &adc->adcdly);
	writel(readl(&adc->adccon) | (1 << 16), &adc->adccon); /* 12 bit */
	udelay(10);
	writel(readl(&adc->adccon) | (1 << 0), &adc->adccon); /* Enable */
	udelay(10);

	do {
		udelay(1);
		reg = readl(&adc->adccon);
	} while (!(reg & (1 << 15)) && (loop++ < 1000));

	ret = readl(&adc->adcdat0) & 0xFFF;

	return ret;
}

static int adc_power_control(int on)
{
	int ret;
	struct pmic *p = pmic_get("MAX8998_PMIC");
	if (!p)
		return -ENODEV;

	if (pmic_probe(p))
		return -1;

	ret = pmic_set_output(p,
			      MAX8998_REG_ONOFF1,
			      MAX8998_LDO4, !!on);

	return ret;
}

static unsigned int get_hw_revision(void)
{
	int hwrev, mode0, mode1;

	adc_power_control(1);

	mode0 = get_adc_value(1);		/* HWREV_MODE0 */
	mode1 = get_adc_value(2);		/* HWREV_MODE1 */

	/*
	 * XXX Always set the default hwrev as the latest board
	 * ADC = (voltage) / 3.3 * 4096
	 */
	hwrev = 3;

#define IS_RANGE(x, min, max)	((x) > (min) && (x) < (max))
	if (IS_RANGE(mode0, 80, 200) && IS_RANGE(mode1, 80, 200))
		hwrev = 0x0;		/* 0.01V	0.01V */
	if (IS_RANGE(mode0, 750, 1000) && IS_RANGE(mode1, 80, 200))
		hwrev = 0x1;		/* 610mV	0.01V */
	if (IS_RANGE(mode0, 1300, 1700) && IS_RANGE(mode1, 80, 200))
		hwrev = 0x2;		/* 1.16V	0.01V */
	if (IS_RANGE(mode0, 2000, 2400) && IS_RANGE(mode1, 80, 200))
		hwrev = 0x3;		/* 1.79V	0.01V */
#undef IS_RANGE

	debug("mode0: %d, mode1: %d, hwrev 0x%x\n", mode0, mode1, hwrev);

	adc_power_control(0);

	return hwrev;
}

static void check_hw_revision(void)
{
	int hwrev;

	hwrev = get_hw_revision();

	board_rev |= hwrev;
}

#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
	int ret = 0;
	struct pmic *p = pmic_get("MAX8998_PMIC");
	if (!p)
		return -ENODEV;

	if (pmic_probe(p))
		return -1;

	if (on) {
		ret |= pmic_set_output(p,
				       MAX8998_REG_BUCK_ACTIVE_DISCHARGE3,
				       MAX8998_SAFEOUT1, LDO_ON);
		ret |= pmic_set_output(p, MAX8998_REG_ONOFF1,
				      MAX8998_LDO3, LDO_ON);
		ret |= pmic_set_output(p, MAX8998_REG_ONOFF2,
				      MAX8998_LDO8, LDO_ON);

	} else {
		ret |= pmic_set_output(p, MAX8998_REG_ONOFF2,
				      MAX8998_LDO8, LDO_OFF);
		ret |= pmic_set_output(p, MAX8998_REG_ONOFF1,
				      MAX8998_LDO3, LDO_OFF);
		ret |= pmic_set_output(p,
				       MAX8998_REG_BUCK_ACTIVE_DISCHARGE3,
				       MAX8998_SAFEOUT1, LDO_OFF);
	}

	if (ret) {
		puts("MAX8998 LDO setting error!\n");
		return -1;
	}

	return 0;
}

struct s3c_plat_otg_data s5pc210_otg_data = {
	.phy_control = s5pc210_phy_control,
	.regs_phy = EXYNOS4_USBPHY_BASE,
	.regs_otg = EXYNOS4_USBOTG_BASE,
	.usb_phy_ctrl = EXYNOS4_USBPHY_CONTROL,
	.usb_flags = PHY0_SLEEP,
};
#endif

int board_usb_init(int index, enum usb_init_type init)
{
	debug("USB_udc_probe\n");
	return s3c_udc_probe(&s5pc210_otg_data);
}

int exynos_early_init_f(void)
{
	wdt_stop();

	return 0;
}

#ifdef CONFIG_SOFT_SPI
static void soft_spi_init(void)
{
	gpio_direction_output(CONFIG_SOFT_SPI_GPIO_SCLK,
		CONFIG_SOFT_SPI_MODE & SPI_CPOL);
	gpio_direction_output(CONFIG_SOFT_SPI_GPIO_MOSI, 1);
	gpio_direction_input(CONFIG_SOFT_SPI_GPIO_MISO);
	gpio_direction_output(CONFIG_SOFT_SPI_GPIO_CS,
		!(CONFIG_SOFT_SPI_MODE & SPI_CS_HIGH));
}

void spi_cs_activate(struct spi_slave *slave)
{
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_CS,
		!(CONFIG_SOFT_SPI_MODE & SPI_CS_HIGH));
	SPI_SCL(1);
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_CS,
		CONFIG_SOFT_SPI_MODE & SPI_CS_HIGH);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_CS,
		!(CONFIG_SOFT_SPI_MODE & SPI_CS_HIGH));
}

int  spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void universal_spi_scl(int bit)
{
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_SCLK, bit);
}

void universal_spi_sda(int bit)
{
	gpio_set_value(CONFIG_SOFT_SPI_GPIO_MOSI, bit);
}

int universal_spi_read(void)
{
	return gpio_get_value(CONFIG_SOFT_SPI_GPIO_MISO);
}
#endif

static void init_pmic_lcd(void)
{
	unsigned char val;
	int ret = 0;

	struct pmic *p = pmic_get("MAX8998_PMIC");

	if (!p)
		return;

	if (pmic_probe(p))
		return;

	/* LDO7 1.8V */
	val = 0x02; /* (1800 - 1600) / 100; */
	ret |= pmic_reg_write(p,  MAX8998_REG_LDO7, val);

	/* LDO17 3.0V */
	val = 0xe; /* (3000 - 1600) / 100; */
	ret |= pmic_reg_write(p,  MAX8998_REG_LDO17, val);

	/* Disable unneeded regulators */
	/*
	 * ONOFF1
	 * Buck1 ON, Buck2 OFF, Buck3 ON, Buck4 ON
	 * LDO2 ON, LDO3 OFF, LDO4 OFF, LDO5 ON
	 */
	val = 0xB9;
	ret |= pmic_reg_write(p,  MAX8998_REG_ONOFF1, val);

	/* ONOFF2
	 * LDO6 OFF, LDO7 ON, LDO8 OFF, LDO9 ON,
	 * LDO10 OFF, LDO11 OFF, LDO12 OFF, LDO13 OFF
	 */
	val = 0x50;
	ret |= pmic_reg_write(p,  MAX8998_REG_ONOFF2, val);

	/* ONOFF3
	 * LDO14 OFF, LDO15 OFF, LGO16 OFF, LDO17 OFF
	 * EPWRHOLD OFF, EBATTMON OFF, ELBCNFG2 OFF, ELBCNFG1 OFF
	 */
	val = 0x00;
	ret |= pmic_reg_write(p,  MAX8998_REG_ONOFF3, val);

	if (ret)
		puts("LCD pmic initialisation error!\n");
}

void exynos_cfg_lcd_gpio(void)
{
	unsigned int i, f3_end = 4;

	for (i = 0; i < 8; i++) {
		/* set GPF0,1,2[0:7] for RGB Interface and Data lines (32bit) */
		s5p_gpio_cfg_pin(&gpio1->f0, i, GPIO_FUNC(2));
		s5p_gpio_cfg_pin(&gpio1->f1, i, GPIO_FUNC(2));
		s5p_gpio_cfg_pin(&gpio1->f2, i, GPIO_FUNC(2));
		/* pull-up/down disable */
		s5p_gpio_set_pull(&gpio1->f0, i, GPIO_PULL_NONE);
		s5p_gpio_set_pull(&gpio1->f1, i, GPIO_PULL_NONE);
		s5p_gpio_set_pull(&gpio1->f2, i, GPIO_PULL_NONE);

		/* drive strength to max (24bit) */
		s5p_gpio_set_drv(&gpio1->f0, i, GPIO_DRV_4X);
		s5p_gpio_set_rate(&gpio1->f0, i, GPIO_DRV_SLOW);
		s5p_gpio_set_drv(&gpio1->f1, i, GPIO_DRV_4X);
		s5p_gpio_set_rate(&gpio1->f1, i, GPIO_DRV_SLOW);
		s5p_gpio_set_drv(&gpio1->f2, i, GPIO_DRV_4X);
		s5p_gpio_set_rate(&gpio1->f0, i, GPIO_DRV_SLOW);
	}

	for (i = 0; i < f3_end; i++) {
		/* set GPF3[0:3] for RGB Interface and Data lines (32bit) */
		s5p_gpio_cfg_pin(&gpio1->f3, i, GPIO_FUNC(2));
		/* pull-up/down disable */
		s5p_gpio_set_pull(&gpio1->f3, i, GPIO_PULL_NONE);
		/* drive strength to max (24bit) */
		s5p_gpio_set_drv(&gpio1->f3, i, GPIO_DRV_4X);
		s5p_gpio_set_rate(&gpio1->f3, i, GPIO_DRV_SLOW);
	}

	/* gpio pad configuration for LCD reset. */
	s5p_gpio_cfg_pin(&gpio2->y4, 5, GPIO_OUTPUT);

	spi_init();
}

int mipi_power(void)
{
	return 0;
}

void exynos_reset_lcd(void)
{
	s5p_gpio_set_value(&gpio2->y4, 5, 1);
	udelay(10000);
	s5p_gpio_set_value(&gpio2->y4, 5, 0);
	udelay(10000);
	s5p_gpio_set_value(&gpio2->y4, 5, 1);
	udelay(100);
}

void exynos_lcd_power_on(void)
{
	struct pmic *p = pmic_get("MAX8998_PMIC");

	if (!p)
		return;

	if (pmic_probe(p))
		return;

	pmic_set_output(p, MAX8998_REG_ONOFF3, MAX8998_LDO17, LDO_ON);
	pmic_set_output(p, MAX8998_REG_ONOFF2, MAX8998_LDO7, LDO_ON);
}

void exynos_cfg_ldo(void)
{
	ld9040_cfg_ldo();
}

void exynos_enable_ldo(unsigned int onoff)
{
	ld9040_enable_ldo(onoff);
}

int exynos_init(void)
{
	gpio1 = (struct exynos4_gpio_part1 *) EXYNOS4_GPIO_PART1_BASE;
	gpio2 = (struct exynos4_gpio_part2 *) EXYNOS4_GPIO_PART2_BASE;

	gd->bd->bi_arch_number = MACH_TYPE_UNIVERSAL_C210;

	switch (get_hwrev()) {
	case 0:
		/*
		 * Set the low to enable LDO_EN
		 * But when you use the test board for eMMC booting
		 * you should set it HIGH since it removes the inverter
		 */
		/* MASSMEMORY_EN: XMDMDATA_6: GPE3[6] */
		s5p_gpio_direction_output(&gpio1->e3, 6, 0);
		break;
	default:
		/*
		 * Default reset state is High and there's no inverter
		 * But set it as HIGH to ensure
		 */
		/* MASSMEMORY_EN: XMDMADDR_3: GPE1[3] */
		s5p_gpio_direction_output(&gpio1->e1, 3, 1);
		break;
	}

#ifdef CONFIG_SOFT_SPI
	soft_spi_init();
#endif
	check_hw_revision();
	printf("HW Revision:\t0x%x\n", board_rev);

	return 0;
}

void exynos_lcd_misc_init(vidinfo_t *vid)
{
#ifdef CONFIG_TIZEN
	get_tizen_logo_info(vid);
#endif

	/* for LD9040. */
	vid->pclk_name = 1;	/* MPLL */
	vid->sclk_div = 1;

	setenv("lcdinfo", "lcd=ld9040");
}
