/*
 * (C) Copyright 2010-2012
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 * Tom Rini <trini@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <config.h>
#include <spl.h>
#include <image.h>
#include <linux/compiler.h>

/* Pointer to as well as the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;
gd_t gdata __attribute__ ((section(".data")));

/*
 * In the context of SPL, board_init_f must ensure that any clocks/etc for
 * DDR are enabled, ensure that the stack pointer is valid, clear the BSS
 * and call board_init_f.  We provide this version by default but mark it
 * as __weak to allow for platforms to do this in their own way if needed.
 */
void __weak board_init_f(ulong dummy)
{
	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* Set global data pointer. */
	gd = &gdata;

	board_init_r(NULL, 0);
}

/*
 * This function jumps to an image with argument. Normally an FDT or ATAGS
 * image.
 * arg: Pointer to paramter image in RAM
 */
#ifdef CONFIG_SPL_OS_BOOT
void __noreturn jump_to_image_linux(void *arg)
{
	unsigned long machid = 0xffffffff;
#ifdef CONFIG_MACH_TYPE
	machid = CONFIG_MACH_TYPE;
#endif

	debug("Entering kernel arg pointer: 0x%p\n", arg);
	typedef void (*image_entry_arg_t)(int, int, void *)
		__attribute__ ((noreturn));
	image_entry_arg_t image_entry =
		(image_entry_arg_t) spl_image.entry_point;
	cleanup_before_linux();
	image_entry(0, machid, arg);
}
#endif
