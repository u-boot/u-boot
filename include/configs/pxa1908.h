/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024
 * Duje Mihanović <duje.mihanovic@skole.hr>
 */

#ifndef __PXA1908_H
#define __PXA1908_H

#define CFG_SYS_SDRAM_BASE	0x1000000
#define CFG_SYS_INIT_RAM_ADDR	0x10000000
#define CFG_SYS_INIT_RAM_SIZE	0x4000
#define CFG_SYS_NS16550_IER	0x40
#define CFG_SYS_BAUDRATE_TABLE	{ 115200, 230400, 460800, 921600 }
#define CFG_EXTRA_ENV_SETTINGS \
	"bootcmd=bootm $prevbl_initrd_start_addr\0"

#endif
