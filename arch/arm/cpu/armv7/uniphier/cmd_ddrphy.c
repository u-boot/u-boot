/*
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ddrphy-regs.h>

/* Select either decimal or hexadecimal */
#if 1
#define PRINTF_FORMAT "%2d"
#else
#define PRINTF_FORMAT "%02x"
#endif
/* field separator */
#define FS "   "

static u32 read_bdl(struct ddrphy_datx8 __iomem *dx, int index)
{
	return (readl(&dx->bdlr[index / 5]) >> (index % 5 * 6)) & 0x3f;
}

static void dump_loop(void (*callback)(struct ddrphy_datx8 __iomem *))
{
	int ch, p, dx;
	struct ddrphy __iomem *phy;

	for (ch = 0; ch < NR_DDRCH; ch++) {
		for (p = 0; p < NR_DDRPHY_PER_CH; p++) {
			phy = (struct ddrphy __iomem *)DDRPHY_BASE(ch, p);

			for (dx = 0; dx < NR_DATX8_PER_DDRPHY; dx++) {
				printf("CH%dP%dDX%d:", ch, p, dx);
				(*callback)(&phy->dx[dx]);
				printf("\n");
			}
		}
	}
}

static void __wbdl_dump(struct ddrphy_datx8 __iomem *dx)
{
	int i;

	for (i = 0; i < 10; i++)
		printf(FS PRINTF_FORMAT, read_bdl(dx, i));

	printf(FS "(+" PRINTF_FORMAT ")", readl(&dx->lcdlr[1]) & 0xff);
}

void wbdl_dump(void)
{
	printf("\n--- Write Bit Delay Line ---\n");
	printf("           DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  DQS  (WDQD)\n");

	dump_loop(&__wbdl_dump);
}

static void __rbdl_dump(struct ddrphy_datx8 __iomem *dx)
{
	int i;

	for (i = 15; i < 24; i++)
		printf(FS PRINTF_FORMAT, read_bdl(dx, i));

	printf(FS "(+" PRINTF_FORMAT ")", (readl(&dx->lcdlr[1]) >> 8) & 0xff);
}

void rbdl_dump(void)
{
	printf("\n--- Read Bit Delay Line ---\n");
	printf("           DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  (RDQSD)\n");

	dump_loop(&__rbdl_dump);
}

static void __wld_dump(struct ddrphy_datx8 __iomem *dx)
{
	int rank;
	u32 lcdlr0 = readl(&dx->lcdlr[0]);
	u32 gtr = readl(&dx->gtr);

	for (rank = 0; rank < 4; rank++) {
		u32 wld = (lcdlr0 >> (8 * rank)) & 0xff; /* Delay */
		u32 wlsl = (gtr >> (12 + 2 * rank)) & 0x3; /* System Latency */

		printf(FS PRINTF_FORMAT "%sT", wld,
		       wlsl == 0 ? "-1" : wlsl == 1 ? "+0" : "+1");
	}
}

void wld_dump(void)
{
	printf("\n--- Write Leveling Delay ---\n");
	printf("            Rank0   Rank1   Rank2   Rank3\n");

	dump_loop(&__wld_dump);
}

static void __dqsgd_dump(struct ddrphy_datx8 __iomem *dx)
{
	int rank;
	u32 lcdlr2 = readl(&dx->lcdlr[2]);
	u32 gtr = readl(&dx->gtr);

	for (rank = 0; rank < 4; rank++) {
		u32 dqsgd = (lcdlr2 >> (8 * rank)) & 0xff; /* Delay */
		u32 dgsl = (gtr >> (3 * rank)) & 0x7; /* System Latency */

		printf(FS PRINTF_FORMAT "+%dT", dqsgd, dgsl);
	}
}

void dqsgd_dump(void)
{
	printf("\n--- DQS Gating Delay ---\n");
	printf("            Rank0   Rank1   Rank2   Rank3\n");

	dump_loop(&__dqsgd_dump);
}

static void __mdl_dump(struct ddrphy_datx8 __iomem *dx)
{
	int i;
	u32 mdl = readl(&dx->mdlr);
	for (i = 0; i < 3; i++)
		printf(FS PRINTF_FORMAT, (mdl >> (8 * i)) & 0xff);
}

void mdl_dump(void)
{
	printf("\n--- Master Delay Line ---\n");
	printf("          IPRD TPRD MDLD\n");

	dump_loop(&__mdl_dump);
}

#define REG_DUMP(x) \
	{ u32 __iomem *p = &phy->x; printf("%3d: %-10s: %p : %08x\n", \
					p - (u32 *)phy, #x, p, readl(p)); }

void reg_dump(void)
{
	int ch, p;
	struct ddrphy __iomem *phy;

	printf("\n--- DDR PHY registers ---\n");

	for (ch = 0; ch < NR_DDRCH; ch++) {
		for (p = 0; p < NR_DDRPHY_PER_CH; p++) {
			printf("== Ch%d, PHY%d ==\n", ch, p);
			printf(" No: Name      : Address  : Data\n");

			phy = (struct ddrphy __iomem *)DDRPHY_BASE(ch, p);

			REG_DUMP(ridr);
			REG_DUMP(pir);
			REG_DUMP(pgcr[0]);
			REG_DUMP(pgcr[1]);
			REG_DUMP(pgsr[0]);
			REG_DUMP(pgsr[1]);
			REG_DUMP(pllcr);
			REG_DUMP(ptr[0]);
			REG_DUMP(ptr[1]);
			REG_DUMP(ptr[2]);
			REG_DUMP(ptr[3]);
			REG_DUMP(ptr[4]);
			REG_DUMP(acmdlr);
			REG_DUMP(acbdlr);
			REG_DUMP(dxccr);
			REG_DUMP(dsgcr);
			REG_DUMP(dcr);
			REG_DUMP(dtpr[0]);
			REG_DUMP(dtpr[1]);
			REG_DUMP(dtpr[2]);
			REG_DUMP(mr0);
			REG_DUMP(mr1);
			REG_DUMP(mr2);
			REG_DUMP(mr3);
			REG_DUMP(dx[0].gcr);
			REG_DUMP(dx[0].gtr);
			REG_DUMP(dx[1].gcr);
			REG_DUMP(dx[1].gtr);
		}
	}
}

static int do_ddr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *cmd = argv[1];

	if (argc == 1)
		cmd = "all";

	if (!strcmp(cmd, "wbdl") || !strcmp(cmd, "all"))
		wbdl_dump();

	if (!strcmp(cmd, "rbdl") || !strcmp(cmd, "all"))
		rbdl_dump();

	if (!strcmp(cmd, "wld") || !strcmp(cmd, "all"))
		wld_dump();

	if (!strcmp(cmd, "dqsgd") || !strcmp(cmd, "all"))
		dqsgd_dump();

	if (!strcmp(cmd, "mdl") || !strcmp(cmd, "all"))
		mdl_dump();

	if (!strcmp(cmd, "reg") || !strcmp(cmd, "all"))
		reg_dump();

	return 0;
}

U_BOOT_CMD(
	ddr,	2,	1,	do_ddr,
	"UniPhier DDR PHY parameters dumper",
	"- dump all of the followings\n"
	"ddr wbdl - dump Write Bit Delay\n"
	"ddr rbdl - dump Read Bit Delay\n"
	"ddr wld - dump Write Leveling\n"
	"ddr dqsgd - dump DQS Gating Delay\n"
	"ddr mdl - dump Master Delay Line\n"
	"ddr reg - dump registers\n"
);
