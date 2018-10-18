// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass.h>
#include <errno.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch-imx/cpu.h>
#include <asm/armv8/cpu.h>

DECLARE_GLOBAL_DATA_PTR;

u32 get_cpu_rev(void)
{
	u32 id = 0, rev = 0;
	int ret;

	ret = sc_misc_get_control(-1, SC_R_SYSTEM, SC_C_ID, &id);
	if (ret)
		return 0;

	rev = (id >> 5)  & 0xf;
	id = (id & 0x1f) + MXC_SOC_IMX8;  /* Dummy ID for chip */

	return (id << 12) | rev;
}

#ifdef CONFIG_DISPLAY_CPUINFO
const char *get_imx8_type(u32 imxtype)
{
	switch (imxtype) {
	case MXC_CPU_IMX8QXP:
		return "8QXP";
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
	else
		return "?";
}

int print_cpuinfo(void)
{
	struct udevice *dev;
	struct clk cpu_clk;
	int ret;

	ret = uclass_get_device(UCLASS_CPU, 0, &dev);
	if (ret)
		return 0;

	ret = clk_get_by_index(dev, 0, &cpu_clk);
	if (ret) {
		dev_err(dev, "failed to clk\n");
		return 0;
	}

	u32 cpurev;

	cpurev = get_cpu_rev();

	printf("CPU:   Freescale i.MX%s rev%s %s at %ld MHz\n",
	       get_imx8_type((cpurev & 0xFF000) >> 12),
	       get_imx8_rev((cpurev & 0xFFF)),
	       get_core_name(),
	       clk_get_rate(&cpu_clk) / 1000000);

	return 0;
}
#endif
