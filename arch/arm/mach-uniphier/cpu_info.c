/*
 * Copyright (C) 2013-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>

#include "sg-regs.h"

int print_cpuinfo(void)
{
	u32 revision, type, model, rev, required_model = 1, required_rev = 1;

	revision = readl(SG_REVISION);
	type = (revision & SG_REVISION_TYPE_MASK) >> SG_REVISION_TYPE_SHIFT;
	model = (revision & SG_REVISION_MODEL_MASK) >> SG_REVISION_MODEL_SHIFT;
	rev = (revision & SG_REVISION_REV_MASK) >> SG_REVISION_REV_SHIFT;

	puts("CPU:   ");

	switch (type) {
	case 0x25:
		puts("PH1-sLD3 (MN2WS0220)");
		required_model = 2;
		break;
	case 0x26:
		puts("PH1-LD4 (MN2WS0250)");
		required_rev = 2;
		break;
	case 0x28:
		puts("PH1-Pro4 (MN2WS0230)");
		break;
	case 0x29:
		puts("PH1-sLD8 (MN2WS0270)");
		break;
	case 0x2A:
		puts("PH1-Pro5 (MN2WS0300)");
		break;
	case 0x2E:
		puts("ProXstream2 (MN2WS0310)");
		break;
	case 0x2F:
		puts("PH1-LD6b (MN2WS0320)");
		break;
	case 0x31:
		puts("PH1-LD11 ()");
		break;
	case 0x32:
		puts("PH1-LD20 ()");
		break;
	default:
		printf("Unknown Processor ID (0x%x)\n", revision);
		return -1;
	}

	printf(" model %d", model);

	printf(" (rev. %d)\n", rev);

	if (model < required_model) {
		printf("Sorry, this model is not supported.\n");
		printf("Required model is %d.", required_model);
		return -1;
	} else if (rev < required_rev) {
		printf("Sorry, this revision is not supported.\n");
		printf("Required revision is %d.", required_rev);
		return -1;
	}

	return 0;
}
