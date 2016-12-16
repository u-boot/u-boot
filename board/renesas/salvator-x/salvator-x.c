/*
 * board/renesas/salvator-x/salvator-x.c
 *     This file is Salvator-X board support.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <i2c.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#define CPGWPCR	0xE6150904
#define CPGWPR  0xE615090C

#define CLK2MHZ(clk)	(clk / 1000 / 1000)
void s_init(void)
{
	struct rcar_rwdt *rwdt = (struct rcar_rwdt *)RWDT_BASE;
	struct rcar_swdt *swdt = (struct rcar_swdt *)SWDT_BASE;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	writel(0xA5A50000, CPGWPCR);
	writel(0xFFFFFFFF, CPGWPR);
}

#define GSX_MSTP112	(1 << 12)	/* 3DG */
#define TMU0_MSTP125	(1 << 25)	/* secure */
#define TMU1_MSTP124	(1 << 24)	/* non-secure */
#define SCIF2_MSTP310	(1 << 10)	/* SCIF2 */

int board_early_init_f(void)
{
	/* TMU0,1 */		/* which use ? */
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125 | TMU1_MSTP124);
	/* SCIF2 */
	mstp_clrbits_le32(MSTPSR3, SMSTPCR3, SCIF2_MSTP310);

	return 0;
}

/* SYSC */
/* R/- 32 Power status register 2(3DG) */
#define	SYSC_PWRSR2	0xE6180100
/* -/W 32 Power resume control register 2 (3DG) */
#define	SYSC_PWRONCR2	0xE618010C

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	/* Init PFC controller */
	r8a7795_pinmux_init();

	/* GSX: force power and clock supply */
	writel(0x0000001F, SYSC_PWRONCR2);
	while (readl(SYSC_PWRSR2) != 0x000003E0)
		mdelay(20);

	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, GSX_MSTP112);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_RCAR_BOARD_STRING
};

#define RST_BASE	0xE6160000
#define RST_CA57RESCNT	(RST_BASE + 0x40)
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_RSTOUTCR	(RST_BASE + 0x58)
#define RST_CODE	0xA5A5000F

void reset_cpu(ulong addr)
{
	/* only CA57 ? */
	writel(RST_CODE, RST_CA57RESCNT);
}

static const struct sh_serial_platdata serial_platdata = {
	.base = SCIF2_BASE,
	.type = PORT_SCIF,
	.clk = 14745600,		/* 0xE10000 */
	.clk_mode = EXT_CLK,
};

U_BOOT_DEVICE(salvator_x_scif2) = {
	.name = "serial_sh",
	.platdata = &serial_platdata,
};
