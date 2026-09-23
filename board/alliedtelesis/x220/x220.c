// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Allied Telesis Labs
 */

#include <i2c.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <linux/bitops.h>
#include <linux/mbus.h>
#include <linux/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

#define X220_GPP_OUT_ENA_LOW	(~(BIT(12) | BIT(17) | BIT(18) | BIT(31)))
#define X220_GPP_OUT_ENA_MID	(~(0))
#define X220_GPP_OUT_VAL_LOW	(BIT(12) | BIT(18))
#define X220_GPP_OUT_VAL_MID	0x0
#define X220_GPP_POL_LOW	0x0
#define X220_GPP_POL_MID	0x0

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x44042222, MVEBU_MPP_BASE + 0x00);
	writel(0x11000004, MVEBU_MPP_BASE + 0x04);
	writel(0x44444004, MVEBU_MPP_BASE + 0x08);
	writel(0x04444444, MVEBU_MPP_BASE + 0x0c);
	writel(0x00000004, MVEBU_MPP_BASE + 0x10);

	/* Set GPP Out value */
	writel(X220_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(X220_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(X220_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(X220_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(X220_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(X220_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	/* Disable MBUS Err Prop - in order to avoid data aborts */
	clrbits_le32(MVEBU_CPU_WIN_BASE + 0x200, (1 << 8));

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board: Allied Telesis x220\n");

	return 0;
}
#endif
