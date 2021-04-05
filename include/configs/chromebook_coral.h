/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Google LLC
 */

/*
 * board/config.h - configuration options, board-specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_BOOTCOMMAND	\
	"tpm init; tpm startup TPM2_SU_CLEAR; " \
	"read mmc 0:2 100000 0 80; setexpr loader *001004f0; " \
	"setexpr size *00100518; setexpr blocks $size / 200; " \
	"read mmc 0:2 100000 80 $blocks; setexpr setup $loader - 1000; " \
	"setexpr cmdline_ptr $loader - 2000; " \
	"setexpr.s cmdline *$cmdline_ptr; " \
	"setexpr cmdline gsub %U \\\\${uuid}; " \
	"if part uuid mmc 0:2 uuid; then " \
	"zboot start 100000 0 0 0 $setup cmdline; " \
	"zboot load; zboot setup; zboot dump; zboot go;" \
	"fi"

#include <configs/x86-common.h>
#include <configs/x86-chromebook.h>

#undef CONFIG_STD_DEVICES_SETTINGS
#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,i8042-kbd,serial\0" \
					"stdout=vidconsole,serial\0" \
					"stderr=vidconsole,serial\0"

#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0x003f8000

#define CONFIG_TPL_TEXT_BASE		0xffff8000

#define CONFIG_SYS_NS16550_MEM32
#undef CONFIG_SYS_NS16550_PORT_MAPPED

#endif	/* __CONFIG_H */
