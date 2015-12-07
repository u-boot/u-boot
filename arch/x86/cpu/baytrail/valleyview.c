/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mmc.h>
#include <pci_ids.h>
#include <asm/irq.h>
#include <asm/post.h>

static struct pci_device_id mmc_supported[] = {
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_VALLEYVIEW_SDIO },
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_VALLEYVIEW_SDCARD },
};

int cpu_mmc_init(bd_t *bis)
{
	return pci_mmc_init("ValleyView SDHCI", mmc_supported,
			    ARRAY_SIZE(mmc_supported));
}

#ifndef CONFIG_EFI_APP
int arch_cpu_init(void)
{
	int ret;

	post_code(POST_CPU_INIT);
#ifdef CONFIG_SYS_X86_TSC_TIMER
	timer_set_base(rdtsc());
#endif

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	return 0;
}

int arch_misc_init(void)
{
	if (!ll_boot_init())
		return 0;

	return pirq_init();
}
#endif
