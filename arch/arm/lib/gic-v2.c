// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Broadcom.
 */
#include <dm.h>
#include <irq.h>
#include <asm/gic.h>
#include <asm/acpi_table.h>
#include <cpu_func.h>
#include <dm/acpi.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>

#ifdef CONFIG_ACPIGEN
/**
 * acpi_gicv2_fill_madt() - Fill out the body of the MADT
 *
 * Write GICD and GICR tables based on collected devicetree data.
 *
 * @dev: Device to write ACPI tables for
 * @ctx: ACPI context to write MADT sub-tables to
 * Return: 0 if OK
 */
static int acpi_gicv2_fill_madt(const struct udevice *dev, struct acpi_ctx *ctx)
{
	struct acpi_madt_gicd *gicd;
	fdt_addr_t addr;

	addr = dev_read_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE) {
		pr_err("%s: failed to get GICD address\n", __func__);
		return -EINVAL;
	}

	gicd = ctx->current;
	acpi_write_madt_gicd(gicd, dev_seq(dev), addr, 2);
	acpi_inc(ctx, gicd->length);

	return 0;
}

static struct acpi_ops gic_v2_acpi_ops = {
	.fill_madt	= acpi_gicv2_fill_madt,
};
#endif

static const struct udevice_id gic_v2_ids[] = {
	{ .compatible = "arm,arm11mp-gic" },
	{ .compatible = "arm,cortex-a15-gic" },
	{ .compatible = "arm,cortex-a7-gic" },
	{ .compatible = "arm,cortex-a5-gic" },
	{ .compatible = "arm,cortex-a9-gic" },
	{ .compatible = "arm,eb11mp-gic" },
	{ .compatible = "arm,gic-400" },
	{ .compatible = "arm,pl390" },
	{ .compatible = "arm,tc11mp-gic" },
	{ .compatible = "qcom,msm-8660-qgic" },
	{ .compatible = "qcom,msm-qgic2" },
	{}
};

static int arm_gic_v2_of_xlate(struct irq *irq, struct ofnode_phandle_args *args)
{
	if (args->args_count != 3) {
		log_debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	/* ARM Generic Interrupt Controller v1 and v2 */
	if (args->args[0] == GIC_SPI)
		irq->id = args->args[1] + 32;
	else
		irq->id = args->args[1] + 16;

	irq->flags = args->args[2];

	return 0;
}

static const struct irq_ops arm_gic_v2_ops = {
	.of_xlate		=  arm_gic_v2_of_xlate,
};

U_BOOT_DRIVER(arm_gic_v2) = {
	.name		= "gic-v2",
	.id		= UCLASS_IRQ,
	.of_match	= gic_v2_ids,
	.ops		= &arm_gic_v2_ops,
	ACPI_OPS_PTR(&gic_v2_acpi_ops)
};
