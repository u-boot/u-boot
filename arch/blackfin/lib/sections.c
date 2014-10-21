/*
 * U-boot - section.c
 *
 * Copyright (c) 2014 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

char __bss_start[0] __attribute__((section(".__bss_start")));
char __bss_end[0] __attribute__((section(".__bss_end")));
char __init_end[0] __attribute__((section(".__init_end")));
