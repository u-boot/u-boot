// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <thermal.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx/cpu.h>
#include <asm/armv8/cpu.h>

DECLARE_GLOBAL_DATA_PTR;

struct cpu_imx_platdata {
	const char *name;
	const char *rev;
	const char *type;
	u32 cpurev;
	u32 freq_mhz;
};

const char *get_imx8_type(u32 imxtype)
{
	switch (imxtype) {
	case MXC_CPU_IMX8QXP:
	case MXC_CPU_IMX8QXP_A0:
		return "QXP";
	case MXC_CPU_IMX8QM:
		return "QM";
	default:
		return "??";
	}
}

const char *get_imx8_rev(u32 rev)
{
	switch (rev) {
	case CHIP_REV_A:
		return "A";
	case CHIP_REV_B:
		return "B";
	default:
		return "?";
	}
}

const char *get_core_name(void)
{
	if (is_cortex_a35())
		return "A35";
	else if (is_cortex_a53())
		return "A53";
	else if (is_cortex_a72())
		return "A72";
	else
		return "?";
}

#if IS_ENABLED(CONFIG_IMX_SCU_THERMAL)
static int cpu_imx_get_temp(void)
{
	struct udevice *thermal_dev;
	int cpu_tmp, ret;

	ret = uclass_get_device_by_name(UCLASS_THERMAL, "cpu-thermal0",
					&thermal_dev);

	if (!ret) {
		ret = thermal_get_temp(thermal_dev, &cpu_tmp);
		if (ret)
			return 0xdeadbeef;
	} else {
		return 0xdeadbeef;
	}

	return cpu_tmp;
}
#else
static int cpu_imx_get_temp(void)
{
	return 0;
}
#endif

int cpu_imx_get_desc(struct udevice *dev, char *buf, int size)
{
	struct cpu_imx_platdata *plat = dev_get_platdata(dev);
	int ret;

	if (size < 100)
		return -ENOSPC;

	ret = snprintf(buf, size, "NXP i.MX8%s Rev%s %s at %u MHz",
		       plat->type, plat->rev, plat->name, plat->freq_mhz);

	if (IS_ENABLED(CONFIG_IMX_SCU_THERMAL)) {
		buf = buf + ret;
		size = size - ret;
		ret = snprintf(buf, size, " at %dC", cpu_imx_get_temp());
	}

	snprintf(buf + ret, size - ret, "\n");

	return 0;
}

static int cpu_imx_get_info(struct udevice *dev, struct cpu_info *info)
{
	struct cpu_imx_platdata *plat = dev_get_platdata(dev);

	info->cpu_freq = plat->freq_mhz * 1000;
	info->features = BIT(CPU_FEAT_L1_CACHE) | BIT(CPU_FEAT_MMU);
	return 0;
}

static int cpu_imx_get_count(struct udevice *dev)
{
	return 4;
}

static int cpu_imx_get_vendor(struct udevice *dev,  char *buf, int size)
{
	snprintf(buf, size, "NXP");
	return 0;
}

static const struct cpu_ops cpu_imx8_ops = {
	.get_desc	= cpu_imx_get_desc,
	.get_info	= cpu_imx_get_info,
	.get_count	= cpu_imx_get_count,
	.get_vendor	= cpu_imx_get_vendor,
};

static const struct udevice_id cpu_imx8_ids[] = {
	{ .compatible = "arm,cortex-a35" },
	{ .compatible = "arm,cortex-a53" },
	{ }
};

static ulong imx8_get_cpu_rate(void)
{
	ulong rate;
	int ret;
	int type = is_cortex_a35() ? SC_R_A35 : is_cortex_a53() ?
		   SC_R_A53 : SC_R_A72;

	ret = sc_pm_get_clock_rate(-1, type, SC_PM_CLK_CPU,
				   (sc_pm_clock_rate_t *)&rate);
	if (ret) {
		printf("Could not read CPU frequency: %d\n", ret);
		return 0;
	}

	return rate;
}

static int imx8_cpu_probe(struct udevice *dev)
{
	struct cpu_imx_platdata *plat = dev_get_platdata(dev);
	u32 cpurev;

	cpurev = get_cpu_rev();
	plat->cpurev = cpurev;
	plat->name = get_core_name();
	plat->rev = get_imx8_rev(cpurev & 0xFFF);
	plat->type = get_imx8_type((cpurev & 0xFF000) >> 12);
	plat->freq_mhz = imx8_get_cpu_rate() / 1000000;
	return 0;
}

U_BOOT_DRIVER(cpu_imx8_drv) = {
	.name		= "imx8x_cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_imx8_ids,
	.ops		= &cpu_imx8_ops,
	.probe		= imx8_cpu_probe,
	.platdata_auto_alloc_size = sizeof(struct cpu_imx_platdata),
	.flags		= DM_FLAG_PRE_RELOC,
};
