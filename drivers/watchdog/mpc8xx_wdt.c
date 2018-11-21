// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 CS Systemes d'Information
 */

#include <common.h>
#include <mpc8xx.h>
#include <asm/cpm_8xx.h>
#include <asm/io.h>

void hw_watchdog_reset(void)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;

	out_be16(&immap->im_siu_conf.sc_swsr, 0x556c);	/* write magic1 */
	out_be16(&immap->im_siu_conf.sc_swsr, 0xaa39);	/* write magic2 */
}

