/*
 * (C) Copyright 2000
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
#include <config.h>
#include <mpc8xx.h>
#include "fads.h"

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

#if defined(CONFIG_DRAM_50MHZ)
/* 50MHz tables */
const uint dram_60ns[] =
{ 0x8fffec24, 0x0fffec04, 0x0cffec04, 0x00ffec04,
  0x00ffec00, 0x37ffec47, 0xffffffff, 0xffffffff,
  0x8fffec24, 0x0fffec04, 0x08ffec04, 0x00ffec0c,
  0x03ffec00, 0x00ffec44, 0x00ffcc08, 0x0cffcc44,
  0x00ffec0c, 0x03ffec00, 0x00ffec44, 0x00ffcc00,
  0x3fffc847, 0xffffffff, 0xffffffff, 0xffffffff,
  0x8fafcc24, 0x0fafcc04, 0x0cafcc00, 0x11bfcc47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x8fafcc24, 0x0fafcc04, 0x0cafcc00, 0x03afcc4c,
  0x0cafcc00, 0x03afcc4c, 0x0cafcc00, 0x03afcc4c,
  0x0cafcc00, 0x33bfcc4f, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xc0ffcc84, 0x00ffcc04, 0x07ffcc04, 0x3fffcc06,
  0xffffcc85, 0xffffcc05, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x33ffcc07, 0xffffffff, 0xffffffff, 0xffffffff };

const uint dram_70ns[] =
{ 0x8fffcc24, 0x0fffcc04, 0x0cffcc04, 0x00ffcc04,
  0x00ffcc00, 0x37ffcc47, 0xffffffff, 0xffffffff,
  0x8fffcc24, 0x0fffcc04, 0x0cffcc04, 0x00ffcc04,
  0x00ffcc08, 0x0cffcc44, 0x00ffec0c, 0x03ffec00,
  0x00ffec44, 0x00ffcc08, 0x0cffcc44, 0x00ffec04,
  0x00ffec00, 0x3fffec47, 0xffffffff, 0xffffffff,
  0x8fafcc24, 0x0fafcc04, 0x0cafcc00, 0x11bfcc47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x8fafcc24, 0x0fafcc04, 0x0cafcc00, 0x03afcc4c,
  0x0cafcc00, 0x03afcc4c, 0x0cafcc00, 0x03afcc4c,
  0x0cafcc00, 0x33bfcc4f, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xe0ffcc84, 0x00ffcc04, 0x00ffcc04, 0x0fffcc04,
  0x7fffcc06, 0xffffcc85, 0xffffcc05, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x33ffcc07, 0xffffffff, 0xffffffff, 0xffffffff };

const uint edo_60ns[] =
{ 0x8ffbec24, 0x0ff3ec04, 0x0cf3ec04, 0x00f3ec04,
  0x00f3ec00, 0x37f7ec47, 0xffffffff, 0xffffffff,
  0x8fffec24, 0x0ffbec04, 0x0cf3ec04, 0x00f3ec0c,
  0x0cf3ec00, 0x00f3ec4c, 0x0cf3ec00, 0x00f3ec4c,
  0x0cf3ec00, 0x00f3ec44, 0x03f3ec00, 0x3ff7ec47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x8fffcc24, 0x0fefcc04, 0x0cafcc00, 0x11bfcc47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x8fffcc24, 0x0fefcc04, 0x0cafcc00, 0x03afcc4c,
  0x0cafcc00, 0x03afcc4c, 0x0cafcc00, 0x03afcc4c,
  0x0cafcc00, 0x33bfcc4f, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xc0ffcc84, 0x00ffcc04, 0x07ffcc04, 0x3fffcc06,
  0xffffcc85, 0xffffcc05, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x33ffcc07, 0xffffffff, 0xffffffff, 0xffffffff };

const uint edo_70ns[] =
{ 0x8ffbcc24, 0x0ff3cc04, 0x0cf3cc04, 0x00f3cc04,
  0x00f3cc00, 0x37f7cc47, 0xffffffff, 0xffffffff,
  0x8fffcc24, 0x0ffbcc04, 0x0cf3cc04, 0x00f3cc0c,
  0x03f3cc00, 0x00f3cc44, 0x00f3ec0c, 0x0cf3ec00,
  0x00f3ec4c, 0x03f3ec00, 0x00f3ec44, 0x00f3cc00,
  0x33f7cc47, 0xffffffff, 0xffffffff, 0xffffffff,
  0x8fffcc24, 0x0fefcc04, 0x0cafcc00, 0x11bfcc47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x8fffcc24, 0x0fefcc04, 0x0cafcc00, 0x03afcc4c,
  0x0cafcc00, 0x03afcc4c, 0x0cafcc00, 0x03afcc4c,
  0x0cafcc00, 0x33bfcc47, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xe0ffcc84, 0x00ffcc04, 0x00ffcc04, 0x0fffcc04,
  0x7fffcc04, 0xffffcc86, 0xffffcc05, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x33ffcc07, 0xffffffff, 0xffffffff, 0xffffffff };

#elif defined(CONFIG_DRAM_25MHZ)

/* 25MHz tables */

const uint dram_60ns[] =
{ 0x0fffcc04, 0x08ffcc00, 0x33ffcc47, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fffcc24, 0x0fffcc04, 0x08ffcc00, 0x03ffcc4c,
  0x08ffcc00, 0x03ffcc4c, 0x08ffcc00, 0x03ffcc4c,
  0x08ffcc00, 0x33ffcc47, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fafcc04, 0x08afcc00, 0x3fbfcc47, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fafcc04, 0x0cafcc00, 0x01afcc4c, 0x0cafcc00,
  0x01afcc4c, 0x0cafcc00, 0x01afcc4c, 0x0cafcc00,
  0x31bfcc43, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x80ffcc84, 0x13ffcc04, 0xffffcc87, 0xffffcc05,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x33ffcc07, 0xffffffff, 0xffffffff, 0xffffffff };

const uint dram_70ns[] =
{ 0x0fffec04, 0x08ffec04, 0x00ffec00, 0x3fffcc47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fffcc24, 0x0fffcc04, 0x08ffcc00, 0x03ffcc4c,
  0x08ffcc00, 0x03ffcc4c, 0x08ffcc00, 0x03ffcc4c,
  0x08ffcc00, 0x33ffcc47, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fafcc04, 0x08afcc00, 0x3fbfcc47, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fafcc04, 0x0cafcc00, 0x01afcc4c, 0x0cafcc00,
  0x01afcc4c, 0x0cafcc00, 0x01afcc4c, 0x0cafcc00,
  0x31bfcc43, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xc0ffcc84, 0x01ffcc04, 0x7fffcc86, 0xffffcc05,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x33ffcc07, 0xffffffff, 0xffffffff, 0xffffffff };

const uint edo_60ns[] =
{ 0x0ffbcc04, 0x0cf3cc04, 0x00f3cc00, 0x33f7cc47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0ffbcc04, 0x09f3cc0c, 0x09f3cc0c, 0x09f3cc0c,
  0x08f3cc00, 0x3ff7cc47, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fefcc04, 0x08afcc04, 0x00afcc00, 0x3fbfcc47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fefcc04, 0x08afcc00, 0x07afcc48, 0x08afcc48,
  0x08afcc48, 0x39bfcc47, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x80ffcc84, 0x13ffcc04, 0xffffcc87, 0xffffcc05,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x33ffcc07, 0xffffffff, 0xffffffff, 0xffffffff };

const uint edo_70ns[] =
{ 0x0ffbcc04, 0x0cf3cc04, 0x00f3cc00, 0x33f7cc47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0ffbec04, 0x08f3ec04, 0x03f3ec48, 0x08f3cc00,
  0x0ff3cc4c, 0x08f3cc00, 0x0ff3cc4c, 0x08f3cc00,
  0x3ff7cc47, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fefcc04, 0x08afcc04, 0x00afcc00, 0x3fbfcc47,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x0fefcc04, 0x08afcc00, 0x07afcc4c, 0x08afcc00,
  0x07afcc4c, 0x08afcc00, 0x07afcc4c, 0x08afcc00,
  0x37bfcc47, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xc0ffcc84, 0x01ffcc04, 0x7fffcc86, 0xffffcc05,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x33ffcc07, 0xffffffff, 0xffffffff, 0xffffffff };


#else
#error dram not correct defined - use CONFIG_DRAM_25MHZ or CONFIG_DRAM_50MHZ
#endif

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 */

int checkboard (void)
{
	uint k;

	puts ("Board: ");

#ifdef CONFIG_FADS
	k = (*((uint *)BCSR3) >> 24) & 0x3f;

	switch(k) {
	case 0x03 :
	case 0x20 :
	case 0x21 :
	case 0x22 :
	case 0x23 :
	case 0x24 :
	case 0x2a :
	case 0x3f :
		puts ("FADS");
		break;

	default :
		printf("unknown board (0x%02x)\n", k);
		return -1;
	}

	printf(" with db ");

	switch(k) {
	case 0x03 :
		puts ("MPC823");
		break;
	case 0x20 :
		puts ("MPC801");
		break;
	case 0x21 :
		puts ("MPC850");
		break;
	case 0x22 :
		puts ("MPC821, MPC860 / MPC860SAR / MPC860T");
		break;
	case 0x23 :
		puts ("MPC860SAR");
		break;
	case 0x24 :
		puts ("MPC860T");
		break;
	case 0x3f :
		puts ("MPC850SAR");
		break;
	}

	printf(" rev ");

	k = (((*((uint *)BCSR3) >> 23) & 1) << 3)
	  | (((*((uint *)BCSR3) >> 19) & 1) << 2)
	  | (((*((uint *)BCSR3) >> 16) & 3));

	switch(k) {
	case 0x01 :
		puts ("ENG or PILOT\n");
		break;

	default:
		printf("unknown (0x%x)\n", k);
		return -1;
	}

	return 0;
#endif	/* CONFIG_FADS */

#ifdef CONFIG_ADS
  printf("ADS rev ");

  k = (((*((uint *)BCSR3) >> 23) & 1) << 3)
    | (((*((uint *)BCSR3) >> 19) & 1) << 2)
    | (((*((uint *)BCSR3) >> 16) & 3));

  switch(k) {
  case 0x00 : puts ("ENG - this board sucks, check the errata, not supported\n");
              return -1;
  case 0x01 : puts ("PILOT - warning, read errata \n"); break;
  case 0x02 : puts ("A - warning, read errata \n"); break;
  case 0x03 : puts ("B \n"); break;
  default   : printf ("unknown revision (0x%x)\n", k); return -1;
  }

  return 0;
#endif	/* CONFIG_ADS */

}

/* ------------------------------------------------------------------------- */
int _draminit(uint base, uint noMbytes, uint edo, uint delay)
{
	volatile immap_t     *immap = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	/* init upm */

	switch(delay)
	{
		case 70:
		{
			if(edo)
			{
				upmconfig(UPMA, (uint *) edo_70ns, sizeof(edo_70ns)/sizeof(uint));
			}
			else
			{
				upmconfig(UPMA, (uint *) dram_70ns, sizeof(dram_70ns)/sizeof(uint));
			}

			break;
		}

		case 60:
		{
			if(edo)
			{
				upmconfig(UPMA, (uint *) edo_60ns, sizeof(edo_60ns)/sizeof(uint));
			}
			else
			{
				upmconfig(UPMA, (uint *) dram_60ns, sizeof(dram_60ns)/sizeof(uint));
			}

			break;
		}

		default :
			return -1;
	}

	memctl->memc_mptpr = 0x0400; /* divide by 16 */

	switch(noMbytes)
	{

		case 8:  /* 8 Mbyte uses both CS3 and CS2 */
		{
			memctl->memc_mamr = 0x13a01114;
			memctl->memc_or3 = 0xffc00800;
			memctl->memc_br3 = 0x00400081 + base;
			memctl->memc_or2 = 0xffc00800;
			break;
		}

		case 4: /* 4 Mbyte uses only CS2 */
		{
			memctl->memc_mamr = 0x13a01114;
			memctl->memc_or2 = 0xffc00800;
			break;
		}

		case 32: /* 32 Mbyte uses both CS3 and CS2 */
		{
			memctl->memc_mamr = 0x13b01114;
			memctl->memc_or3 = 0xff000800;
			memctl->memc_br3 = 0x01000081 + base;
			memctl->memc_or2 = 0xff000800;
			break;
		}

		case 16: /* 16 Mbyte uses only CS2 */
		{
#ifdef CONFIG_ADS
			memctl->memc_mamr = 0x60b21114;
#else
			memctl->memc_mamr = 0x13b01114;
#endif
			memctl->memc_or2 = 0xff000800;
			break;
		}

		default:
		    return -1;
	}

	memctl->memc_br2 = 0x81 + base;     /* use upma */
	return 0;
}

/* ------------------------------------------------------------------------- */

void _dramdisable(void)
{
	volatile immap_t     *immap = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_br2 = 0x00000000;
	memctl->memc_br3 = 0x00000000;

	/* maybe we should turn off upma here or something */
}

#if defined(CONFIG_SDRAM_100MHZ)

/* ------------------------------------------------------------------------- */
/* sdram table by Dan Malek                                                  */

/* This has the stretched early timing so the 50 MHz
 * processor can make the 100 MHz timing.  This will
 * work at all processor speeds.
 */

#define SDRAM_MPTPRVALUE 0x0400

#define SDRAM_MBMRVALUE0 0xc3802114  /* (16-14) 50 MHz */
#define SDRAM_MBMRVALUE1 SDRAM_MBMRVALUE0

#define SDRAM_OR4VALUE   0xffc00a00
#define SDRAM_BR4VALUE   0x000000c1   /* base address will be or:ed on */

#define SDRAM_MARVALUE   0x88

#define SDRAM_MCRVALUE0  0x80808111   /* run pattern 0x11 */
#define SDRAM_MCRVALUE1  SDRAM_MCRVALUE0


const uint sdram_table[] =
{
	/* single read. (offset 0 in upm RAM) */
	0xefebfc24, 0x1f07fc24, 0xeeaefc04, 0x11adfc04,
	0xefbbbc00, 0x1ff77c45, 0xffffffff, 0xffffffff,

	/* burst read. (offset 8 in upm RAM) */
	0xefebfc24, 0x1f07fc24, 0xeeaefc04, 0x10adfc04,
	0xf0affc00, 0xf0affc00, 0xf1affc00, 0xefbbbc00,
	0x1ff77c45, 0xeffbbc04, 0x1ff77c34, 0xefeabc34,
	0x1fb57c35, 0xffffffff, 0xffffffff, 0xffffffff,

	/* single write. (offset 18 in upm RAM) */
	0xefebfc24, 0x1f07fc24, 0xeeaebc00, 0x01b93c04,
	0x1ff77c45, 0xffffffff, 0xffffffff, 0xffffffff,

	/* burst write. (offset 20 in upm RAM) */
	0xefebfc24, 0x1f07fc24, 0xeeaebc00, 0x10ad7c00,
	0xf0affc00, 0xf0affc00, 0xe1bbbc04, 0x1ff77c45,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,

	/* refresh. (offset 30 in upm RAM) */
	0xeffafc84, 0x1ff5fc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,

	/* exception. (offset 3c in upm RAM) */
	0xeffffc06, 0x1ffffc07, 0xffffffff, 0xffffffff };

#elif defined(CONFIG_SDRAM_50MHZ)

/* ------------------------------------------------------------------------- */
/* sdram table stolen from the fads manual                                   */
/* for chip MB811171622A-100                                                 */

/* this table is for 32-50MHz operation */

#define _not_used_ 0xffffffff

#define SDRAM_MPTPRVALUE 0x0400

#define SDRAM_MBMRVALUE0 0x80802114   /* refresh at 32MHz */
#define SDRAM_MBMRVALUE1 0x80802118

#define SDRAM_OR4VALUE   0xffc00a00
#define SDRAM_BR4VALUE   0x000000c1   /* base address will be or:ed on */

#define SDRAM_MARVALUE   0x88

#define SDRAM_MCRVALUE0  0x80808105
#define SDRAM_MCRVALUE1  0x80808130

const uint sdram_table[] =
{
	/* single read. (offset 0 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x11adfc04, 0xefbbbc00,
	0x1ff77c47,

	/* MRS initialization (offset 5) */

	0x1ff77c34, 0xefeabc34, 0x1fb57c35,

	/* burst read. (offset 8 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x10adfc04, 0xf0affc00,
	0xf0affc00, 0xf1affc00, 0xefbbbc00, 0x1ff77c47,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* single write. (offset 18 in upm RAM) */
	0x1f27fc04, 0xeeaebc00, 0x01b93c04, 0x1ff77c47,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* burst write. (offset 20 in upm RAM) */
	0x1f07fc04, 0xeeaebc00, 0x10ad7c00, 0xf0affc00,
	0xf0affc00, 0xe1bbbc04, 0x1ff77c47, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* refresh. (offset 30 in upm RAM) */
	0x1ff5fc84, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* exception. (offset 3c in upm RAM) */
	0x7ffffc07, _not_used_, _not_used_, _not_used_ };

/* ------------------------------------------------------------------------- */
#else
#error SDRAM not correctly configured
#endif

int _initsdram(uint base, uint noMbytes)
{
	volatile immap_t     *immap = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	if(noMbytes != 4)
	{
		return -1;
	}

	upmconfig(UPMB, (uint *)sdram_table,sizeof(sdram_table)/sizeof(uint));

	memctl->memc_mptpr = SDRAM_MPTPRVALUE;

	/* Configure the refresh (mostly).  This needs to be
	* based upon processor clock speed and optimized to provide
	* the highest level of performance.  For multiple banks,
	* this time has to be divided by the number of banks.
	* Although it is not clear anywhere, it appears the
	* refresh steps through the chip selects for this UPM
	* on each refresh cycle.
	* We have to be careful changing
	* UPM registers after we ask it to run these commands.
	*/

	memctl->memc_mbmr = SDRAM_MBMRVALUE0;
	memctl->memc_mar = SDRAM_MARVALUE;  /* MRS code */

	udelay(200);

	/* Now run the precharge/nop/mrs commands.
	*/

	memctl->memc_mcr = 0x80808111;   /* run pattern 0x11 */

	udelay(200);

	/* Run 8 refresh cycles */

	memctl->memc_mcr = SDRAM_MCRVALUE0;

	udelay(200);

	memctl->memc_mbmr = SDRAM_MBMRVALUE1;
	memctl->memc_mcr = SDRAM_MCRVALUE1;

	udelay(200);

	memctl->memc_mbmr = SDRAM_MBMRVALUE0;

	memctl->memc_or4 = SDRAM_OR4VALUE;
	memctl->memc_br4 = SDRAM_BR4VALUE | base;

	return 0;
}

/* ------------------------------------------------------------------------- */

void _sdramdisable(void)
{
	volatile immap_t     *immap = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_br4 = 0x00000000;

	/* maybe we should turn off upmb here or something */
}

/* ------------------------------------------------------------------------- */

int initsdram(uint base, uint *noMbytes)
{
	uint m = 4;

	*((uint *)BCSR1) |= BCSR1_SDRAM_EN; /* enable sdram */
								  /* _fads_sdraminit needs access to sdram */
	*noMbytes = m;

	if(!_initsdram(base, m))
	{

		return 0;
	}
	else
	{
		*((uint *)BCSR1) &= ~BCSR1_SDRAM_EN; /* disable sdram */

		_sdramdisable();

		return -1;
	}
}

long int initdram (int board_type)
{
#ifdef CONFIG_ADS
	/* ADS: has no SDRAM, so start DRAM at 0 */
	uint base = (unsigned long)0x0;
#else
	/* FADS: has 4MB SDRAM, put DRAM above it */
	uint base = (unsigned long)0x00400000;
#endif
	uint k, m, s;

	k = (*((uint *)BCSR2) >> 23) & 0x0f;

	m = 0;

	switch(k & 0x3)
	{
		/* "MCM36100 / MT8D132X" */
		case 0x00 :
			m = 4;
			break;

		/* "MCM36800 / MT16D832X" */
		case 0x01 :
			m = 32;
			break;
		/* "MCM36400 / MT8D432X" */
		case 0x02 :
			m = 16;
			break;
		/* "MCM36200 / MT16D832X ?" */
		case 0x03 :
			m = 8;
			break;

	}

	switch(k >> 2)
	{
		case 0x02 :
			k = 70;
			break;

		case 0x03 :
			k = 60;
			break;

		default :
			printf("unknown dramdelay (0x%x) - defaulting to 70 ns", k);
			k = 70;
	}

#ifdef CONFIG_FADS
	/* the FADS is missing this bit, all rams treated as non-edo */
	s = 0;
#else
	s = (*((uint *)BCSR2) >> 27) & 0x01;
#endif

	if(!_draminit(base, m, s, k))
	{
#ifdef CONFIG_FADS
		uint	sdramsz;
#endif
		*((uint *)BCSR1) &= ~BCSR1_DRAM_EN;  /* enable dram */

#ifdef CONFIG_FADS
		if (!initsdram(0x00000000, &sdramsz)) {
				m += sdramsz;
				printf("(%u MB SDRAM) ", sdramsz);
		} else {
				_dramdisable();

				/********************************
				*DRAM ERROR, HALT PROCESSOR
				*********************************/
				while(1);

				return -1;
		}
#endif

		return (m << 20);
	}
	else
	{
		_dramdisable();

		/********************************
		*DRAM ERROR, HALT PROCESSOR
		*********************************/
		while(1);

		return -1;
	}
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
    /* TODO: XXX XXX XXX */
    printf ("test: 16 MB - ok\n");

    return (0);
}


#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA)

#ifdef CFG_PCMCIA_MEM_ADDR
volatile unsigned char *pcmcia_mem = (unsigned char*)CFG_PCMCIA_MEM_ADDR;
#endif

int pcmcia_init(void)
{
	volatile pcmconf8xx_t	*pcmp;
	uint v, slota, slotb;

	/*
	** Enable the PCMCIA for a Flash card.
	*/
	pcmp = (pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));

#if 0
	pcmp->pcmc_pbr0 = CFG_PCMCIA_MEM_ADDR;
	pcmp->pcmc_por0 = 0xc00ff05d;
#endif

	/* Set all slots to zero by default. */
	pcmp->pcmc_pgcra = 0;
	pcmp->pcmc_pgcrb = 0;
#ifdef PCMCIA_SLOT_A
	pcmp->pcmc_pgcra = 0x40;
#endif
#ifdef PCMCIA_SLOT_B
	pcmp->pcmc_pgcrb = 0x40;
#endif

	/* enable PCMCIA buffers */
	*((uint *)BCSR1) &= ~BCSR1_PCCEN;

	/* Check if any PCMCIA card is plugged in. */

	slota = (pcmp->pcmc_pipr & 0x18000000) == 0 ;
	slotb = (pcmp->pcmc_pipr & 0x00001800) == 0 ;

	if (!(slota || slotb))
	{
		printf("No card present\n");
#ifdef PCMCIA_SLOT_A
		pcmp->pcmc_pgcra = 0;
#endif
#ifdef PCMCIA_SLOT_B
		pcmp->pcmc_pgcrb = 0;
#endif
		return -1;
	}
	else
		printf("Card present (");

	v = 0;

	/* both the ADS and the FADS have a 5V keyed pcmcia connector (?)
	**
	** Paolo - Yes, but i have to insert some 3.3V card in that slot on
	**	   my FADS... :-)
	*/

#if defined(CONFIG_MPC860)
	switch( (pcmp->pcmc_pipr >> 30) & 3 )
#elif defined(CONFIG_MPC823) || defined(CONFIG_MPC850)
	switch( (pcmp->pcmc_pipr >> 14) & 3 )
#endif
	{
		case 0x00 :
			printf("5V");
			v = 5;
			break;
		case 0x01 :
			printf("5V and 3V");
#ifdef CONFIG_FADS
			v = 3; /* User lower voltage if supported! */
#else
			v = 5;
#endif
			break;
		case 0x03 :
			printf("5V, 3V and x.xV");
#ifdef CONFIG_FADS
			v = 3; /* User lower voltage if supported! */
#else
			v = 5;
#endif
			break;
	}

	switch(v){
#ifdef CONFIG_FADS
	case 3:
	    printf("; using 3V");
	    /*
	    ** Enable 3 volt Vcc.
	    */
	    *((uint *)BCSR1) &= ~BCSR1_PCCVCC1;
	    *((uint *)BCSR1) |= BCSR1_PCCVCC0;
	    break;
#endif
	case 5:
	    printf("; using 5V");
#ifdef CONFIG_ADS
	    /*
	    ** Enable 5 volt Vcc.
	    */
	    *((uint *)BCSR1) &= ~BCSR1_PCCVCCON;
#endif
#ifdef CONFIG_FADS
	    /*
	    ** Enable 5 volt Vcc.
	    */
	    *((uint *)BCSR1) &= ~BCSR1_PCCVCC0;
	    *((uint *)BCSR1) |= BCSR1_PCCVCC1;
#endif
	    break;

	default:
		*((uint *)BCSR1) |= BCSR1_PCCEN;  /* disable pcmcia */

		printf("; unknown voltage");
		return -1;
	}
	printf(")\n");
	/* disable pcmcia reset after a while */

	udelay(20);

#ifdef MPC860
	pcmp->pcmc_pgcra = 0;
#elif MPC823
	pcmp->pcmc_pgcrb = 0;
#endif

	/* If you using a real hd you should give a short
	* spin-up time. */
#ifdef CONFIG_DISK_SPINUP_TIME
	udelay(CONFIG_DISK_SPINUP_TIME);
#endif

	return 0;
}

#endif	/* CFG_CMD_PCMCIA */

/* ------------------------------------------------------------------------- */

#ifdef CFG_PC_IDE_RESET

void ide_set_reset(int on)
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;

	/*
	 * Configure PC for IDE Reset Pin
	 */
	if (on) {		/* assert RESET */
		immr->im_ioport.iop_pcdat &= ~(CFG_PC_IDE_RESET);
	} else {		/* release RESET */
		immr->im_ioport.iop_pcdat |=   CFG_PC_IDE_RESET;
	}

	/* program port pin as GPIO output */
	immr->im_ioport.iop_pcpar &= ~(CFG_PC_IDE_RESET);
	immr->im_ioport.iop_pcso  &= ~(CFG_PC_IDE_RESET);
	immr->im_ioport.iop_pcdir |=   CFG_PC_IDE_RESET;
}

#endif	/* CFG_PC_IDE_RESET */
/* ------------------------------------------------------------------------- */
