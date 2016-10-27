/*
 * Copyright (C) 2014      Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "../soc-info.h"
#include "ddrphy-init.h"
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

static void print_bdl(void __iomem *reg, int n)
{
	u32 val = readl(reg);
	int i;

	for (i = 0; i < n; i++)
		printf(FS PRINTF_FORMAT, (val >> i * 6) & 0x3f);
}

static void dump_loop(unsigned long *base,
		      void (*callback)(void __iomem *))
{
	void __iomem *phy_base, *dx_base;
	int p, dx;

	for (p = 0; *base; base++, p++) {
		phy_base = ioremap(*base, SZ_4K);
		dx_base = phy_base + PHY_DX_BASE;

		for (dx = 0; dx < NR_DATX8_PER_DDRPHY; dx++) {
			printf("PHY%dDX%d:", p, dx);
			(*callback)(dx_base);
			dx_base += PHY_DX_STRIDE;
			printf("\n");
		}

		iounmap(phy_base);
	}
}

static void __wbdl_dump(void __iomem *dx_base)
{
	print_bdl(dx_base + PHY_DX_BDLR0, 5);
	print_bdl(dx_base + PHY_DX_BDLR1, 5);

	printf(FS "(+" PRINTF_FORMAT ")",
	       readl(dx_base + PHY_DX_LCDLR1) & 0xff);
}

static void wbdl_dump(unsigned long *base)
{
	printf("\n--- Write Bit Delay Line ---\n");
	printf("           DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  DQS  (WDQD)\n");

	dump_loop(base, &__wbdl_dump);
}

static void __rbdl_dump(void __iomem *dx_base)
{
	print_bdl(dx_base + PHY_DX_BDLR3, 5);
	print_bdl(dx_base + PHY_DX_BDLR4, 4);

	printf(FS "(+" PRINTF_FORMAT ")",
	       (readl(dx_base + PHY_DX_LCDLR1) >> 8) & 0xff);
}

static void rbdl_dump(unsigned long *base)
{
	printf("\n--- Read Bit Delay Line ---\n");
	printf("           DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  (RDQSD)\n");

	dump_loop(base, &__rbdl_dump);
}

static void __wld_dump(void __iomem *dx_base)
{
	int rank;
	u32 lcdlr0 = readl(dx_base + PHY_DX_LCDLR0);
	u32 gtr = readl(dx_base + PHY_DX_GTR);

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

static void __dqsgd_dump(void __iomem *dx_base)
{
	int rank;
	u32 lcdlr2 = readl(dx_base + PHY_DX_LCDLR2);
	u32 gtr = readl(dx_base + PHY_DX_GTR);

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

static void __mdl_dump(void __iomem *dx_base)
{
	int i;
	u32 mdl = readl(dx_base + PHY_DX_MDLR);
	for (i = 0; i < 3; i++)
		printf(FS PRINTF_FORMAT, (mdl >> (8 * i)) & 0xff);
}

static void mdl_dump(unsigned long *base)
{
	printf("\n--- Master Delay Line ---\n");
	printf("          IPRD TPRD MDLD\n");

	dump_loop(base, &__mdl_dump);
}

#define REG_DUMP(x)							\
	{ int ofst = PHY_ ## x; void __iomem *reg = phy_base + ofst;	\
		printf("%3d: %-10s: %p : %08x\n",			\
		       ofst >> PHY_REG_SHIFT, #x, reg, readl(reg)); }

#define DX_REG_DUMP(dx, x)						\
	{ int ofst = PHY_DX_BASE + PHY_DX_STRIDE * (dx) +		\
			PHY_DX_## x;					\
		void __iomem *reg = phy_base + ofst;			\
		printf("%3d: DX%d%-7s: %p : %08x\n",			\
		       ofst >> PHY_REG_SHIFT, (dx), #x, reg, readl(reg)); }

static void reg_dump(unsigned long *base)
{
	void __iomem *phy_base;
	int p, dx;

	printf("\n--- DDR PHY registers ---\n");

	for (p = 0; *base; base++, p++) {
		phy_base = ioremap(*base, SZ_4K);

		printf("== PHY%d (base: %p) ==\n", p, phy_base);
		printf(" No: Name      : Address  : Data\n");

		REG_DUMP(RIDR);
		REG_DUMP(PIR);
		REG_DUMP(PGCR0);
		REG_DUMP(PGCR1);
		REG_DUMP(PGSR0);
		REG_DUMP(PGSR1);
		REG_DUMP(PLLCR);
		REG_DUMP(PTR0);
		REG_DUMP(PTR1);
		REG_DUMP(PTR2);
		REG_DUMP(PTR3);
		REG_DUMP(PTR4);
		REG_DUMP(ACMDLR);
		REG_DUMP(ACBDLR);
		REG_DUMP(DXCCR);
		REG_DUMP(DSGCR);
		REG_DUMP(DCR);
		REG_DUMP(DTPR0);
		REG_DUMP(DTPR1);
		REG_DUMP(DTPR2);
		REG_DUMP(MR0);
		REG_DUMP(MR1);
		REG_DUMP(MR2);
		REG_DUMP(MR3);

		for (dx = 0; dx < NR_DATX8_PER_DDRPHY; dx++) {
			DX_REG_DUMP(dx, GCR);
			DX_REG_DUMP(dx, GTR);
		}

		iounmap(phy_base);
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
	"- dump all of the following\n"
	"ddr wbdl - dump Write Bit Delay\n"
	"ddr rbdl - dump Read Bit Delay\n"
	"ddr wld - dump Write Leveling\n"
	"ddr dqsgd - dump DQS Gating Delay\n"
	"ddr mdl - dump Master Delay Line\n"
	"ddr reg - dump registers\n"
);
