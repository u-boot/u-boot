// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 9elements GmbH
 */

#include <cpu.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdt_support.h>
#include <acpi/acpigen.h>
#include <asm/armv8/cpu.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <asm-generic/sections.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include "armv8_cpu.h"

DECLARE_GLOBAL_DATA_PTR;

struct bcm_plat {
	u64 release_addr;
};

static int cpu_bcm_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);
	const char *name;

	if (size < 32)
		return -ENOSPC;

	if (device_is_compatible(dev, "arm,cortex-a53"))
		name = "A53";
	else if (device_is_compatible(dev, "arm,cortex-a72"))
		name = "A72";
	else
		name = "?";

	snprintf(buf, size, "Broadcom Cortex-%s at %u MHz\n",
		 name, plat->timebase_freq);

	return 0;
}

static int cpu_bcm_get_info(const struct udevice *dev, struct cpu_info *info)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	info->cpu_freq = plat->timebase_freq * 1000;
	info->features = BIT(CPU_FEAT_L1_CACHE) | BIT(CPU_FEAT_MMU);

	return 0;
}

static int cpu_bcm_get_count(const struct udevice *dev)
{
	return uclass_id_count(UCLASS_CPU);
}

static int cpu_bcm_get_vendor(const struct udevice *dev,  char *buf, int size)
{
	snprintf(buf, size, "Broadcom");

	return 0;
}

static int cpu_bcm_is_current(struct udevice *dev)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	if (plat->cpu_id == (read_mpidr() & 0xffff))
		return 1;

	return 0;
}

/**
 * bcm_cpu_on - Releases the secondary CPU from it's spintable
 *
 * Write the CPU's spintable mailbox and let the CPU enter U-Boot.
 *
 * @dev: Device to start
 * @return: zero on success or error code on failure.
 */
static int bcm_cpu_on(struct udevice *dev)
{
	struct bcm_plat *plat = dev_get_plat(dev);
	ulong *start_address;

	if (plat->release_addr == ~0ULL)
		return -ENODATA;

	start_address = map_physmem(plat->release_addr, sizeof(uintptr_t), MAP_NOCACHE);

	/* Point secondary CPU to U-Boot entry */
	*start_address = (uintptr_t)_start;

	/* Make sure the other CPUs see the written start address */
	if (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		flush_dcache_all();

	/* Send an event to wake up the secondary CPU. */
	asm("dsb	ishst\n"
	    "sev");

	unmap_physmem(start_address, MAP_NOCACHE);

	return 0;
}

static const struct cpu_ops cpu_bcm_ops = {
	.get_desc	= cpu_bcm_get_desc,
	.get_info	= cpu_bcm_get_info,
	.get_count	= cpu_bcm_get_count,
	.get_vendor	= cpu_bcm_get_vendor,
	.is_current	= cpu_bcm_is_current,
};

static const struct udevice_id cpu_bcm_ids[] = {
	{ .compatible = "arm,cortex-a53" },	/* RPi 3 */
	{ .compatible = "arm,cortex-a72" },	/* RPi 4 */
	{ }
};

static int bcm_cpu_bind(struct udevice *dev)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	plat->cpu_id = dev_read_addr(dev);

	return 0;
}

/**
 * bcm_cpu_of_to_plat - Gather spin-table release address
 *
 * Read the spin-table release address to allow all seconary CPUs to enter
 * U-Boot when necessary.
 *
 * @dev: Device to start
 */
static int bcm_cpu_of_to_plat(struct udevice *dev)
{
	struct bcm_plat *plat = dev_get_plat(dev);
	const char *prop;

	if (CONFIG_IS_ENABLED(ARMV8_MULTIENTRY)) {
		plat->release_addr = ~0ULL;

		prop = dev_read_string(dev, "enable-method");
		if (!prop || strcmp(prop, "spin-table"))
			return -ENODEV;

		plat->release_addr = dev_read_u64_default(dev, "cpu-release-addr", ~0ULL);

		if (plat->release_addr == ~0ULL)
			return -ENODEV;
	}

	return 0;
}

static int bcm_cpu_probe(struct udevice *dev)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);
	struct clk clk;
	int ret;

	/* Get a clock if it exists */
	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret) {
		ret = clk_enable(&clk);
		if (ret && (ret != -ENOSYS || ret != -EOPNOTSUPP))
			return ret;
		ret = clk_get_rate(&clk);
		if (IS_ERR_VALUE(ret))
			return ret;
		plat->timebase_freq = ret;
	}

	/*
	 * The armstub holds the secondary CPUs in a spinloop. When
	 * ARMV8_MULTIENTRY is enabled release the secondary CPUs and
	 * let them enter U-Boot as well.
	 */
	if (CONFIG_IS_ENABLED(ARMV8_MULTIENTRY)) {
		ret = bcm_cpu_on(dev);
		if (ret)
			return ret;
	}

	return ret;
}

struct acpi_ops bcm283x_cpu_acpi_ops = {
	.fill_ssdt	= armv8_cpu_fill_ssdt,
	.fill_madt	= armv8_cpu_fill_madt,
};

U_BOOT_DRIVER(cpu_bcm_drv) = {
	.name		= "bcm283x_cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_bcm_ids,
	.ops		= &cpu_bcm_ops,
	.probe		= bcm_cpu_probe,
	.bind		= bcm_cpu_bind,
	.of_to_plat	= bcm_cpu_of_to_plat,
	.plat_auto	= sizeof(struct bcm_plat),
	ACPI_OPS_PTR(&bcm283x_cpu_acpi_ops)
};
