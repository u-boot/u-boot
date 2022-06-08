/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * GXP board
 *
 * (C) Copyright 2022 Hewlett Packard Enterprise Development LP.
 * Author: Nick Hawkins <nick.hawkins@hpe.com>
 * Author: Jean-Marie Verdun <verdun@hpe.com>
 */

#ifndef _GXP_H_
#define _GXP_H_

#define CONFIG_SYS_SDRAM_BASE   0x40000000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"recover_file=openbmc-hpe-recovery-image.mtd\0" \
	"recover_cmd=usb start; " \
	"mw.b 0xD100000D 0x40; " \
	"if fatload usb 0 0x50000000 $recover_file 0x4C0000 0x80000; then " \
		"setenv bootargs console=ttyS0,115200 recovery; " \
		"setenv force_recovery; " \
		"saveenv; " \
		"bootm  0x50000000; " \
	"else " \
		"while itest 0 < 1; do " \
		"mw.b 0xd1000005 0xc0; " \
		"sleep .1; " \
		"mw.b 0xd1000005 0x00; " \
		"sleep .1; " \
		"done; " \
	"fi; " \
	"reset;\0" \
	"spiboot=if itest.b *0xD10000B2 == 6; then " \
		"run recover_cmd;" \
	"fi;" \
	"if printenv force_recovery; then " \
		"run recover_cmd; " \
	"else " \
		"bootm 0xfc080000; " \
		"run recover_cmd; " \
	"fi;\0"

#endif
