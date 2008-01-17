/*
 * Copyright 2006 Freescale Semiconductor
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
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
#include <command.h>
#include <watchdog.h>
#include <asm/cache.h>

#include "pixis.h"


static ulong strfractoint(uchar *strptr);


/*
 * Simple board reset.
 */
void pixis_reset(void)
{
    out8(PIXIS_BASE + PIXIS_RST, 0);
}


/*
 * Per table 27, page 58 of MPC8641HPCN spec.
 */
int set_px_sysclk(ulong sysclk)
{
	u8 sysclk_s, sysclk_r, sysclk_v, vclkh, vclkl, sysclk_aux;

	switch (sysclk) {
	case 33:
		sysclk_s = 0x04;
		sysclk_r = 0x04;
		sysclk_v = 0x07;
		sysclk_aux = 0x00;
		break;
	case 40:
		sysclk_s = 0x01;
		sysclk_r = 0x1F;
		sysclk_v = 0x20;
		sysclk_aux = 0x01;
		break;
	case 50:
		sysclk_s = 0x01;
		sysclk_r = 0x1F;
		sysclk_v = 0x2A;
		sysclk_aux = 0x02;
		break;
	case 66:
		sysclk_s = 0x01;
		sysclk_r = 0x04;
		sysclk_v = 0x04;
		sysclk_aux = 0x03;
		break;
	case 83:
		sysclk_s = 0x01;
		sysclk_r = 0x1F;
		sysclk_v = 0x4B;
		sysclk_aux = 0x04;
		break;
	case 100:
		sysclk_s = 0x01;
		sysclk_r = 0x1F;
		sysclk_v = 0x5C;
		sysclk_aux = 0x05;
		break;
	case 134:
		sysclk_s = 0x06;
		sysclk_r = 0x1F;
		sysclk_v = 0x3B;
		sysclk_aux = 0x06;
		break;
	case 166:
		sysclk_s = 0x06;
		sysclk_r = 0x1F;
		sysclk_v = 0x4B;
		sysclk_aux = 0x07;
		break;
	default:
		printf("Unsupported SYSCLK frequency.\n");
		return 0;
	}

	vclkh = (sysclk_s << 5) | sysclk_r;
	vclkl = sysclk_v;

	out8(PIXIS_BASE + PIXIS_VCLKH, vclkh);
	out8(PIXIS_BASE + PIXIS_VCLKL, vclkl);

	out8(PIXIS_BASE + PIXIS_AUX, sysclk_aux);

	return 1;
}


int set_px_mpxpll(ulong mpxpll)
{
	u8 tmp;
	u8 val;

	switch (mpxpll) {
	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	case 14:
	case 16:
		val = (u8) mpxpll;
		break;
	default:
		printf("Unsupported MPXPLL ratio.\n");
		return 0;
	}

	tmp = in8(PIXIS_BASE + PIXIS_VSPEED1);
	tmp = (tmp & 0xF0) | (val & 0x0F);
	out8(PIXIS_BASE + PIXIS_VSPEED1, tmp);

	return 1;
}


int set_px_corepll(ulong corepll)
{
	u8 tmp;
	u8 val;

	switch ((int)corepll) {
	case 20:
		val = 0x08;
		break;
	case 25:
		val = 0x0C;
		break;
	case 30:
		val = 0x10;
		break;
	case 35:
		val = 0x1C;
		break;
	case 40:
		val = 0x14;
		break;
	case 45:
		val = 0x0E;
		break;
	default:
		printf("Unsupported COREPLL ratio.\n");
		return 0;
	}

	tmp = in8(PIXIS_BASE + PIXIS_VSPEED0);
	tmp = (tmp & 0xE0) | (val & 0x1F);
	out8(PIXIS_BASE + PIXIS_VSPEED0, tmp);

	return 1;
}


void read_from_px_regs(int set)
{
	u8 mask = 0x1C;	/* COREPLL, MPXPLL, SYSCLK controlled by PIXIS */
	u8 tmp = in8(PIXIS_BASE + PIXIS_VCFGEN0);

	if (set)
		tmp = tmp | mask;
	else
		tmp = tmp & ~mask;
	out8(PIXIS_BASE + PIXIS_VCFGEN0, tmp);
}


void read_from_px_regs_altbank(int set)
{
	u8 mask = 0x04;	/* FLASHBANK and FLASHMAP controlled by PIXIS */
	u8 tmp = in8(PIXIS_BASE + PIXIS_VCFGEN1);

	if (set)
		tmp = tmp | mask;
	else
		tmp = tmp & ~mask;
	out8(PIXIS_BASE + PIXIS_VCFGEN1, tmp);
}

#ifndef CFG_PIXIS_VBOOT_MASK
#define CFG_PIXIS_VBOOT_MASK	(0x40)
#endif

void clear_altbank(void)
{
	u8 tmp;

	tmp = in8(PIXIS_BASE + PIXIS_VBOOT);
	tmp &= ~CFG_PIXIS_VBOOT_MASK;

	out8(PIXIS_BASE + PIXIS_VBOOT, tmp);
}


void set_altbank(void)
{
	u8 tmp;

	tmp = in8(PIXIS_BASE + PIXIS_VBOOT);
	tmp |= CFG_PIXIS_VBOOT_MASK;

	out8(PIXIS_BASE + PIXIS_VBOOT, tmp);
}


void set_px_go(void)
{
	u8 tmp;

	tmp = in8(PIXIS_BASE + PIXIS_VCTL);
	tmp = tmp & 0x1E;			/* clear GO bit */
	out8(PIXIS_BASE + PIXIS_VCTL, tmp);

	tmp = in8(PIXIS_BASE + PIXIS_VCTL);
	tmp = tmp | 0x01;	/* set GO bit - start reset sequencer */
	out8(PIXIS_BASE + PIXIS_VCTL, tmp);
}


void set_px_go_with_watchdog(void)
{
	u8 tmp;

	tmp = in8(PIXIS_BASE + PIXIS_VCTL);
	tmp = tmp & 0x1E;
	out8(PIXIS_BASE + PIXIS_VCTL, tmp);

	tmp = in8(PIXIS_BASE + PIXIS_VCTL);
	tmp = tmp | 0x09;
	out8(PIXIS_BASE + PIXIS_VCTL, tmp);
}


int pixis_disable_watchdog_cmd(cmd_tbl_t *cmdtp,
			       int flag, int argc, char *argv[])
{
	u8 tmp;

	tmp = in8(PIXIS_BASE + PIXIS_VCTL);
	tmp = tmp & 0x1E;
	out8(PIXIS_BASE + PIXIS_VCTL, tmp);

	/* setting VCTL[WDEN] to 0 to disable watch dog */
	tmp = in8(PIXIS_BASE + PIXIS_VCTL);
	tmp &= ~0x08;
	out8(PIXIS_BASE + PIXIS_VCTL, tmp);

	return 0;
}

U_BOOT_CMD(
	   diswd, 1, 0, pixis_disable_watchdog_cmd,
	   "diswd	- Disable watchdog timer \n",
	   NULL);

/*
 * This function takes the non-integral cpu:mpx pll ratio
 * and converts it to an integer that can be used to assign
 * FPGA register values.
 * input: strptr i.e. argv[2]
 */

static ulong strfractoint(uchar *strptr)
{
	int i, j, retval;
	int mulconst;
	int intarr_len = 0, decarr_len = 0, no_dec = 0;
	ulong intval = 0, decval = 0;
	uchar intarr[3], decarr[3];

	/* Assign the integer part to intarr[]
	 * If there is no decimal point i.e.
	 * if the ratio is an integral value
	 * simply create the intarr.
	 */
	i = 0;
	while (strptr[i] != '.') {
		if (strptr[i] == 0) {
			no_dec = 1;
			break;
		}
		intarr[i] = strptr[i];
		i++;
	}

	/* Assign length of integer part to intarr_len. */
	intarr_len = i;
	intarr[i] = '\0';

	if (no_dec) {
		/* Currently needed only for single digit corepll ratios */
		mulconst = 10;
		decval = 0;
	} else {
		j = 0;
		i++;		/* Skipping the decimal point */
		while ((strptr[i] >= '0') && (strptr[i] <= '9')) {
			decarr[j] = strptr[i];
			i++;
			j++;
		}

		decarr_len = j;
		decarr[j] = '\0';

		mulconst = 1;
		for (i = 0; i < decarr_len; i++)
			mulconst *= 10;
		decval = simple_strtoul((char *)decarr, NULL, 10);
	}

	intval = simple_strtoul((char *)intarr, NULL, 10);
	intval = intval * mulconst;

	retval = intval + decval;

	return retval;
}


int
pixis_reset_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int i;
	char *p_cf = NULL;
	char *p_cf_sysclk = NULL;
	char *p_cf_corepll = NULL;
	char *p_cf_mpxpll = NULL;
	char *p_altbank = NULL;
	char *p_wd = NULL;
	unsigned int unknown_param = 0;

	/*
	 * No args is a simple reset request.
	 */
	if (argc <= 1) {
		pixis_reset();
		/* not reached */
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "cf") == 0) {
			p_cf = argv[i];
			if (i + 3 >= argc) {
				break;
			}
			p_cf_sysclk = argv[i+1];
			p_cf_corepll = argv[i+2];
			p_cf_mpxpll = argv[i+3];
			i += 3;
			continue;
		}

		if (strcmp(argv[i], "altbank") == 0) {
			p_altbank = argv[i];
			continue;
		}

		if (strcmp(argv[i], "wd") == 0) {
			p_wd = argv[i];
			continue;
		}

		unknown_param = 1;
	}

	/*
	 * Check that cf has all required parms
	 */
	if ((p_cf && !(p_cf_sysclk && p_cf_corepll && p_cf_mpxpll))
	    || 	unknown_param) {
		puts(cmdtp->help);
		return 1;
	}

	/*
	 * PIXIS seems to be sensitive to the ordering of
	 * the registers that are touched.
	 */
	read_from_px_regs(0);

	if (p_altbank) {
		read_from_px_regs_altbank(0);
	}
	clear_altbank();

	/*
	 * Clock configuration specified.
	 */
	if (p_cf) {
		unsigned long sysclk;
		unsigned long corepll;
		unsigned long mpxpll;

		sysclk = simple_strtoul(p_cf_sysclk, NULL, 10);
		corepll = strfractoint((uchar *) p_cf_corepll);
		mpxpll = simple_strtoul(p_cf_mpxpll, NULL, 10);

		if (!(set_px_sysclk(sysclk)
		      && set_px_corepll(corepll)
		      && set_px_mpxpll(mpxpll))) {
			puts(cmdtp->help);
			return 1;
		}
		read_from_px_regs(1);
	}

	/*
	 * Altbank specified
	 *
	 * NOTE CHANGE IN BEHAVIOR: previous code would default
	 * to enabling watchdog if altbank is specified.
	 * Now the watchdog must be enabled explicitly using 'wd'.
	 */
	if (p_altbank) {
		set_altbank();
		read_from_px_regs_altbank(1);
	}

	/*
	 * Reset with watchdog specified.
	 */
	if (p_wd) {
		set_px_go_with_watchdog();
	} else {
		set_px_go();
	}

	/*
	 * Shouldn't be reached.
	 */
	return 0;
}


U_BOOT_CMD(
	pixis_reset, CFG_MAXARGS, 1, pixis_reset_cmd,
	"pixis_reset - Reset the board using the FPGA sequencer\n",
	"    pixis_reset\n"
	"    pixis_reset [altbank]\n"
	"    pixis_reset altbank wd\n"
	"    pixis_reset altbank cf <SYSCLK freq> <COREPLL ratio> <MPXPLL ratio>\n"
	"    pixis_reset cf <SYSCLK freq> <COREPLL ratio> <MPXPLL ratio>\n"
	);
