/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/ulcb.h
 *     This file is ULCB board configuration.
 *
 * Copyright (C) 2017 Renesas Electronics Corporation
 */

#ifndef __ULCB_H
#define __ULCB_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

/* Environment in eMMC, at the end of 2nd "boot sector" */

#endif /* __ULCB_H */
