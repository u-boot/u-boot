/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/arch/tnc.h>
#include <asm/fsp/fsp_support.h>
#include <asm/processor.h>

static void unprotect_spi_flash(void)
{
	u32 bc;

	bc = pci_read_config32(PCH_LPC_DEV, 0xd8);
	bc |= 0x1;	/* unprotect the flash */
	pci_write_config32(PCH_LPC_DEV, 0xd8, bc);
}

int arch_cpu_init(void)
{
	struct pci_controller *hose;
	int ret;

	post_code(POST_CPU_INIT);
#ifdef CONFIG_SYS_X86_TSC_TIMER
	timer_set_base(rdtsc());
#endif

	ret = x86_cpu_init_f();
	if (ret)
		return ret;

	ret = pci_early_init_hose(&hose);
	if (ret)
		return ret;

	unprotect_spi_flash();

	return 0;
}
