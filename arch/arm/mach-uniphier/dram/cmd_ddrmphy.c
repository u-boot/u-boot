/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>

#include "../init.h"
#include "ddrmphy-regs.h"

/* Select either decimal or hexadecimal */
#if 1
#define PRINTF_FORMAT "%2d"
#else
#define PRINTF_FORMAT "%02x"
#endif
/* field separator */
#define FS "   "

static void __iomem *get_phy_base(int ch)
{
	return (void __iomem *)(0x5b830000 + ch * 0x00200000);
}

static int get_nr_ch(void)
{
	const struct uniphier_board_data *bd = uniphier_get_board_param();

	return bd->dram_ch[2].size ? 3 : 2;
}

static int get_nr_datx8(int ch)
{
	const struct uniphier_board_data *bd = uniphier_get_board_param();

	return bd->dram_ch[ch].width / 8;
}

static void print_bdl(void __iomem *reg, int n)
{
	u32 val = readl(reg);
	int i;

	for (i = 0; i < n; i++)
		printf(FS PRINTF_FORMAT, (val >> i * 8) & 0x1f);
}

static void dump_loop(void (*callback)(void __iomem *))
{
	int ch, dx, nr_ch, nr_dx;
	void __iomem *dx_base;

	nr_ch = get_nr_ch();

	for (ch = 0; ch < nr_ch; ch++) {
		dx_base = get_phy_base(ch) + DMPHY_DX_BASE;
		nr_dx = get_nr_datx8(ch);

		for (dx = 0; dx < nr_dx; dx++) {
			printf("CH%dDX%d:", ch, dx);
			(*callback)(dx_base);
			dx_base += DMPHY_DX_STRIDE;
			printf("\n");
		}
	}
}

static void zq_dump(void)
{
	int ch, zq, nr_ch, nr_zq, i;
	void __iomem *zq_base;
	u32 dr, pr;

	printf("\n--- Impedance Data ---\n");
	printf("         ZPD  ZPU  OPD  OPU  ZDV  ODV\n");

	nr_ch = get_nr_ch();

	for (ch = 0; ch < nr_ch; ch++) {
		zq_base = get_phy_base(ch) + DMPHY_ZQ_BASE;
		nr_zq = 3;

		for (zq = 0; zq < nr_zq; zq++) {
			printf("CH%dZQ%d:", ch, zq);

			dr = readl(zq_base + DMPHY_ZQ_DR);
			for (i = 0; i < 4; i++) {
				printf(FS PRINTF_FORMAT, dr & 0x7f);
				dr >>= 7;
			}

			pr = readl(zq_base + DMPHY_ZQ_PR);
			for (i = 0; i < 2; i++) {
				printf(FS PRINTF_FORMAT, pr & 0xf);
				pr >>= 4;
			}

			zq_base += DMPHY_ZQ_STRIDE;
			printf("\n");
		}
	}
}

static void __wbdl_dump(void __iomem *dx_base)
{
	print_bdl(dx_base + DMPHY_DX_BDLR0, 4);
	print_bdl(dx_base + DMPHY_DX_BDLR1, 4);
	print_bdl(dx_base + DMPHY_DX_BDLR2, 2);

	printf(FS "(+" PRINTF_FORMAT ")",
	       readl(dx_base + DMPHY_DX_LCDLR1) & 0xff);
}

static void wbdl_dump(void)
{
	printf("\n--- Write Bit Delay Line ---\n");
	printf("         DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  DQS  (WDQD)\n");

	dump_loop(&__wbdl_dump);
}

static void __rbdl_dump(void __iomem *dx_base)
{
	print_bdl(dx_base + DMPHY_DX_BDLR3, 4);
	print_bdl(dx_base + DMPHY_DX_BDLR4, 4);
	print_bdl(dx_base + DMPHY_DX_BDLR5, 1);

	printf(FS "(+" PRINTF_FORMAT ")",
	       (readl(dx_base + DMPHY_DX_LCDLR1) >> 8) & 0xff);

	printf(FS "(+" PRINTF_FORMAT ")",
	       (readl(dx_base + DMPHY_DX_LCDLR1) >> 16) & 0xff);
}

static void rbdl_dump(void)
{
	printf("\n--- Read Bit Delay Line ---\n");
	printf("         DQ0  DQ1  DQ2  DQ3  DQ4  DQ5  DQ6  DQ7   DM  (RDQSD) (RDQSND)\n");

	dump_loop(&__rbdl_dump);
}

static void __wld_dump(void __iomem *dx_base)
{
	int rank;
	u32 lcdlr0 = readl(dx_base + DMPHY_DX_LCDLR0);
	u32 gtr = readl(dx_base + DMPHY_DX_GTR);

	for (rank = 0; rank < 4; rank++) {
		u32 wld = (lcdlr0 >> (8 * rank)) & 0xff; /* Delay */
		u32 wlsl = (gtr >> (12 + 2 * rank)) & 0x3; /* System Latency */

		printf(FS PRINTF_FORMAT "%sT", wld,
		       wlsl == 0 ? "-1" : wlsl == 1 ? "+0" : "+1");
	}
}

static void wld_dump(void)
{
	printf("\n--- Write Leveling Delay ---\n");
	printf("          Rank0   Rank1   Rank2   Rank3\n");

	dump_loop(&__wld_dump);
}

static void __dqsgd_dump(void __iomem *dx_base)
{
	int rank;
	u32 lcdlr2 = readl(dx_base + DMPHY_DX_LCDLR2);
	u32 gtr = readl(dx_base + DMPHY_DX_GTR);

	for (rank = 0; rank < 4; rank++) {
		u32 dqsgd = (lcdlr2 >> (8 * rank)) & 0xff; /* Delay */
		u32 dgsl = (gtr >> (3 * rank)) & 0x7; /* System Latency */

		printf(FS PRINTF_FORMAT "+%dT", dqsgd, dgsl);
	}
}

static void dqsgd_dump(void)
{
	printf("\n--- DQS Gating Delay ---\n");
	printf("          Rank0   Rank1   Rank2   Rank3\n");

	dump_loop(&__dqsgd_dump);
}

static void __mdl_dump(void __iomem *dx_base)
{
	int i;
	u32 mdl = readl(dx_base + DMPHY_DX_MDLR);

	for (i = 0; i < 3; i++)
		printf(FS PRINTF_FORMAT, (mdl >> (8 * i)) & 0xff);
}

static void mdl_dump(void)
{
	printf("\n--- Master Delay Line ---\n");
	printf("        IPRD TPRD MDLD\n");

	dump_loop(&__mdl_dump);
}

#define REG_DUMP(x)							\
	{ int ofst = DMPHY_ ## x; void __iomem *reg = phy_base + ofst;	\
		printf("%3d: %-10s: %p : %08x\n",			\
		       ofst >> DMPHY_SHIFT, #x, reg, readl(reg)); }

#define DX_REG_DUMP(dx, x)						\
	{ int ofst = DMPHY_DX_BASE + DMPHY_DX_STRIDE * (dx) +		\
			DMPHY_DX_## x;					\
		void __iomem *reg = phy_base + ofst;			\
		printf("%3d: DX%d%-7s: %p : %08x\n",			\
		       ofst >> DMPHY_SHIFT, (dx), #x, reg, readl(reg)); }

static void reg_dump(void)
{
	int ch, dx, nr_ch, nr_dx;
	void __iomem *phy_base;

	printf("\n--- DDR PHY registers ---\n");

	nr_ch = get_nr_ch();

	for (ch = 0; ch < nr_ch; ch++) {
		phy_base = get_phy_base(ch);
		nr_dx = get_nr_datx8(ch);

		printf("== Ch%d ==\n", ch);
		printf(" No: Name      : Address  : Data\n");

		REG_DUMP(RIDR);
		REG_DUMP(PIR);
		REG_DUMP(PGCR0);
		REG_DUMP(PGCR1);
		REG_DUMP(PGCR2);
		REG_DUMP(PGCR3);
		REG_DUMP(PGSR0);
		REG_DUMP(PGSR1);
		REG_DUMP(PLLCR);
		REG_DUMP(PTR0);
		REG_DUMP(PTR1);
		REG_DUMP(PTR2);
		REG_DUMP(PTR3);
		REG_DUMP(PTR4);
		REG_DUMP(ACMDLR);
		REG_DUMP(ACBDLR0);
		REG_DUMP(DXCCR);
		REG_DUMP(DSGCR);
		REG_DUMP(DCR);
		REG_DUMP(DTPR0);
		REG_DUMP(DTPR1);
		REG_DUMP(DTPR2);
		REG_DUMP(DTPR3);
		REG_DUMP(MR0);
		REG_DUMP(MR1);
		REG_DUMP(MR2);
		REG_DUMP(MR3);

		for (dx = 0; dx < nr_dx; dx++) {
			DX_REG_DUMP(dx, GCR0);
			DX_REG_DUMP(dx, GCR1);
			DX_REG_DUMP(dx, GCR2);
			DX_REG_DUMP(dx, GCR3);
			DX_REG_DUMP(dx, GTR);
		}
	}
}

static int do_ddrm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *cmd = argv[1];

	if (argc == 1)
		cmd = "all";

	if (!strcmp(cmd, "zq") || !strcmp(cmd, "all"))
		zq_dump();

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
	ddrm,	2,	1,	do_ddrm,
	"UniPhier DDR PHY parameters dumper",
	"- dump all of the following\n"
	"ddrm zq - dump Impedance Data\n"
	"ddrm wbdl - dump Write Bit Delay\n"
	"ddrm rbdl - dump Read Bit Delay\n"
	"ddrm wld - dump Write Leveling\n"
	"ddrm dqsgd - dump DQS Gating Delay\n"
	"ddrm mdl - dump Master Delay Line\n"
	"ddrm reg - dump registers\n"
);
