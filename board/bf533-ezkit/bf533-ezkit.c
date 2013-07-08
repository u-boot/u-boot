/*
 * U-boot - main board file
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include "psd4256.h"
#include "flash-defines.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: ADI BF533 EZ-Kit Lite board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

/* miscellaneous platform dependent initialisations */
int misc_init_r(void)
{
	/* Set direction bits for Video en/decoder reset as output      */
	*(volatile unsigned char *)(CONFIG_SYS_FLASH1_BASE + PSD_PORTA_DIR) =
	    PSDA_VDEC_RST | PSDA_VENC_RST;
	/* Deactivate Video en/decoder reset lines                      */
	*(volatile unsigned char *)(CONFIG_SYS_FLASH1_BASE + PSD_PORTA_DOUT) =
	    PSDA_VDEC_RST | PSDA_VENC_RST;

	return 0;
}

#ifdef CONFIG_SMC91111
int board_eth_init(bd_t *bis)
{
	return smc91111_initialize(0, CONFIG_SMC91111_BASE);
}
#endif
