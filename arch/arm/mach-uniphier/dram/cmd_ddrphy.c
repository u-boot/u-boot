/*
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mapmem.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "../soc-info.h"
#include "ddrphy-regs.h"

/* Select either decimal or hexadecimal */
#if 1
#define PRINTF_FORMAT "%2d"
#else
#define PRINTF_FORMAT "%02x"
#endif
/* field separator */
#define FS "   "

static unsigned long uniphier_ld4_base[] = {
	0x5bc01000,
	0x5be01000,
	0 /* sentinel */
};

static unsigned long uniphier_pro4_base[] = {
	0x5bc01000,
	0x5be01000,
	0 /* sentinel */
};

static unsigned long uniphier_sld8_base[] = {
	0x5bc01000,
	0x5be01000,
	0 /* sentinel */
};

static u32 read_bdl(struct ddrphy_datx8 __iomem *dx, int index)
{
	return (readl(&dx->bdlr[index / 5]) >> (index % 5 * 6)) & 0x3f;
}

static void dump_loop(unsigned long *base,
		      void (*callback)(struct ddrphy_datx8 __iomem *))
{
	struct ddrphy __iomem *phy;
	int p, dx;

	for (p = 0; *base; base++, p++) {
		phy = map_sysmem(*base, SZ_4K);

		for (dx = 0; dx < NR_DATX8_PER_DDRPHY; dx++) {
			printf("PHY%dDX%d:", p, dx);
			(*callback)(&phy->dx[dx]);
			printf("\n");
		}

		unmap_sysmem(phy);
	}
}

static void __wbdl_dump(struct ddrphy_datx8 __iomem *dx)
{
	int i;

	for (i = 0; i < 10; i++)
		printf(FS PRINTF_FORMAT, read_bdl(dx, i));

	printf(FS "(+" PRINTF_FORMAT ")", readl(&dx->lcdlr[1]) & 0xff);
}

static void wbdl_dump(unsigned long *base)
{
	printf("\n--- Write Bit Delay Line ---\n");
	printf("           DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  DQS  (WDQD)\n");

	dump_loop(base, &__wbdl_dump);
}

static void __rbdl_dump(struct ddrphy_datx8 __iomem *dx)
{
	int i;

	for (i = 15; i < 24; i++)
		printf(FS PRINTF_FORMAT, read_bdl(dx, i));

	printf(FS "(+" PRINTF_FORMAT ")", (readl(&dx->lcdlr[1]) >> 8) & 0xff);
}

static void rbdl_dump(unsigned long *base)
{
	printf("\n--- Read Bit Delay Line ---\n");
	printf("           DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  (RDQSD)\n");

	dump_loop(base, &__rbdl_dump);
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

static void wld_dump(unsigned long *base)
{
	printf("\n--- Write Leveling Delay ---\n");
	printf("            Rank0   Rank1   Rank2   Rank3\n");

	dump_loop(base, &__wld_dump);
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

static void dqsgd_dump(unsigned long *base)
{
	printf("\n--- DQS Gating Delay ---\n");
	printf("            Rank0   Rank1   Rank2   Rank3\n");

	dump_loop(base, &__dqsgd_dump);
}

static void __mdl_dump(struct ddrphy_datx8 __iomem *dx)
{
	int i;
	u32 mdl = readl(&dx->mdlr);
	for (i = 0; i < 3; i++)
		printf(FS PRINTF_FORMAT, (mdl >> (8 * i)) & 0xff);
}

static void mdl_dump(unsigned long *base)
{
	printf("\n--- Master Delay Line ---\n");
	printf("          IPRD TPRD MDLD\n");

	dump_loop(base, &__mdl_dump);
}

#define REG_DUMP(x) \
	{ u32 __iomem *p = &phy->x; printf("%3d: %-10s: %p : %08x\n", \
					p - (u32 *)phy, #x, p, readl(p)); }

static void reg_dump(unsigned long *base)
{
	struct ddrphy __iomem *phy;
	int p;

	printf("\n--- DDR PHY registers ---\n");

	for (p = 0; *base; base++, p++) {
		phy = map_sysmem(*base, SZ_4K);

		printf("== PHY%d (base: %p) ==\n", p, phy);
		printf(" No: Name      : Address  : Data\n");

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

		unmap_sysmem(phy);
	}
}

static int do_ddr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *cmd = argv[1];
	unsigned long *base;

	switch (uniphier_get_soc_type()) {
	case SOC_UNIPHIER_LD4:
		base = uniphier_ld4_base;
		break;
	case SOC_UNIPHIER_PRO4:
		base = uniphier_pro4_base;
		break;
	case SOC_UNIPHIER_SLD8:
		base = uniphier_sld8_base;
		break;
	default:
		printf("unsupported SoC\n");
		return CMD_RET_FAILURE;
	}

	if (argc == 1)
		cmd = "all";

	if (!strcmp(cmd, "wbdl") || !strcmp(cmd, "all"))
		wbdl_dump(base);

	if (!strcmp(cmd, "rbdl") || !strcmp(cmd, "all"))
		rbdl_dump(base);

	if (!strcmp(cmd, "wld") || !strcmp(cmd, "all"))
		wld_dump(base);

	if (!strcmp(cmd, "dqsgd") || !strcmp(cmd, "all"))
		dqsgd_dump(base);

	if (!strcmp(cmd, "mdl") || !strcmp(cmd, "all"))
		mdl_dump(base);

	if (!strcmp(cmd, "reg") || !strcmp(cmd, "all"))
		reg_dump(base);

	return CMD_RET_SUCCESS;
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
