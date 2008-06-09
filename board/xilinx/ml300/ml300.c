/*
 * ml300.c: U-Boot platform support for Xilinx ML300 board
 *
 *     Author: Xilinx, Inc.
 *
 *
 *     This program is free software; you can redistribute it and/or modify it
 *     under the terms of the GNU General Public License as published by the
 *     Free Software Foundation; either version 2 of the License, or (at your
 *     option) any later version.
 *
 *
 *     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 *     COURTESY TO YOU. BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 *     ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD,
 *     XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
 *     FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING
 *     ANY THIRD PARTY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 *     XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 *     THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY
 *     WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM
 *     CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *     FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 *     Xilinx hardware products are not intended for use in life support
 *     appliances, devices, or systems. Use in such applications is
 *     expressly prohibited.
 *
 *
 *     (c) Copyright 2002-2004 Xilinx Inc.
 *     All rights reserved.
 *
 *
 *     You should have received a copy of the GNU General Public License along
 *     with this program; if not, write to the Free Software Foundation, Inc.,
 *     675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <config.h>
#include <common.h>
#include <asm/processor.h>

#ifdef CFG_ENV_IS_IN_EEPROM
extern void convert_env(void);
#endif

int
board_pre_init(void)
{
	return 0;
}

int
checkboard(void)
{
	char tmp[64];		/* long enough for environment variables */
	char *s, *e;
	int i = getenv_r("L", tmp, sizeof (tmp));

	if (i < 0) {
		printf("### No HW ID - assuming ML300");
	} else {
		for (e = tmp; *e; ++e) {
			if (*e == ' ')
				break;
		}

		printf("### Board Serial# is ");

		for (s = tmp; s < e; ++s) {
			putc(*s);
		}

	}
	putc('\n');

	return (0);
}

phys_size_t
initdram(int board_type)
{
	return 128 * 1024 * 1024;
}

int
testdram(void)
{
	printf("test: xxx MB - ok\n");

	return (0);
}

/* implement functions originally in cpu/ppc4xx/speed.c */
void
get_sys_info(sys_info_t * sysInfo)
{
	sysInfo->freqProcessor = XPAR_CORE_CLOCK_FREQ_HZ;

	/* only correct if the PLB and OPB run at the same frequency */
	sysInfo->freqPLB = XPAR_UARTNS550_0_CLOCK_FREQ_HZ;
	sysInfo->freqPCI = XPAR_UARTNS550_0_CLOCK_FREQ_HZ / 3;
}

ulong
get_PCI_freq(void)
{
	ulong val;
	PPC4xx_SYS_INFO sys_info;

	get_sys_info(&sys_info);
	val = sys_info.freqPCI;
	return val;
}

#ifdef CONFIG_MISC_INIT_R

int
misc_init_r()
{
	/* convert env name and value to u-boot standard */
	convert_env();
	return 0;
}

#endif
