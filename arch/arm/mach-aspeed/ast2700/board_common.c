// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <dm.h>
#include <ram.h>
#include <init.h>
#include <timer.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <dm/uclass.h>
#include <asm/arch-aspeed/scu_ast2700.h>

#define AHBC_GROUP(x)				(0x40 * (x))
#define AHBC_HREADY_WAIT_CNT_REG		0x34
#define   AHBC_HREADY_WAIT_CNT_MAX		0x3f

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	int ret;
	struct udevice *dev;
	struct ram_info ram;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		printf("cannot get DRAM driver\n");
		debug("cannot get DRAM driver\n");
		return ret;
	}

	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("cannot get DRAM information\n");
		return ret;
	}

	gd->ram_size = ram.size;

	return 0;
}

static void ahbc_init(void)
{
	u32 reg_val;
	int i;

	reg_val = readl(ASPEED_CPU_REVISION_ID);
	if (FIELD_GET(SCU_CPU_REVISION_ID_HW, reg_val))
		return;

	/* CPU-die AHBC timeout counter */
	for (i = 0; i < 4; i++)
		writel(AHBC_HREADY_WAIT_CNT_MAX,
		       (void *)ASPEED_CPU_AHBC_BASE + AHBC_GROUP(i) + AHBC_HREADY_WAIT_CNT_REG);

	/* IO-die AHBC timeout counter */
	for (i = 0; i < 8; i++)
		writel(AHBC_HREADY_WAIT_CNT_MAX,
		       (void *)ASPEED_IO_AHBC_BASE + AHBC_GROUP(i) + AHBC_HREADY_WAIT_CNT_REG);
}

int board_init(void)
{
	struct udevice *dev;
	int i = 0;
	int ret;

	ahbc_init();

	/*
	 * Loop over all MISC uclass drivers to call the comphy code
	 * and init all CP110 devices enabled in the DT
	 */
	while (1) {
		/* Call the comphy code via the MISC uclass driver */
		ret = uclass_get_device(UCLASS_MISC, i++, &dev);

		/* We're done, once no further CP110 device is found */
		if (ret)
			break;
	}

	return 0;
}
