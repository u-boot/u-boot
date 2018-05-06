// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/ulcb/ulcb.c
 *     This file is ULCB board support.
 *
 * Copyright (C) 2017 Renesas Electronics Corporation
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
#include <asm/arch/sh_sdhi.h>
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

#define GSX_MSTP112		BIT(12)	/* 3DG */
#define TMU0_MSTP125		BIT(25)	/* secure */
#define TMU1_MSTP124		BIT(24)	/* non-secure */
#define SCIF2_MSTP310		BIT(10)	/* SCIF2 */
#define DVFS_MSTP926		BIT(26)
#define HSUSB_MSTP704		BIT(4)	/* HSUSB */

int board_early_init_f(void)
{
	/* TMU0,1 */		/* which use ? */
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125 | TMU1_MSTP124);

#if defined(CONFIG_SYS_I2C) && defined(CONFIG_SYS_I2C_SH)
	/* DVFS for reset */
	mstp_clrbits_le32(MSTPSR9, SMSTPCR9, DVFS_MSTP926);
#endif
	return 0;
}

/* SYSC */
/* R/- 32 Power status register 2(3DG) */
#define	SYSC_PWRSR2	0xE6180100
/* -/W 32 Power resume control register 2 (3DG) */
#define	SYSC_PWRONCR2	0xE618010C

/* HSUSB block registers */
#define HSUSB_REG_LPSTS			0xE6590102
#define HSUSB_REG_LPSTS_SUSPM_NORMAL	BIT(14)
#define HSUSB_REG_UGCTRL2		0xE6590184
#define HSUSB_REG_UGCTRL2_USB0SEL	0x30
#define HSUSB_REG_UGCTRL2_USB0SEL_EHCI	0x10

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	/* USB1 pull-up */
	setbits_le32(PFC_PUEN6, PUEN_USB1_OVC | PUEN_USB1_PWEN);

	/* Configure the HSUSB block */
	mstp_clrbits_le32(MSTPSR7, SMSTPCR7, HSUSB_MSTP704);
	/* Choice USB0SEL */
	clrsetbits_le32(HSUSB_REG_UGCTRL2, HSUSB_REG_UGCTRL2_USB0SEL,
			HSUSB_REG_UGCTRL2_USB0SEL_EHCI);
	/* low power status */
	setbits_le16(HSUSB_REG_LPSTS, HSUSB_REG_LPSTS_SUSPM_NORMAL);

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_memory_size() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}
