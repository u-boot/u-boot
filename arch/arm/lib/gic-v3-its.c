// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Broadcom.
 */
#include <common.h>
#include <asm/gic.h>
#include <asm/gic-v3.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/sizes.h>

static u32 lpi_id_bits;

#define LPI_NRBITS		lpi_id_bits
#define LPI_PROPBASE_SZ		ALIGN(BIT(LPI_NRBITS), SZ_64K)
#define LPI_PENDBASE_SZ		ALIGN(BIT(LPI_NRBITS) / 8, SZ_64K)

/*
 * Program the GIC LPI configuration tables for all
 * the re-distributors and enable the LPI table
 * base: Configuration table address
 * num_redist: number of redistributors
 */
int gic_lpi_tables_init(u64 base, u32 num_redist)
{
	u32 gicd_typer;
	u64 val;
	u64 tmp;
	int i;
	u64 redist_lpi_base;
	u64 pend_base = GICR_BASE + GICR_PENDBASER;

	gicd_typer = readl(GICD_BASE + GICD_TYPER);

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
	for (i = 0; i < num_redist; i++) {
		u32 offset = i * GIC_REDISTRIBUTOR_OFFSET;

		if ((readl((uintptr_t)(GICR_BASE + offset))) &
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
	val = (base |
	       GICR_PROPBASER_INNERSHAREABLE |
	       GICR_PROPBASER_RAWAWB |
	       ((LPI_NRBITS - 1) & GICR_PROPBASER_IDBITS_MASK));

	writeq(val, (GICR_BASE + GICR_PROPBASER));
	tmp = readl(GICR_BASE + GICR_PROPBASER);
	if ((tmp ^ val) & GICR_PROPBASER_SHAREABILITY_MASK) {
		if (!(tmp & GICR_PROPBASER_SHAREABILITY_MASK)) {
			val &= ~(GICR_PROPBASER_SHAREABILITY_MASK |
				GICR_PROPBASER_CACHEABILITY_MASK);
			val |= GICR_PROPBASER_NC;
			writeq(val, (GICR_BASE + GICR_PROPBASER));
		}
	}

	redist_lpi_base = base + LPI_PROPBASE_SZ;

	for (i = 0; i < num_redist; i++) {
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
		writel(GICR_CTLR_ENABLE_LPIS, (uintptr_t)(GICR_BASE + offset));
	}

	return 0;
}

