/*
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same PLL and clock machinery inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>
#ifdef CONFIG_S3C24X0

#include <asm/io.h>
#include <asm/arch/s3c24x0_cpu.h>

#define MPLL 0
#define UPLL 1

/* ------------------------------------------------------------------------- */
/* NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency of the PLL.
 *
 * get_FCLK(), get_HCLK(), get_PCLK() and get_UCLK() return the clock of
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */

static ulong get_PLLCLK(int pllreg)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	ulong r, m, p, s;

	if (pllreg == MPLL)
		r = readl(&clk_power->mpllcon);
	else if (pllreg == UPLL)
		r = readl(&clk_power->upllcon);
	else
		hang();

	m = ((r & 0xFF000) >> 12) + 8;
	p = ((r & 0x003F0) >> 4) + 2;
	s = r & 0x3;

#if defined(CONFIG_S3C2440)
	if (pllreg == MPLL)
		return 2 * m * (CONFIG_SYS_CLK_FREQ / (p << s));
#endif
	return (CONFIG_SYS_CLK_FREQ * m) / (p << s);

}

/* return FCLK frequency */
ulong get_FCLK(void)
{
	return get_PLLCLK(MPLL);
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
#ifdef CONFIG_S3C2440
	switch (readl(&clk_power->clkdivn) & 0x6) {
	default:
	case 0:
		return get_FCLK();
	case 2:
		return get_FCLK() / 2;
	case 4:
		return (readl(&clk_power->camdivn) & (1 << 9)) ?
			get_FCLK() / 8 : get_FCLK() / 4;
	case 6:
		return (readl(&clk_power->camdivn) & (1 << 8)) ?
			get_FCLK() / 6 : get_FCLK() / 3;
	}
#else
	return (readl(&clk_power->clkdivn) & 2) ? get_FCLK() / 2 : get_FCLK();
#endif
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();

	return (readl(&clk_power->clkdivn) & 1) ? get_HCLK() / 2 : get_HCLK();
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
	return get_PLLCLK(UPLL);
}

#endif /* CONFIG_S3C24X0 */
