// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Broadcom.
 */
#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/gic.h>
#include <asm/gic-v3.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/sizes.h>

static u32 lpi_id_bits;

#define LPI_NRBITS		lpi_id_bits
#define LPI_PROPBASE_SZ		ALIGN(BIT(LPI_NRBITS), SZ_64K)
#define LPI_PENDBASE_SZ		ALIGN(BIT(LPI_NRBITS) / 8, SZ_64K)

/* Number of GIC re-distributors */
#define MAX_GIC_REDISTRIBUTORS	8

/*
 * gic_v3_its_priv - gic details
 *
 * @gicd_base: gicd base address
 * @gicr_base: gicr base address
 * @lpi_base: gic lpi base address
 * @num_redist: number of gic re-distributors
 */
struct gic_v3_its_priv {
	ulong gicd_base;
	ulong gicr_base;
	ulong lpi_base;
	u32 num_redist;
};

static int gic_v3_its_get_gic_addr(struct gic_v3_its_priv *priv)
{
	struct udevice *dev;
	fdt_addr_t addr;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_IRQ,
					  DM_GET_DRIVER(arm_gic_v3_its), &dev);
	if (ret) {
		pr_err("%s: failed to get %s irq device\n", __func__,
		       DM_GET_DRIVER(arm_gic_v3_its)->name);
		return ret;
	}

	addr = dev_read_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE) {
		pr_err("%s: failed to get GICD address\n", __func__);
		return -EINVAL;
	}
	priv->gicd_base = addr;

	addr = dev_read_addr_index(dev, 1);
	if (addr == FDT_ADDR_T_NONE) {
		pr_err("%s: failed to get GICR address\n", __func__);
		return -EINVAL;
	}
	priv->gicr_base = addr;

	return 0;
}

static int gic_v3_its_get_gic_lpi_addr(struct gic_v3_its_priv *priv)
{
	struct regmap *regmap;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_SYSCON,
					  DM_GET_DRIVER(gic_lpi_syscon), &dev);
	if (ret) {
		pr_err("%s: failed to get %s syscon device\n", __func__,
		       DM_GET_DRIVER(gic_lpi_syscon)->name);
		return ret;
	}

	regmap = syscon_get_regmap(dev);
	if (!regmap) {
		pr_err("%s: failed to regmap for %s syscon device\n", __func__,
		       DM_GET_DRIVER(gic_lpi_syscon)->name);
		return -ENODEV;
	}
	priv->lpi_base = regmap->ranges[0].start;

	priv->num_redist = dev_read_u32_default(dev, "max-gic-redistributors",
						MAX_GIC_REDISTRIBUTORS);

	return 0;
}

/*
 * Program the GIC LPI configuration tables for all
 * the re-distributors and enable the LPI table
 */
int gic_lpi_tables_init(void)
{
	struct gic_v3_its_priv priv;
	u32 gicd_typer;
	u64 val;
	u64 tmp;
	int i;
	u64 redist_lpi_base;
	u64 pend_base;

	if (gic_v3_its_get_gic_addr(&priv))
		return -EINVAL;

	if (gic_v3_its_get_gic_lpi_addr(&priv))
		return -EINVAL;

	gicd_typer = readl((uintptr_t)(priv.gicd_base + GICD_TYPER));
	/* GIC support for Locality specific peripheral interrupts (LPI's) */
	if (!(gicd_typer & GICD_TYPER_LPIS)) {
		pr_err("GIC implementation does not support LPI's\n");
		return -EINVAL;
	}

	/*
	 * Check for LPI is disabled for all the redistributors.
	 * Once the LPI table is enabled, can not program the
	 * LPI configuration tables again, unless the GIC is reset.
	 */
	for (i = 0; i < priv.num_redist; i++) {
		u32 offset = i * GIC_REDISTRIBUTOR_OFFSET;

		if ((readl((uintptr_t)(priv.gicr_base + offset))) &
		    GICR_CTLR_ENABLE_LPIS) {
			pr_err("Re-Distributor %d LPI is already enabled\n",
			       i);
			return -EINVAL;
		}
	}

	/* lpi_id_bits to get LPI_PENDBASE_SZ and LPi_PROPBASE_SZ */
	lpi_id_bits = min_t(u32, GICD_TYPER_ID_BITS(gicd_typer),
			    ITS_MAX_LPI_NRBITS);

	/* Set PropBase */
	val = (priv.lpi_base |
	       GICR_PROPBASER_INNERSHAREABLE |
	       GICR_PROPBASER_RAWAWB |
	       ((LPI_NRBITS - 1) & GICR_PROPBASER_IDBITS_MASK));

	writeq(val, (uintptr_t)(priv.gicr_base + GICR_PROPBASER));
	tmp = readl((uintptr_t)(priv.gicr_base + GICR_PROPBASER));
	if ((tmp ^ val) & GICR_PROPBASER_SHAREABILITY_MASK) {
		if (!(tmp & GICR_PROPBASER_SHAREABILITY_MASK)) {
			val &= ~(GICR_PROPBASER_SHAREABILITY_MASK |
				GICR_PROPBASER_CACHEABILITY_MASK);
			val |= GICR_PROPBASER_NC;
			writeq(val,
			       (uintptr_t)(priv.gicr_base + GICR_PROPBASER));
		}
	}

	redist_lpi_base = priv.lpi_base + LPI_PROPBASE_SZ;

	pend_base = priv.gicr_base + GICR_PENDBASER;
	for (i = 0; i < priv.num_redist; i++) {
		u32 offset = i * GIC_REDISTRIBUTOR_OFFSET;

		val = ((redist_lpi_base + (i * LPI_PENDBASE_SZ)) |
			GICR_PENDBASER_INNERSHAREABLE |
			GICR_PENDBASER_RAWAWB);

		writeq(val, (uintptr_t)(pend_base + offset));
		tmp = readq((uintptr_t)(pend_base + offset));
		if (!(tmp & GICR_PENDBASER_SHAREABILITY_MASK)) {
			val &= ~(GICR_PENDBASER_SHAREABILITY_MASK |
				 GICR_PENDBASER_CACHEABILITY_MASK);
			val |= GICR_PENDBASER_NC;
			writeq(val, (uintptr_t)(pend_base + offset));
		}

		/* Enable LPI for the redistributor */
		writel(GICR_CTLR_ENABLE_LPIS,
		       (uintptr_t)(priv.gicr_base + offset));
	}

	return 0;
}

static const struct udevice_id gic_v3_its_ids[] = {
	{ .compatible = "arm,gic-v3" },
	{}
};

U_BOOT_DRIVER(arm_gic_v3_its) = {
	.name		= "gic-v3",
	.id		= UCLASS_IRQ,
	.of_match	= gic_v3_its_ids,
};

static const struct udevice_id gic_lpi_syscon_ids[] = {
	{ .compatible = "gic-lpi-base" },
	{}
};

U_BOOT_DRIVER(gic_lpi_syscon) = {
	.name		= "gic-lpi-base",
	.id		= UCLASS_SYSCON,
	.of_match	= gic_lpi_syscon_ids,
};
