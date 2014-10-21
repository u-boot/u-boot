/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sg-regs.h>

void sg_init(void)
{
	u32 tmp;

	/* Set DDR size */
	tmp = sg_memconf_val_ch0(CONFIG_SDRAM0_SIZE, CONFIG_DDR_NUM_CH0);
	tmp |= sg_memconf_val_ch1(CONFIG_SDRAM1_SIZE, CONFIG_DDR_NUM_CH1);
#if CONFIG_SDRAM0_BASE + CONFIG_SDRAM0_SIZE < CONFIG_SDRAM1_BASE
	tmp |= SG_MEMCONF_SPARSEMEM;
#endif
	writel(tmp, SG_MEMCONF);

	/* Input ports must be enabled deasserting reset of cores */
	tmp = readl(SG_IECTRL);
	tmp |= 0x1;
	writel(tmp, SG_IECTRL);
}
