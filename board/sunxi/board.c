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
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>

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
		/* CMD-PH22, CLK-PH23, D0~D3-PH24~27 : 5 */
		for (pin = SUNXI_GPH(22); pin <= SUNXI_GPH(27); pin++) {
			sunxi_gpio_set_cfgpin(pin, SUN4I_GPH22_SDC1);
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
	mmc_pinmux_setup(CONFIG_MMC_SUNXI_SLOT);
	sunxi_mmc_init(CONFIG_MMC_SUNXI_SLOT);
#if !defined (CONFIG_SPL_BUILD) && defined (CONFIG_MMC_SUNXI_SLOT_EXTRA)
	mmc_pinmux_setup(CONFIG_MMC_SUNXI_SLOT_EXTRA);
	sunxi_mmc_init(CONFIG_MMC_SUNXI_SLOT_EXTRA);
#endif

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
void sunxi_board_init(void)
{
	unsigned long ramsize;

	printf("DRAM:");
	ramsize = sunxi_dram_init();
	printf(" %lu MiB\n", ramsize >> 20);
	if (!ramsize)
		hang();
}
#endif
