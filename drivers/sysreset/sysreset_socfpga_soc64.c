// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Pepperl+Fuchs
 * Simon Goldschmidt <simon.k.r.goldschmidt@gmail.com>
 */

#include <command.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <sysreset.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/reset_manager.h>
#include <asm/secure.h>

#define GICD_CTRL_ADDRESS	0xfffc1000

static __always_inline void __l2_reset_cpu(void)
{
	asm volatile(/* Disable GIC distributor (IRQs). */
		"str    wzr, [%3]\n"
		/* Set Magic Number */
		"str	%0, [%1]\n"
		/* Increase timeout in rstmgr.hdsktimeout */
		"ldr	x2, =0xFFFFFF\n"
		"str	w2, [%2, #0x64]\n"
		"ldr	w2, [%2, #0x10]\n"
		/*
		 * Set l2flushen = 1, etrstallen = 1,
		 * fpgahsen = 1 and sdrselfrefen = 1
		 * in rstmgr.hdsken to perform handshake
		 * in certain peripherals before trigger
		 * L2 reset.
		 */
		"ldr	x3, =0x10D\n"
		"orr	x2, x2, x3\n"
		"str	w2, [%2, #0x10]\n"
		/* Trigger L2 reset in rstmgr.coldmodrst */
		"ldr	w2, [%2, #0x34]\n"
		"orr	x2, x2, #0x100\n"
		"isb\n"
		"dsb	sy\n"
		"str	w2, [%2, #0x34]\n"
		/* Put all cores into WFI mode */
		"1:\n"
		"	wfi\n"
		"	b	1b\n"
		: : "r" (L2_RESET_DONE_STATUS),
		    "r" (L2_RESET_DONE_REG),
		    "r" (SOCFPGA_RSTMGR_ADDRESS),
		    "r" (GICD_CTRL_ADDRESS)
		: "x1", "x2", "x3");
}

void l2_reset_cpu(void)
{
	__l2_reset_cpu();
}

void __secure l2_reset_cpu_psci(void)
{
	__l2_reset_cpu();
}

static int socfpga_sysreset_request(struct udevice *dev,
				    enum sysreset_t type)
{
#if !IS_ENABLED(CONFIG_XPL_BUILD)
	const char *reset = env_get("reset");

	if (reset && !strcmp(reset, "warm")) {
		/* flush dcache */
		flush_dcache_all();

		/* request a warm reset */
		puts("Do warm reset now...\n");
		l2_reset_cpu();
	} else {
#endif
		puts("Mailbox: Issuing mailbox cmd REBOOT_HPS\n");
		mbox_reset_cold();
#if !IS_ENABLED(CONFIG_XPL_BUILD)
	}
#endif

	return -EINPROGRESS;
}

static struct sysreset_ops socfpga_sysreset = {
	.request = socfpga_sysreset_request,
};

U_BOOT_DRIVER(sysreset_socfpga) = {
	.id	= UCLASS_SYSRESET,
	.name	= "socfpga_sysreset",
	.ops	= &socfpga_sysreset,
};
