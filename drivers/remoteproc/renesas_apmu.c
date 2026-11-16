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

#define RCAR_APMU_TIMEOUT_US		1000000

/* R-Car V4H/V4M contain 3 lockstep Cortex-R52 and 4 Cortex-A76 cores. */
#define RCAR4_CR52_CORES		3
#define RCAR4_CA76_CORES		4

/* Reset Control Register for Cortex-R52 #n */
#define APMU_CRRSTCTRL(n)		(0x304 + ((n) * 0x40))
#define APMU_CRRSTCTRL_CR52RST		BIT(0)

/* Base Address Register for Cortex-R52 #n */
#define APMU_CRBARP(n)			(0x33c + ((n) * 0x40))
#define APMU_CRBARP_CR_VLD_BARP		BIT(0)
#define APMU_CRBARP_CR_BAREN_VALID	BIT(4)
#define APMU_CRBARP_CR_RBAR_MASK	0xfffc0000
#define APMU_CRBARP_CR_RBAR_ALIGN	0x40000

/* Base Address Register for Cortex-A76 #n */
#define APMU_CL_BASE(n)			(0x400 + (((n) & 0x3) * 0x40))
#define APMU_CL_PWRCTRL(n)		APMU_CL_BASE(n)
#define APMU_CL_PWRCTRL_PCHPDNEN	BIT(17)
#define APMU_CA_BASE(n)			(0x800 + (((n) & 0x6) * 0x100) + (((n) & 0x1) * 0x40))
#define APMU_CA_PWRCTRL(n)		APMU_CA_BASE(n)
#define APMU_CA_PWRCTRL_WUP_REQ		BIT(0)
#define APMU_CA_PWRCTRL_IWUP_EN		BIT(4)
#define APMU_CA_PWRCTRL_PCHPDNEN	BIT(17)
#define APMU_CA_FSMSTSR(n)		(APMU_CA_BASE(n) + 0x18)
#define APMU_CA_SAFECTRL(n)		(APMU_CA_BASE(n) + 0x20)
#define APMU_CA_SAFECTRL_DBGGEN		BIT(13)
#define APMU_CA_RVBARPL(n)		(APMU_CA_BASE(n) + 0x38)
#define APMU_CA_RVBARPL_MASK		0xFFFFFFFC
#define APMU_CA_RVBARPL_VLD		BIT(0)
#define APMU_CA_RVBARPH(n)		(APMU_CA_BASE(n) + 0x3c)

/* Cortex-A76 APSREG */
#define CA76_APSREG_BASE		0xe6280000
#define CA76_APSREG_AP_CLUSTER_AUX(n)	(0x10 + (((n) & 0x3) * 0x1000))
#define CA76_APSREG_AP_CLUSTER_AUX_INIT	0x3

/**
 * struct renesas_apmu_rproc_cr52_privdata - remote processor private data
 * @regs:		controller registers
 * @core_id:		CPU core id
 * @trampoline:		jump trampoline code
 */
struct renesas_apmu_rproc_cr52_privdata {
	void __iomem	*regs;
	ulong		core_id;
	u32		*trampoline;
};

/**
 * struct renesas_apmu_rproc_ca76_privdata - remote processor private data
 * @regs:		controller registers
 * @core_id:		CPU core id
 * @trampoline:		jump trampoline code
 */
struct renesas_apmu_rproc_ca76_privdata {
	void __iomem	*regs;
	ulong		core_id;
	u32		*trampoline;
};

/*
 * CRBARP address is aligned to 0x40000 / 256 kiB , this trampoline
 * allows arbitrary address alignment at instruction granularity.
 */
static const u32 renesas_apmu_rproc_cr52_trampoline[4] = {
	0xe59f0004,	/* ldr r0, [pc, #4] */
	0xe1a0f000,	/* mov pc, r0 */
	0xeafffffe,	/* 1: b 1b */
	0xabcd1234	/* jump target (rewritten on load) */
};

/*
 * CRBARP address is aligned to 0x40000 / 256 kiB , this trampoline
 * allows arbitrary address alignment at instruction granularity.
 */
static const u32 renesas_apmu_rproc_ca76_trampoline[4] = {
	0x58000040,	/* ldr x0, 8 */
	0xd61f0000,	/* br x0 */
	0x567890ef,	/* jump target LSB (rewritten on load) */
	0x1234abcd,	/* jump target MSB (rewritten on load) */
};

/**
 * renesas_apmu_rproc_cr52_load() - Load the remote processor
 * @dev:	corresponding remote processor device
 * @addr:	Address in memory where image is stored
 * @size:	Size in bytes of the image
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_cr52_load(struct udevice *dev, ulong addr, ulong size)
{
	struct renesas_apmu_rproc_cr52_privdata *priv = dev_get_priv(dev);
	u32 trampolineaddr = (u32)(uintptr_t)(priv->trampoline);

	priv->trampoline[3] = addr;
	flush_dcache_range(trampolineaddr,
			   trampolineaddr +
			   sizeof(renesas_apmu_rproc_cr52_trampoline));
	invalidate_dcache_range(trampolineaddr,
				trampolineaddr +
				sizeof(renesas_apmu_rproc_cr52_trampoline));
	flush_dcache_range(addr, addr + size);
	invalidate_dcache_range(addr, addr + size);
	asm volatile("dsb sy\n");
	asm volatile("isb sy\n");

	return 0;
}

/**
 * renesas_apmu_rproc_cr52_start() - Start the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_cr52_start(struct udevice *dev)
{
	struct renesas_apmu_rproc_cr52_privdata *priv = dev_get_priv(dev);
	u32 trampolineaddr = (u32)(uintptr_t)(priv->trampoline);

	/* CR52 boot address and valid flags set */
	writel(trampolineaddr | APMU_CRBARP_CR_VLD_BARP,
	       priv->regs + APMU_CRBARP(priv->core_id));
	writel(trampolineaddr | APMU_CRBARP_CR_VLD_BARP | APMU_CRBARP_CR_BAREN_VALID,
	       priv->regs + APMU_CRBARP(priv->core_id));

	/* Clear APMU_CRRSTCTRL_CR52RST, the only bit in this register */
	writel(0, priv->regs + APMU_CRRSTCTRL(priv->core_id));

	return 0;
}

/**
 * renesas_apmu_rproc_cr52_stop() - Stop the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_cr52_stop(struct udevice *dev)
{
	struct renesas_apmu_rproc_cr52_privdata *priv = dev_get_priv(dev);

	/* Set APMU_CRRSTCTRL_CR52RST, the only bit in this register */
	writel(APMU_CRRSTCTRL_CR52RST,
	       priv->regs + APMU_CRRSTCTRL(priv->core_id));

	return 0;
}

/**
 * renesas_apmu_rproc_cr52_reset() - Reset the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_cr52_reset(struct udevice *dev)
{
	renesas_apmu_rproc_cr52_stop(dev);
	renesas_apmu_rproc_cr52_start(dev);
	return 0;
}

/**
 * renesas_apmu_rproc_cr52_is_running() - Is the remote processor running
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if the remote processor is running, 1 otherwise
 */
static int renesas_apmu_rproc_cr52_is_running(struct udevice *dev)
{
	struct renesas_apmu_rproc_cr52_privdata *priv = dev_get_priv(dev);

	return readl(priv->regs + APMU_CRRSTCTRL(priv->core_id)) &
	       APMU_CRRSTCTRL_CR52RST;
}

/**
 * renesas_apmu_rproc_cr52_init() - Initialize the remote processor CRBAR registers
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_cr52_init(struct udevice *dev)
{
	struct renesas_apmu_rproc_cr52_privdata *priv = dev_get_priv(dev);

	/* If the core is running already, do nothing. */
	if (renesas_apmu_rproc_cr52_is_running(dev))
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
 * @is_iomem:	optional pointer filled in to indicate if @da is iomapped memory
 *
 * Return: converted virtual address
 */
static void *renesas_apmu_rproc_device_to_virt(struct udevice *dev, ulong da,
					       ulong size, bool *is_iomem)
{
	/*
	 * The Cortex R52 and A76 share the same address space,
	 * this operation is a no-op.
	 */
	return (void *)da;
}

static const struct dm_rproc_ops renesas_apmu_rproc_cr52_ops = {
	.init		= renesas_apmu_rproc_cr52_init,
	.load		= renesas_apmu_rproc_cr52_load,
	.start		= renesas_apmu_rproc_cr52_start,
	.stop		= renesas_apmu_rproc_cr52_stop,
	.reset		= renesas_apmu_rproc_cr52_reset,
	.is_running	= renesas_apmu_rproc_cr52_is_running,
	.device_to_virt	= renesas_apmu_rproc_device_to_virt,
};

/**
 * renesas_apmu_rproc_cr52_of_to_plat() - Convert OF data to platform data
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_cr52_of_to_plat(struct udevice *dev)
{
	struct renesas_apmu_rproc_cr52_privdata *priv = dev_get_priv(dev);

	priv->core_id = dev_get_driver_data(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->trampoline = memalign(APMU_CRBARP_CR_RBAR_ALIGN,
				    sizeof(renesas_apmu_rproc_cr52_trampoline));
	if (!priv->trampoline)
		return -ENOMEM;

	memcpy(priv->trampoline, renesas_apmu_rproc_cr52_trampoline,
	       sizeof(renesas_apmu_rproc_cr52_trampoline));

	return 0;
}

U_BOOT_DRIVER(renesas_apmu_cr52) = {
	.name		= "rcar-apmu-cr52",
	.id		= UCLASS_REMOTEPROC,
	.ops		= &renesas_apmu_rproc_cr52_ops,
	.of_to_plat	= renesas_apmu_rproc_cr52_of_to_plat,
	.priv_auto	= sizeof(struct renesas_apmu_rproc_cr52_privdata),
};

/**
 * renesas_apmu_rproc_ca76_load() - Load the remote processor
 * @dev:	corresponding remote processor device
 * @addr:	Address in memory where image is stored
 * @size:	Size in bytes of the image
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_ca76_load(struct udevice *dev, ulong addr, ulong size)
{
	struct renesas_apmu_rproc_ca76_privdata *priv = dev_get_priv(dev);
	u32 trampolineaddr = (u32)(uintptr_t)(priv->trampoline);

	priv->trampoline[2] = lower_32_bits(addr);
	priv->trampoline[3] = upper_32_bits(addr);
	flush_dcache_range(trampolineaddr,
			   trampolineaddr +
			   sizeof(renesas_apmu_rproc_ca76_trampoline));
	invalidate_dcache_range(trampolineaddr,
				trampolineaddr +
				sizeof(renesas_apmu_rproc_ca76_trampoline));
	flush_dcache_range(addr, addr + size);
	invalidate_dcache_range(addr, addr + size);
	asm volatile("dsb sy\n");
	asm volatile("isb sy\n");

	return 0;
}

/**
 * renesas_apmu_rproc_ca76_cluster() - Return cluster ID for core ID
 * @core:	core ID
 *
 * Return: cluster ID
 */
static int renesas_apmu_rproc_ca76_cluster(int core)
{
	/* R-Car V4M has 4 cores per cluster */
	if (renesas_get_cpu_type() == RENESAS_CPU_TYPE_R8A779H0)
		return core / 4;
	else
		return core / 2;
}

/**
 * renesas_apmu_rproc_ca76_start() - Start the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_ca76_start(struct udevice *dev)
{
	struct renesas_apmu_rproc_ca76_privdata *priv = dev_get_priv(dev);
	u32 val, trampolineaddr = (u32)(uintptr_t)(priv->trampoline);
	int cluster = renesas_apmu_rproc_ca76_cluster(priv->core_id);
	int ret;

	/* CA76 boot address and valid flags set */
	trampolineaddr &= APMU_CA_RVBARPL_MASK;
	trampolineaddr |= APMU_CA_RVBARPL_VLD;

	writel(upper_32_bits(trampolineaddr),
	       priv->regs + APMU_CA_RVBARPH(priv->core_id));
	writel(lower_32_bits(trampolineaddr),
	       priv->regs + APMU_CA_RVBARPL(priv->core_id));

	/* Clear Cluster OFF bit */
	clrbits_le32(priv->regs + APMU_CL_PWRCTRL(cluster),
		     APMU_CL_PWRCTRL_PCHPDNEN);

	/* Clear Core OFF bit */
	clrbits_le32(priv->regs + APMU_CA_PWRCTRL(priv->core_id),
		     APMU_CA_PWRCTRL_PCHPDNEN);
	ret = readl_poll_timeout(priv->regs + APMU_CA_PWRCTRL(priv->core_id),
				 val, !(val & APMU_CA_PWRCTRL_PCHPDNEN),
				 RCAR_APMU_TIMEOUT_US);
	if (ret)
		return -ETIMEDOUT;

	setbits_le32(priv->regs + APMU_CA_PWRCTRL(priv->core_id),
		     APMU_CA_PWRCTRL_WUP_REQ);

	/* Wait until CAXX wake up sequence finishes */
	return readl_poll_timeout(priv->regs + APMU_CA_PWRCTRL(priv->core_id),
				  val, !(val & APMU_CA_PWRCTRL_WUP_REQ),
				  RCAR_APMU_TIMEOUT_US);
}

/**
 * renesas_apmu_rproc_ca76_stop() - Stop the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_ca76_stop(struct udevice *dev)
{
	struct renesas_apmu_rproc_ca76_privdata *priv = dev_get_priv(dev);

	/* Clear DBGGEN_PPDN bit for core down to 'OFF' mode */
	clrbits_le32(priv->regs + APMU_CA_SAFECTRL(priv->core_id),
		     APMU_CA_SAFECTRL_DBGGEN);

	/* For Core OFF */
	setbits_le32(priv->regs + APMU_CA_PWRCTRL(priv->core_id),
		     APMU_CA_PWRCTRL_PCHPDNEN);

	return 0;
}

/**
 * renesas_apmu_rproc_ca76_reset() - Reset the remote processor
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_ca76_reset(struct udevice *dev)
{
	renesas_apmu_rproc_ca76_stop(dev);
	renesas_apmu_rproc_ca76_start(dev);
	return 0;
}

/**
 * renesas_apmu_rproc_ca76_is_running() - Is the remote processor running
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if the remote processor is running, 1 otherwise
 */
static int renesas_apmu_rproc_ca76_is_running(struct udevice *dev)
{
	struct renesas_apmu_rproc_ca76_privdata *priv = dev_get_priv(dev);

	return !readl(priv->regs + APMU_CA_FSMSTSR(priv->core_id));
}

/**
 * renesas_apmu_rproc_ca76_init() - Initialize the remote processor APSREG register
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_ca76_init(struct udevice *dev)
{
	struct renesas_apmu_rproc_ca76_privdata *priv = dev_get_priv(dev);
	int cluster = renesas_apmu_rproc_ca76_cluster(priv->core_id);

	/* If the core is running already, do nothing. */
	if (renesas_apmu_rproc_ca76_is_running(dev))
		return 0;

	/* Disable AC TEST */
	setbits_le32(((void __iomem *)CA76_APSREG_BASE) +
		     CA76_APSREG_AP_CLUSTER_AUX(cluster),
		     CA76_APSREG_AP_CLUSTER_AUX_INIT);

	return 0;
}

static const struct dm_rproc_ops renesas_apmu_rproc_ca76_ops = {
	.init		= renesas_apmu_rproc_ca76_init,
	.load		= renesas_apmu_rproc_ca76_load,
	.start		= renesas_apmu_rproc_ca76_start,
	.stop		= renesas_apmu_rproc_ca76_stop,
	.reset		= renesas_apmu_rproc_ca76_reset,
	.is_running	= renesas_apmu_rproc_ca76_is_running,
	.device_to_virt	= renesas_apmu_rproc_device_to_virt,
};

/**
 * renesas_apmu_rproc_ca76_of_to_plat() - Convert OF data to platform data
 * @dev:	corresponding remote processor device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_ca76_of_to_plat(struct udevice *dev)
{
	struct renesas_apmu_rproc_ca76_privdata *priv = dev_get_priv(dev);

	priv->core_id = dev_get_driver_data(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->trampoline = memalign(APMU_CRBARP_CR_RBAR_ALIGN,
				    sizeof(renesas_apmu_rproc_ca76_trampoline));
	if (!priv->trampoline)
		return -ENOMEM;

	memcpy(priv->trampoline, renesas_apmu_rproc_ca76_trampoline,
	       sizeof(renesas_apmu_rproc_ca76_trampoline));

	return 0;
}

U_BOOT_DRIVER(renesas_apmu_ca76) = {
	.name		= "rcar-apmu-ca76",
	.id		= UCLASS_REMOTEPROC,
	.ops		= &renesas_apmu_rproc_ca76_ops,
	.of_to_plat	= renesas_apmu_rproc_ca76_of_to_plat,
	.priv_auto	= sizeof(struct renesas_apmu_rproc_ca76_privdata),
};

/**
 * renesas_apmu_rproc_bind() - Bind rproc driver to each core control
 * @dev:	corresponding remote processor parent device
 *
 * Return: 0 if all went ok, else corresponding -ve error
 */
static int renesas_apmu_rproc_bind(struct udevice *parent)
{
	ofnode pnode = dev_ofnode(parent);
	struct driver *cdrvr52, *cdrva76;
	struct udevice *cdev;
	char name[32];
	ulong i;
	int ret;

	cdrvr52 = lists_driver_lookup_name("rcar-apmu-cr52");
	if (!cdrvr52)
		return -ENOENT;

	cdrva76 = lists_driver_lookup_name("rcar-apmu-ca76");
	if (!cdrva76)
		return -ENOENT;

	for (i = 0; i < RCAR4_CR52_CORES; i++) {
		snprintf(name, sizeof(name), "rcar-apmu-cr52.%ld", i);
		ret = device_bind_with_driver_data(parent, cdrvr52, strdup(name),
						   i, pnode, &cdev);
		if (ret)
			return ret;
	}

	for (i = 0; i < RCAR4_CA76_CORES; i++) {
		snprintf(name, sizeof(name), "rcar-apmu-ca76.%ld", i);
		ret = device_bind_with_driver_data(parent, cdrva76, strdup(name),
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
