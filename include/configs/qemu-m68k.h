/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#ifndef __QEMU_M68K_H
#define __QEMU_M68K_H

/* Memory Configuration */
#define CFG_SYS_SDRAM_BASE       0x00000000

/*
 * Initial Stack Pointer:
 * Place the stack at 4MB offset to avoid overwriting U-Boot code/data.
 */
#define CFG_SYS_INIT_SP_ADDR     (CFG_SYS_SDRAM_BASE + 0x400000)

#endif /* __QEMU_M68K_H */
