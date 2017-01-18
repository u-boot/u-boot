/*
 * (C) Copyright 2016 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/wdt.h>
#include <linux/err.h>

void wdt_stop(struct ast_wdt *wdt)
{
	clrbits_le32(&wdt->ctrl, WDT_CTRL_EN);
}

void wdt_start(struct ast_wdt *wdt, u32 timeout)
{
	writel(timeout, &wdt->counter_reload_val);
	writel(WDT_COUNTER_RESTART_VAL, &wdt->counter_restart);
	/*
	 * Setting CLK1MHZ bit is just for compatibility with ast2400 part.
	 * On ast2500 watchdog timer clock is fixed at 1MHz and the bit is
	 * read-only
	 */
	setbits_le32(&wdt->ctrl,
		     WDT_CTRL_EN | WDT_CTRL_RESET | WDT_CTRL_CLK1MHZ);
}

struct ast_wdt *ast_get_wdt(u8 wdt_number)
{
	if (wdt_number > CONFIG_WDT_NUM - 1)
		return ERR_PTR(-EINVAL);

	return (struct ast_wdt *)(WDT_BASE +
				  sizeof(struct ast_wdt) * wdt_number);
}

int ast_wdt_reset_masked(struct ast_wdt *wdt, u32 mask)
{
#ifdef CONFIG_ASPEED_AST2500
	if (!mask)
		return -EINVAL;

	writel(mask, &wdt->reset_mask);
	clrbits_le32(&wdt->ctrl,
		     WDT_CTRL_RESET_MASK << WDT_CTRL_RESET_MODE_SHIFT);
	wdt_start(wdt, 1);

	/* Wait for WDT to reset */
	while (readl(&wdt->ctrl) & WDT_CTRL_EN)
		;
	wdt_stop(wdt);

	return 0;
#else
	return -EINVAL;
#endif
}
