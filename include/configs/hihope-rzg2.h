/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/hihope-rzg2.h
 *     This file is HOPERUN HiHope RZ/G2 board configuration.
 *
 * Copyright (C) 2020 Renesas Electronics Corporation
 */

#ifndef __HIHOPE_RZG2_H
#define __HIHOPE_RZG2_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

#endif /* __HIHOPE_RZG2_H */
