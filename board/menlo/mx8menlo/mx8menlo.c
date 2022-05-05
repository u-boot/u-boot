// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021-2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <spl.h>

#define SNVS_BASE_ADDR		0x30370000
#define SNVS_LPSR		0x4c
#define SNVS_LPLVDR		0x64
#define SNVS_LPPGDR_INIT	0x41736166

static void setup_snvs(void)
{
	/* Enable SNVS clock */
	clock_enable(CCGR_SNVS, 1);
	/* Initialize glitch detect */
	writel(SNVS_LPPGDR_INIT, SNVS_BASE_ADDR + SNVS_LPLVDR);
	/* Clear interrupt status */
	writel(0xffffffff, SNVS_BASE_ADDR + SNVS_LPSR);
}

void board_early_init(void)
{
	init_uart_clk(1);

	setup_snvs();
}
