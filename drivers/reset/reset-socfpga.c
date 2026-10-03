// SPDX-License-Identifier: GPL-2.0+
/*
 * Socfpga Reset Controller Driver
 *
 * Copyright 2014 Steffen Trumtrar <s.trumtrar@pengutronix.de>
 *
 * based on
 * Allwinner SoCs Reset Controller driver
 *
 * Copyright 2013 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 */

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <env.h>
#include <reset-uclass.h>
#include <wait_bit.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/kconfig.h>

#define BANK_INCREMENT		4
#define NR_BANKS		8

struct socfpga_reset_data {
	void __iomem *modrst_base;
};

/**
 * socfpga_reset_keep_enabled() - decide whether peripheral resets stay deasserted
 *
 * Returns true if the driver should deassert all peripheral resets before
 * handing off to the OS (legacy behaviour required by kernels without full
 * peripheral reset driver support, typically gen5 / Arria10).
 *
 * Decision priority:
 *  1. If CONFIG_SOCFPGA_RESET_LEGACY_COMPAT is disabled at build time, always
 *     return false (no bulk deassertion), regardless of the environment.
 *  2. Otherwise, if the environment variable "socfpga_legacy_reset_compat" is
 *     present and set to '1', return true; if set to any other value, return
 *     false.
 *  3. If the variable is absent, fall back to returning true (safe default for
 *     platforms that have not yet opted out).
 */
static bool socfpga_reset_keep_enabled(void)
{
	if (!IS_ENABLED(CONFIG_SOCFPGA_RESET_LEGACY_COMPAT))
		return false;

#if !defined(CONFIG_XPL_BUILD) || CONFIG_IS_ENABLED(ENV_SUPPORT)
	const char *env_str = env_get("socfpga_legacy_reset_compat");
	long val;

	if (env_str) {
		val = simple_strtol(env_str, NULL, 0);
		return val == 1;
	}
#endif

	/*
	 * ENV_SUPPORT is absent in some SPL builds; dm_remove_devices_active()
	 * is never called from SPL so this path is dead there. Default to true
	 * to preserve legacy behaviour for U-Boot proper.
	 */
	return true;
}

static int socfpga_reset_assert(struct reset_ctl *reset_ctl)
{
	struct socfpga_reset_data *data = dev_get_priv(reset_ctl->dev);
	int id = reset_ctl->id;
	int reg_width = sizeof(u32);
	int bank = id / (reg_width * BITS_PER_BYTE);
	int offset = id % (reg_width * BITS_PER_BYTE);

	setbits_le32(data->modrst_base + (bank * BANK_INCREMENT), BIT(offset));
	return 0;
}

static int socfpga_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct socfpga_reset_data *data = dev_get_priv(reset_ctl->dev);
	int id = reset_ctl->id;
	int reg_width = sizeof(u32);
	int bank = id / (reg_width * BITS_PER_BYTE);
	int offset = id % (reg_width * BITS_PER_BYTE);

	clrbits_le32(data->modrst_base + (bank * BANK_INCREMENT), BIT(offset));

	return wait_for_bit_le32(data->modrst_base + (bank * BANK_INCREMENT),
				 BIT(offset),
				 false, 500, false);
}

static const struct reset_ops socfpga_reset_ops = {
	.rst_assert = socfpga_reset_assert,
	.rst_deassert = socfpga_reset_deassert,
};

static int socfpga_reset_probe(struct udevice *dev)
{
	struct socfpga_reset_data *data = dev_get_priv(dev);
	u32 modrst_offset;
	void __iomem *membase;

	membase = dev_read_addr_ptr(dev);

	modrst_offset = dev_read_u32_default(dev, "altr,modrst-offset", 0x10);
	data->modrst_base = membase + modrst_offset;

	return 0;
}

static int socfpga_reset_remove(struct udevice *dev)
{
	struct socfpga_reset_data *data = dev_get_priv(dev);

	if (socfpga_reset_keep_enabled()) {
		puts("Deasserting all peripheral resets\n");
		writel(0, data->modrst_base + 4);
		if (IS_ENABLED(CONFIG_ARCH_SOCFPGA_ARRIA10))
			writel(0, data->modrst_base + 8);
	}

	return 0;
}

static int socfpga_reset_bind(struct udevice *dev)
{
	int ret;
	struct udevice *sys_child;

	/*
	 * The sysreset driver does not have a device node, so bind it here.
	 * Bind it to the node, too, so that it can get its base address.
	 */
	ret = device_bind_driver_to_node(dev, "socfpga_sysreset", "sysreset",
					 dev_ofnode(dev), &sys_child);
	if (ret)
		debug("Warning: No sysreset driver: ret=%d\n", ret);

	return 0;
}

static const struct udevice_id socfpga_reset_match[] = {
	{ .compatible = "altr,rst-mgr" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(socfpga_reset) = {
	.name = "socfpga-reset",
	.id = UCLASS_RESET,
	.of_match = socfpga_reset_match,
	.bind = socfpga_reset_bind,
	.probe = socfpga_reset_probe,
	.priv_auto	= sizeof(struct socfpga_reset_data),
	.ops = &socfpga_reset_ops,
	.remove = socfpga_reset_remove,
	.flags	= DM_FLAG_OS_PREPARE,
};
