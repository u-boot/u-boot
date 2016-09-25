/*
 * (C) Copyright 2013 - 2014 Xilinx, Inc
 *
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <image.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/u-boot.h>

DECLARE_GLOBAL_DATA_PTR;

bool boot_linux;

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NOR;
}

/* Board initialization after bss clearance */
void spl_board_init(void)
{
	/* enable console uart printing */
	preloader_console_init();
}

#ifdef CONFIG_SPL_OS_BOOT
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image, void *arg)
{
	debug("Entering kernel arg pointer: 0x%p\n", arg);
	typedef void (*image_entry_arg_t)(char *, ulong, ulong)
		__attribute__ ((noreturn));
	image_entry_arg_t image_entry =
		(image_entry_arg_t)spl_image->entry_point;

	image_entry(NULL, 0, (ulong)arg);
}
#endif /* CONFIG_SPL_OS_BOOT */

int spl_start_uboot(void)
{
#ifdef CONFIG_SPL_OS_BOOT
	if (boot_linux)
		return 0;
#endif

	return 1;
}
