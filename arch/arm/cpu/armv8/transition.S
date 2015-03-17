/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

ENTRY(armv8_switch_to_el2)
	switch_el x0, 1f, 0f, 0f
0:	ret
1:	armv8_switch_to_el2_m x0
ENDPROC(armv8_switch_to_el2)

ENTRY(armv8_switch_to_el1)
	switch_el x0, 0f, 1f, 0f
0:	ret
1:	armv8_switch_to_el1_m x0, x1
ENDPROC(armv8_switch_to_el1)
