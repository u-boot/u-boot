// SPDX-License-Identifier: GPL-2.0+
/*
 * x86-specific information for the 'bd' command
 *
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <efi.h>
#include <init.h>
#include <asm/cpu.h>
#include <asm/efi.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void arch_print_bdinfo(void)
{
	bdinfo_print_num_l("prev table", gd->arch.table);
	bdinfo_print_num_l("clock_rate", gd->arch.clock_rate);
	bdinfo_print_num_l("tsc_base", gd->arch.tsc_base);
	bdinfo_print_num_l("vendor", gd->arch.x86_vendor);
	bdinfo_print_str(" name", cpu_vendor_name(gd->arch.x86_vendor));
	bdinfo_print_num_l("model", gd->arch.x86_model);

	if (IS_ENABLED(CONFIG_EFI_STUB))
		efi_show_bdinfo();
}
