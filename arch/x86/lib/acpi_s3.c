/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/acpi_s3.h>
#include <asm/acpi_table.h>
#include <asm/post.h>

static void asmlinkage (*acpi_do_wakeup)(void *vector) = (void *)WAKEUP_BASE;

static void acpi_jump_to_wakeup(void *vector)
{
	/* Copy wakeup trampoline in place */
	memcpy((void *)WAKEUP_BASE, __wakeup, __wakeup_size);

	printf("Jumping to OS waking vector %p\n", vector);
	acpi_do_wakeup(vector);
}

void acpi_resume(struct acpi_fadt *fadt)
{
	void *wake_vec;

	wake_vec = acpi_find_wakeup_vector(fadt);

	post_code(POST_OS_RESUME);
	acpi_jump_to_wakeup(wake_vec);
}
