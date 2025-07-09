// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <env.h>
#include <init.h>
#include <mapmem.h>
#include <asm/io.h>
#include "fw.h"

/* OTP Controller base address and register offsets */
#define EXYNOS850_OTP_BASE	0x10000000
#define OTP_CHIPID0		0x4
#define OTP_CHIPID1		0x8

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

/* Read the unique SoC ID from OTP registers */
static u64 get_chip_id(void)
{
	void __iomem *otp_base;
	u64 val;

	otp_base = map_sysmem(EXYNOS850_OTP_BASE, 12);
	val = readl(otp_base + OTP_CHIPID0);
	val |= (u64)readl(otp_base + OTP_CHIPID1) << 32UL;
	unmap_sysmem(otp_base);

	return val;
}

static void setup_serial(void)
{
	char serial_str[17] = { 0 };
	u64 serial_num;

	if (env_get("serial#"))
		return;

	serial_num = get_chip_id();
	snprintf(serial_str, sizeof(serial_str), "%016llx", serial_num);
	env_set("serial#", serial_str);
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	int err;

	setup_serial();

	/*
	 * Do this in board_late_init() to make sure MMC is not probed before
	 * efi_init_early().
	 */
	err = load_ldfw();
	if (err)
		printf("ERROR: LDFW loading failed (%d)\n", err);

	return 0;
}
