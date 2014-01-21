/*
 * Copyright 2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <mpc85xx.h>
#include <asm/io.h>
#include <ns16550.h>
#include <nand.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_law.h>

#define SYSCLK_MASK     0x00200000
#define BOARDREV_MASK   0x10100000
#define BOARDREV_B      0x10100000
#define BOARDREV_C      0x00100000

#define SYSCLK_66       66666666
#define SYSCLK_50       50000000
#define SYSCLK_100      100000000

DECLARE_GLOBAL_DATA_PTR;

void board_init_f(ulong bootflag)
{
	uint plat_ratio, bus_clk, sys_clk = 0;
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);
	uint val, temp, sysclk_mask;

	val = pgpio->gpdat;
	sysclk_mask = val & SYSCLK_MASK;
	temp = val & BOARDREV_MASK;
	if (temp == BOARDREV_C) {
		if(sysclk_mask == 0)
			sys_clk = SYSCLK_66;
		else
			sys_clk = SYSCLK_100;
	} else if (temp == BOARDREV_B) {
		if(sysclk_mask == 0)
			sys_clk = SYSCLK_66;
		else
			sys_clk = SYSCLK_50;
	}

	plat_ratio = gur->porpllsr & 0x0000003e;
	plat_ratio >>= 1;
	bus_clk = plat_ratio * sys_clk;
	NS16550_init((NS16550_t)CONFIG_SYS_NS16550_COM1,
			bus_clk / 16 / CONFIG_BAUDRATE);

	puts("\nNAND boot... ");

	/* copy code to DDR and jump to it - this should not return */
	/* NOTE - code has to be copied out of NAND buffer before
	 * other blocks can be read.
	 */
	relocate_code(CONFIG_SYS_NAND_U_BOOT_RELOC_SP, 0,
			CONFIG_SYS_NAND_U_BOOT_RELOC);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
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
