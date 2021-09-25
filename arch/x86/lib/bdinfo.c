// SPDX-License-Identifier: GPL-2.0+
/*
 * x86-specific information for the 'bd' command
 *
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <efi.h>
#include <init.h>
#include <asm/efi.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void arch_print_bdinfo(void)
{
	bdinfo_print_num_l("prev table", gd->arch.table);

	if (IS_ENABLED(CONFIG_EFI_STUB))
		efi_show_bdinfo();
}
