/*
 * U-boot - main board file
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <net.h>
#include <netdev.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/otp.h>
#include <asm/sdh.h>

int checkboard(void)
{
	printf("Board: Bluetechnix TCM-BF518 board\n");
	printf("       Support: http://www.bluetechnix.com/\n");
	printf("                http://blackfin.uclinux.org/\n");
	return 0;
}

#if defined(CONFIG_BFIN_MAC)
int board_eth_init(bd_t *bis)
{
	return bfin_EMAC_initialize(bis);
}
#endif

#ifdef CONFIG_BFIN_SDH
int board_mmc_init(bd_t *bis)
{
	return bfin_mmc_init(bis);
}
#endif
