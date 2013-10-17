/*
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <nand.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/arch/nand_defs.h>
#include <asm/arch/davinci_misc.h>
#include <net.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	struct davinci_gpio *gpio01_base =
			(struct davinci_gpio *)DAVINCI_GPIO_BANK01;
	struct davinci_gpio *gpio23_base =
			(struct davinci_gpio *)DAVINCI_GPIO_BANK23;
	struct davinci_gpio *gpio67_base =
			(struct davinci_gpio *)DAVINCI_GPIO_BANK67;

	gd->bd->bi_arch_number = MACH_TYPE_DM355_LEOPARD;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	/* GIO 9 & 10 are used for IO */
	writel((readl(PINMUX3) & 0XF8FFFFFF), PINMUX3);

	/* Interrupt set GIO 9 */
	writel((readl(DAVINCI_GPIO_BINTEN) | 0x1), DAVINCI_GPIO_BINTEN);

	/* set GIO 9 input */
	writel((readl(&gpio01_base->dir) | (1 << 9)), &gpio01_base->dir);

	/* Both edge trigger GIO 9 */
	writel((readl(&gpio01_base->set_rising) | (1 << 9)),
						&gpio01_base->set_rising);
	writel((readl(&gpio01_base->dir) & ~(1 << 5)), &gpio01_base->dir);

	/* output low */
	writel((readl(&gpio01_base->set_data) & ~(1 << 5)),
						&gpio01_base->set_data);

	/* set GIO 10 output */
	writel((readl(&gpio01_base->dir) & ~(1 << 10)), &gpio01_base->dir);

	/* output high */
	writel((readl(&gpio01_base->set_data) | (1 << 10)),
						&gpio01_base->set_data);

	/* set GIO 32 output */
	writel((readl(&gpio23_base->dir) & ~(1 << 0)), &gpio23_base->dir);

	/* output High */
	writel((readl(&gpio23_base->set_data) | (1 << 0)),
						&gpio23_base->set_data);

	/* Enable UART1 MUX Lines */
	writel((readl(PINMUX0) & ~3), PINMUX0);
	writel((readl(&gpio67_base->dir) & ~(1 << 6)), &gpio67_base->dir);
	writel((readl(&gpio67_base->set_data) | (1 << 6)),
						&gpio67_base->set_data);

	return 0;
}

#ifdef CONFIG_DRIVER_DM9000
int board_eth_init(bd_t *bis)
{
	return dm9000_initialize(bis);
}
#endif

#ifdef CONFIG_NAND_DAVINCI
int board_nand_init(struct nand_chip *nand)
{
	davinci_nand_init(nand);

	return 0;
}
#endif
