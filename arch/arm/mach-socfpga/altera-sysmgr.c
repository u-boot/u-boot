// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

/*
 * This driver supports the SOCFPGA System Manager Register block which
 * aggregates different peripheral function into one area.
 * On 64 bit ARM parts, the system manager only can be accessed during
 * EL3 mode. At lower exception level a SMC call is required to perform
 * the read and write operation.
 */

#define LOG_CATEGORY UCLASS_NOP

#include <dm.h>
#include <log.h>
#include <misc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/altera-sysmgr.h>
#include <asm/arch/smc_api.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/intel-smc.h>

static int altr_sysmgr_read_generic(struct udevice *dev, u32 *addr, u32 *value)
{
	u64 args[1];
	u64 ret_arg;
	int ret = 0;

	debug("%s: %s(dev=%p, addr=0x%lx):\n", __func__,
	      dev->name, dev, (uintptr_t)addr);

	if (current_el() == 3) {
		ret_arg = readl((uintptr_t)addr);
	} else {
		if (!(IS_ENABLED(CONFIG_SPL_BUILD)) && IS_ENABLED(CONFIG_SPL_ATF)) {
			args[0] = (u64)(uintptr_t)addr;
			ret = invoke_smc(INTEL_SIP_SMC_REG_READ, args, 1, &ret_arg, 1);
		} else {
			pr_err("%s Failed to read system manager at lower privilege and without BL31\n",
			       dev->name);
			return -EPROTONOSUPPORT;
		}
	}

	*value = (u32)ret_arg;
	return ret;
}

static int altr_sysmgr_write_generic(struct udevice *dev, u32 *addr, u32 value)
{
	u64 args[2];
	int ret = 0;

	debug("%s: %s(dev=%p, addr=0x%lx, val=0x%x):\n", __func__,
	      dev->name, dev, (uintptr_t)addr, value);

	if (current_el() == 3) {
		writel(value, (uintptr_t)addr);
	} else {
		if (!(IS_ENABLED(CONFIG_SPL_BUILD)) && IS_ENABLED(CONFIG_SPL_ATF)) {
			args[0] = (u64)(uintptr_t)(addr);
			args[1] = value;
			ret = invoke_smc(INTEL_SIP_SMC_REG_WRITE, args, 2, NULL, 0);
		} else {
			pr_err("%s Failed to write to system manager at lower privilege and without BL31\n",
			       dev->name);
			return -EPROTONOSUPPORT;
		}
	}

	return ret;
}

static int altr_sysmgr_probe(struct udevice *dev)
{
	fdt_addr_t addr;
	struct altr_sysmgr_priv *altr_priv = dev_get_priv(dev);

	debug("%s: %s(dev=%p):\n", __func__, dev->name, dev);
	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE) {
		pr_err("%s dev_read_addr() failed\n", dev->name);
		return -ENODEV;
	}

	altr_priv->regs = (void __iomem *)addr;
	return 0;
}

static const struct altr_sysmgr_ops sysmgr_ops = {
	.read = altr_sysmgr_read_generic,
	.write = altr_sysmgr_write_generic,
};

static const struct udevice_id altr_sysmgr_ids[] = {
	{ .compatible = "altr,sys-mgr-s10" },
	{ .compatible = "altr,sys-mgr" },
	{ },
};

U_BOOT_DRIVER(altr_sysmgr) = {
	.name	= "altr_sysmgr",
	.id	= UCLASS_NOP,
	.of_match = altr_sysmgr_ids,
	.probe	= altr_sysmgr_probe,
	.ops	= &sysmgr_ops,
	.priv_auto = sizeof(struct altr_sysmgr_priv),
	.flags = DM_FLAG_PRE_RELOC,
};
