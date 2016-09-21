/*
 * board/renesas/blanche/blanche.c
 *     This file is blanche board support.
 *
 * Copyright (C) 2016 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0
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
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/mmc.h>
#include <asm/arch/sh_sdhi.h>
#include <miiphy.h>
#include <i2c.h>
#include <mmc.h>
#include "qos.h"

DECLARE_GLOBAL_DATA_PTR;

struct pin_db {
	u32	addr;	/* register address */
	u32	mask;	/* mask value */
	u32	val;	/* setting value */
};

#define	PMMR		0xE6060000
#define	GPSR0		0xE6060004
#define	GPSR1		0xE6060008
#define	GPSR4		0xE6060014
#define	GPSR5		0xE6060018
#define	GPSR6		0xE606001C
#define	GPSR7		0xE6060020
#define	GPSR8		0xE6060024
#define	GPSR9		0xE6060028
#define	GPSR10		0xE606002C
#define	GPSR11		0xE6060030
#define	IPSR6		0xE6060058
#define	PUPR2		0xE6060108
#define	PUPR3		0xE606010C
#define	PUPR4		0xE6060110
#define	PUPR5		0xE6060114
#define	PUPR7		0xE606011C
#define	PUPR9		0xE6060124
#define	PUPR10		0xE6060128
#define	PUPR11		0xE606012C

#define	CPG_PLL1CR	0xE6150028
#define	CPG_PLL3CR	0xE61500DC

#define	SetREG(x) \
	writel((readl((x)->addr) & ~((x)->mask)) | ((x)->val), (x)->addr)

#define	SetGuardREG(x)				\
{ \
	u32	val; \
	val = (readl((x)->addr) & ~((x)->mask)) | ((x)->val); \
	writel(~val, PMMR); \
	writel(val, (x)->addr); \
}

struct pin_db	pin_guard[] = {
	{ GPSR0,	0xFFFFFFFF,	0x0BFFFFFF },
	{ GPSR1,	0xFFFFFFFF,	0x002FFFFF },
	{ GPSR4,	0xFFFFFFFF,	0x00000FFF },
	{ GPSR5,	0xFFFFFFFF,	0x00010FFF },
	{ GPSR6,	0xFFFFFFFF,	0x00010FFF },
	{ GPSR7,	0xFFFFFFFF,	0x00010FFF },
	{ GPSR8,	0xFFFFFFFF,	0x00010FFF },
	{ GPSR9,	0xFFFFFFFF,	0x00010FFF },
	{ GPSR10,	0xFFFFFFFF,	0x04006000 },
	{ GPSR11,	0xFFFFFFFF,	0x303FEFE0 },
	{ IPSR6,	0xFFFFFFFF,	0x0002000E },
};

struct pin_db	pin_tbl[] = {
	{ PUPR2,	0xFFFFFFFF,	0x00000000 },
	{ PUPR3,	0xFFFFFFFF,	0x0803FF40 },
	{ PUPR4,	0xFFFFFFFF,	0x0000FFFF },
	{ PUPR5,	0xFFFFFFFF,	0x00010FFF },
	{ PUPR7,	0xFFFFFFFF,	0x0001AFFF },
	{ PUPR9,	0xFFFFFFFF,	0x0001CFFF },
	{ PUPR10,	0xFFFFFFFF,	0xC0438001 },
	{ PUPR11,	0xFFFFFFFF,	0x0FC00007 },
};

void pin_init(void)
{
	struct pin_db	*db;

	for (db = pin_guard; db < &pin_guard[sizeof(pin_guard)/sizeof(struct pin_db)]; db++) {
		SetGuardREG(db);
	}
	for (db = pin_tbl; db < &pin_tbl[sizeof(pin_tbl) /sizeof(struct pin_db)]; db++) {
		SetREG(db);
	}
}

#define s_init_wait(cnt) \
		({	\
			volatile u32 i = 0x10000 * cnt;	\
			while (i > 0)	\
				i--;	\
		})

void s_init(void)
{
	struct rcar_rwdt *rwdt = (struct rcar_rwdt *)RWDT_BASE;
	struct rcar_swdt *swdt = (struct rcar_swdt *)SWDT_BASE;
	u32 cpu_type;

	cpu_type = rmobile_get_cpu_type();
	if (cpu_type == 0x4A) {
		writel(0x4D000000, CPG_PLL1CR);
		writel(0x4F000000, CPG_PLL3CR);
	}

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* QoS(Quality-of-Service) Init */
	qos_init();

	/* SCIF Init */
	pin_init();

#if !defined(CONFIG_SYS_NO_FLASH)
	struct rcar_lbsc *lbsc = (struct rcar_lbsc *)LBSC_BASE;
	struct rcar_dbsc3 *dbsc3_0 = (struct rcar_dbsc3 *)DBSC3_0_BASE;

	/* LBSC */
	writel(0x00000020, &lbsc->cs0ctrl);
	writel(0x00000020, &lbsc->cs1ctrl);
	writel(0x00002020, &lbsc->ecs0ctrl);
	writel(0x00002020, &lbsc->ecs1ctrl);

	writel(0x2A103320, &lbsc->cswcr0);
	writel(0x2A103320, &lbsc->cswcr1);
	writel(0x19102110, &lbsc->ecswcr0);
	writel(0x19102110, &lbsc->ecswcr1);

	/* DBSC3 */
	s_init_wait(10);

	writel(0x0000A55A, &dbsc3_0->dbpdlck);

	writel(0x21000000, &dbsc3_0->dbcmd);		/* opc=RstH (RESET => H) */
	writel(0x11000000, &dbsc3_0->dbcmd);		/* opc=PDXt(CKE=H) */
	writel(0x10000000, &dbsc3_0->dbcmd);		/* opc=PDEn(CKE=L) */

	/* Stop Auto-Calibration */
	writel(0x00000001, &dbsc3_0->dbpdrga);
	writel(0x80000000, &dbsc3_0->dbpdrgd);

	writel(0x00000004, &dbsc3_0->dbpdrga);
	while ((readl(&dbsc3_0->dbpdrgd) & 0x00000001) != 0x00000001);

	/* PLLCR: PLL Control Register */
	writel(0x00000006, &dbsc3_0->dbpdrga);
	writel(0x0001C000, &dbsc3_0->dbpdrgd);	// > DDR1440

	/* DXCCR: DATX8 Common Configuration Register */
	writel(0x0000000F, &dbsc3_0->dbpdrga);
	writel(0x00181EE4, &dbsc3_0->dbpdrgd);

	/* DSGCR	:DDR System General Configuration Register */
	writel(0x00000010, &dbsc3_0->dbpdrga);
	writel(0xF00464DB, &dbsc3_0->dbpdrgd);

	writel(0x00000061, &dbsc3_0->dbpdrga);
	writel(0x0000008D, &dbsc3_0->dbpdrgd);

	/* Re-Execute ZQ calibration */
	writel(0x00000001, &dbsc3_0->dbpdrga);
	writel(0x00000073, &dbsc3_0->dbpdrgd);

	writel(0x00000007, &dbsc3_0->dbkind);
	writel(0x0F030A02, &dbsc3_0->dbconf0);
	writel(0x00000001, &dbsc3_0->dbphytype);
	writel(0x00000000, &dbsc3_0->dbbl);

	writel(0x0000000B, &dbsc3_0->dbtr0);	// tCL=11
	writel(0x00000008, &dbsc3_0->dbtr1);	// tCWL=8
	writel(0x00000000, &dbsc3_0->dbtr2);	// tAL=0
	writel(0x0000000B, &dbsc3_0->dbtr3);	// tRCD=11
	writel(0x000C000B, &dbsc3_0->dbtr4);	// tRPA=12,tRP=11
	writel(0x00000027, &dbsc3_0->dbtr5);	// tRC = 39
	writel(0x0000001C, &dbsc3_0->dbtr6);	// tRAS = 28
	writel(0x00000006, &dbsc3_0->dbtr7);	// tRRD = 6
	writel(0x00000020, &dbsc3_0->dbtr8);	// tRFAW = 32
	writel(0x00000008, &dbsc3_0->dbtr9);	// tRDPR = 8
	writel(0x0000000C, &dbsc3_0->dbtr10);	// tWR = 12
	writel(0x00000009, &dbsc3_0->dbtr11);	// tRDWR = 9
	writel(0x00000012, &dbsc3_0->dbtr12);	// tWRRD = 18
	writel(0x000000D0, &dbsc3_0->dbtr13);	// tRFC = 208
	writel(0x00140005, &dbsc3_0->dbtr14);
	writel(0x00050004, &dbsc3_0->dbtr15);
	writel(0x70233005, &dbsc3_0->dbtr16);		/* DQL = 35, WDQL = 5 */
	writel(0x000C0000, &dbsc3_0->dbtr17);
	writel(0x00000300, &dbsc3_0->dbtr18);
	writel(0x00000040, &dbsc3_0->dbtr19);
	writel(0x00000001, &dbsc3_0->dbrnk0);
	writel(0x00020001, &dbsc3_0->dbadj0);
	writel(0x20082004, &dbsc3_0->dbadj2);		/* blanche QoS rev0.1 */
	writel(0x00020002, &dbsc3_0->dbwt0cnf0);	/* 1600 */
	writel(0x0000001F, &dbsc3_0->dbwt0cnf4);

	while ((readl(&dbsc3_0->dbdfistat) & 0x00000001) != 0x00000001);
	writel(0x00000011, &dbsc3_0->dbdficnt);

	/* PGCR1	:PHY General Configuration Register 1 */
	writel(0x00000003, &dbsc3_0->dbpdrga);
	writel(0x0300C4E1, &dbsc3_0->dbpdrgd);		/* DDR3 */

	/* PGCR2: PHY General Configuration Registers 2 */
	writel(0x00000023, &dbsc3_0->dbpdrga);
	writel(0x00FCDB60, &dbsc3_0->dbpdrgd);

	writel(0x00000011, &dbsc3_0->dbpdrga);
	writel(0x1000040B, &dbsc3_0->dbpdrgd);

	/* DTPR0	:DRAM Timing Parameters Register 0 */
	writel(0x00000012, &dbsc3_0->dbpdrga);
	writel(0x9D9CBB66, &dbsc3_0->dbpdrgd);

	/* DTPR1	:DRAM Timing Parameters Register 1 */
	writel(0x00000013, &dbsc3_0->dbpdrga);
	writel(0x1A868400, &dbsc3_0->dbpdrgd);

	/* DTPR2	::DRAM Timing Parameters Register 2 */
	writel(0x00000014, &dbsc3_0->dbpdrga);
	writel(0x300214D8, &dbsc3_0->dbpdrgd);

	/* MR0	:Mode Register 0 */
	writel(0x00000015, &dbsc3_0->dbpdrga);
	writel(0x00000D70, &dbsc3_0->dbpdrgd);

	/* MR1	:Mode Register 1 */
	writel(0x00000016, &dbsc3_0->dbpdrga);
	writel(0x00000004, &dbsc3_0->dbpdrgd);	/* DRAM Drv 40ohm */

	/* MR2	:Mode Register 2 */
	writel(0x00000017, &dbsc3_0->dbpdrga);
	writel(0x00000018, &dbsc3_0->dbpdrgd);	/* CWL=8 */

	/* VREF(ZQCAL) */
	writel(0x0000001A, &dbsc3_0->dbpdrga);
	writel(0x910035C7, &dbsc3_0->dbpdrgd);

	/* PGSR0	:PHY General Status Registers 0 */
	writel(0x00000004, &dbsc3_0->dbpdrga);
	while ((readl(&dbsc3_0->dbpdrgd) & 0x00000001) != 0x00000001);

	/* DRAM Init (set MRx etc) */
	writel(0x00000001, &dbsc3_0->dbpdrga);
	writel(0x00000181, &dbsc3_0->dbpdrgd);

	/* CKE  = H */
	writel(0x11000000, &dbsc3_0->dbcmd);		/* opc=PDXt(CKE=H) */

	/* PGSR0	:PHY General Status Registers 0 */
	writel(0x00000004, &dbsc3_0->dbpdrga);
	while ((readl(&dbsc3_0->dbpdrgd) & 0x00000001) != 0x00000001);

	/* RAM ACC Training */
	writel(0x00000001, &dbsc3_0->dbpdrga);
	writel(0x0000FE01, &dbsc3_0->dbpdrgd);

	/* Bus control 0 */
	writel(0x00000000, &dbsc3_0->dbbs0cnt1);
	/* DDR3 Calibration set */
	writel(0x01004C20, &dbsc3_0->dbcalcnf);
	/* DDR3 Calibration timing */
	writel(0x014000AA, &dbsc3_0->dbcaltr);
	/* Refresh */
	writel(0x00000140, &dbsc3_0->dbrfcnf0);
	writel(0x00081860, &dbsc3_0->dbrfcnf1);
	writel(0x00010000, &dbsc3_0->dbrfcnf2);

	/* PGSR0	:PHY General Status Registers 0 */
	writel(0x00000004, &dbsc3_0->dbpdrga);
	while ((readl(&dbsc3_0->dbpdrgd) & 0x00000001) != 0x00000001);

	/* Enable Auto-Refresh */
	writel(0x00000001, &dbsc3_0->dbrfen);
	/* Permit DDR-Access */
	writel(0x00000001, &dbsc3_0->dbacen);

	/* This locks the access to the PHY unit registers */
	writel(0x00000000, &dbsc3_0->dbpdlck);
#endif /* CONFIG_SYS_NO_FLASH */

}

#define TMU0_MSTP125	(1 << 25)
#define SCIF0_MSTP721	(1 << 21)
#define SDHI0_MSTP314	(1 << 14)
#define QSPI_MSTP917	(1 << 17)

int board_early_init_f(void)
{
	/* TMU0 */
	mstp_clrbits_le32(MSTPSR1, SMSTPCR1, TMU0_MSTP125);
	/* SCIF0 */
	mstp_clrbits_le32(MSTPSR7, SMSTPCR7, SCIF0_MSTP721);
	/* SDHI0 */
	mstp_clrbits_le32(MSTPSR3, SMSTPCR3, SDHI0_MSTP314);
	/* QSPI */
	mstp_clrbits_le32(MSTPSR9, SMSTPCR9, QSPI_MSTP917);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7792_pinmux_init();

	gpio_request(GPIO_FN_D0, NULL);
	gpio_request(GPIO_FN_D1, NULL);
	gpio_request(GPIO_FN_D2, NULL);
	gpio_request(GPIO_FN_D3, NULL);
	gpio_request(GPIO_FN_D4, NULL);
	gpio_request(GPIO_FN_D5, NULL);
	gpio_request(GPIO_FN_D6, NULL);
	gpio_request(GPIO_FN_D7, NULL);
	gpio_request(GPIO_FN_D8, NULL);
	gpio_request(GPIO_FN_D9, NULL);
	gpio_request(GPIO_FN_D10, NULL);
	gpio_request(GPIO_FN_D11, NULL);
	gpio_request(GPIO_FN_D12, NULL);
	gpio_request(GPIO_FN_D13, NULL);
	gpio_request(GPIO_FN_D14, NULL);
	gpio_request(GPIO_FN_D15, NULL);
	gpio_request(GPIO_FN_A0, NULL);
	gpio_request(GPIO_FN_A1, NULL);
	gpio_request(GPIO_FN_A2, NULL);
	gpio_request(GPIO_FN_A3, NULL);
	gpio_request(GPIO_FN_A4, NULL);
	gpio_request(GPIO_FN_A5, NULL);
	gpio_request(GPIO_FN_A6, NULL);
	gpio_request(GPIO_FN_A7, NULL);
	gpio_request(GPIO_FN_A8, NULL);
	gpio_request(GPIO_FN_A9, NULL);
	gpio_request(GPIO_FN_A10, NULL);
	gpio_request(GPIO_FN_A11, NULL);
	gpio_request(GPIO_FN_A12, NULL);
	gpio_request(GPIO_FN_A13, NULL);
	gpio_request(GPIO_FN_A14, NULL);
	gpio_request(GPIO_FN_A15, NULL);
	gpio_request(GPIO_FN_A16, NULL);
	gpio_request(GPIO_FN_A17, NULL);
	gpio_request(GPIO_FN_A18, NULL);
	gpio_request(GPIO_FN_A19, NULL);
#if defined(CONFIG_SYS_NO_FLASH)
	gpio_request(GPIO_FN_MOSI_IO0, NULL);
	gpio_request(GPIO_FN_MISO_IO1, NULL);
	gpio_request(GPIO_FN_IO2, NULL);
	gpio_request(GPIO_FN_IO3, NULL);
	gpio_request(GPIO_FN_SPCLK, NULL);
	gpio_request(GPIO_FN_SSL, NULL);
#else	/* CONFIG_SYS_NO_FLASH */
	gpio_request(GPIO_FN_A20, NULL);
	gpio_request(GPIO_FN_A21, NULL);
	gpio_request(GPIO_FN_A22, NULL);
	gpio_request(GPIO_FN_A23, NULL);
	gpio_request(GPIO_FN_A24, NULL);
	gpio_request(GPIO_FN_A25, NULL);
#endif	/* CONFIG_SYS_NO_FLASH */

	gpio_request(GPIO_FN_CS1_A26, NULL);
	gpio_request(GPIO_FN_EX_CS0, NULL);
	gpio_request(GPIO_FN_EX_CS1, NULL);
	gpio_request(GPIO_FN_BS, NULL);
	gpio_request(GPIO_FN_RD, NULL);
	gpio_request(GPIO_FN_WE0, NULL);
	gpio_request(GPIO_FN_WE1, NULL);
	gpio_request(GPIO_FN_EX_WAIT0, NULL);
	gpio_request(GPIO_FN_IRQ0, NULL);
	gpio_request(GPIO_FN_IRQ2, NULL);
	gpio_request(GPIO_FN_IRQ3, NULL);
	gpio_request(GPIO_FN_CS0, NULL);

	/* Init timer */
	timer_init();

	return 0;
}

/*
 Added for BLANCHE(R-CarV2H board)
*/
int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_SMC911X
#define STR_ENV_ETHADDR	"ethaddr"

	struct eth_device *dev;
	uchar eth_addr[6];

	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);

	if (!eth_getenv_enetaddr(STR_ENV_ETHADDR, eth_addr)) {
		dev = eth_get_dev_by_index(0);
		if (dev) {
			eth_setenv_enetaddr(STR_ENV_ETHADDR, dev->enetaddr);
		} else {
			printf("blanche: Couldn't get eth device\n");
			rc = -1;
		}
	}

#endif

	return rc;
}

int board_mmc_init(bd_t *bis)
{
	int ret = -ENODEV;

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD0_DAT0, NULL);
	gpio_request(GPIO_FN_SD0_DAT1, NULL);
	gpio_request(GPIO_FN_SD0_DAT2, NULL);
	gpio_request(GPIO_FN_SD0_DAT3, NULL);
	gpio_request(GPIO_FN_SD0_CLK, NULL);
	gpio_request(GPIO_FN_SD0_CMD, NULL);
	gpio_request(GPIO_FN_SD0_CD, NULL);

	gpio_request(GPIO_GP_11_12, NULL);
	gpio_direction_output(GPIO_GP_11_12, 1);	/* power on */


	ret = sh_sdhi_init(CONFIG_SYS_SH_SDHI0_BASE, 0,
			   SH_SDHI_QUIRK_16BIT_BUF);

	if (ret)
		return ret;
#endif
	return ret;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_RMOBILE_BOARD_STRING
};

void reset_cpu(ulong addr)
{
}

static const struct sh_serial_platdata serial_platdata = {
	.base = SCIF0_BASE,
	.type = PORT_SCIF,
	.clk = 14745600,
	.clk_mode = EXT_CLK,
};

U_BOOT_DEVICE(blanche_serials) = {
	.name = "serial_sh",
	.platdata = &serial_platdata,
};
