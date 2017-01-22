/*
 * Copyright (C) 2013-2014 Panasonic Corporation
 * Copyright (C) 2015-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/errno.h>
#include <linux/io.h>

#include "soc-info.h"

int print_cpuinfo(void)
{
	unsigned int id, model, rev, required_model = 1, required_rev = 1;

	id = uniphier_get_soc_id();
	model = uniphier_get_soc_model();
	rev = uniphier_get_soc_revision();

	puts("CPU:   ");

	switch (id) {
	case UNIPHIER_SLD3_ID:
		puts("sLD3 (MN2WS0220)");
		required_model = 2;
		break;
	case UNIPHIER_LD4_ID:
		puts("LD4 (MN2WS0250)");
		required_rev = 2;
		break;
	case UNIPHIER_PRO4_ID:
		puts("Pro4 (MN2WS0230)");
		break;
	case UNIPHIER_SLD8_ID:
		puts("sLD8 (MN2WS0270)");
		break;
	case UNIPHIER_PRO5_ID:
		puts("Pro5 (MN2WS0300)");
		break;
	case UNIPHIER_PXS2_ID:
		puts("PXs2 (MN2WS0310)");
		break;
	case UNIPHIER_LD6B_ID:
		puts("LD6b (MN2WS0320)");
		break;
	case UNIPHIER_LD11_ID:
		puts("LD11 (SC1405AP1)");
		break;
	case UNIPHIER_LD20_ID:
		puts("LD20 (SC1401AJ1)");
		break;
	case UNIPHIER_PXS3_ID:
		puts("PXs3");
		break;
	default:
		printf("Unknown Processor ID (0x%x)\n", id);
		return -ENOTSUPP;
	}

	printf(" model %d (revision %d)\n", model, rev);

	if (model < required_model) {
		printf("Only model %d or newer is supported.\n",
		       required_model);
		return -ENOTSUPP;
	} else if (rev < required_rev) {
		printf("Only revision %d or newer is supported.\n",
		       required_rev);
		return -ENOTSUPP;
	}

	return 0;
}
