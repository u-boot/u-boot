/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Igor Lisitsin <igor@emcraft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

/*
 * SPR test
 *
 * The test checks the contents of Special Purpose Registers (SPR) listed
 * in the spr_test_list array below.
 * Each SPR value is read using mfspr instruction, some bits are masked
 * according to the table and the resulting value is compared to the
 * corresponding table value.
 */

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_SPR

#include <asm/processor.h>

#ifdef CONFIG_4xx_DCACHE
#include <asm/mmu.h>

DECLARE_GLOBAL_DATA_PTR;
#endif

static struct {
	int number;
	char * name;
	unsigned long mask;
	unsigned long value;
} spr_test_list [] = {
	/* Standard Special-Purpose Registers */

	{0x001,	"XER",		0x00000000,	0x00000000},
	{0x008,	"LR",		0x00000000,	0x00000000},
	{0x009,	"CTR",		0x00000000,	0x00000000},
	{0x016,	"DEC",		0x00000000,	0x00000000},
	{0x01a,	"SRR0",		0x00000000,	0x00000000},
	{0x01b,	"SRR1",		0x00000000,	0x00000000},
	{0x110,	"SPRG0",	0x00000000,	0x00000000},
	{0x111,	"SPRG1",	0x00000000,	0x00000000},
	{0x112,	"SPRG2",	0x00000000,	0x00000000},
	{0x113,	"SPRG3",	0x00000000,	0x00000000},
	{0x11f,	"PVR",		0x00000000,	0x00000000},

	/* Additional Special-Purpose Registers.
	 * The values must match the initialization
	 * values from arch/powerpc/cpu/ppc4xx/start.S
	 */
	{0x30,	"PID",		0x00000000,	0x00000000},
	{0x3a,	"CSRR0",	0x00000000,	0x00000000},
	{0x3b,	"CSRR1",	0x00000000,	0x00000000},
	{0x3d,	"DEAR",		0x00000000,	0x00000000},
	{0x3e,	"ESR",		0x00000000,	0x00000000},
#ifdef CONFIG_440
	{0x3f,	"IVPR",		0xffff0000,	0x00000000},
#endif
	{0x100,	"USPRG0",	0x00000000,	0x00000000},
	{0x104,	"SPRG4",	0x00000000,	0x00000000},
	{0x105,	"SPRG5",	0x00000000,	0x00000000},
	{0x106,	"SPRG6",	0x00000000,	0x00000000},
	{0x107,	"SPRG7",	0x00000000,	0x00000000},
	{0x10c,	"TBL",		0x00000000,	0x00000000},
	{0x10d,	"TBU",		0x00000000,	0x00000000},
#ifdef CONFIG_440
	{0x11e,	"PIR",		0x0000000f,	0x00000000},
#endif
	{0x130,	"DBSR",		0x00000000,	0x00000000},
	{0x134,	"DBCR0",	0x00000000,	0x00000000},
	{0x135,	"DBCR1",	0x00000000,	0x00000000},
	{0x136,	"DBCR2",	0x00000000,	0x00000000},
	{0x138,	"IAC1",		0x00000000,	0x00000000},
	{0x139,	"IAC2",		0x00000000,	0x00000000},
	{0x13a,	"IAC3",		0x00000000,	0x00000000},
	{0x13b,	"IAC4",		0x00000000,	0x00000000},
	{0x13c,	"DAC1",		0x00000000,	0x00000000},
	{0x13d,	"DAC2",		0x00000000,	0x00000000},
	{0x13e,	"DVC1",		0x00000000,	0x00000000},
	{0x13f,	"DVC2",		0x00000000,	0x00000000},
	{0x150,	"TSR",		0x00000000,	0x00000000},
	{0x154,	"TCR",		0x00000000,	0x00000000},
#ifdef CONFIG_440
	{0x190,	"IVOR0",	0x0000fff0,	0x00000100},
	{0x191,	"IVOR1",	0x0000fff0,	0x00000200},
	{0x192,	"IVOR2",	0x0000fff0,	0x00000300},
	{0x193,	"IVOR3",	0x0000fff0,	0x00000400},
	{0x194,	"IVOR4",	0x0000fff0,	0x00000500},
	{0x195,	"IVOR5",	0x0000fff0,	0x00000600},
	{0x196,	"IVOR6",	0x0000fff0,	0x00000700},
	{0x197,	"IVOR7",	0x0000fff0,	0x00000800},
	{0x198,	"IVOR8",	0x0000fff0,	0x00000c00},
	{0x199,	"IVOR9",	0x00000000,	0x00000000},
	{0x19a,	"IVOR10",	0x0000fff0,	0x00000900},
	{0x19b,	"IVOR11",	0x00000000,	0x00000000},
	{0x19c,	"IVOR12",	0x00000000,	0x00000000},
	{0x19d,	"IVOR13",	0x0000fff0,	0x00001300},
	{0x19e,	"IVOR14",	0x0000fff0,	0x00001400},
	{0x19f,	"IVOR15",	0x0000fff0,	0x00002000},
#endif
	{0x23a,	"MCSRR0",	0x00000000,	0x00000000},
	{0x23b,	"MCSRR1",	0x00000000,	0x00000000},
	{0x23c,	"MCSR",		0x00000000,	0x00000000},
	{0x370,	"INV0",		0x00000000,	0x00000000},
	{0x371,	"INV1",		0x00000000,	0x00000000},
	{0x372,	"INV2",		0x00000000,	0x00000000},
	{0x373,	"INV3",		0x00000000,	0x00000000},
	{0x374,	"ITV0",		0x00000000,	0x00000000},
	{0x375,	"ITV1",		0x00000000,	0x00000000},
	{0x376,	"ITV2",		0x00000000,	0x00000000},
	{0x377,	"ITV3",		0x00000000,	0x00000000},
	{0x378,	"CCR1",		0x00000000,	0x00000000},
	{0x390,	"DNV0",		0x00000000,	0x00000000},
	{0x391,	"DNV1",		0x00000000,	0x00000000},
	{0x392,	"DNV2",		0x00000000,	0x00000000},
	{0x393,	"DNV3",		0x00000000,	0x00000000},
	{0x394,	"DTV0",		0x00000000,	0x00000000},
	{0x395,	"DTV1",		0x00000000,	0x00000000},
	{0x396,	"DTV2",		0x00000000,	0x00000000},
	{0x397,	"DTV3",		0x00000000,	0x00000000},
#ifdef CONFIG_440
	{0x398,	"DVLIM",	0x0fc1f83f,	0x0001f800},
	{0x399,	"IVLIM",	0x0fc1f83f,	0x0001f800},
#endif
	{0x39b,	"RSTCFG",	0x00000000,	0x00000000},
	{0x39c,	"DCDBTRL",	0x00000000,	0x00000000},
	{0x39d,	"DCDBTRH",	0x00000000,	0x00000000},
	{0x39e,	"ICDBTRL",	0x00000000,	0x00000000},
	{0x39f,	"ICDBTRH",	0x00000000,	0x00000000},
	{0x3b2,	"MMUCR",	0x00000000,	0x00000000},
	{0x3b3,	"CCR0",		0x00000000,	0x00000000},
	{0x3d3,	"ICDBDR",	0x00000000,	0x00000000},
	{0x3f3,	"DBDR",		0x00000000,	0x00000000},
};

static int spr_test_list_size = ARRAY_SIZE(spr_test_list);

int spr_post_test (int flags)
{
	int ret = 0;
	int i;

	unsigned long code[] = {
		0x7c6002a6,				/* mfspr r3,SPR */
		0x4e800020				/* blr          */
	};
	unsigned long (*get_spr) (void) = (void *) code;

#ifdef CONFIG_4xx_DCACHE
	/* disable cache */
	change_tlb(gd->bd->bi_memstart, gd->bd->bi_memsize, TLB_WORD2_I_ENABLE);
#endif
	for (i = 0; i < spr_test_list_size; i++) {
		int num = spr_test_list[i].number;

		/* mfspr r3,num */
		code[0] = 0x7c6002a6 | ((num & 0x1F) << 16) | ((num & 0x3E0) << 6);

		asm volatile ("isync");

		if ((get_spr () & spr_test_list[i].mask) !=
			(spr_test_list[i].value & spr_test_list[i].mask)) {
			post_log ("The value of %s special register "
				  "is incorrect: 0x%08X\n",
					spr_test_list[i].name, get_spr ());
			ret = -1;
		}
	}
#ifdef CONFIG_4xx_DCACHE
	/* enable cache */
	change_tlb(gd->bd->bi_memstart, gd->bd->bi_memsize, 0);
#endif

	return ret;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_SPR */
