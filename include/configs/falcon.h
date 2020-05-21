/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/falcon.h
 *     This file is Falcon board configuration.
 *
 * Copyright (C) 2020 Renesas Electronics Corp.
 */

#ifndef __FALCON_H
#define __FALCON_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Board Clock */
/* XTAL_CLK : 16.66MHz */
#define CONFIG_SYS_CLK_FREQ	16666666u

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

#endif /* __FALCON_H */
