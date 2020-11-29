/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Ивайло Димитров <freemangordon@abv.bg>
 *
 * (C) Copyright 2011-2012
 * Pali Rohár <pali@kernel.org>
 *
 * (C) Copyright 2008
 * Dirk Behme <dirk.behme@gmail.com>
 */
#ifndef _RX51_H_
#define _RX51_H_

/* Needed for ROM SMC call */
struct emu_hal_params_rx51 {
	u32 num_params;
	u32 param1;
	u32 param2;
	u32 param3;
	u32 param4;
};

#define ONENAND_GPMC_CONFIG1_RX51	0xfb001202
#define ONENAND_GPMC_CONFIG2_RX51	0x00111100
#define ONENAND_GPMC_CONFIG3_RX51	0x00020200
#define ONENAND_GPMC_CONFIG4_RX51	0x11001102
#define ONENAND_GPMC_CONFIG5_RX51	0x03101616
#define ONENAND_GPMC_CONFIG6_RX51	0x90060000

#endif
