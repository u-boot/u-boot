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
#ifdef CONFIG_AXP152_POWER
#include <axp152.h>
#endif
#ifdef CONFIG_AXP209_POWER
#include <axp209.h>
#endif
#ifdef CONFIG_AXP221_POWER
#include <axp221.h>
#endif
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/display.h>
#include <asm/arch/dram.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/usbc.h>
#include <asm/io.h>
#include <linux/usb/musb.h>
#include <net.h>

#if defined CONFIG_VIDEO_LCD_PANEL_I2C && !(defined CONFIG_SPL_BUILD)
/* So that we can use pin names in Kconfig and sunxi_name_to_gpio() */
int soft_i2c_gpio_sda;
int soft_i2c_gpio_scl;
#endif

DECLARE_GLOBAL_DATA_PTR;

/* add board specific code here */
int board_init(void)
{
	int id_pfr1;

	gd->bd->bi_boot_params = (PHYS_SDRAM_0 + 0x100);

	asm volatile("mrc p15, 0, %0, c0, c1, 1" : "=r"(id_pfr1));
	debug("id_pfr1: 0x%08x\n", id_pfr1);
	/* Generic Timer Extension available? */
	if ((id_pfr1 >> 16) & 0xf) {
		debug("Setting CNTFRQ\n");
		/* CNTFRQ == 24 MHz */
		asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r"(24000000));
	}

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_0, PHYS_SDRAM_0_SIZE);

	return 0;
}

#ifdef CONFIG_GENERIC_MMC
static void mmc_pinmux_setup(int sdc)
{
	unsigned int pin;

	switch (sdc) {
	case 0:
		/* D1-PF0, D0-PF1, CLK-PF2, CMD-PF3, D3-PF4, D4-PF5 */
		for (pin = SUNXI_GPF(0); pin <= SUNXI_GPF(5); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPF0_SDC0);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
		break;

	case 1:
		/* CMD-PG3, CLK-PG4, D0~D3-PG5-8 */
		for (pin = SUNXI_GPG(3); pin <= SUNXI_GPG(8); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUN5I_GPG3_SDC1);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
		break;

	case 2:
		/* CMD-PC6, CLK-PC7, D0-PC8, D1-PC9, D2-PC10, D3-PC11 */
		for (pin = SUNXI_GPC(6); pin <= SUNXI_GPC(11); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUNXI_GPC6_SDC2);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
		break;

	case 3:
		/* CMD-PI4, CLK-PI5, D0~D3-PI6~9 : 2 */
		for (pin = SUNXI_GPI(4); pin <= SUNXI_GPI(9); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUN4I_GPI4_SDC3);
			sunxi_gpio_set_pull(pin, SUNXI_GPIO_PULL_UP);
			sunxi_gpio_set_drv(pin, 2);
		}
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

#if CONFIG_MMC_SUNXI_SLOT == 0 && CONFIG_MMC_SUNXI_SLOT_EXTRA == 2
	/*
	 * Both mmc0 and mmc2 are bootable, figure out where we're booting
	 * from. Try mmc0 first, just like the brom does.
	 */
	if (mmc_getcd(mmc0) && mmc_init(mmc0) == 0 &&
	    mmc0->block_dev.block_read(0, 16, 1, buf) == 1) {
		buf[12] = 0;
		if (strcmp(&buf[4], "eGON.BT0") == 0)
			return 0;
	}

	/* no bootable card in mmc0, so we must be booting from mmc2, swap */
	mmc0->block_dev.dev = 1;
	mmc1->block_dev.dev = 0;
#endif

	return 0;
}
#endif

void i2c_init_board(void)
{
	sunxi_gpio_set_cfgpin(SUNXI_GPB(0), SUNXI_GPB0_TWI0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(1), SUNXI_GPB0_TWI0);
	clock_twi_onoff(0, 1);
#if defined CONFIG_VIDEO_LCD_PANEL_I2C && !(defined CONFIG_SPL_BUILD)
	soft_i2c_gpio_sda = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_PANEL_I2C_SDA);
	soft_i2c_gpio_scl = sunxi_name_to_gpio(CONFIG_VIDEO_LCD_PANEL_I2C_SCL);
#endif
}

#ifdef CONFIG_SPL_BUILD
void sunxi_board_init(void)
{
	int power_failed = 0;
	unsigned long ramsize;

#ifdef CONFIG_AXP152_POWER
	power_failed = axp152_init();
	power_failed |= axp152_set_dcdc2(1400);
	power_failed |= axp152_set_dcdc3(1500);
	power_failed |= axp152_set_dcdc4(1250);
	power_failed |= axp152_set_ldo2(3000);
#endif
#ifdef CONFIG_AXP209_POWER
	power_failed |= axp209_init();
	power_failed |= axp209_set_dcdc2(1400);
	power_failed |= axp209_set_dcdc3(1250);
	power_failed |= axp209_set_ldo2(3000);
	power_failed |= axp209_set_ldo3(2800);
	power_failed |= axp209_set_ldo4(2800);
#endif
#ifdef CONFIG_AXP221_POWER
	power_failed = axp221_init();
	power_failed |= axp221_set_dcdc1(CONFIG_AXP221_DCDC1_VOLT);
	power_failed |= axp221_set_dcdc2(1200); /* A31:VDD-GPU, A23:VDD-SYS */
	power_failed |= axp221_set_dcdc3(1200); /* VDD-CPU */
#ifdef CONFIG_MACH_SUN6I
	power_failed |= axp221_set_dcdc4(1200); /* A31:VDD-SYS */
#else
	power_failed |= axp221_set_dcdc4(0);    /* A23:unused */
#endif
	power_failed |= axp221_set_dcdc5(1500); /* VCC-DRAM */
	power_failed |= axp221_set_dldo1(CONFIG_AXP221_DLDO1_VOLT);
	power_failed |= axp221_set_dldo4(CONFIG_AXP221_DLDO4_VOLT);
	power_failed |= axp221_set_aldo1(CONFIG_AXP221_ALDO1_VOLT);
	power_failed |= axp221_set_aldo2(CONFIG_AXP221_ALDO2_VOLT);
	power_failed |= axp221_set_aldo3(CONFIG_AXP221_ALDO3_VOLT);
	power_failed |= axp221_set_eldo(3, CONFIG_AXP221_ELDO3_VOLT);
#endif

	printf("DRAM:");
	ramsize = sunxi_dram_init();
	printf(" %lu MiB\n", ramsize >> 20);
	if (!ramsize)
		hang();

	/*
	 * Only clock up the CPU to full speed if we are reasonably
	 * assured it's being powered with suitable core voltage
	 */
	if (!power_failed)
		clock_set_pll1(CONFIG_CLK_FULL_SPEED);
	else
		printf("Failed to set core voltage! Can't set CPU frequency\n");
}
#endif

#if defined(CONFIG_MUSB_HOST) || defined(CONFIG_MUSB_GADGET)
static struct musb_hdrc_config musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 6,
	.ram_bits       = 11,
};

static struct musb_hdrc_platform_data musb_plat = {
#if defined(CONFIG_MUSB_HOST)
	.mode           = MUSB_HOST,
#else
	.mode		= MUSB_PERIPHERAL,
#endif
	.config         = &musb_config,
	.power          = 250,
	.platform_ops	= &sunxi_musb_ops,
};
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	unsigned int sid[4];

	if (!getenv("ethaddr") && sunxi_get_sid(sid) == 0 &&
			sid[0] != 0 && sid[3] != 0) {
		uint8_t mac_addr[6];

		mac_addr[0] = 0x02; /* Non OUI / registered MAC address */
		mac_addr[1] = (sid[0] >>  0) & 0xff;
		mac_addr[2] = (sid[3] >> 24) & 0xff;
		mac_addr[3] = (sid[3] >> 16) & 0xff;
		mac_addr[4] = (sid[3] >>  8) & 0xff;
		mac_addr[5] = (sid[3] >>  0) & 0xff;

		eth_setenv_enetaddr("ethaddr", mac_addr);
	}

#if defined(CONFIG_MUSB_HOST) || defined(CONFIG_MUSB_GADGET)
	musb_register(&musb_plat, NULL, (void *)SUNXI_USB0_BASE);
#endif
	return 0;
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_VIDEO_DT_SIMPLEFB
	return sunxi_simplefb_setup(blob);
#endif
}
#endif /* CONFIG_OF_BOARD_SETUP */
