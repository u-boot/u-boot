/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/silinux-ek874.h
 *     This file is Silicon Linux EK874 board configuration.
 *
 * Copyright (C) 2021 Renesas Electronics Corporation
 */

#ifndef __SILINUX_EK874_H
#define __SILINUX_EK874_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

#endif /* __SILINUX_EK874_H */
