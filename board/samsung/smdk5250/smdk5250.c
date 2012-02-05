/*
 * Copyright (C) 2012 Samsung Electronics
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
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/sromc.h>

DECLARE_GLOBAL_DATA_PTR;
struct exynos5_gpio_part1 *gpio1;

int board_init(void)
{
	gpio1 = (struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();

	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100UL);
	return 0;
}

int dram_init(void)
{
	gd->ram_size	= get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_3, PHYS_SDRAM_3_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_4, PHYS_SDRAM_4_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_5, PHYS_SDRAM_7_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_6, PHYS_SDRAM_7_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_7, PHYS_SDRAM_7_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_8, PHYS_SDRAM_8_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((long *)PHYS_SDRAM_1,
							PHYS_SDRAM_1_SIZE);
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((long *)PHYS_SDRAM_2,
							PHYS_SDRAM_2_SIZE);
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = get_ram_size((long *)PHYS_SDRAM_3,
							PHYS_SDRAM_3_SIZE);
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = get_ram_size((long *)PHYS_SDRAM_4,
							PHYS_SDRAM_4_SIZE);
	gd->bd->bi_dram[4].start = PHYS_SDRAM_5;
	gd->bd->bi_dram[4].size = get_ram_size((long *)PHYS_SDRAM_5,
							PHYS_SDRAM_5_SIZE);
	gd->bd->bi_dram[5].start = PHYS_SDRAM_6;
	gd->bd->bi_dram[5].size = get_ram_size((long *)PHYS_SDRAM_6,
							PHYS_SDRAM_6_SIZE);
	gd->bd->bi_dram[6].start = PHYS_SDRAM_7;
	gd->bd->bi_dram[6].size = get_ram_size((long *)PHYS_SDRAM_7,
							PHYS_SDRAM_7_SIZE);
	gd->bd->bi_dram[7].start = PHYS_SDRAM_8;
	gd->bd->bi_dram[7].size = get_ram_size((long *)PHYS_SDRAM_8,
							PHYS_SDRAM_8_SIZE);
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("\nBoard: SMDK5250\n");

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
	 * GPC2[0]	SD_2_CLK(2)
	 * GPC2[1]	SD_2_CMD(2)
	 * GPC2[2]	SD_2_CDn
	 * GPC2[3:6]	SD_2_DATA[0:3](2)
	 */
	for (i = 0; i < 7; i++) {
		/* GPC2[0:6] special function 2 */
		s5p_gpio_cfg_pin(&gpio1->c2, i, GPIO_FUNC(0x2));

		/* GPK2[0:6] drv 4x */
		s5p_gpio_set_drv(&gpio1->c2, i, GPIO_DRV_4X);

		/* GPK2[0:1] pull disable */
		if (i == 0 || i == 1) {
			s5p_gpio_set_pull(&gpio1->c2, i, GPIO_PULL_NONE);
			continue;
		}

		/* GPK2[2:6] pull up */
		s5p_gpio_set_pull(&gpio1->c2, i, GPIO_PULL_UP);
	}

	err = s5p_mmc_init(2, 4);
	return err;
}
#endif

static void board_uart_init(void)
{
	struct exynos5_gpio_part1 *gpio1 =
		(struct exynos5_gpio_part1 *) samsung_get_base_gpio_part1();
	int i;

	/* UART1 GPIOs (part1) : GPA0CON[7:4] 0x2222 */
	for (i = 4; i < 8; i++) {
		s5p_gpio_set_pull(&gpio1->a0, i, GPIO_PULL_NONE);
		s5p_gpio_cfg_pin(&gpio1->a0, i, GPIO_FUNC(0x2));
	}
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	board_uart_init();
	return 0;
}
#endif
