/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _RELOCATE_H_
#define _RELOCATE_H_

#include <common.h>

int copy_uboot_to_ram(void);
int clear_bss(void);
int do_elf_reloc_fixups(void);

#endif	/* !_RELOCATE_H_ */
