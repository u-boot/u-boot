/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot file cpu/intel/model_206ax/cache_as_ram.inc
 *
 * Copyright (C) 2000,2007 Ronald G. Minnich <rminnich@gmail.com>
 * Copyright (C) 2005 Tyan (written by Yinghai Lu for Tyan)
 * Copyright (C) 2007-2008 coresystems GmbH
 * Copyright (C) 2012 Kyösti Mälkki <kyosti.malkki@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>

	/* Note: ebp must not be touched in this code */
.globl car_init
car_init:
	/* TODO: Add cache-as-RAM init here */
	jmp	car_init_ret
