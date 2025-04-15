// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2016 Google, Inc
 */

#include <dm.h>
#include <errno.h>
#include <log.h>
#include <sysreset.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/wdt.h>
#include <linux/err.h>
#include <hang.h>

static int ast_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct udevice *wdt;
	u32 reset_mode;
	int ret = uclass_first_device_err(UCLASS_WDT, &wdt);

	if (ret)
		return ret;

	switch (type) {
	case SYSRESET_WARM:
		reset_mode = WDT_CTRL_RESET_CPU;
		break;
	case SYSRESET_COLD:
		reset_mode = WDT_CTRL_RESET_CHIP;
		break;
	default:
		return -EPROTONOSUPPORT;
	}

#if !defined(CONFIG_XPL_BUILD)
	ret = wdt_expire_now(wdt, reset_mode);
	if (ret) {
		debug("Sysreset failed: %d", ret);
		return ret;
	}
#else
	hang();
#endif

	return -EINPROGRESS;
}

static struct sysreset_ops ast_sysreset = {
	.request	= ast_sysreset_request,
};

U_BOOT_DRIVER(sysreset_ast) = {
	.name	= "ast_sysreset",
	.id	= UCLASS_SYSRESET,
	.ops	= &ast_sysreset,
};
