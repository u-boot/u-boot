// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019, 2024 NXP
 */

#include <cpu.h>
#include <dm.h>
#include <thermal.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx/cpu.h>
#include <asm/armv8/cpu.h>
#include <imx_thermal.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/psci.h>

DECLARE_GLOBAL_DATA_PTR;

#define IMX_REV_LEN	4
struct cpu_imx_plat {
	const char *name;
	const char *type;
	char rev[IMX_REV_LEN];
	u32 cpu_rsrc;
	u32 cpurev;
	u32 freq_mhz;
	u32 mpidr;
};

static const char *get_imx_type_str(u32 imxtype)
{
	switch (imxtype) {
	case MXC_CPU_IMX8MM:
		return "8MMQ";	/* Quad-core version of the imx8mm */
	case MXC_CPU_IMX8MML:
		return "8MMQL";	/* Quad-core Lite version of the imx8mm */
	case MXC_CPU_IMX8MMD:
		return "8MMD";	/* Dual-core version of the imx8mm */
	case MXC_CPU_IMX8MMDL:
		return "8MMDL";	/* Dual-core Lite version of the imx8mm */
	case MXC_CPU_IMX8MMS:
		return "8MMS";	/* Single-core version of the imx8mm */
	case MXC_CPU_IMX8MMSL:
		return "8MMSL";	/* Single-core Lite version of the imx8mm */
	case MXC_CPU_IMX8MN:
		return "8MNano Quad"; /* Quad-core version */
	case MXC_CPU_IMX8MND:
		return "8MNano Dual"; /* Dual-core version */
	case MXC_CPU_IMX8MNS:
		return "8MNano Solo"; /* Single-core version */
	case MXC_CPU_IMX8MNL:
		return "8MNano QuadLite"; /* Quad-core Lite version */
	case MXC_CPU_IMX8MNDL:
		return "8MNano DualLite"; /* Dual-core Lite version */
	case MXC_CPU_IMX8MNSL:
		return "8MNano SoloLite";/* Single-core Lite version of the imx8mn */
	case MXC_CPU_IMX8MNUQ:
		return "8MNano UltraLite Quad";/* Quad-core UltraLite version of the imx8mn */
	case MXC_CPU_IMX8MNUD:
		return "8MNano UltraLite Dual";/* Dual-core UltraLite version of the imx8mn */
	case MXC_CPU_IMX8MNUS:
		return "8MNano UltraLite Solo";/* Single-core UltraLite version of the imx8mn */
	case MXC_CPU_IMX8MP:
		return "8MP[8]";	/* Quad-core version of the imx8mp */
	case MXC_CPU_IMX8MPD:
		return "8MP Dual[3]";	/* Dual-core version of the imx8mp */
	case MXC_CPU_IMX8MPL:
		return "8MP Lite[4]";	/* Quad-core Lite version of the imx8mp */
	case MXC_CPU_IMX8MP6:
		return "8MP[6]";	/* Quad-core version of the imx8mp, NPU fused */
	case MXC_CPU_IMX8MQ:
		return "8MQ";	/* Quad-core version of the imx8mq */
	case MXC_CPU_IMX8MQL:
		return "8MQLite";	/* Quad-core Lite version of the imx8mq */
	case MXC_CPU_IMX8MD:
		return "8MD";	/* Dual-core version of the imx8mq */
	case MXC_CPU_IMX8QXP:
	case MXC_CPU_IMX8QXP_A0:
		return "8QXP";
	case MXC_CPU_IMX8QM:
		return "8QM";
	case MXC_CPU_IMX93:
		return "93(52)";/* iMX93 Dual core with NPU */
	case MXC_CPU_IMX9351:
		return "93(51)";/* iMX93 Single core with NPU */
	case MXC_CPU_IMX9332:
		return "93(32)";/* iMX93 Dual core without NPU */
	case MXC_CPU_IMX9331:
		return "93(31)";/* iMX93 Single core without NPU */
	case MXC_CPU_IMX9322:
		return "93(22)";/* iMX93 9x9 Dual core  */
	case MXC_CPU_IMX9321:
		return "93(21)";/* iMX93 9x9 Single core  */
	case MXC_CPU_IMX9312:
		return "93(12)";/* iMX93 9x9 Dual core without NPU */
	case MXC_CPU_IMX9311:
		return "93(11)";/* iMX93 9x9 Single core without NPU */
	case MXC_CPU_IMX9302:
		return "93(02)";/* iMX93 900Mhz Low performance Dual core without NPU */
	case MXC_CPU_IMX9301:
		return "93(01)";/* iMX93 900Mhz Low performance Single core without NPU */
	case MXC_CPU_IMX91:
		return "91(31)";/* iMX91 11x11 Full feature */
	case MXC_CPU_IMX9121:
		return "91(21)";/* iMX91 11x11 Low drive mode */
	case MXC_CPU_IMX9111:
		return "91(11)";/* iMX91 9x9 Reduced feature */
	case MXC_CPU_IMX9101:
		return "91(01)";/* iMX91 9x9 Specific feature */
	case MXC_CPU_IMX95:
		return "95";
	default:
		return "??";
	}
}

static void get_imx_rev_str(struct cpu_imx_plat *plat, u32 rev)
{
	if (IS_ENABLED(CONFIG_IMX8)) {
		switch (rev) {
		case CHIP_REV_A:
			plat->rev[0] = 'A';
			break;
		case CHIP_REV_B:
			plat->rev[0] = 'B';
			break;
		case CHIP_REV_C:
			plat->rev[0] = 'C';
			break;
		default:
			plat->rev[0] = '?';
			break;
		}
		plat->rev[1] = '\0';
	} else {
		plat->rev[0] = '1' + (((rev & 0xf0) - CHIP_REV_1_0) >> 4);
		plat->rev[1] = '.';
		plat->rev[2] = '0' + (rev & 0xf);
		plat->rev[3] = '\0';
	}
}

static void set_core_data(struct udevice *dev)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);

	if (device_is_compatible(dev, "arm,cortex-a35")) {
		plat->cpu_rsrc = SC_R_A35;
		plat->name = "A35";
	} else if (device_is_compatible(dev, "arm,cortex-a53")) {
		plat->cpu_rsrc = SC_R_A53;
		plat->name = "A53";
	} else if (device_is_compatible(dev, "arm,cortex-a72")) {
		plat->cpu_rsrc = SC_R_A72;
		plat->name = "A72";
	} else if (device_is_compatible(dev, "arm,cortex-a55")) {
		plat->name = "A55";
	} else {
		plat->cpu_rsrc = SC_R_A53;
		plat->name = "?";
	}
}

#if IS_ENABLED(CONFIG_DM_THERMAL)
static int cpu_imx_get_temp(struct cpu_imx_plat *plat)
{
	struct udevice *thermal_dev;
	int cpu_tmp, ret;
	int idx = 1; /* use "cpu-thermal0" device */

	if (IS_ENABLED(CONFIG_IMX8)) {
		if (plat->cpu_rsrc == SC_R_A72)
			idx = 2; /* use "cpu-thermal1" device */
	} else if (IS_ENABLED(CONFIG_IMX91)) {
		idx = 0;
	} else {
		idx = 1;
	}

	ret = uclass_get_device(UCLASS_THERMAL, idx, &thermal_dev);
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
static int cpu_imx_get_temp(struct cpu_imx_plat *plat)
{
	return 0;
}
#endif

__weak u32 get_cpu_temp_grade(int *minc, int *maxc)
{
	if (minc && maxc) {
		*minc = 0;
		*maxc = 95;
	}
	return 0;
}

static int cpu_imx_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);
	const char *grade;
	int ret, temp;
	int minc, maxc;

	if (size < 100)
		return -ENOSPC;

	ret = snprintf(buf, size, "NXP i.MX%s Rev%s %s at %u MHz",
		       plat->type, plat->rev, plat->name, plat->freq_mhz);

	if (IS_ENABLED(CONFIG_IMX_TMU)) {
		switch (get_cpu_temp_grade(&minc, &maxc)) {
		case TEMP_AUTOMOTIVE:
			grade = "Automotive temperature grade";
			break;
		case TEMP_INDUSTRIAL:
			grade = "Industrial temperature grade";
			break;
		case TEMP_EXTCOMMERCIAL:
			grade = "Extended Consumer temperature grade";
			break;
		default:
			grade = "Consumer temperature grade";
			break;
		}

		buf = buf + ret;
		size = size - ret;
		ret = snprintf(buf, size, "\nCPU:   %s (%dC to %dC)", grade, minc, maxc);
	}

	if (IS_ENABLED(CONFIG_DM_THERMAL)) {
		temp = cpu_imx_get_temp(plat);
		buf = buf + ret;
		size = size - ret;
		if (temp != 0xdeadbeef)
			ret = snprintf(buf, size, " at %dC", temp);
		else
			ret = snprintf(buf, size, " - invalid sensor data");
	}

	return 0;
}

static int cpu_imx_get_info(const struct udevice *dev, struct cpu_info *info)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);

	info->cpu_freq = plat->freq_mhz * 1000000;
	info->features = BIT(CPU_FEAT_L1_CACHE) | BIT(CPU_FEAT_MMU);
	return 0;
}

static int cpu_imx_get_count(const struct udevice *dev)
{
	ofnode node;
	int num = 0;

	ofnode_for_each_subnode(node, dev_ofnode(dev->parent)) {
		const char *device_type;

		if (!ofnode_is_enabled(node))
			continue;

		device_type = ofnode_read_string(node, "device_type");
		if (!device_type)
			continue;

		if (!strcmp(device_type, "cpu"))
			num++;
	}

	return num;
}

static int cpu_imx_get_vendor(const struct udevice *dev,  char *buf, int size)
{
	snprintf(buf, size, "NXP");
	return 0;
}

static int cpu_imx_is_current(struct udevice *dev)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);

	if (plat->mpidr == (read_mpidr() & 0xffff))
		return 1;

	return 0;
}

static int cpu_imx_release_core(const struct udevice *dev, phys_addr_t addr)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);
	struct pt_regs regs;

	regs.regs[0] = PSCI_0_2_FN64_CPU_ON;
	regs.regs[1] = plat->mpidr;
	regs.regs[2] = addr;
	regs.regs[3] = 0;

	smc_call(&regs);
	if (regs.regs[0]) {
		printf("Failed to release CPU core (mpidr: 0x%x)\n", plat->mpidr);
		return -1;
	}

	printf("Released CPU core (mpidr: 0x%x) to address 0x%llx\n", plat->mpidr, addr);

	return 0;
}

static const struct cpu_ops cpu_imx_ops = {
	.get_desc	= cpu_imx_get_desc,
	.get_info	= cpu_imx_get_info,
	.get_count	= cpu_imx_get_count,
	.get_vendor	= cpu_imx_get_vendor,
	.is_current	= cpu_imx_is_current,
	.release_core	= cpu_imx_release_core,
};

static const struct udevice_id cpu_imx_ids[] = {
	{ .compatible = "arm,cortex-a35" },
	{ .compatible = "arm,cortex-a53" },
	{ .compatible = "arm,cortex-a55" },
	{ .compatible = "arm,cortex-a72" },
	{ }
};

static ulong imx_get_cpu_rate(struct udevice *dev)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);
	struct clk clk;
	ulong rate;
	int ret;

	if (IS_ENABLED(CONFIG_IMX8)) {
		ret = sc_pm_get_clock_rate(-1, plat->cpu_rsrc, SC_PM_CLK_CPU,
					   (sc_pm_clock_rate_t *)&rate);
	} else {
		ret = clk_get_by_index(dev, 0, &clk);
		if (!ret) {
			rate = clk_get_rate(&clk);
			if (!rate)
				ret = -EOPNOTSUPP;
		}
	}
	if (ret) {
		printf("Could not read CPU frequency: %d\n", ret);
		return 0;
	}

	return rate;
}

static int imx_cpu_probe(struct udevice *dev)
{
	struct cpu_imx_plat *plat = dev_get_plat(dev);
	u32 cpurev;

	set_core_data(dev);
	cpurev = get_cpu_rev();
	plat->cpurev = cpurev;
	get_imx_rev_str(plat, cpurev & 0xFFF);
	plat->type = get_imx_type_str((cpurev & 0x1FF000) >> 12);
	plat->freq_mhz = imx_get_cpu_rate(dev) / 1000000;
	plat->mpidr = dev_read_addr(dev);
	if (plat->mpidr == FDT_ADDR_T_NONE) {
		printf("%s: Failed to get CPU reg property\n", __func__);
		return -EINVAL;
	}

	return 0;
}

U_BOOT_DRIVER(cpu_imx_drv) = {
	.name		= "imx_cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_imx_ids,
	.ops		= &cpu_imx_ops,
	.probe		= imx_cpu_probe,
	.plat_auto	= sizeof(struct cpu_imx_plat),
	.flags		= DM_FLAG_PRE_RELOC,
};
