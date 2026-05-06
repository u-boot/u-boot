// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * Shengzhou Liu <Shengzhou.Liu@freescale.com>
 */

#include <config.h>
#include <command.h>
#include <fdt_support.h>
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
#include "../common/fman.h"

void fdt_fixup_board_enet(void *fdt)
{
}
