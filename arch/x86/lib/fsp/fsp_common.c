/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/fsp/fsp_support.h>

DECLARE_GLOBAL_DATA_PTR;

int print_cpuinfo(void)
{
	post_code(POST_CPU_INFO);
	return default_print_cpuinfo();
}

int fsp_init_phase_pci(void)
{
	u32 status;

	/* call into FspNotify */
	debug("Calling into FSP (notify phase INIT_PHASE_PCI): ");
	status = fsp_notify(NULL, INIT_PHASE_PCI);
	if (status)
		debug("fail, error code %x\n", status);
	else
		debug("OK\n");

	return status ? -EPERM : 0;
}

int board_pci_post_scan(struct pci_controller *hose)
{
	return fsp_init_phase_pci();
}

void board_final_cleanup(void)
{
	u32 status;

	/* call into FspNotify */
	debug("Calling into FSP (notify phase INIT_PHASE_BOOT): ");
	status = fsp_notify(NULL, INIT_PHASE_BOOT);
	if (status)
		debug("fail, error code %x\n", status);
	else
		debug("OK\n");

	return;
}

int x86_fsp_init(void)
{
	if (!gd->arch.hob_list) {
		/*
		 * The first time we enter here, call fsp_init().
		 * Note the execution does not return to this function,
		 * instead it jumps to fsp_continue().
		 */
		fsp_init(CONFIG_FSP_TEMP_RAM_ADDR, BOOT_FULL_CONFIG, NULL);
	} else {
		/*
		 * The second time we enter here, adjust the size of malloc()
		 * pool before relocation. Given gd->malloc_base was adjusted
		 * after the call to board_init_f_mem() in arch/x86/cpu/start.S,
		 * we should fix up gd->malloc_limit here.
		 */
		gd->malloc_limit += CONFIG_FSP_SYS_MALLOC_F_LEN;
	}

	return 0;
}
