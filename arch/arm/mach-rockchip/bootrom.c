/**
 * Copyright (c) 2017 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/bootrom.h>
#include <asm/setjmp.h>
#include <asm/system.h>

/*
 * Force the jmp_buf to the data-section, as .bss will not be valid
 * when save_boot_params is invoked.
 */
static jmp_buf brom_ctx __section(".data");

void back_to_bootrom(void)
{
#if CONFIG_IS_ENABLED(LIBCOMMON_SUPPORT)
	puts("Returning to boot ROM...\n");
#endif
	longjmp(brom_ctx, BROM_BOOT_NEXTSTAGE);
}

/*
 * All Rockchip BROM implementations enter with a valid stack-pointer,
 * so this can safely be implemented in C (providing a single
 * implementation both for ARMv7 and AArch64).
 */
int save_boot_params(void)
{
	int  ret = setjmp(brom_ctx);

	switch (ret) {
	case 0:
		/*
		 * This is the initial pass through this function
		 * (i.e. saving the context), setjmp just setup up the
		 * brom_ctx: transfer back into the startup-code at
		 * 'save_boot_params_ret' and let the compiler know
		 * that this will not return.
		 */
		save_boot_params_ret();
		while (true)
			/* does not return */;
		break;

	case BROM_BOOT_NEXTSTAGE:
		/*
		 * To instruct the BROM to boot the next stage, we
		 * need to return 0 to it: i.e. we need to rewrite
		 * the return code once more.
		 */
		ret = 0;
		break;

	default:
#if CONFIG_IS_ENABLED(LIBCOMMON_SUPPORT)
		puts("FATAL: unexpected command to back_to_bootrom()\n");
#endif
		hang();
	};

	return ret;
}
