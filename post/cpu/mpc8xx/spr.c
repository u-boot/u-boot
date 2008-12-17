/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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

static struct
{
    int number;
    char * name;
    unsigned long mask;
    unsigned long value;
} spr_test_list [] = {
	/* Standard Special-Purpose Registers */

	{1,	"XER",		0x00000000,	0x00000000},
	{8,	"LR",		0x00000000,	0x00000000},
	{9,	"CTR",		0x00000000,	0x00000000},
	{18,	"DSISR",	0x00000000,	0x00000000},
	{19,	"DAR",		0x00000000,	0x00000000},
	{22,	"DEC",		0x00000000,	0x00000000},
	{26,	"SRR0",		0x00000000,	0x00000000},
	{27,	"SRR1",		0x00000000,	0x00000000},
	{272,	"SPRG0",	0x00000000,	0x00000000},
	{273,	"SPRG1",	0x00000000,	0x00000000},
	{274,	"SPRG2",	0x00000000,	0x00000000},
	{275,	"SPRG3",	0x00000000,	0x00000000},
	{287,	"PVR",		0xFFFF0000,	0x00500000},

	/* Additional Special-Purpose Registers */

	{144,	"CMPA",		0x00000000,	0x00000000},
	{145,	"CMPB",		0x00000000,	0x00000000},
	{146,	"CMPC",		0x00000000,	0x00000000},
	{147,	"CMPD",		0x00000000,	0x00000000},
	{148,	"ICR",		0xFFFFFFFF,	0x00000000},
	{149,	"DER",		0x00000000,	0x00000000},
	{150,	"COUNTA",	0xFFFFFFFF,	0x00000000},
	{151,	"COUNTB",	0xFFFFFFFF,	0x00000000},
	{152,	"CMPE",		0x00000000,	0x00000000},
	{153,	"CMPF",		0x00000000,	0x00000000},
	{154,	"CMPG",		0x00000000,	0x00000000},
	{155,	"CMPH",		0x00000000,	0x00000000},
	{156,	"LCTRL1",	0xFFFFFFFF,	0x00000000},
	{157,	"LCTRL2",	0xFFFFFFFF,	0x00000000},
	{158,	"ICTRL",	0xFFFFFFFF,	0x00000007},
	{159,	"BAR",		0x00000000,	0x00000000},
	{630,	"DPDR",		0x00000000,	0x00000000},
	{631,	"DPIR",		0x00000000,	0x00000000},
	{638,	"IMMR",		0xFFFF0000,	CONFIG_SYS_IMMR  },
	{560,	"IC_CST",	0x8E380000,	0x00000000},
	{561,	"IC_ADR",	0x00000000,	0x00000000},
	{562,	"IC_DAT",	0x00000000,	0x00000000},
	{568,	"DC_CST",	0xEF380000,	0x00000000},
	{569,	"DC_ADR",	0x00000000,	0x00000000},
	{570,	"DC_DAT",	0x00000000,	0x00000000},
	{784,	"MI_CTR",	0xFFFFFFFF,	0x00000000},
	{786,	"MI_AP",	0x00000000,	0x00000000},
	{787,	"MI_EPN",	0x00000000,	0x00000000},
	{789,	"MI_TWC",	0xFFFFFE02,	0x00000000},
	{790,	"MI_RPN",	0x00000000,	0x00000000},
	{816,	"MI_DBCAM",	0x00000000,	0x00000000},
	{817,	"MI_DBRAM0",	0x00000000,	0x00000000},
	{818,	"MI_DBRAM1",	0x00000000,	0x00000000},
	{792,	"MD_CTR",	0xFFFFFFFF,	0x04000000},
	{793,	"M_CASID",	0xFFFFFFF0,	0x00000000},
	{794,	"MD_AP",	0x00000000,	0x00000000},
	{795,	"MD_EPN",	0x00000000,	0x00000000},
	{796,	"M_TWB",	0x00000003,	0x00000000},
	{797,	"MD_TWC",	0x00000003,	0x00000000},
	{798,	"MD_RPN",	0x00000000,	0x00000000},
	{799,	"M_TW",		0x00000000,	0x00000000},
	{824,	"MD_DBCAM",	0x00000000,	0x00000000},
	{825,	"MD_DBRAM0",	0x00000000,	0x00000000},
	{826,	"MD_DBRAM1",	0x00000000,	0x00000000},
};

static int spr_test_list_size =
		sizeof (spr_test_list) / sizeof (spr_test_list[0]);

int spr_post_test (int flags)
{
	int ret = 0;
	int ic = icache_status ();
	int i;

	unsigned long code[] = {
		0x7c6002a6,				/* mfspr r3,SPR */
		0x4e800020				/* blr          */
	};
	unsigned long (*get_spr) (void) = (void *) code;

	if (ic)
		icache_disable ();

	for (i = 0; i < spr_test_list_size; i++) {
		int num = spr_test_list[i].number;

		/* mfspr r3,num */
		code[0] = 0x7c6002a6 | ((num & 0x1F) << 16) | ((num & 0x3E0) << 6);

		if ((get_spr () & spr_test_list[i].mask) !=
			(spr_test_list[i].value & spr_test_list[i].mask)) {
			post_log ("The value of %s special register "
				  "is incorrect: 0x%08X\n",
					spr_test_list[i].name, get_spr ());
			ret = -1;
		}
	}

	if (ic)
		icache_enable ();

	return ret;
}
#endif /* CONFIG_POST & CONFIG_SYS_POST_SPR */
