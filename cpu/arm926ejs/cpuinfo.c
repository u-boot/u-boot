/*
 * OMAP1 CPU identification code
 *
 * Copyright (C) 2004 Nokia Corporation
 * Written by Tony Lindgren <tony@atomide.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <arm926ejs.h>

#if defined(CONFIG_DISPLAY_CPUINFO) && defined(CONFIG_OMAP)

#define omap_readw(x)		*(volatile unsigned short *)(x)
#define omap_readl(x)		*(volatile unsigned long *)(x)

#define OMAP_DIE_ID_0		0xfffe1800
#define OMAP_DIE_ID_1		0xfffe1804
#define OMAP_PRODUCTION_ID_0	0xfffe2000
#define OMAP_PRODUCTION_ID_1	0xfffe2004
#define OMAP32_ID_0		0xfffed400
#define OMAP32_ID_1		0xfffed404

struct omap_id {
	u16	jtag_id;	/* Used to determine OMAP type */
	u8	die_rev;	/* Processor revision */
	u32	omap_id;	/* OMAP revision */
	u32	type;		/* Cpu id bits [31:08], cpu class bits [07:00] */
};

/* Register values to detect the OMAP version */
static struct omap_id omap_ids[] = {
	{ .jtag_id = 0xb574, .die_rev = 0x2, .omap_id = 0x03310315, .type = 0x03100000},
	{ .jtag_id = 0x355f, .die_rev = 0x0, .omap_id = 0x03320000, .type = 0x07300100},
	{ .jtag_id = 0xb55f, .die_rev = 0x0, .omap_id = 0x03320000, .type = 0x07300300},
	{ .jtag_id = 0xb470, .die_rev = 0x0, .omap_id = 0x03310100, .type = 0x15100000},
	{ .jtag_id = 0xb576, .die_rev = 0x0, .omap_id = 0x03320000, .type = 0x16100000},
	{ .jtag_id = 0xb576, .die_rev = 0x2, .omap_id = 0x03320100, .type = 0x16110000},
	{ .jtag_id = 0xb576, .die_rev = 0x3, .omap_id = 0x03320100, .type = 0x16100c00},
	{ .jtag_id = 0xb576, .die_rev = 0x0, .omap_id = 0x03320200, .type = 0x16100d00},
	{ .jtag_id = 0xb613, .die_rev = 0x0, .omap_id = 0x03320300, .type = 0x1610ef00},
	{ .jtag_id = 0xb613, .die_rev = 0x0, .omap_id = 0x03320300, .type = 0x1610ef00},
	{ .jtag_id = 0xb576, .die_rev = 0x1, .omap_id = 0x03320100, .type = 0x16110000},
	{ .jtag_id = 0xb58c, .die_rev = 0x2, .omap_id = 0x03320200, .type = 0x16110b00},
	{ .jtag_id = 0xb58c, .die_rev = 0x3, .omap_id = 0x03320200, .type = 0x16110c00},
	{ .jtag_id = 0xb65f, .die_rev = 0x0, .omap_id = 0x03320400, .type = 0x16212300},
	{ .jtag_id = 0xb65f, .die_rev = 0x1, .omap_id = 0x03320400, .type = 0x16212300},
	{ .jtag_id = 0xb65f, .die_rev = 0x1, .omap_id = 0x03320500, .type = 0x16212300},
	{ .jtag_id = 0xb5f7, .die_rev = 0x0, .omap_id = 0x03330000, .type = 0x17100000},
	{ .jtag_id = 0xb5f7, .die_rev = 0x1, .omap_id = 0x03330100, .type = 0x17100000},
	{ .jtag_id = 0xb5f7, .die_rev = 0x2, .omap_id = 0x03330100, .type = 0x17100000},
};

/*
 * Get OMAP type from PROD_ID.
 * 1710 has the PROD_ID in bits 15:00, not in 16:01 as documented in TRM.
 * 1510 PROD_ID is empty, and 1610 PROD_ID does not make sense.
 * Undocumented register in TEST BLOCK is used as fallback; This seems to
 * work on 1510, 1610 & 1710. The official way hopefully will work in future
 * processors.
 */
static u16 omap_get_jtag_id(void)
{
	u32 prod_id, omap_id;

	prod_id = omap_readl(OMAP_PRODUCTION_ID_1);
	omap_id = omap_readl(OMAP32_ID_1);

	/* Check for unusable OMAP_PRODUCTION_ID_1 on 1611B/5912 and 730 */
	if (((prod_id >> 20) == 0) || (prod_id == omap_id))
		prod_id = 0;
	else
		prod_id &= 0xffff;

	if (prod_id)
		return prod_id;

	/* Use OMAP32_ID_1 as fallback */
	prod_id = ((omap_id >> 12) & 0xffff);

	return prod_id;
}

/*
 * Get OMAP revision from DIE_REV.
 * Early 1710 processors may have broken OMAP_DIE_ID, it contains PROD_ID.
 * Undocumented register in the TEST BLOCK is used as fallback.
 * REVISIT: This does not seem to work on 1510
 */
static u8 omap_get_die_rev(void)
{
	u32 die_rev;

	die_rev = omap_readl(OMAP_DIE_ID_1);

	/* Check for broken OMAP_DIE_ID on early 1710 */
	if (((die_rev >> 12) & 0xffff) == omap_get_jtag_id())
		die_rev = 0;

	die_rev = (die_rev >> 17) & 0xf;
	if (die_rev)
		return die_rev;

	die_rev = (omap_readl(OMAP32_ID_1) >> 28) & 0xf;

	return die_rev;
}

static unsigned long dpll1(void)
{
	unsigned short pll_ctl_val = omap_readw(DPLL_CTL_REG);
	unsigned long rate;

	rate = CONFIG_SYS_CLK_FREQ; /* Base xtal rate */
	if (pll_ctl_val & 0x10) {
		/* PLL enabled, apply multiplier and divisor */
		if (pll_ctl_val & 0xf80)
			rate *= (pll_ctl_val & 0xf80) >> 7;
		rate /= ((pll_ctl_val & 0x60) >> 5) + 1;
	} else {
		/* PLL disabled, apply bypass divisor */
		switch (pll_ctl_val & 0xc) {
		case 0:
			break;
		case 0x4:
			rate /= 2;
			break;
		default:
			rate /= 4;
			break;
		}
	}

	return rate;
}

static unsigned long armcore(void)
{
	unsigned short arm_ckctl = omap_readw(ARM_CKCTL);

	return (dpll1() >> ((arm_ckctl & 0x0030) >> 4));
}

int print_cpuinfo (void)
{
	int i;
	u16 jtag_id;
	u8 die_rev;
	u32 omap_id;
	u8 cpu_type;
	u32 system_serial_high;
	u32 system_serial_low;
	u32 system_rev = 0;

	jtag_id = omap_get_jtag_id();
	die_rev = omap_get_die_rev();
	omap_id = omap_readl(OMAP32_ID_0);

#ifdef DEBUG
	printf("OMAP_DIE_ID_0: 0x%08x\n", omap_readl(OMAP_DIE_ID_0));
	printf("OMAP_DIE_ID_1: 0x%08x DIE_REV: %i\n",
	       omap_readl(OMAP_DIE_ID_1),
	       (omap_readl(OMAP_DIE_ID_1) >> 17) & 0xf);
	printf("OMAP_PRODUCTION_ID_0: 0x%08x\n", omap_readl(OMAP_PRODUCTION_ID_0));
	printf("OMAP_PRODUCTION_ID_1: 0x%08x JTAG_ID: 0x%04x\n",
	       omap_readl(OMAP_PRODUCTION_ID_1),
	       omap_readl(OMAP_PRODUCTION_ID_1) & 0xffff);
	printf("OMAP32_ID_0: 0x%08x\n", omap_readl(OMAP32_ID_0));
	printf("OMAP32_ID_1: 0x%08x\n", omap_readl(OMAP32_ID_1));
	printf("JTAG_ID: 0x%04x DIE_REV: %i\n", jtag_id, die_rev);
#endif

	system_serial_high = omap_readl(OMAP_DIE_ID_0);
	system_serial_low = omap_readl(OMAP_DIE_ID_1);

	/* First check only the major version in a safe way */
	for (i = 0; i < ARRAY_SIZE(omap_ids); i++) {
		if (jtag_id == (omap_ids[i].jtag_id)) {
			system_rev = omap_ids[i].type;
			break;
		}
	}

	/* Check if we can find the die revision */
	for (i = 0; i < ARRAY_SIZE(omap_ids); i++) {
		if (jtag_id == omap_ids[i].jtag_id && die_rev == omap_ids[i].die_rev) {
			system_rev = omap_ids[i].type;
			break;
		}
	}

	/* Finally check also the omap_id */
	for (i = 0; i < ARRAY_SIZE(omap_ids); i++) {
		if (jtag_id == omap_ids[i].jtag_id
		    && die_rev == omap_ids[i].die_rev
		    && omap_id == omap_ids[i].omap_id) {
			system_rev = omap_ids[i].type;
			break;
		}
	}

	/* Add the cpu class info (7xx, 15xx, 16xx, 24xx) */
	cpu_type = system_rev >> 24;

	switch (cpu_type) {
	case 0x07:
		system_rev |= 0x07;
		break;
	case 0x03:
	case 0x15:
		system_rev |= 0x15;
		break;
	case 0x16:
	case 0x17:
		system_rev |= 0x16;
		break;
	case 0x24:
		system_rev |= 0x24;
		break;
	default:
		printf("Unknown OMAP cpu type: 0x%02x\n", cpu_type);
	}

	printf("CPU:   OMAP%04x", system_rev >> 16);
	if ((system_rev >> 8) & 0xff)
		printf("%x", (system_rev >> 8) & 0xff);
#ifdef DEBUG
	printf(" revision %i handled as %02xxx id: %08x%08x",
	       die_rev, system_rev & 0xff, system_serial_low, system_serial_high);
#endif
	printf(" at %ld.%01ld MHz (DPLL1=%ld.%01ld MHz)\n",
	       armcore() / 1000000, (armcore() / 100000) % 10,
	       dpll1() / 1000000, (dpll1() / 100000) % 10);

	return 0;
}

#endif /* #if defined(CONFIG_DISPLAY_CPUINFO) && defined(CONFIG_OMAP) */
