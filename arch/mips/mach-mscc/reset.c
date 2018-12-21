// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>

#include <asm/sections.h>
#include <asm/io.h>

#include <asm/reboot.h>

void _machine_restart(void)
{
	register u32 resetbits = PERF_SOFT_RST_SOFT_CHIP_RST;
	(void)readl(BASE_DEVCPU_GCB + PERF_SOFT_RST);

	/* Make sure VCore is NOT protected from reset */
	clrbits_le32(BASE_CFG + ICPU_RESET, ICPU_RESET_CORE_RST_PROTECT);

	/* Change to SPI bitbang for SPI reset workaround... */
	writel(ICPU_SW_MODE_SW_SPI_CS_OE(1) | ICPU_SW_MODE_SW_SPI_CS(1) |
	       ICPU_SW_MODE_SW_PIN_CTRL_MODE, BASE_CFG + ICPU_SW_MODE);

	/* Do the global reset */
	writel(resetbits, BASE_DEVCPU_GCB + PERF_SOFT_RST);

	while (1)
		; /* NOP */
}
