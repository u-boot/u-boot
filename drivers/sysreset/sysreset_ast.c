/*
 * (C) Copyright 2016 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/io.h>
#include <asm/arch/wdt.h>
#include <linux/err.h>

/* Number of Watchdog Timer ticks before reset */
#define AST_WDT_RESET_TIMEOUT	10
#define AST_WDT_FOR_RESET	0

static int ast_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct ast_wdt *wdt = ast_get_wdt(AST_WDT_FOR_RESET);
	u32 reset_mode = 0;

	if (IS_ERR(wdt))
		return PTR_ERR(wdt);

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

	/* Clear reset mode bits */
	clrsetbits_le32(&wdt->ctrl,
			(WDT_CTRL_RESET_MODE_MASK << WDT_CTRL_RESET_MODE_SHIFT),
			(reset_mode << WDT_CTRL_RESET_MODE_SHIFT));
	wdt_start(wdt, AST_WDT_RESET_TIMEOUT);

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
