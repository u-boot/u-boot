// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Alexander Feilke, Max Merchel
 */

#include <init.h>
#include <asm/io.h>
#include <linux/sizes.h>

#include "tq_som.h"

void __weak tq_som_ram_init(void)
{
	;
}

/*
 * checks if the accessible range equals the requested RAM size.
 * returns true if successful, false otherwise
 */
bool tq_som_ram_check_size(long ram_size)
{
	long size;

	size = get_ram_size((void *)PHYS_SDRAM, ram_size);
	debug("SPL: requested RAM size %lu MiB. accessible %lu MiB\n",
	      ram_size / (SZ_1M), size / (SZ_1M));

	return (size == ram_size);
}
