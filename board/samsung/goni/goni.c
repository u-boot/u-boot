/*
 *  Copyright (C) 2008-2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <power/pmic.h>
#include <usb/s3c_udc.h>
#include <asm/arch/cpu.h>
#include <power/max8998_pmic.h>
DECLARE_GLOBAL_DATA_PTR;

static struct s5pc110_gpio *s5pc110_gpio;

int board_init(void)
{
	/* Set Initial global variables */
	s5pc110_gpio = (struct s5pc110_gpio *)S5PC110_GPIO_BASE;

	gd->bd->bi_arch_number = MACH_TYPE_GONI;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int power_init_board(void)
{
	int ret;

	/*
	 * For PMIC the I2C bus is named as I2C5, but it is connected
	 * to logical I2C adapter 0
	 */
	ret = pmic_init(I2C_0);
	if (ret)
		return ret;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE + PHYS_SDRAM_2_SIZE +
			PHYS_SDRAM_3_SIZE;

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
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board:\tGoni\n");
	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int i, ret, ret_sd = 0;

	/* MASSMEMORY_EN: XMSMDATA7: GPJ2[7] output high */
	s5p_gpio_direction_output(&s5pc110_gpio->j2, 7, 1);

	/*
	 * MMC0 GPIO
	 * GPG0[0]	SD_0_CLK
	 * GPG0[1]	SD_0_CMD
	 * GPG0[2]	SD_0_CDn	-> Not used
	 * GPG0[3:6]	SD_0_DATA[0:3]
	 */
	for (i = 0; i < 7; i++) {
		if (i == 2)
			continue;
		/* GPG0[0:6] special function 2 */
		s5p_gpio_cfg_pin(&s5pc110_gpio->g0, i, 0x2);
		/* GPG0[0:6] pull disable */
		s5p_gpio_set_pull(&s5pc110_gpio->g0, i, GPIO_PULL_NONE);
		/* GPG0[0:6] drv 4x */
		s5p_gpio_set_drv(&s5pc110_gpio->g0, i, GPIO_DRV_4X);
	}

	ret = s5p_mmc_init(0, 4);
	if (ret)
		error("MMC: Failed to init MMC:0.\n");

	/*
	 * SD card (T_FLASH) detect and init
	 * T_FLASH_DETECT: EINT28: GPH3[4] input mode
	 */
	s5p_gpio_cfg_pin(&s5pc110_gpio->h3, 4, GPIO_INPUT);
	s5p_gpio_set_pull(&s5pc110_gpio->h3, 4, GPIO_PULL_UP);

	if (!s5p_gpio_get_value(&s5pc110_gpio->h3, 4)) {
		for (i = 0; i < 7; i++) {
			if (i == 2)
				continue;

			/* GPG2[0:6] special function 2 */
			s5p_gpio_cfg_pin(&s5pc110_gpio->g2, i, 0x2);
			/* GPG2[0:6] pull disable */
			s5p_gpio_set_pull(&s5pc110_gpio->g2, i, GPIO_PULL_NONE);
			/* GPG2[0:6] drv 4x */
			s5p_gpio_set_drv(&s5pc110_gpio->g2, i, GPIO_DRV_4X);
		}

		ret_sd = s5p_mmc_init(2, 4);
		if (ret_sd)
			error("MMC: Failed to init SD card (MMC:2).\n");
	}

	return ret & ret_sd;
}
#endif

#ifdef CONFIG_USB_GADGET
static int s5pc1xx_phy_control(int on)
{
	int ret;
	static int status;
	struct pmic *p = pmic_get("MAX8998_PMIC");
	if (!p)
		return -ENODEV;

	if (pmic_probe(p))
		return -1;

	if (on && !status) {
		ret = pmic_set_output(p, MAX8998_REG_ONOFF1,
				      MAX8998_LDO3, LDO_ON);
		ret = pmic_set_output(p, MAX8998_REG_ONOFF2,
				      MAX8998_LDO8, LDO_ON);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -1;
		}
		status = 1;
	} else if (!on && status) {
		ret = pmic_set_output(p, MAX8998_REG_ONOFF1,
				      MAX8998_LDO3, LDO_OFF);
		ret = pmic_set_output(p, MAX8998_REG_ONOFF2,
				      MAX8998_LDO8, LDO_OFF);
		if (ret) {
			puts("MAX8998 LDO setting error!\n");
			return -1;
		}
		status = 0;
	}
	udelay(10000);

	return 0;
}

struct s3c_plat_otg_data s5pc110_otg_data = {
	.phy_control = s5pc1xx_phy_control,
	.regs_phy = S5PC110_PHY_BASE,
	.regs_otg = S5PC110_OTG_BASE,
	.usb_phy_ctrl = S5PC110_USB_PHY_CONTROL,
};
#endif
