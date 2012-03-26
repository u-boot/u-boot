/*
 *  Copyright (C) 2010 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
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
#include <asm/io.h>
#include <asm/arch/adc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <pmic.h>
#include <usb/s3c_udc.h>
#include <asm/arch/cpu.h>
#include <max8998_pmic.h>

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

static void check_hw_revision(void);

int board_init(void)
{
	gpio1 = (struct exynos4_gpio_part1 *) EXYNOS4_GPIO_PART1_BASE;
	gpio2 = (struct exynos4_gpio_part2 *) EXYNOS4_GPIO_PART2_BASE;

	gd->bd->bi_arch_number = MACH_TYPE_UNIVERSAL_C210;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

#if defined(CONFIG_PMIC)
	pmic_init();
#endif

	check_hw_revision();
	printf("HW Revision:\t0x%x\n", board_rev);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE) +
		get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE);

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
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
	struct pmic *p = get_pmic();

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

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board:\tUniversal C210\n");
	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int i, err;

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

	/*
	 * eMMC GPIO:
	 * SDR 8-bit@48MHz at MMC0
	 * GPK0[0]	SD_0_CLK(2)
	 * GPK0[1]	SD_0_CMD(2)
	 * GPK0[2]	SD_0_CDn	-> Not used
	 * GPK0[3:6]	SD_0_DATA[0:3](2)
	 * GPK1[3:6]	SD_0_DATA[0:3](3)
	 *
	 * DDR 4-bit@26MHz at MMC4
	 * GPK0[0]	SD_4_CLK(3)
	 * GPK0[1]	SD_4_CMD(3)
	 * GPK0[2]	SD_4_CDn	-> Not used
	 * GPK0[3:6]	SD_4_DATA[0:3](3)
	 * GPK1[3:6]	SD_4_DATA[4:7](4)
	 */
	for (i = 0; i < 7; i++) {
		if (i == 2)
			continue;
		/* GPK0[0:6] special function 2 */
		s5p_gpio_cfg_pin(&gpio2->k0, i, 0x2);
		/* GPK0[0:6] pull disable */
		s5p_gpio_set_pull(&gpio2->k0, i, GPIO_PULL_NONE);
		/* GPK0[0:6] drv 4x */
		s5p_gpio_set_drv(&gpio2->k0, i, GPIO_DRV_4X);
	}

	for (i = 3; i < 7; i++) {
		/* GPK1[3:6] special function 3 */
		s5p_gpio_cfg_pin(&gpio2->k1, i, 0x3);
		/* GPK1[3:6] pull disable */
		s5p_gpio_set_pull(&gpio2->k1, i, GPIO_PULL_NONE);
		/* GPK1[3:6] drv 4x */
		s5p_gpio_set_drv(&gpio2->k1, i, GPIO_DRV_4X);
	}

	/* T-flash detect */
	s5p_gpio_cfg_pin(&gpio2->x3, 4, 0xf);
	s5p_gpio_set_pull(&gpio2->x3, 4, GPIO_PULL_UP);

	/*
	 * MMC device init
	 * mmc0	 : eMMC (8-bit buswidth)
	 * mmc2	 : SD card (4-bit buswidth)
	 */
	err = s5p_mmc_init(0, 8);

	/*
	 * Check the T-flash  detect pin
	 * GPX3[4] T-flash detect pin
	 */
	if (!s5p_gpio_get_value(&gpio2->x3, 4)) {
		/*
		 * SD card GPIO:
		 * GPK2[0]	SD_2_CLK(2)
		 * GPK2[1]	SD_2_CMD(2)
		 * GPK2[2]	SD_2_CDn	-> Not used
		 * GPK2[3:6]	SD_2_DATA[0:3](2)
		 */
		for (i = 0; i < 7; i++) {
			if (i == 2)
				continue;
			/* GPK2[0:6] special function 2 */
			s5p_gpio_cfg_pin(&gpio2->k2, i, 0x2);
			/* GPK2[0:6] pull disable */
			s5p_gpio_set_pull(&gpio2->k2, i, GPIO_PULL_NONE);
			/* GPK2[0:6] drv 4x */
			s5p_gpio_set_drv(&gpio2->k2, i, GPIO_DRV_4X);
		}
		err = s5p_mmc_init(2, 4);
	}

	return err;

}
#endif

#ifdef CONFIG_USB_GADGET
static int s5pc210_phy_control(int on)
{
	int ret = 0;
	struct pmic *p = get_pmic();

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
