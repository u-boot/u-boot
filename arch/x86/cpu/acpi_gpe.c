// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google, LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <asm/io.h>

/**
 * struct acpi_gpe_priv - private driver information
 *
 * @acpi_base: Base I/O address of ACPI registers
 */
struct acpi_gpe_priv {
	ulong acpi_base;
};

#define GPE0_STS(x)		(0x20 + ((x) * 4))

static int acpi_gpe_read_and_clear(struct irq *irq)
{
	struct acpi_gpe_priv *priv = dev_get_priv(irq->dev);
	u32 mask, sts;
	ulong start;
	int ret = 0;
	int bank;

	bank = irq->id / 32;
	mask = 1 << (irq->id % 32);

	/* Wait up to 1ms for GPE status to clear */
	start = get_timer(0);
	do {
		if (get_timer(start) > 1)
			return ret;

		sts = inl(priv->acpi_base + GPE0_STS(bank));
		if (sts & mask) {
			outl(mask, priv->acpi_base + GPE0_STS(bank));
			ret = 1;
		}
	} while (sts & mask);

	return ret;
}

static int acpi_gpe_ofdata_to_platdata(struct udevice *dev)
{
	struct acpi_gpe_priv *priv = dev_get_priv(dev);

	priv->acpi_base = dev_read_addr(dev);
	if (!priv->acpi_base || priv->acpi_base == FDT_ADDR_T_NONE)
		return log_msg_ret("acpi_base", -EINVAL);

	return 0;
}

static int acpi_gpe_of_xlate(struct irq *irq, struct ofnode_phandle_args *args)
{
	irq->id = args->args[0];

	return 0;
}

static const struct irq_ops acpi_gpe_ops = {
	.read_and_clear		= acpi_gpe_read_and_clear,
	.of_xlate		= acpi_gpe_of_xlate,
};

static const struct udevice_id acpi_gpe_ids[] = {
	{ .compatible = "intel,acpi-gpe", .data = X86_IRQT_ACPI_GPE },
	{ }
};

U_BOOT_DRIVER(acpi_gpe_drv) = {
	.name		= "acpi_gpe",
	.id		= UCLASS_IRQ,
	.of_match	= acpi_gpe_ids,
	.ops		= &acpi_gpe_ops,
	.ofdata_to_platdata	= acpi_gpe_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct acpi_gpe_priv),
};
