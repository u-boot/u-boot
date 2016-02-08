/*
 * U-Boot - main board file
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <asm/blackfin.h>
#include <net.h>
#include <asm/mach-common/bits/bootrom.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: ADI BF537 stamp board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

#ifdef CONFIG_BFIN_MAC
static void board_init_enetaddr(uchar *mac_addr)
{
#ifndef CONFIG_SYS_NO_FLASH
	/* we cram the MAC in the last flash sector */
	uchar *board_mac_addr = (uchar *)0x203F0000;
	if (is_valid_ethaddr(board_mac_addr)) {
		memcpy(mac_addr, board_mac_addr, 6);
		eth_setenv_enetaddr("ethaddr", mac_addr);
	}
#endif
}

int board_eth_init(bd_t *bis)
{
	return bfin_EMAC_initialize(bis);
}
#endif

/* miscellaneous platform dependent initialisations */
int misc_init_r(void)
{
#ifdef CONFIG_BFIN_MAC
	uchar enetaddr[6];
	if (!eth_getenv_enetaddr("ethaddr", enetaddr))
		board_init_enetaddr(enetaddr);
#endif

#ifndef CONFIG_SYS_NO_FLASH
	/* we use the last sector for the MAC address / POST LDR */
	extern flash_info_t flash_info[];
	flash_protect(FLAG_PROTECT_SET, 0x203F0000, 0x203FFFFF, &flash_info[0]);
#endif

#ifdef CONFIG_BFIN_IDE
	cf_ide_init();
#endif

	return 0;
}
