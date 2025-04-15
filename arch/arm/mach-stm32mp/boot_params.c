// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <config.h>
#include <errno.h>
#include <log.h>
#include <linux/libfdt.h>
#include <asm/arch/sys_proto.h>
#include <asm/sections.h>
#include <asm/system.h>

/*
 * Use the saved FDT address provided by TF-A at boot time (NT_FW_CONFIG =
 * Non Trusted Firmware configuration file) when the pointer is valid
 */
int board_fdt_blob_setup(void **fdtp)
{
	unsigned long nt_fw_dtb = get_stm32mp_bl2_dtb();

	log_debug("%s: nt_fw_dtb=%lx\n", __func__, nt_fw_dtb);

	/* use external device tree only if address is valid */
	if (nt_fw_dtb < STM32_DDR_BASE ||
	    fdt_magic(nt_fw_dtb) != FDT_MAGIC) {
		log_debug("DTB not found.\n");
		log_debug("fall back to builtin DTB, %p\n", _end);

		return -EEXIST;
	}

	*fdtp = (void *)nt_fw_dtb;

	return 0;
}
