/*
 * sdh.h, export bfin_mmc_init
 *
 * Copyright (c) 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __ASM_SDH_H__
#define __ASM_SDH_H__

#include <mmc.h>
#include <asm/u-boot.h>

int bfin_mmc_init(bd_t *bis);

#endif
