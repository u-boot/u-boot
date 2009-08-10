/*
 * Copyright (C) 2007 Sascha Hauer, Pengutronix
 * Copyright (C) 2008,2009 Eric Jarrige <jorasse@users.sourceforge.net>
 * Copyright (C) 2009 Ilya Yanok <yanok@emcraft.com>
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
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init (void)
{
	struct gpio_regs *regs = (struct gpio_regs *)IMX_GPIO_BASE;

	gd->bd->bi_arch_number = MACH_TYPE_IMX27LITE;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

#ifdef CONFIG_MXC_UART
	mx27_uart_init_pins();
#endif
#ifdef CONFIG_FEC_MXC
	mx27_fec_init_pins();
	imx_gpio_mode((GPIO_PORTC | GPIO_OUT | GPIO_PUEN | GPIO_GPIO | 31));
	writel(readl(&regs->port[PORTC].dr) | (1 << 31),
				&regs->port[PORTC].dr);
#endif
#ifdef CONFIG_MXC_MMC
	mx27_sd2_init_pins();
#endif

	return 0;
}

int dram_init (void)
{

#if CONFIG_NR_DRAM_BANKS > 0
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((volatile void *)PHYS_SDRAM_1,
			PHYS_SDRAM_1_SIZE);
#endif
#if CONFIG_NR_DRAM_BANKS > 1
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((volatile void *)PHYS_SDRAM_2,
			PHYS_SDRAM_2_SIZE);
#endif

	return 0;
}

int checkboard(void)
{
	printf("LogicPD imx27lite\n");
	return 0;
}
