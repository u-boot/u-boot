/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * For some reason linker sets linker-generated symbols to zero in PIE mode.
 * A work-around is substitution of linker-generated symbols with
 * compiler-generated symbols which are properly handled by linker in PAE mode.
 */

char __bss_start[0] __attribute__((section(".__bss_start")));
char __bss_end[0] __attribute__((section(".__bss_end")));
char __image_copy_start[0] __attribute__((section(".__image_copy_start")));
char __image_copy_end[0] __attribute__((section(".__image_copy_end")));
char __rel_dyn_start[0] __attribute__((section(".__rel_dyn_start")));
char __rel_dyn_end[0] __attribute__((section(".__rel_dyn_end")));
char __text_start[0] __attribute__((section(".__text_start")));
char __text_end[0] __attribute__((section(".__text_end")));
char __init_end[0] __attribute__((section(".__init_end")));
char __ivt_start[0] __attribute__((section(".__ivt_start")));
char __ivt_end[0] __attribute__((section(".__ivt_end")));
