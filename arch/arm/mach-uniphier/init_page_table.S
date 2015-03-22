/*
 * Copyright (C) 2015 Panasonic Corporation
 * Copyright (C) 2015 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/linkage.h>

/* page table */
#define NR_SECTIONS	4096
#define SECTION_SHIFT	20
#define DEVICE	0x00002002 /* Non-shareable Device */
#define NORMAL	0x0000000e /* Normal Memory Write-Back, No Write-Allocate */

#define TEXT_SECTION	((CONFIG_SPL_TEXT_BASE) >> (SECTION_SHIFT))
#define STACK_SECTION	((CONFIG_SPL_STACK) >> (SECTION_SHIFT))

	.section ".rodata"
	.align 14
ENTRY(init_page_table)
	section = 0
	.rept NR_SECTIONS
	.if section == TEXT_SECTION || section == STACK_SECTION
	attr = NORMAL
	.else
	attr = DEVICE
	.endif
	.word (section << SECTION_SHIFT) | attr
	section = section + 1
	.endr
END(init_page_table)
