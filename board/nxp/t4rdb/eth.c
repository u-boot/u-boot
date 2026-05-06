// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * Chunhe Lan <Chunhe.Lan@freescale.com>
 */

#include <config.h>
#include <command.h>
#include <fdt_support.h>
#include <net.h>
#include <netdev.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <fsl_ddr_sdram.h>
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

#include "../common/fman.h"
#include "t4rdb.h"

void fdt_fixup_board_enet(void *fdt)
{
	return;
}
