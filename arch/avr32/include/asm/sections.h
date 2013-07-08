/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_SECTIONS_H
#define __ASM_AVR32_SECTIONS_H

#include <asm-generic/sections.h>

/* References to section boundaries */

extern char __data_lma[], __edata_lma[];
extern char __got_start[], __got_lma[], __got_end[];

#endif /* __ASM_AVR32_SECTIONS_H */
