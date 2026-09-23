// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 * Ryan Chen <ryan_chen@aspeedtech.com>
 */

#include <command.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/arch/scu_ast2700.h>

/* SoC mapping Table */
#define SOC_ID(str, rev) { .name = str, .rev_id = rev, }

struct soc_id {
	const char *name;
	u64 rev_id;
};

static struct soc_id soc_map_table[] = {
	SOC_ID("AST2750-A0", 0x0600000306000003),
	SOC_ID("AST2700-A0", 0x0600010306000103),
	SOC_ID("AST2720-A0", 0x0600020306000203),
	SOC_ID("AST2750-A1", 0x0601000306010003),
	SOC_ID("AST2700-A1", 0x0601010306010103),
	SOC_ID("AST2720-A1", 0x0601020306010203),
	SOC_ID("AST2750-A2", 0x0602000306020003),
	SOC_ID("AST2700-A2", 0x0602010306020103),
	SOC_ID("AST2720-A2", 0x0602020306020203),
};

void ast2700_print_soc_id(void)
{
	int i;
	u64 rev_id;

	rev_id = readl(ASPEED_CPU_REVISION_ID);
	rev_id = ((u64)readl(ASPEED_IO_REVISION_ID) << 32) | rev_id;

	for (i = 0; i < ARRAY_SIZE(soc_map_table); i++) {
		if (rev_id == soc_map_table[i].rev_id)
			break;
	}
	if (i == ARRAY_SIZE(soc_map_table))
		printf("Unknown-SOC: %llx\n", rev_id);
	else
		printf("SOC: %4s\n", soc_map_table[i].name);
}

#define SYS_DRAM_ECCRST	BIT(3)
#define SYS_ABRRST		BIT(2)
#define SYS_EXTRST		BIT(1)
#define SYS_SRST		BIT(0)

#define WDT_RST_BIT_MASK(s)	(GENMASK(3, 0) << (s))
#define BIT_WDT_FULL(s)		(BIT(0) << (s))
#define BIT_WDT_ARM(s)		(BIT(1) << (s))
#define BIT_WDT_SOC(s)		(BIT(2) << (s))
#define BIT_WDT_SW(s)		(BIT(3) << (s))

void ast2700_print_wdtrst_info(void)
{
	u32 wdt_rst = readl(ASPEED_IO_RESET_LOG4);
	int i;

	for (i = 0; i < 8; i++) {
		if (wdt_rst & WDT_RST_BIT_MASK(i * 4)) {
			printf("RST: WDT%d ", i);
			if (wdt_rst & BIT_WDT_SOC(i * 4)) {
				printf("SOC ");
				writel(BIT_WDT_SOC(i * 4), ASPEED_IO_RESET_LOG4);
			}
			if (wdt_rst & BIT_WDT_FULL(i * 4)) {
				printf("FULL ");
				writel(BIT_WDT_FULL(i * 4), ASPEED_IO_RESET_LOG4);
			}
			if (wdt_rst & BIT_WDT_ARM(i * 4)) {
				printf("ARM ");
				writel(BIT_WDT_ARM(i * 4), ASPEED_IO_RESET_LOG4);
			}
			if (wdt_rst & BIT_WDT_SW(i * 4)) {
				printf("SW ");
				writel(BIT_WDT_SW(i * 4), ASPEED_IO_RESET_LOG4);
			}
			printf("\n");
		}
	}
}

#define SYS_EXTRST		BIT(1)
#define SYS_SRST		BIT(0)

void ast2700_print_sysrst_info(void)
{
	u32 sys_rst = readl(ASPEED_CPU_RESET_LOG1);

	if (sys_rst & SYS_SRST) {
		printf("RST: Power On\n");
		writel(SYS_SRST, ASPEED_CPU_RESET_LOG1);
	} else if (sys_rst & SYS_EXTRST) {
		printf("RST: EXTRST\n");
		writel(SYS_EXTRST, ASPEED_CPU_RESET_LOG1);
	} else {
		ast2700_print_wdtrst_info();
	}
}

int print_cpuinfo(void)
{
	ast2700_print_soc_id();
	ast2700_print_sysrst_info();

	return 0;
}
