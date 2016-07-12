/*
 * (C) Copyright 2012-2013 Henrik Nordstrom <henrik@henriknordstrom.net>
 * (C) Copyright 2013 Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Some board init for the Allwinner A10-evb board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <axp_pmic.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/display.h>
#include <asm/arch/dram.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/usb_phy.h>
#ifndef CONFIG_ARM64
#include <asm/armv7.h>
#endif
#include <asm/gpio.h>
#include <asm/io.h>
#include <nand.h>
#include <net.h>
#include <sy8106a.h>

#if defined CONFIG_VIDEO_LCD_PANEL_I2C && !(defined CONFIG_SPL_BUILD)
/* So that we can use pin names in Kconfig and sunxi_name_to_gpio() */
int soft_i2c_gpio_sda;
int soft_i2c_gpio_scl;

static int soft_i2c_board_init(void)
{
	int ret;

	soft_i2c_gpio_sda = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_PANEL_I2C_SDA);
	if (soft_i2c_gpio_sda < 0) {
		printf("Error invalid soft i2c sda pin: '%s', err %d\n",
		       CONFIG_VIDEO_LCD_PANEL_I2C_SDA, soft_i2c_gpio_sda);
		return soft_i2c_gpio_sda;
	}
	ret = gpio_request(soft_i2c_gpio_sda, "soft-i2c-sda");
	if (ret) {
		printf("Error requesting soft i2c sda pin: '%s', err %d\n",
		       CONFIG_VIDEO_LCD_PANEL_I2C_SDA, ret);
		return ret;
	}

	soft_i2c_gpio_scl = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_PANEL_I2C_SCL);
	if (soft_i2c_gpio_scl < 0) {
		printf("Error invalid soft i2c scl pin: '%s', err %d\n",
		       CONFIG_VIDEO_LCD_PANEL_I2C_SCL, soft_i2c_gpio_scl);
		return soft_i2c_gpio_scl;
	}
	ret = gpio_request(soft_i2c_gpio_scl, "soft-i2c-scl");
	if (ret) {
		printf("Error requesting soft i2c scl pin: '%s', err %d\n",
		       CONFIG_VIDEO_LCD_PANEL_I2C_SCL, ret);
		return ret;
	}

	return 0;
}
#else
static int soft_i2c_board_init(void) { return 0; }
#endif

DECLARE_GLOBAL_DATA_PTR;

/* add board specific code here */
int board_init(void)
{
	__maybe_unused int id_pfr1, ret;

	gd->bd->bi_boot_params = (PHYS_SDRAM_0 + 0x100);

#ifndef CONFIG_ARM64
	asm volatile("mrc p15, 0, %0, c0, c1, 1" : "=r"(id_pfr1));
	debug("id_pfr1: 0x%08x\n", id_pfr1);
	/* Generic Timer Extension available? */
	if ((id_pfr1 >> CPUID_ARM_GENTIMER_SHIFT) & 0xf) {
		uint32_t freq;

		debug("Setting CNTFRQ\n");

		/*
		 * CNTFRQ is a secure register, so we will crash if we try to
		 * write this from the non-secure world (read is OK, though).
		 * In case some bootcode has already set the correct value,
		 * we avoid the risk of writing to it.
		 */
		asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r"(freq));
		if (freq != CONFIG_TIMER_CLK_FREQ) {
			debug("arch timer frequency is %d Hz, should be %d, fixing ...\n",
			      freq, CONFIG_TIMER_CLK_FREQ);
#ifdef CONFIG_NON_SECURE
			printf("arch timer frequency is wrong, but cannot adjust it\n");
#else
			asm volatile("mcr p15, 0, %0, c14, c0, 0"
				     : : "r"(CONFIG_TIMER_CLK_FREQ));
#endif
		}
	}
#endif /* !CONFIG_ARM64 */

	ret = axp_gpio_init();
	if (ret)
		return ret;

#ifdef CONFIG_SATAPWR
	gpio_request(CONFIG_SATAPWR, "satapwr");
	gpio_direction_output(CONFIG_SATAPWR, 1);
#endif
#ifdef CONFIG_MACPWR
	gpio_request(CONFIG_MACPWR, "macpwr");
	gpio_direction_output(CONFIG_MACPWR, 1);
#endif

	/* Uses dm gpio code so do this here and not in i2c_init_board() */
	return soft_i2c_board_init();
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_0, PHYS_SDRAM_0_SIZE);

	return 0;
}

#if defined(CONFIG_NAND_SUNXI) && defined(CONFIG_SPL_BUILD)
static void nand_pinmux_setup(void)
{
	unsigned int pin;

	for (pin = SUNXI_GPC(0); pin <= SUNXI_GPC(19); pin++)
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_NAND);

#if defined CONFIG_MACH_SUN4I || defined CONFIG_MACH_SUN7I
	for (pin = SUNXI_GPC(20); pin <= SUNXI_GPC(22); pin++)
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_NAND);
#endif
	/* sun4i / sun7i do have a PC23, but it is not used for nand,
	 * only sun7i has a PC24 */
#ifdef CONFIG_MACH_SUN7I
	sunxi_gpio_set_cfgpin(SUNXI_GPC(24), SUNXI_GPC_NAND);
#endif
}

static void nand_clock_setup(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	setbits_le32(&ccm->ahb_gate0, (CLK_GATE_OPEN << AHB_GATE_OFFSET_NAND0));
#ifdef CONFIG_MACH_SUN9I
	setbits_le32(&ccm->ahb_gate1, (1 << AHB_GATE_OFFSET_DMA));
#else
	setbits_le32(&ccm->ahb_gate0, (1 << AHB_GATE_OFFSET_DMA));
#endif
	setbits_le32(&ccm->nand0_clk_cfg, CCM_NAND_CTRL_ENABLE | AHB_DIV_1);
}

void board_nand_init(void)
{
	nand_pinmux_setup();
	nand_clock_setup();
}
#endif

#ifdef CONFIG_GENERIC_MMC
static void mmc_pinmux_setup(int sdc)
{
	unsigned int pin;
	__maybe_unused int pins;

	switch (sdc) {
	case 0:
		/* SDC0: PF0-PF5 */
		for (pin = SUNXI_GPF(0); pin <= SUNXI_GPF(5); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPF_SDC0);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
		break;

	case 1:
		pins = sunxi_name_to_gpio_bank(CONFIG_MMC1_PINS);

#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN7I)
		if (pins == SUNXI_GPIO_H) {
			/* SDC1: PH22-PH-27 */
			for (pin = SUNXI_GPH(22); pin <= SUNXI_GPH(27); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUN4I_GPH_SDC1);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}
		} else {
			/* SDC1: PG0-PG5 */
			for (pin = SUNXI_GPG(0); pin <= SUNXI_GPG(5); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUN4I_GPG_SDC1);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}
		}
#elif defined(CONFIG_MACH_SUN5I)
		/* SDC1: PG3-PG8 */
		for (pin = SUNXI_GPG(3); pin <= SUNXI_GPG(8); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUN5I_GPG_SDC1);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
#elif defined(CONFIG_MACH_SUN6I)
		/* SDC1: PG0-PG5 */
		for (pin = SUNXI_GPG(0); pin <= SUNXI_GPG(5); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUN6I_GPG_SDC1);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
#elif defined(CONFIG_MACH_SUN8I)
		if (pins == SUNXI_GPIO_D) {
			/* SDC1: PD2-PD7 */
			for (pin = SUNXI_GPD(2); pin <= SUNXI_GPD(7); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUN8I_GPD_SDC1);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}
		} else {
			/* SDC1: PG0-PG5 */
			for (pin = SUNXI_GPG(0); pin <= SUNXI_GPG(5); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUN8I_GPG_SDC1);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}
		}
#endif
		break;

	case 2:
		pins = sunxi_name_to_gpio_bank(CONFIG_MMC2_PINS);

#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN7I)
		/* SDC2: PC6-PC11 */
		for (pin = SUNXI_GPC(6); pin <= SUNXI_GPC(11); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_SDC2);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
#elif defined(CONFIG_MACH_SUN5I)
		if (pins == SUNXI_GPIO_E) {
			/* SDC2: PE4-PE9 */
			for (pin = SUNXI_GPE(4); pin <= SUNXI_GPD(9); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUN5I_GPE_SDC2);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}
		} else {
			/* SDC2: PC6-PC15 */
			for (pin = SUNXI_GPC(6); pin <= SUNXI_GPC(15); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_SDC2);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}
		}
#elif defined(CONFIG_MACH_SUN6I)
		if (pins == SUNXI_GPIO_A) {
			/* SDC2: PA9-PA14 */
			for (pin = SUNXI_GPA(9); pin <= SUNXI_GPA(14); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_SDC2);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}
		} else {
			/* SDC2: PC6-PC15, PC24 */
			for (pin = SUNXI_GPC(6); pin <= SUNXI_GPC(15); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_SDC2);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}

			sunxi_gpio_set_cfgpin(SUNXI_GPC(24), SUNXI_GPC_SDC2);
			sunxi_gpio_set_pull(SUNXI_GPC(24), SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(SUNXI_GPC(24), 2);
		}
#elif defined(CONFIG_MACH_SUN8I) || defined(CONFIG_MACH_SUN50I)
		/* SDC2: PC5-PC6, PC8-PC16 */
		for (pin = SUNXI_GPC(5); pin <= SUNXI_GPC(6); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_SDC2);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}

		for (pin = SUNXI_GPC(8); pin <= SUNXI_GPC(16); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPC_SDC2);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
#endif
		break;

	case 3:
		pins = sunxi_name_to_gpio_bank(CONFIG_MMC3_PINS);

#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN7I)
		/* SDC3: PI4-PI9 */
		for (pin = SUNXI_GPI(4); pin <= SUNXI_GPI(9); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPI_SDC3);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
#elif defined(CONFIG_MACH_SUN6I)
		if (pins == SUNXI_GPIO_A) {
			/* SDC3: PA9-PA14 */
			for (pin = SUNXI_GPA(9); pin <= SUNXI_GPA(14); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUN6I_GPA_SDC3);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}
		} else {
			/* SDC3: PC6-PC15, PC24 */
			for (pin = SUNXI_GPC(6); pin <= SUNXI_GPC(15); pin++) {
				sunxi_gpio_set_cfgpin(pin, SUN6I_GPC_SDC3);
				sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
				sunxi_gpio_set_drv(pin, 2);
			}

			sunxi_gpio_set_cfgpin(SUNXI_GPC(24), SUN6I_GPC_SDC3);
			sunxi_gpio_set_pull(SUNXI_GPC(24), SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(SUNXI_GPC(24), 2);
		}
#endif
		break;

	default:
		printf("sunxi: invalid MMC slot %d for pinmux setup\n", sdc);
		break;
	}
}

int board_mmc_init(bd_t *bis)
{
	__maybe_unused struct mmc *mmc0, *mmc1;
	__maybe_unused char buf[512];

	mmc_pinmux_setup(CONFIG_MMC_SUNXI_SLOT);
	mmc0 = sunxi_mmc_init(CONFIG_MMC_SUNXI_SLOT);
	if (!mmc0)
		return -1;

#if CONFIG_MMC_SUNXI_SLOT_EXTRA != -1
	mmc_pinmux_setup(CONFIG_MMC_SUNXI_SLOT_EXTRA);
	mmc1 = sunxi_mmc_init(CONFIG_MMC_SUNXI_SLOT_EXTRA);
	if (!mmc1)
		return -1;
#endif

#if !defined(CONFIG_SPL_BUILD) && CONFIG_MMC_SUNXI_SLOT_EXTRA == 2
	/*
	 * On systems with an emmc (mmc2), figure out if we are booting from
	 * the emmc and if we are make it "mmc dev 0" so that boot.scr, etc.
	 * are searched there first. Note we only do this for u-boot proper,
	 * not for the SPL, see spl_boot_device().
	 */
	if (!sunxi_mmc_has_egon_boot_signature(mmc0) &&
	    sunxi_mmc_has_egon_boot_signature(mmc1)) {
		/* Booting from emmc / mmc2, swap */
		mmc0->block_dev.devnum = 1;
		mmc1->block_dev.devnum = 0;
	}
#endif

	return 0;
}
#endif

void i2c_init_board(void)
{
#ifdef CONFIG_I2C0_ENABLE
#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN5I) || defined(CONFIG_MACH_SUN7I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(0), SUN4I_GPB_TWI0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(1), SUN4I_GPB_TWI0);
	clock_twi_onoff(0, 1);
#elif defined(CONFIG_MACH_SUN6I)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(14), SUN6I_GPH_TWI0);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(15), SUN6I_GPH_TWI0);
	clock_twi_onoff(0, 1);
#elif defined(CONFIG_MACH_SUN8I)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(2), SUN8I_GPH_TWI0);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(3), SUN8I_GPH_TWI0);
	clock_twi_onoff(0, 1);
#endif
#endif

#ifdef CONFIG_I2C1_ENABLE
#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN7I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(18), SUN4I_GPB_TWI1);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(19), SUN4I_GPB_TWI1);
	clock_twi_onoff(1, 1);
#elif defined(CONFIG_MACH_SUN5I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(15), SUN5I_GPB_TWI1);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(16), SUN5I_GPB_TWI1);
	clock_twi_onoff(1, 1);
#elif defined(CONFIG_MACH_SUN6I)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(16), SUN6I_GPH_TWI1);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(17), SUN6I_GPH_TWI1);
	clock_twi_onoff(1, 1);
#elif defined(CONFIG_MACH_SUN8I)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(4), SUN8I_GPH_TWI1);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(5), SUN8I_GPH_TWI1);
	clock_twi_onoff(1, 1);
#endif
#endif

#ifdef CONFIG_I2C2_ENABLE
#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN7I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(20), SUN4I_GPB_TWI2);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(21), SUN4I_GPB_TWI2);
	clock_twi_onoff(2, 1);
#elif defined(CONFIG_MACH_SUN5I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(17), SUN5I_GPB_TWI2);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(18), SUN5I_GPB_TWI2);
	clock_twi_onoff(2, 1);
#elif defined(CONFIG_MACH_SUN6I)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(18), SUN6I_GPH_TWI2);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(19), SUN6I_GPH_TWI2);
	clock_twi_onoff(2, 1);
#elif defined(CONFIG_MACH_SUN8I)
	sunxi_gpio_set_cfgpin(SUNXI_GPE(12), SUN8I_GPE_TWI2);
	sunxi_gpio_set_cfgpin(SUNXI_GPE(13), SUN8I_GPE_TWI2);
	clock_twi_onoff(2, 1);
#endif
#endif

#ifdef CONFIG_I2C3_ENABLE
#if defined(CONFIG_MACH_SUN6I)
	sunxi_gpio_set_cfgpin(SUNXI_GPG(10), SUN6I_GPG_TWI3);
	sunxi_gpio_set_cfgpin(SUNXI_GPG(11), SUN6I_GPG_TWI3);
	clock_twi_onoff(3, 1);
#elif defined(CONFIG_MACH_SUN7I)
	sunxi_gpio_set_cfgpin(SUNXI_GPI(0), SUN7I_GPI_TWI3);
	sunxi_gpio_set_cfgpin(SUNXI_GPI(1), SUN7I_GPI_TWI3);
	clock_twi_onoff(3, 1);
#endif
#endif

#ifdef CONFIG_I2C4_ENABLE
#if defined(CONFIG_MACH_SUN7I)
	sunxi_gpio_set_cfgpin(SUNXI_GPI(2), SUN7I_GPI_TWI4);
	sunxi_gpio_set_cfgpin(SUNXI_GPI(3), SUN7I_GPI_TWI4);
	clock_twi_onoff(4, 1);
#endif
#endif

#ifdef CONFIG_R_I2C_ENABLE
	clock_twi_onoff(5, 1);
	sunxi_gpio_set_cfgpin(SUNXI_GPL(0), SUN8I_H3_GPL_R_TWI);
	sunxi_gpio_set_cfgpin(SUNXI_GPL(1), SUN8I_H3_GPL_R_TWI);
#endif
}

#ifdef CONFIG_SPL_BUILD
void sunxi_board_init(void)
{
	int power_failed = 0;
	unsigned long ramsize;

#ifdef CONFIG_SY8106A_POWER
	power_failed = sy8106a_set_vout1(CONFIG_SY8106A_VOUT1_VOLT);
#endif

#if defined CONFIG_AXP152_POWER || defined CONFIG_AXP209_POWER || \
	defined CONFIG_AXP221_POWER || defined CONFIG_AXP809_POWER || \
	defined CONFIG_AXP818_POWER
	power_failed = axp_init();

#if defined CONFIG_AXP221_POWER || defined CONFIG_AXP809_POWER || \
	defined CONFIG_AXP818_POWER
	power_failed |= axp_set_dcdc1(CONFIG_AXP_DCDC1_VOLT);
#endif
	power_failed |= axp_set_dcdc2(CONFIG_AXP_DCDC2_VOLT);
	power_failed |= axp_set_dcdc3(CONFIG_AXP_DCDC3_VOLT);
#if !defined(CONFIG_AXP209_POWER) && !defined(CONFIG_AXP818_POWER)
	power_failed |= axp_set_dcdc4(CONFIG_AXP_DCDC4_VOLT);
#endif
#if defined CONFIG_AXP221_POWER || defined CONFIG_AXP809_POWER || \
	defined CONFIG_AXP818_POWER
	power_failed |= axp_set_dcdc5(CONFIG_AXP_DCDC5_VOLT);
#endif

#if defined CONFIG_AXP221_POWER || defined CONFIG_AXP809_POWER || \
	defined CONFIG_AXP818_POWER
	power_failed |= axp_set_aldo1(CONFIG_AXP_ALDO1_VOLT);
#endif
	power_failed |= axp_set_aldo2(CONFIG_AXP_ALDO2_VOLT);
#if !defined(CONFIG_AXP152_POWER)
	power_failed |= axp_set_aldo3(CONFIG_AXP_ALDO3_VOLT);
#endif
#ifdef CONFIG_AXP209_POWER
	power_failed |= axp_set_aldo4(CONFIG_AXP_ALDO4_VOLT);
#endif

#if defined(CONFIG_AXP221_POWER) || defined(CONFIG_AXP809_POWER) || \
	defined(CONFIG_AXP818_POWER)
	power_failed |= axp_set_dldo(1, CONFIG_AXP_DLDO1_VOLT);
	power_failed |= axp_set_dldo(2, CONFIG_AXP_DLDO2_VOLT);
#if !defined CONFIG_AXP809_POWER
	power_failed |= axp_set_dldo(3, CONFIG_AXP_DLDO3_VOLT);
	power_failed |= axp_set_dldo(4, CONFIG_AXP_DLDO4_VOLT);
#endif
	power_failed |= axp_set_eldo(1, CONFIG_AXP_ELDO1_VOLT);
	power_failed |= axp_set_eldo(2, CONFIG_AXP_ELDO2_VOLT);
	power_failed |= axp_set_eldo(3, CONFIG_AXP_ELDO3_VOLT);
#endif

#ifdef CONFIG_AXP818_POWER
	power_failed |= axp_set_fldo(1, CONFIG_AXP_FLDO1_VOLT);
	power_failed |= axp_set_fldo(2, CONFIG_AXP_FLDO2_VOLT);
	power_failed |= axp_set_fldo(3, CONFIG_AXP_FLDO3_VOLT);
#endif

#if defined CONFIG_AXP809_POWER || defined CONFIG_AXP818_POWER
	power_failed |= axp_set_sw(IS_ENABLED(CONFIG_AXP_SW_ON));
#endif
#endif
	printf("DRAM:");
	ramsize = sunxi_dram_init();
	printf(" %d MiB\n", (int)(ramsize >> 20));
	if (!ramsize)
		hang();

	/*
	 * Only clock up the CPU to full speed if we are reasonably
	 * assured it's being powered with suitable core voltage
	 */
	if (!power_failed)
		clock_set_pll1(CONFIG_SYS_CLK_FREQ);
	else
		printf("Failed to set core voltage! Can't set CPU frequency\n");
}
#endif

#ifdef CONFIG_USB_GADGET
int g_dnl_board_usb_cable_connected(void)
{
	return sunxi_usb_phy_vbus_detect(0);
}
#endif

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial_string;
	unsigned long long serial;

	serial_string = getenv("serial#");

	if (serial_string) {
		serial = simple_strtoull(serial_string, NULL, 16);

		serialnr->high = (unsigned int) (serial >> 32);
		serialnr->low = (unsigned int) (serial & 0xffffffff);
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}
#endif

#if !defined(CONFIG_SPL_BUILD)
#include <asm/arch/spl.h>

/*
 * Check the SPL header for the "sunxi" variant. If found: parse values
 * that might have been passed by the loader ("fel" utility), and update
 * the environment accordingly.
 */
static void parse_spl_header(const uint32_t spl_addr)
{
	struct boot_file_head *spl = (void *)(ulong)spl_addr;
	if (memcmp(spl->spl_signature, SPL_SIGNATURE, 3) == 0) {
		uint8_t spl_header_version = spl->spl_signature[3];
		if (spl_header_version == SPL_HEADER_VERSION) {
			if (spl->fel_script_address)
				setenv_hex("fel_scriptaddr",
					   spl->fel_script_address);
			return;
		}
		printf("sunxi SPL version mismatch: expected %u, got %u\n",
		       SPL_HEADER_VERSION, spl_header_version);
	}
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	char serial_string[17] = { 0 };
	unsigned int sid[4];
	uint8_t mac_addr[6];
	int ret;

#if !defined(CONFIG_SPL_BUILD)
	setenv("fel_booted", NULL);
	setenv("fel_scriptaddr", NULL);
	/* determine if we are running in FEL mode */
	if (!is_boot0_magic(SPL_ADDR + 4)) { /* eGON.BT0 */
		setenv("fel_booted", "1");
		parse_spl_header(SPL_ADDR);
	}
#endif

	ret = sunxi_get_sid(sid);
	if (ret == 0 && sid[0] != 0 && sid[3] != 0) {
		if (!getenv("ethaddr")) {
			/* Non OUI / registered MAC address */
			mac_addr[0] = 0x02;
			mac_addr[1] = (sid[0] >>  0) & 0xff;
			mac_addr[2] = (sid[3] >> 24) & 0xff;
			mac_addr[3] = (sid[3] >> 16) & 0xff;
			mac_addr[4] = (sid[3] >>  8) & 0xff;
			mac_addr[5] = (sid[3] >>  0) & 0xff;

			eth_setenv_enetaddr("ethaddr", mac_addr);
		}

		if (!getenv("serial#")) {
			snprintf(serial_string, sizeof(serial_string),
				"%08x%08x", sid[0], sid[3]);

			setenv("serial#", serial_string);
		}
	}

#ifndef CONFIG_MACH_SUN9I
	ret = sunxi_usb_phy_probe();
	if (ret)
		return ret;
#endif
	sunxi_musb_board_init();

	return 0;
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	int __maybe_unused r;

#ifdef CONFIG_VIDEO_DT_SIMPLEFB
	r = sunxi_simplefb_setup(blob);
	if (r)
		return r;
#endif
	return 0;
}
