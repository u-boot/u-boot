// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * Generic reset driver for x86 processor
 */

#include <common.h>
#include <dm.h>
#include <sysreset.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <efi_loader.h>

static int x86_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	int value;

	switch (type) {
	case SYSRESET_WARM:
		value = SYS_RST | RST_CPU;
		break;
	case SYSRESET_COLD:
		value = SYS_RST | RST_CPU | FULL_RST;
		break;
	default:
		return -ENOSYS;
	}

	outb(value, IO_PORT_RESET);

	return -EINPROGRESS;
}

#ifdef CONFIG_EFI_LOADER
void __efi_runtime EFIAPI efi_reset_system(
			enum efi_reset_type reset_type,
			efi_status_t reset_status,
			unsigned long data_size, void *reset_data)
{
	int value;

	/*
	 * inline this code since we are not caused in the context of a
	 * udevice and passing NULL to x86_sysreset_request() is too horrible.
	 */
	if (reset_type == EFI_RESET_COLD ||
		 reset_type == EFI_RESET_PLATFORM_SPECIFIC)
		value = SYS_RST | RST_CPU | FULL_RST;
	else /* assume EFI_RESET_WARM since we cannot return an error */
		value = SYS_RST | RST_CPU;
	outb(value, IO_PORT_RESET);

	/* TODO EFI_RESET_SHUTDOWN */

	while (1) { }
}
#endif


static const struct udevice_id x86_sysreset_ids[] = {
	{ .compatible = "x86,reset" },
	{ }
};

static struct sysreset_ops x86_sysreset_ops = {
	.request = x86_sysreset_request,
};

U_BOOT_DRIVER(x86_sysreset) = {
	.name = "x86-sysreset",
	.id = UCLASS_SYSRESET,
	.of_match = x86_sysreset_ids,
	.ops = &x86_sysreset_ops,
};
