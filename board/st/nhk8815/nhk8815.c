/*
 * (C) Copyright 2005
 * STMicrolelctronics, <www.st.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
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
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SHOW_BOOT_PROGRESS
void show_boot_progress(int progress)
{
	printf("%i\n", progress);
}
#endif

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_NOMADIK;
	gd->bd->bi_boot_params = 0x00000100;
	writel(0xC37800F0, NOMADIK_GPIO1_BASE + 0x20);
	writel(0x00000000, NOMADIK_GPIO1_BASE + 0x24);
	writel(0x00000000, NOMADIK_GPIO1_BASE + 0x28);
	writel(readl(NOMADIK_SRC_BASE) | 0x8000, NOMADIK_SRC_BASE);

	/* Set up SMCS1 for Ethernet: sram-like, enabled, timing values */
	writel(0x0000305b, REG_FSMC_BCR1);
	writel(0x00033f33, REG_FSMC_BTR1);

	/* Set up SMCS0 for OneNand: sram-like once again */
	writel(0x000030db, NOMADIK_FSMC_BASE + 0x00); /* FSMC_BCR0 */
	writel(0x02100551, NOMADIK_FSMC_BASE + 0x04); /* FSMC_BTR0 */

	icache_enable();
	return 0;
}

int board_late_init(void)
{
	/* Set the two I2C gpio lines to be gpio high */
	nmk_gpio_set(__SCL, 1);	nmk_gpio_set(__SDA, 1);
	nmk_gpio_dir(__SCL, 1);	nmk_gpio_dir(__SDA, 1);
	nmk_gpio_af(__SCL, GPIO_GPIO); nmk_gpio_af(__SDA, GPIO_GPIO);

	/* Reset the I2C port expander, on GPIO77 */
	nmk_gpio_af(77, GPIO_GPIO);
	nmk_gpio_dir(77, 1);
	nmk_gpio_set(77, 0);
	udelay(10);
	nmk_gpio_set(77, 1);

	return 0;
}

int dram_init(void)
{
	/* set dram bank start addr and size */
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	return 0;
}
