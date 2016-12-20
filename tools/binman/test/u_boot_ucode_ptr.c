/*
 * Copyright (c) 2016 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * Simple program to create a _dt_ucode_base_size symbol which can be read
 * by 'nm'. This is used by binman tests.
 *
 * Build with:
 * cc -march=i386 -m32 -o u_boot_ucode_ptr -T u_boot_ucode_ptr.lds -nostdlib \
	u_boot_ucode_ptr.c
 */

static unsigned long _dt_ucode_base_size[2]
		__attribute__((section(".ucode"))) = {1, 2};
