/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ath79.h>
#include <mach/ar71xx_regs.h>

void _machine_restart(void)
{
	void __iomem *base;
	u32 reg = 0;

	base = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
			   MAP_NOCACHE);
	if (soc_is_ar71xx())
		reg = AR71XX_RESET_REG_RESET_MODULE;
	else if (soc_is_ar724x())
		reg = AR724X_RESET_REG_RESET_MODULE;
	else if (soc_is_ar913x())
		reg = AR913X_RESET_REG_RESET_MODULE;
	else if (soc_is_ar933x())
		reg = AR933X_RESET_REG_RESET_MODULE;
	else if (soc_is_ar934x())
		reg = AR934X_RESET_REG_RESET_MODULE;
	else if (soc_is_qca953x())
		reg = QCA953X_RESET_REG_RESET_MODULE;
	else if (soc_is_qca955x())
		reg = QCA955X_RESET_REG_RESET_MODULE;
	else if (soc_is_qca956x())
		reg = QCA956X_RESET_REG_RESET_MODULE;
	else
		puts("Reset register not defined for this SOC\n");

	if (reg)
		setbits_be32(base + reg, AR71XX_RESET_FULL_CHIP);

	while (1)
		/* NOP */;
}

u32 get_bootstrap(void)
{
	void __iomem *base;
	u32 reg = 0;

	base = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
			   MAP_NOCACHE);
	if (soc_is_ar933x())
		reg = AR933X_RESET_REG_BOOTSTRAP;
	else if (soc_is_ar934x())
		reg = AR934X_RESET_REG_BOOTSTRAP;
	else if (soc_is_qca953x())
		reg = QCA953X_RESET_REG_BOOTSTRAP;
	else if (soc_is_qca955x())
		reg = QCA955X_RESET_REG_BOOTSTRAP;
	else if (soc_is_qca956x())
		reg = QCA956X_RESET_REG_BOOTSTRAP;
	else
		puts("Bootstrap register not defined for this SOC\n");

	if (reg)
		return readl(base + reg);

	return 0;
}
