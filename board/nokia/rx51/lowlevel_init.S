/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011-2012
 * Pali Rohár <pali@kernel.org>
 */

#include <config.h>

kernoffs:		/* offset of kernel image from this address */
	.word . - CONFIG_TEXT_BASE - KERNEL_OFFSET

kernaddr:		/* address of kernel after copying */
	.word KERNEL_ADDRESS

kernsize:		/* maximal size of kernel image */
	.word KERNEL_MAXSIZE

imagesize:		/* maximal size of image */
	.word IMAGE_MAXSIZE

ih_magic:		/* IH_MAGIC in big endian from include/image.h */
	.word 0x56190527

z_magic:		/* LINUX_ARM_ZIMAGE_MAGIC */
	.word 0x016f2818

/*
 * Routine: save_boot_params (called after reset from start.S)
 * Description: Copy attached kernel to address KERNEL_ADDRESS
 */

.global save_boot_params
save_boot_params:

/*
 * Copy valid attached kernel to absolute address KERNEL_ADDRESS
 *
 * Nokia X-Loader is loading secondary image to address 0x80400000.
 * NOLO is loading boot image to random place, so it doesn't really
 * matter what is set in CONFIG_TEXT_BASE. We have to detect
 * KERNEL_OFFSET from the current execution address and copy it to
 * absolute address KERNEL_ADDRESS.
 *
 * Note that U-Boot has to be compiled with CONFIG_POSITION_INDEPENDENT
 * because it is loaded at random address and not to the fixed address
 * (CONFIG_TEXT_BASE).
 */

	/* r0 - start of kernel before */
	adr	r0, kernoffs	/* r0 - current address of kernoffs section */
	ldr	r1, kernoffs	/* r1 - offset of kernel image from kernoffs section */
	sub	r0, r0, r1

	/* r3 - start of kernel after */
	ldr	r3, kernaddr

	/* r2 - end of kernel after */
	ldr	r1, kernsize
	add	r2, r3, r1

	/* r1 - end of kernel before */
	add	r1, r0, r1

	/* remove header in target kernel */
	mov	r5, #0
	str	r5, [r3]	/* remove 4 bytes header of kernel uImage */
	str	r5, [r3, #36]	/* remove 4 bytes header of kernel zImage */

	/* check for valid kernel uImage */
	ldr	r4, [r0]	/* r4 - 4 bytes header of kernel */
	ldr	r5, ih_magic	/* r5 - IH_MAGIC */
	cmp	r4, r5
	beq	copy_kernel_loop

	/* check for valid kernel zImage */
	ldr	r4, [r0, #36]	/* r4 - 4 bytes header of kernel at offset 36 */
	ldr	r5, z_magic	/* r5 - LINUX_ARM_ZIMAGE_MAGIC */
	cmp	r4, r5
	bne	skip_copy	/* skip if invalid image */

copy_kernel_loop:
	ldmdb	r1!, {r3 - r10}
	stmdb	r2!, {r3 - r10}
	cmp	r1, r0
	bhi	copy_kernel_loop

	/* remove header in source kernel image */
	mov	r5, #0
	str	r5, [r0]	/* remove 4 bytes header of kernel uImage */
	str	r5, [r0, #36]	/* remove 4 bytes header of kernel zImage */

skip_copy:

	/* Returns */
	b	save_boot_params_ret
