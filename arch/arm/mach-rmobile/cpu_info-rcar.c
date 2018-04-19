// SPDX-License-Identifier: GPL-2.0
/*
 * arch/arm/cpu/armv7/rmobile/cpu_info-rcar.c
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 */
#include <common.h>
#include <asm/io.h>

#define PRR_MASK		0x7fff
#define R8A7796_REV_1_0		0x5200
#define R8A7796_REV_1_1		0x5210

static u32 rmobile_get_prr(void);

u32 rmobile_get_cpu_type(void)
{
	return (rmobile_get_prr() & 0x00007F00) >> 8;
}

u32 rmobile_get_cpu_rev_integer(void)
{
	const u32 prr = rmobile_get_prr();

	if ((prr & PRR_MASK) == R8A7796_REV_1_1)
		return 1;
	else
		return ((prr & 0x000000F0) >> 4) + 1;
}

u32 rmobile_get_cpu_rev_fraction(void)
{
	const u32 prr = rmobile_get_prr();

	if ((prr & PRR_MASK) == R8A7796_REV_1_1)
		return 1;
	else
		return prr & 0x0000000F;
}

#if !CONFIG_IS_ENABLED(DM) || !CONFIG_IS_ENABLED(SYSCON)
static u32 rmobile_get_prr(void)
{
	/*
	 * On RCar/RMobile Gen2 and older systems, the PRR is always
	 * located at the address below. On newer systems, the PRR
	 * may be located at different address, but that information
	 * is obtained from DT. This code will be removed when all
	 * of the older systems get converted to DM and OF control.
	 */
	return readl(0xFF000044);
}
#else

#include <dm.h>
#include <syscon.h>
#include <regmap.h>

struct renesas_prr_priv {
	fdt_addr_t	regs;
};

enum {
	PRR_RCAR,
};

static u32 rmobile_get_prr(void)
{
	struct regmap *map;

	map = syscon_get_regmap_by_driver_data(PRR_RCAR);
	if (!map) {
		printf("PRR regmap failed!\n");
		hang();
	}

	return readl(map->ranges[0].start);
}

static const struct udevice_id renesas_prr_ids[] = {
	{ .compatible = "renesas,prr", .data = PRR_RCAR },
	{ }
};

U_BOOT_DRIVER(renesas_prr) = {
	.name	= "renesas_prr",
	.id	= UCLASS_SYSCON,
	.of_match = renesas_prr_ids,
	.flags	= DM_FLAG_PRE_RELOC,
};
#endif
