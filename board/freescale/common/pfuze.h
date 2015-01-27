/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PFUZE_BOARD_HELPER__
#define __PFUZE_BOARD_HELPER__

struct pmic *pfuze_common_init(unsigned char i2cbus);
int pfuze_mode_init(struct pmic *p, u32 mode);

#endif
