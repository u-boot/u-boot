// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone2: Asynchronous EMIF Configuration
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/ti-common/ti-aemif.h>
#include <dm.h>
#include "ti-aemif-cs.h"

#define AEMIF_WAITCYCLE_CONFIG		(0x4)
#define AEMIF_NAND_CONTROL		(0x60)
#define AEMIF_ONENAND_CONTROL		(0x5c)

static void aemif_configure(int cs, struct aemif_config *cfg)
{
	unsigned long tmp;

	if (cfg->mode == AEMIF_MODE_NAND) {
		tmp = __raw_readl(cfg->base + AEMIF_NAND_CONTROL);
		tmp |= (1 << cs);
		__raw_writel(tmp, cfg->base + AEMIF_NAND_CONTROL);

	} else if (cfg->mode == AEMIF_MODE_ONENAND) {
		tmp = __raw_readl(cfg->base + AEMIF_ONENAND_CONTROL);
		tmp |= (1 << cs);
		__raw_writel(tmp, cfg->base + AEMIF_ONENAND_CONTROL);
	}

	aemif_cs_configure(cs, cfg);
}

void aemif_init(int num_cs, struct aemif_config *config)
{
	int cs;

	if (num_cs > AEMIF_NUM_CS) {
		num_cs = AEMIF_NUM_CS;
		printf("AEMIF: csnum has to be <= 5");
	}

	for (cs = 0; cs < num_cs; cs++)
		aemif_configure(cs, config + cs);
}

static const struct udevice_id aemif_ids[] = {
	{ .compatible = "ti,da850-aemif", },
	{},
};

U_BOOT_DRIVER(ti_aemif) = {
	.name = "ti_aemif",
	.id = UCLASS_MEMORY,
	.of_match = aemif_ids,
};
