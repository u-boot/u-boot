/*
 * board/renesas/gose/gose.c
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/rmobile.h>
#include <i2c.h>
#include "qos.h"

DECLARE_GLOBAL_DATA_PTR;

#define CLK2MHZ(clk)	(clk / 1000 / 1000)
void s_init(void)
{
	struct rcar_rwdt *rwdt = (struct rcar_rwdt *)RWDT_BASE;
	struct rcar_swdt *swdt = (struct rcar_swdt *)SWDT_BASE;
	u32 stc;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* CPU frequency setting. Set to 1.5GHz */
	stc = ((1500 / CLK2MHZ(CONFIG_SYS_CLK_FREQ)) - 1) << PLL0_STC_BIT;
	clrsetbits_le32(PLL0CR, PLL0_STC_MASK, stc);

	/* QoS */
	qos_init();
}

#define MSTPSR1		0xE6150038
#define SMSTPCR1	0xE6150134
#define TMU0_MSTP125	(1 << 25)

#define MSTPSR7		0xE61501C4
#define SMSTPCR7	0xE615014C
#define SCIF0_MSTP721	(1 << 21)

#define mstp_setbits(type, addr, saddr, set) \
	out_##type((saddr), in_##type(addr) | (set))
#define mstp_clrbits(type, addr, saddr, clear) \
	out_##type((saddr), in_##type(addr) & ~(clear))
#define mstp_setbits_le32(addr, saddr, set) \
	mstp_setbits(le32, addr, saddr, set)
#define mstp_clrbits_le32(addr, saddr, clear) \
	mstp_clrbits(le32, addr, saddr, clear)

int board_early_init_f(void)
{
	/* TMU0 */
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125);

	/* SCIF0 */
	mstp_clrbits_le32(MSTPSR7, SMSTPCR7, SCIF0_MSTP721);

	return 0;
}

#define TSTR0		0x04
#define TSTR0_STR0	0x01
void arch_preboot_os(void)
{
	/* stop TMU0 */
	mstp_clrbits_le32(TMU_BASE + TSTR0, TMU_BASE + TSTR0, TSTR0_STR0);
	/* Disable TMU0 */
	mstp_setbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125);
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = GOSE_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7793_pinmux_init();

	return 0;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_RMOBILE_BOARD_STRING
};

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = GOSE_SDRAM_BASE;
	gd->bd->bi_dram[0].size = GOSE_SDRAM_SIZE;
}

void reset_cpu(ulong addr)
{
	u8 val;

	i2c_set_bus_num(2); /* PowerIC connected to ch2 */
	i2c_read(CONFIG_SYS_I2C_POWERIC_ADDR, 0x13, 1, &val, 1);
	val |= 0x02;
	i2c_write(CONFIG_SYS_I2C_POWERIC_ADDR, 0x13, 1, &val, 1);
}
