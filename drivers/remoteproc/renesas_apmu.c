// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2024 Renesas Electronics Corp.
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <errno.h>
#include <hang.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <malloc.h>
#include <remoteproc.h>

/* R-Car V4H/V4M contain 3 clusters / 3 cores */
#define RCAR4_CR52_CORES		3

/* Reset Control Register for Cortex-R52 #n */
#define APMU_CRRSTCTRL(n)		(0x304 + ((n) * 0x40))
#define APMU_CRRSTCTRL_CR52RST		BIT(0)

/* Base Address Register for Cortex-R52 #n */
#define APMU_CRBARP(n)			(0x33c + ((n) * 0x40))
#define APMU_CRBARP_CR_VLD_BARP		BIT(0)
#define APMU_CRBARP_CR_BAREN_VALID	BIT(4)
#define APMU_CRBARP_CR_RBAR_MASK	0xfffc0000
#define APMU_CRBARP_CR_RBAR_ALIGN	0x40000

/**
 * struct renesas_apmu_rproc_privdata - remote processor private data
 * @regs:		controller registers
 * @core_id:		CPU core id
 * @trampoline:		jump trampoline code
 */
struct renesas_apmu_rproc_privdata {
	void __iomem	*regs;
	ulong		core_id;
	u32		*trampoline;
};

/*
 * CRBARP address is aligned to 0x40000 / 256 kiB , this trampoline
 * allows arbitrary address alignment at instruction granularity.
 */
static const u32 renesas_apmu_rproc_trampoline[4] = {
	0xe59f0004,	/* ldr r0, [pc, #4] */
	0xe1a0f000,	/* mov pc, r0 */
	0xeafffffe,	/* 1: b 1b */
	0xabcd1234	/* jump target (rewritten on load) */
};

/**
 * renesas_apmu_rproc_load() - Load the remote processor
 * @dev:	corresponding remote processor device
 * @addr:	Address in memory where image is stored
 * @size:	Size in bytes of the image
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_load(struct udevice *dev, ulong addr, ulong size)
{
	struct renesas_apmu_rproc_privdata *priv = dev_get_priv(dev);
	u32 trampolineaddr = (u32)(uintptr_t)(priv->trampoline);

	priv->trampoline[3] = addr;
	flush_dcache_range(trampolineaddr,
			   trampolineaddr +
			   sizeof(renesas_apmu_rproc_trampoline));
	invalidate_dcache_range(trampolineaddr,
				trampolineaddr +
				sizeof(renesas_apmu_rproc_trampoline));
	flush_dcache_range(addr, addr + size);
	invalidate_dcache_range(addr, addr + size);
	asm volatile("dsb sy\n");
	asm volatile("isb sy\n");

	/* CR52 boot address set */
	writel(trampolineaddr | APMU_CRBARP_CR_VLD_BARP,
	       priv->regs + APMU_CRBARP(priv->core_id));
	writel(trampolineaddr | APMU_CRBARP_CR_VLD_BARP | APMU_CRBARP_CR_BAREN_VALID,
	       priv->regs + APMU_CRBARP(priv->core_id));

	return 0;
}

/**
 * renesas_apmu_rproc_start() - Start the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_start(struct udevice *dev)
{
	struct renesas_apmu_rproc_privdata *priv = dev_get_priv(dev);

	/* Clear APMU_CRRSTCTRL_CR52RST, the only bit in this register */
	writel(0, priv->regs + APMU_CRRSTCTRL(priv->core_id));

	return 0;
}

/**
 * renesas_apmu_rproc_stop() - Stop the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_stop(struct udevice *dev)
{
	struct renesas_apmu_rproc_privdata *priv = dev_get_priv(dev);

	/* Set APMU_CRRSTCTRL_CR52RST, the only bit in this register */
	writel(APMU_CRRSTCTRL_CR52RST,
	       priv->regs + APMU_CRRSTCTRL(priv->core_id));

	return 0;
}

/**
 * renesas_apmu_rproc_reset() - Reset the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_reset(struct udevice *dev)
{
	renesas_apmu_rproc_stop(dev);
	renesas_apmu_rproc_start(dev);
	return 0;
}

/**
 * renesas_apmu_rproc_is_running() - Is the remote processor running
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if the remote processor is running, 1 otherwise
 */
static int renesas_apmu_rproc_is_running(struct udevice *dev)
{
	struct renesas_apmu_rproc_privdata *priv = dev_get_priv(dev);

	return readl(priv->regs + APMU_CRRSTCTRL(priv->core_id)) &
	       APMU_CRRSTCTRL_CR52RST;
}

/**
 * renesas_apmu_rproc_init() - Initialize the remote processor CRBAR registers
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_init(struct udevice *dev)
{
	struct renesas_apmu_rproc_privdata *priv = dev_get_priv(dev);

	/* If the core is running already, do nothing. */
	if (renesas_apmu_rproc_is_running(dev))
		return 0;

	/* Clear and invalidate CRBARP content */
	writel(0, priv->regs + APMU_CRBARP(priv->core_id));

	return 0;
}

/**
 * renesas_apmu_rproc_device_to_virt() - Convert device address to virtual address
 * @dev:	corresponding remote processor device
 * @da:		device address
 * @size:	Size of the memory region @da is pointing to
 *
 * Return: converted virtual address
 */
static void *renesas_apmu_rproc_device_to_virt(struct udevice *dev, ulong da,
					       ulong size)
{
	/*
	 * The Cortex R52 and A76 share the same address space,
	 * this operation is a no-op.
	 */
	return (void *)da;
}

static const struct dm_rproc_ops renesas_apmu_rproc_ops = {
	.init		= renesas_apmu_rproc_init,
	.load		= renesas_apmu_rproc_load,
	.start		= renesas_apmu_rproc_start,
	.stop		= renesas_apmu_rproc_stop,
	.reset		= renesas_apmu_rproc_reset,
	.is_running	= renesas_apmu_rproc_is_running,
	.device_to_virt	= renesas_apmu_rproc_device_to_virt,
};

/**
 * renesas_apmu_rproc_of_to_plat() - Convert OF data to platform data
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_of_to_plat(struct udevice *dev)
{
	struct renesas_apmu_rproc_privdata *priv = dev_get_priv(dev);

	priv->core_id = dev_get_driver_data(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->trampoline = memalign(APMU_CRBARP_CR_RBAR_ALIGN,
				    sizeof(renesas_apmu_rproc_trampoline));
	if (!priv->trampoline)
		return -ENOMEM;

	memcpy(priv->trampoline, renesas_apmu_rproc_trampoline,
	       sizeof(renesas_apmu_rproc_trampoline));

	return 0;
}

U_BOOT_DRIVER(renesas_apmu_cr52) = {
	.name		= "rcar-apmu-cr52",
	.id		= UCLASS_REMOTEPROC,
	.ops		= &renesas_apmu_rproc_ops,
	.of_to_plat	= renesas_apmu_rproc_of_to_plat,
	.priv_auto	= sizeof(struct renesas_apmu_rproc_privdata),
};

/**
 * renesas_apmu_rproc_bind() - Bind rproc driver to each core control
 * @dev:	corresponding remote processor parent device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_bind(struct udevice *parent)
{
	const ulong cr52cores = RCAR4_CR52_CORES;
	ofnode pnode = dev_ofnode(parent);
	struct udevice *cdev;
	struct driver *cdrv;
	char name[32];
	ulong i;
	int ret;

	cdrv = lists_driver_lookup_name("rcar-apmu-cr52");
	if (!cdrv)
		return -ENOENT;

	for (i = 0; i < cr52cores; i++) {
		snprintf(name, sizeof(name), "rcar-apmu-cr52.%ld", i);
		ret = device_bind_with_driver_data(parent, cdrv, strdup(name),
						   i, pnode, &cdev);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id renesas_apmu_rproc_ids[] = {
	{ .compatible = "renesas,r8a779g0-cr52" },
	{ .compatible = "renesas,r8a779h0-cr52" },
	{ }
};

U_BOOT_DRIVER(renesas_apmu_rproc) = {
	.name		= "rcar-apmu-rproc",
	.of_match	= renesas_apmu_rproc_ids,
	.id		= UCLASS_NOP,
	.bind		= renesas_apmu_rproc_bind,
};
