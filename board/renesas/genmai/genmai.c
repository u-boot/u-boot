// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Magnus Damm <damm@opensource.se>
 */

#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>

#define RZA1_BCR_BASE	0x3FFFC000
#define CS0BCR		(RZA1_BCR_BASE + 0x04)
#define CS0WCR		(RZA1_BCR_BASE + 0x28)
#define CS1BCR		(RZA1_BCR_BASE + 0x08)
#define CS1WCR		(RZA1_BCR_BASE + 0x2c)

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	/* setup NOR Flash on CS0 and CS1 */
	writel(0x00000b40, CS0WCR);
	writel(0x10000c00, CS0BCR);
	writel(0x00000b40, CS1WCR);
	writel(0x10000c00, CS1BCR);
	return 0;
}

/*
 * The Genmai DT will most likely contain memory nodes describing the external
 * SDRAM memory connected to CS2 and CS3, however we do not yet have any code
 * in U-Boot to setup the memory controller. For now ignore DT and make use of
 * the RZ/A1H on-chip memory which is 10 MiB at CFG_SYS_SDRAM_BASE.
 */

int dram_init(void)
{
	gd->ram_base = CFG_SYS_SDRAM_BASE;
	gd->ram_size = 10 << 20;
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = gd->ram_size;
	return 0;
}
