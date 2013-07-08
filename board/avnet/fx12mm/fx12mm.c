/*
 * (C) Copyright 2008
 *
 * Author: Xilinx Inc.
 *
 * Modified by:
 *  Georg Schardt <schardt@team-ctech.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <asm/processor.h>

int checkboard(void)
{
	char buf[64];
	int i;
	int l = getenv_f("serial#", buf, sizeof(buf));

	if (l < 0) {
		printf("Avnet Virtex4 FX12 with no serial #");
	} else {
		printf("Avnet Virtex4 FX12 Minimodul # ");
		for (i = 0; i < l; ++i) {
			if (buf[i] == ' ')
				break;
			putc(buf[i]);
		}
	}
	putc('\n');
	return 0;
}
