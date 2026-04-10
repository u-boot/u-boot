/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2025-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Alexander Feilke, Max Merchel
 */

#ifndef __TQ_SOM_H
#define __TQ_SOM_H

#include <init.h>
#include <asm/io.h>
#include <linux/iopoll.h>

void tq_som_ram_init(void);

/* used as a wrapper to write to specific register addresses */
static inline void tq_som_init_write_reg(u32 address, u32 value)
{
	writel_relaxed(value, address);
}

/*
 * checks if the accessible range equals the requested ram size.
 * returns true if successful, false otherwise
 */
bool tq_som_ram_check_size(long ram_size);

static inline void tq_som_check_bits_set(u32 address, u32 mask)
{
	u32 val;
	readl_poll_timeout(address, val, (val & mask) == mask, 1000);
}

#endif /* __TQ_SOM_H */
