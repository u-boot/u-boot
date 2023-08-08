// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <asm/arch/soc.h>
#include <asm/arch/board.h>
#include <dm/util.h>

platform_t read_platform(void)
{
	platform_t plat = PLATFORM_HW;

	const char *model = fdt_get_board_model();

	if (model && !strncmp(model, "ASIM-", 5))
		plat = PLATFORM_ASIM;
	if (model && !strncmp(model, "EMUL-", 5))
		plat = PLATFORM_EMULATOR;
	return plat;
}

static inline u64 read_midr(void)
{
	u64 result;

	asm ("mrs %[rd],MIDR_EL1" : [rd] "=r" (result));
	return result;
}

u8 read_partnum(void)
{
	return ((read_midr() >> 4) & 0xFF);
}

const char *read_board_name(void)
{
	return fdt_get_board_model();
}

bool read_alt_pkg(void)
{
	union mio_fus_dat2 fus_dat2;

	fus_dat2.u = readq(MIO_FUS_DAT2);
	if (fus_dat2.s.chip_id >> 6)
		return true;
	/* Some parts only have lmc_mode32 set */
	if (read_partnum() == CN81XX && fus_dat2.s.lmc_mode32)
		return true;
	return false;
}

