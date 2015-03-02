/*
 * Copyright (C) 2004-2006 Atmel Corporation
 * Copyright (C) 2015 Andreas Bießmann <andreas.devel@googlmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <atmel_mci.h>
#include <asm/arch/hardware.h>

/* provide cpu_mmc_init, to overwrite provide board_mmc_init */
int cpu_mmc_init(bd_t *bd)
{
	/* This calls the atmel_mci_init in gen_atmel_mci.c */
	return atmel_mci_init((void *)ATMEL_BASE_MMCI);
}
