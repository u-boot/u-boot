/*
 * Copyright (c) 2011-2012 The Chromium OS Authors.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

SECTIONS
{

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	__u_boot_sandbox_option_start = .;
	_u_boot_sandbox_getopt : { *(.u_boot_sandbox_getopt) }
	__u_boot_sandbox_option_end = .;

	__bss_start = .;
}

INSERT BEFORE .data;
