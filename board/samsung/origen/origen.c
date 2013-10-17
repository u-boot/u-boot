/*
 * Copyright (C) 2011 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/periph.h>
#include <asm/arch/pinmux.h>

DECLARE_GLOBAL_DATA_PTR;
struct exynos4_gpio_part1 *gpio1;
struct exynos4_gpio_part2 *gpio2;

int board_init(void)
{
	gpio1 = (struct exynos4_gpio_part1 *) EXYNOS4_GPIO_PART1_BASE;
	gpio2 = (struct exynos4_gpio_part2 *) EXYNOS4_GPIO_PART2_BASE;

	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100UL);
	return 0;
}

static int board_uart_init(void)
{
	int err;

	err = exynos_pinmux_config(PERIPH_ID_UART0, PINMUX_FLAG_NONE);
	if (err) {
		debug("UART0 not configured\n");
		return err;
	}

	err = exynos_pinmux_config(PERIPH_ID_UART1, PINMUX_FLAG_NONE);
	if (err) {
		debug("UART1 not configured\n");
		return err;
	}

	err = exynos_pinmux_config(PERIPH_ID_UART2, PINMUX_FLAG_NONE);
	if (err) {
		debug("UART2 not configured\n");
		return err;
	}

	err = exynos_pinmux_config(PERIPH_ID_UART3, PINMUX_FLAG_NONE);
	if (err) {
		debug("UART3 not configured\n");
		return err;
	}

	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	int err;
	err = board_uart_init();
	if (err) {
		debug("UART init failed\n");
		return err;
	}
	return err;
}
#endif

int dram_init(void)
{
	gd->ram_size	= get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE);

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((long *)PHYS_SDRAM_1, \
							PHYS_SDRAM_1_SIZE);
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((long *)PHYS_SDRAM_2, \
							PHYS_SDRAM_2_SIZE);
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = get_ram_size((long *)PHYS_SDRAM_3, \
							PHYS_SDRAM_3_SIZE);
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = get_ram_size((long *)PHYS_SDRAM_4, \
							PHYS_SDRAM_4_SIZE);
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("\nBoard: ORIGEN\n");
	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int i, err;

	/*
	 * MMC2 SD card GPIO:
	 *
	 * GPK2[0]	SD_2_CLK(2)
	 * GPK2[1]	SD_2_CMD(2)
	 * GPK2[2]	SD_2_CDn
	 * GPK2[3:6]	SD_2_DATA[0:3](2)
	 */
	for (i = 0; i < 7; i++) {
		/* GPK2[0:6] special function 2 */
		s5p_gpio_cfg_pin(&gpio2->k2, i, GPIO_FUNC(0x2));

		/* GPK2[0:6] drv 4x */
		s5p_gpio_set_drv(&gpio2->k2, i, GPIO_DRV_4X);

		/* GPK2[0:1] pull disable */
		if (i == 0 || i == 1) {
			s5p_gpio_set_pull(&gpio2->k2, i, GPIO_PULL_NONE);
			continue;
		}

		/* GPK2[2:6] pull up */
		s5p_gpio_set_pull(&gpio2->k2, i, GPIO_PULL_UP);
	}

	err = s5p_mmc_init(2, 4);
	return err;
}
#endif
