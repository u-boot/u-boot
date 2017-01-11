/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/wdog.h>
#include <efi_loader.h>

#define RESET_TIMEOUT 10

/*
 * The Raspberry Pi firmware uses the RSTS register to know which partiton
 * to boot from. The partiton value is spread into bits 0, 2, 4, 6, 8, 10.
 * Partiton 63 is a special partition used by the firmware to indicate halt.
 */
#define BCM2835_WDOG_RSTS_RASPBERRYPI_HALT	0x555

__efi_runtime_data struct bcm2835_wdog_regs *wdog_regs =
	(struct bcm2835_wdog_regs *)BCM2835_WDOG_PHYSADDR;

void __efi_runtime reset_cpu(ulong addr)
{
	uint32_t rstc;

	rstc = readl(&wdog_regs->rstc);
	rstc &= ~BCM2835_WDOG_RSTC_WRCFG_MASK;
	rstc |= BCM2835_WDOG_RSTC_WRCFG_FULL_RESET;

	writel(BCM2835_WDOG_PASSWORD | RESET_TIMEOUT, &wdog_regs->wdog);
	writel(BCM2835_WDOG_PASSWORD | rstc, &wdog_regs->rstc);
}

#ifdef CONFIG_EFI_LOADER

void __efi_runtime EFIAPI efi_reset_system(
			enum efi_reset_type reset_type,
			efi_status_t reset_status,
			unsigned long data_size, void *reset_data)
{
	u32 val;

	switch (reset_type) {
	case EFI_RESET_COLD:
	case EFI_RESET_WARM:
		reset_cpu(0);
		break;
	case EFI_RESET_SHUTDOWN:
		/*
		 * We set the watchdog hard reset bit here to distinguish this reset
		 * from the normal (full) reset. bootcode.bin will not reboot after a
		 * hard reset.
		 */
		val = readl(&wdog_regs->rsts);
		val |= BCM2835_WDOG_PASSWORD;
		val |= BCM2835_WDOG_RSTS_RASPBERRYPI_HALT;
		writel(val, &wdog_regs->rsts);
		reset_cpu(0);
		break;
	}

	while (1) { }
}

void efi_reset_system_init(void)
{
	efi_add_runtime_mmio(&wdog_regs, sizeof(*wdog_regs));
}

#endif
