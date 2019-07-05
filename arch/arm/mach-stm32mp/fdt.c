// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <fdt_support.h>
#include <asm/arch/sys_proto.h>
#include <dt-bindings/pinctrl/stm32-pinfunc.h>

/*
 * This function is called right before the kernel is booted. "blob" is the
 * device tree that will be passed to the kernel.
 */
int ft_system_setup(void *blob, bd_t *bd)
{
	int ret = 0;
	u32 pkg;

	switch (get_cpu_package()) {
	case PKG_AA_LBGA448:
		pkg = STM32MP_PKG_AA;
		break;
	case PKG_AB_LBGA354:
		pkg = STM32MP_PKG_AB;
		break;
	case PKG_AC_TFBGA361:
		pkg = STM32MP_PKG_AC;
		break;
	case PKG_AD_TFBGA257:
		pkg = STM32MP_PKG_AD;
		break;
	default:
		pkg = 0;
		break;
	}
	if (pkg) {
		do_fixup_by_compat_u32(blob, "st,stm32mp157-pinctrl",
				       "st,package", pkg, false);
		do_fixup_by_compat_u32(blob, "st,stm32mp157-z-pinctrl",
				       "st,package", pkg, false);
	}

	return ret;
}
