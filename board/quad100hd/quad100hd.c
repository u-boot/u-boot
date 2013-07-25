/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 *
 * Based in part on board/icecube/icecube.c from PPCBoot
 * (C) Copyright 2003 Intrinsyc Software
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <environment.h>
#include <logbuff.h>
#include <post.h>

#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* taken from PPCBoot */
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(UIC0ER, 0x00000000);	/* disable all ints */
	mtdcr(UIC0CR, 0x00000000);
	mtdcr(UIC0PR, 0xFFFF7FFE);	/* set int polarities */
	mtdcr(UIC0TR, 0x00000000);	/* set int trigger levels */
	mtdcr(UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr(UIC0VCR, 0x00000001);	/* set vect base=0,INT0 highest priority */

	mtdcr(CPC0_SRR, 0x00040000);   /* Hold PCI bridge in reset */

	return 0;
}

/*
 * Check Board Identity:
 */
int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));
#ifdef DISPLAY_BOARD_INFO
	sys_info_t sysinfo;
#endif

	puts("Board: Quad100hd");

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

#ifdef DISPLAY_BOARD_INFO
	/* taken from ppcboot */
	get_sys_info(&sysinfo);

	printf("\tVCO: %lu MHz\n", sysinfo.freqVCOMhz);
	printf("\tCPU: %lu MHz\n", sysinfo.freqProcessor / 1000000);
	printf("\tPLB: %lu MHz\n", sysinfo.freqPLB / 1000000);
	printf("\tOPB: %lu MHz\n", sysinfo.freqOPB / 1000000);
	printf("\tEPB: %lu MHz\n", sysinfo.freqPLB / (sysinfo.pllExtBusDiv *
		1000000));
	printf("\tPCI: %lu MHz\n", sysinfo.freqPCI / 1000000);
#endif

	return 0;
}
