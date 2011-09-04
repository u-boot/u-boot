/*
 *  Copyright (C) 2008-2009 Samsung Electronics
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
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>

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
	int i;

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

	return s5p_mmc_init(0, 4);
}
#endif
