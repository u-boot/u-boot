/*
 * board/renesas/salvator-x/salvator-x.c
 *     This file is Salvator-X/Salvator-XS board support.
 *
 * Copyright (C) 2015-2017 Renesas Electronics Corporation
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

#if defined(CONFIG_R8A7795)
	/* GSX: force power and clock supply */
	writel(0x0000001F, SYSC_PWRONCR2);
	while (readl(SYSC_PWRSR2) != 0x000003E0)
		mdelay(20);

	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, GSX_MSTP112);
#endif

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
	gd->ram_size = PHYS_SDRAM_1_SIZE;
#if (CONFIG_NR_DRAM_BANKS >= 2)
	gd->ram_size += PHYS_SDRAM_2_SIZE;
#endif
#if (CONFIG_NR_DRAM_BANKS >= 3)
	gd->ram_size += PHYS_SDRAM_3_SIZE;
#endif
#if (CONFIG_NR_DRAM_BANKS >= 4)
	gd->ram_size += PHYS_SDRAM_4_SIZE;
#endif

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
#if (CONFIG_NR_DRAM_BANKS >= 2)
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif
#if (CONFIG_NR_DRAM_BANKS >= 3)
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
#endif
#if (CONFIG_NR_DRAM_BANKS >= 4)
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = PHYS_SDRAM_4_SIZE;
#endif
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
#if defined(CONFIG_SYS_I2C) && defined(CONFIG_SYS_I2C_SH)
	i2c_reg_write(CONFIG_SYS_I2C_POWERIC_ADDR, 0x20, 0x80);
#else
	/* only CA57 ? */
	writel(RST_CODE, RST_CA57RESCNT);
#endif
}
