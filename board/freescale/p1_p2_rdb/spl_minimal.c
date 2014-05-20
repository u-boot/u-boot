/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ns16550.h>
#include <asm/io.h>
#include <nand.h>
#include <linux/compiler.h>
#include <asm/fsl_law.h>
#include <fsl_ddr_sdram.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;
#define SYSCLK_MASK	0x00200000
#define BOARDREV_MASK	0x10100000

#define SYSCLK_66	66666666
#define SYSCLK_100	100000000

unsigned long get_board_sys_clk(ulong dummy)
{
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
	u32 val_gpdat, sysclk_gpio;

	val_gpdat = in_be32(&pgpio->gpdat);
	sysclk_gpio = val_gpdat & SYSCLK_MASK;

	if (sysclk_gpio == 0)
		return SYSCLK_66;
	else
		return SYSCLK_100;

	return 0;
}

void board_init_f(ulong bootflag)
{
	u32 plat_ratio;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

#if defined(CONFIG_SYS_NAND_BR_PRELIM) && defined(CONFIG_SYS_NAND_OR_PRELIM)
	set_lbc_br(0, CONFIG_SYS_NAND_BR_PRELIM);
	set_lbc_or(0, CONFIG_SYS_NAND_OR_PRELIM);
#endif

	/* initialize selected port with appropriate baud rate */
	plat_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_PLAT_RATIO;
	plat_ratio >>= 1;
	gd->bus_clk = CONFIG_SYS_CLK_FREQ * plat_ratio;

	NS16550_init((NS16550_t)CONFIG_SYS_NS16550_COM1,
		     gd->bus_clk / 16 / CONFIG_BAUDRATE);

	puts("\nNAND boot... ");

	/* copy code to RAM and jump to it - this should not return */
	/* NOTE - code has to be copied out of NAND buffer before
	 * other blocks can be read.
	 */
	relocate_code(CONFIG_SPL_RELOC_STACK, 0, CONFIG_SPL_RELOC_TEXT_BASE);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
	puts("\nSecond program loader running in sram...");
	nand_boot();
}

void putc(char c)
{
	if (c == '\n')
		NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM1, '\r');

	NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM1, c);
}

void puts(const char *str)
{
	while (*str)
		putc(*str++);
}
