/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/alt.h
 *     This file is alt board configuration.
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
 */

#ifndef __ALT_H
#define __ALT_H

#include "rcar-gen2-common.h"

#define STACK_AREA_SIZE			0x00100000
#define LOW_LEVEL_MERAM_STACK \
		(SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define RCAR_GEN2_SDRAM_BASE		0x40000000
#define RCAR_GEN2_SDRAM_SIZE		(1024u * 1024 * 1024)
#define RCAR_GEN2_UBOOT_SDRAM_SIZE	(512 * 1024 * 1024)

#define CFG_EXTRA_ENV_SETTINGS	\
	"bootm_size=0x10000000\0"	\
	"usb_pgood_delay=2000\0"

#endif /* __ALT_H */
