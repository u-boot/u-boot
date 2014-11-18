/*
 * (C) Copyright 2014
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/stv0991_wdru.h>
void reset_cpu(ulong ignored)
{
	puts("System is going to reboot ...\n");
	/*
	 * This 1 second delay will allow the above message
	 * to be printed before reset
	 */
	udelay((1000 * 1000));

	/* Setting bit 1 of the WDRU unit will reset the SoC */
	writel(WDRU_RST_SYS, &stv0991_wd_ru_ptr->wdru_ctrl1);

	/* system will restart */
	while (1)
		;
}
