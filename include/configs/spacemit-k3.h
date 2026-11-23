/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2026 Yixun Lan <dlan@kernel.org>
 */

#ifndef __SPACEMIT_K3_H
#define __SPACEMIT_K3_H

#define CFG_EXTRA_ENV_SETTINGS \
	"ramdisk_addr_r=0x130000000\0" \
	"fdt_addr_r=0x138000000\0" \
	"kernel_addr_r=0x140000000\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0"

#define CFG_SYS_NS16550_IER	0x40

#define RISCV_SMODE_TIMER_FREQ	24000000

#endif /* __SPACEMIT_K3_H */
