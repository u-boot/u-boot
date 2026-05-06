// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 *
 * Shengzhou Liu <Shengzhou.Liu@freescale.com>
 */

#include <config.h>
#include <command.h>
#include <fdt_support.h>
#include <log.h>
#include <net.h>
#include <netdev.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fsl_dtsec.h>
#include <asm/fsl_serdes.h>
#include <hwconfig.h>
#include "../common/qixis.h"
#include "../common/fman.h"
#include "t208xqds_qixis.h"
#include <linux/libfdt.h>

void fdt_fixup_board_enet(void *fdt)
{
	return;
}
