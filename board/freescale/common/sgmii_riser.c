/*
 * Freescale SGMII Riser Card
 *
 * This driver supports the SGMII Riser card found on the
 * "DS" style of development board from Freescale.
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 */

#include <config.h>
#include <common.h>
#include <tsec.h>

void fsl_sgmii_riser_init(struct tsec_info_struct *tsec_info, int num)
{
	int i;

	for (i = 0; i < num; i++)
		if (tsec_info[i].flags & TSEC_SGMII)
			tsec_info[i].phyaddr += SGMII_RISER_PHY_OFFSET;
}
