// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <image.h>
#include <spl.h>

void __noreturn jump_to_image(struct spl_image_info *spl_image)
{
	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	if (spl_image->os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		typedef void (*image_entry_arg_t)(int, int, int, int)
			__attribute__ ((noreturn));
		image_entry_arg_t image_entry =
			(image_entry_arg_t)(uintptr_t) spl_image->entry_point;
		image_entry(IH_MAGIC, CONFIG_SPL_TEXT_BASE, 0, 0);
	} else {
		typedef void __noreturn (*image_entry_noargs_t)(void);
		image_entry_noargs_t image_entry =
			(image_entry_noargs_t)spl_image->entry_point;
		image_entry();
	}
}
