/*
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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

/*
 * This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same PLL and clock machinery inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>
#include <asm/arch/s3c6400.h>

#define APLL 0
#define MPLL 1
#define EPLL 2

/* ------------------------------------------------------------------------- */
/*
 * NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency of the PLL.
 *
 * get_FCLK(), get_HCLK(), get_PCLK() and get_UCLK() return the clock of
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */

static ulong get_PLLCLK(int pllreg)
{
	ulong r, m, p, s;

	switch (pllreg) {
	case APLL:
		r = APLL_CON_REG;
		break;
	case MPLL:
		r = MPLL_CON_REG;
		break;
	case EPLL:
		r = EPLL_CON0_REG;
		break;
	default:
		hang();
	}

	m = (r >> 16) & 0x3ff;
	p = (r >> 8) & 0x3f;
	s = r & 0x7;

	return m * (CONFIG_SYS_CLK_FREQ / (p * (1 << s)));
}

/* return ARMCORE frequency */
ulong get_ARMCLK(void)
{
	ulong div;

	div = CLK_DIV0_REG;

	return get_PLLCLK(APLL) / ((div & 0x7) + 1);
}

/* return FCLK frequency */
ulong get_FCLK(void)
{
	return get_PLLCLK(APLL);
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
	ulong fclk;

	uint hclkx2_div = ((CLK_DIV0_REG >> 9) & 0x7) + 1;
	uint hclk_div = ((CLK_DIV0_REG >> 8) & 0x1) + 1;

	/*
	 * Bit 7 exists on s3c6410, and not on s3c6400, it is reserved on
	 * s3c6400 and is always 0, and it is indeed running in ASYNC mode
	 */
	if (OTHERS_REG & 0x80)
		fclk = get_FCLK();		/* SYNC Mode	*/
	else
		fclk = get_PLLCLK(MPLL);	/* ASYNC Mode	*/

	return fclk / (hclk_div * hclkx2_div);
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
	ulong fclk;
	uint hclkx2_div = ((CLK_DIV0_REG >> 9) & 0x7) + 1;
	uint pre_div = ((CLK_DIV0_REG >> 12) & 0xf) + 1;

	if (OTHERS_REG & 0x80)
		fclk = get_FCLK();		/* SYNC Mode	*/
	else
		fclk = get_PLLCLK(MPLL);	/* ASYNC Mode	*/

	return fclk / (hclkx2_div * pre_div);
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
	return get_PLLCLK(EPLL);
}

int print_cpuinfo(void)
{
	printf("\nCPU:     S3C6400@%luMHz\n", get_ARMCLK() / 1000000);
	printf("         Fclk = %luMHz, Hclk = %luMHz, Pclk = %luMHz ",
	       get_FCLK() / 1000000, get_HCLK() / 1000000,
	       get_PCLK() / 1000000);

	if (OTHERS_REG & 0x80)
		printf("(SYNC Mode) \n");
	else
		printf("(ASYNC Mode) \n");
	return 0;
}
