/*
 * Copyright (C) 2013-2015 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARC_RELOCATE_H
#define _ASM_ARC_RELOCATE_H

#include <common.h>

int copy_uboot_to_ram(void);
int clear_bss(void);
int do_elf_reloc_fixups(void);

#endif	/* _ASM_ARC_RELOCATE_H */
