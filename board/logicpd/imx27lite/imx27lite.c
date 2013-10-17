/*
 * Copyright (C) 2007 Sascha Hauer, Pengutronix
 * Copyright (C) 2008,2009 Eric Jarrige <jorasse@users.sourceforge.net>
 * Copyright (C) 2009 Ilya Yanok <yanok@emcraft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
#if defined(CONFIG_SYS_NAND_LARGEPAGE)
	struct system_control_regs *sc_regs =
		(struct system_control_regs *)IMX_SYSTEM_CTL_BASE;
#endif

	gd->bd->bi_arch_number = MACH_TYPE_IMX27LITE;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

#ifdef CONFIG_MXC_UART
	mx27_uart1_init_pins();
#endif
#ifdef CONFIG_FEC_MXC
	mx27_fec_init_pins();
	imx_gpio_mode((GPIO_PORTC | GPIO_OUT | GPIO_PUEN | GPIO_GPIO | 31));
	gpio_set_value(GPIO_PORTC | 31, 1);
#endif
#ifdef CONFIG_MXC_MMC
#if defined(CONFIG_MAGNESIUM)
	mx27_sd1_init_pins();
#else
	mx27_sd2_init_pins();
#endif
#endif

#if defined(CONFIG_SYS_NAND_LARGEPAGE)
	/*
	 * set in FMCR NF_FMS Bit(5) to 1
	 * (NAND Flash with 2 Kbyte page size)
	 */
	writel(readl(&sc_regs->fmcr) | (1 << 5), &sc_regs->fmcr);
#endif
	return 0;
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
			PHYS_SDRAM_1_SIZE);
#if CONFIG_NR_DRAM_BANKS > 1
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((void *)PHYS_SDRAM_2,
			PHYS_SDRAM_2_SIZE);
#endif
}

int checkboard(void)
{
	puts("Board: ");
	puts(CONFIG_BOARDNAME);
	return 0;
}
