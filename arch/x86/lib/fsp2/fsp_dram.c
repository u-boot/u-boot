// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <acpi_s3.h>
#include <handoff.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/fsp/fsp_support.h>
#include <asm/fsp2/fsp_api.h>
#include <asm/fsp2/fsp_internal.h>

int dram_init(void)
{
	int ret;

	if (spl_phase() == PHASE_SPL) {
#ifdef CONFIG_HAVE_ACPI_RESUME
		bool s3wake = gd->arch.prev_sleep_state == ACPI_S3;
#else
		bool s3wake = false;
#endif

		ret = fsp_memory_init(s3wake,
			      IS_ENABLED(CONFIG_APL_BOOT_FROM_FAST_SPI_FLASH));
		if (ret) {
			debug("Memory init failed (err=%x)\n", ret);
			return ret;
		}

		/* The FSP has already set up DRAM, so grab the info we need */
		ret = fsp_scan_for_ram_size();
		if (ret)
			return ret;

#ifdef CONFIG_ENABLE_MRC_CACHE
		gd->arch.mrc[MRC_TYPE_NORMAL].buf =
			fsp_get_nvs_data(gd->arch.hob_list,
					 &gd->arch.mrc[MRC_TYPE_NORMAL].len);
		gd->arch.mrc[MRC_TYPE_VAR].buf =
			fsp_get_var_nvs_data(gd->arch.hob_list,
					     &gd->arch.mrc[MRC_TYPE_VAR].len);
		log_debug("normal %x, var %x\n",
			  gd->arch.mrc[MRC_TYPE_NORMAL].len,
			  gd->arch.mrc[MRC_TYPE_VAR].len);
#endif
	} else {
#if CONFIG_IS_ENABLED(HANDOFF)
		struct spl_handoff *ho = gd->spl_handoff;

		if (!ho) {
			debug("No SPL handoff found\n");
			return -ESTRPIPE;
		}
		gd->ram_size = ho->ram_size;
		handoff_load_dram_banks(ho);
#endif
		ret = arch_fsps_preinit();
		if (ret)
			return log_msg_ret("fsp_s_preinit", ret);
	}

	return 0;
}

ulong board_get_usable_ram_top(ulong total_size)
{
#if CONFIG_IS_ENABLED(HANDOFF)
	struct spl_handoff *ho = gd->spl_handoff;

	return ho->arch.usable_ram_top;
#endif

	return gd->ram_top;
}
