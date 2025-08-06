/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2025 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#ifndef __E850_96_ACPM_H
#define __E850_96_ACPM_H

#include <linux/types.h>

/**
 * struct acpm - Data for I3C communication over ACPM IPC protocol
 * @mbox_base: Base address of APM mailbox block
 * @sram_base: Base address of shared memory used for APM messages
 * @ipc_ch: Mailbox channel number used for communication with I3C block (0-15)
 */
struct acpm {
	void __iomem *mbox_base;
	void __iomem *sram_base;
	u8 ipc_ch;
};

int acpm_i3c_read(struct acpm *acpm, u8 ch, u8 addr, u8 reg, u8 *val);
int acpm_i3c_write(struct acpm *acpm, u8 ch, u8 addr, u8 reg, u8 val);

#endif /* __E850_96_ACPM_H */
