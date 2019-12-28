// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 *
 * Based on common/spl/spl_atf.c
 */
#include <common.h>
#include <cpu_func.h>
#include <errno.h>
#include <hang.h>
#include <spl.h>
#include <asm/smp.h>
#include <opensbi.h>

DECLARE_GLOBAL_DATA_PTR;

struct fw_dynamic_info opensbi_info;

static int spl_opensbi_find_uboot_node(void *blob, int *uboot_node)
{
	int fit_images_node, node;
	const char *fit_os;

	fit_images_node = fdt_path_offset(blob, "/fit-images");
	if (fit_images_node < 0)
		return -ENODEV;

	fdt_for_each_subnode(node, blob, fit_images_node) {
		fit_os = fdt_getprop(blob, node, FIT_OS_PROP, NULL);
		if (!fit_os)
			continue;

		if (genimg_get_os_id(fit_os) == IH_OS_U_BOOT) {
			*uboot_node = node;
			return 0;
		}
	}

	return -ENODEV;
}

void spl_invoke_opensbi(struct spl_image_info *spl_image)
{
	int ret, uboot_node;
	ulong uboot_entry;
	void (*opensbi_entry)(ulong hartid, ulong dtb, ulong info);

	if (!spl_image->fdt_addr) {
		pr_err("No device tree specified in SPL image\n");
		hang();
	}

	/* Find U-Boot image in /fit-images */
	ret = spl_opensbi_find_uboot_node(spl_image->fdt_addr, &uboot_node);
	if (ret) {
		pr_err("Can't find U-Boot node, %d", ret);
		hang();
	}

	/* Get U-Boot entry point */
	uboot_entry = fdt_getprop_u32(spl_image->fdt_addr, uboot_node,
				      "entry-point");
	if (uboot_entry == FDT_ERROR)
		uboot_entry = fdt_getprop_u32(spl_image->fdt_addr, uboot_node,
					      "load-addr");

	/* Prepare obensbi_info object */
	opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
	opensbi_info.version = FW_DYNAMIC_INFO_VERSION;
	opensbi_info.next_addr = uboot_entry;
	opensbi_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
	opensbi_info.options = SBI_SCRATCH_NO_BOOT_PRINTS;
	opensbi_info.boot_hart = gd->arch.boot_hart;

	opensbi_entry = (void (*)(ulong, ulong, ulong))spl_image->entry_point;
	invalidate_icache_all();

#ifdef CONFIG_SMP
	/*
	 * Start OpenSBI on all secondary harts and wait for acknowledgment.
	 *
	 * OpenSBI first relocates itself to its link address. This is done by
	 * the main hart. To make sure no hart is still running U-Boot SPL
	 * during relocation, we wait for all secondary harts to acknowledge
	 * the call-function request before entering OpenSBI on the main hart.
	 * Otherwise, code corruption can occur if the link address ranges of
	 * U-Boot SPL and OpenSBI overlap.
	 */
	ret = smp_call_function((ulong)spl_image->entry_point,
				(ulong)spl_image->fdt_addr,
				(ulong)&opensbi_info, 1);
	if (ret)
		hang();
#endif
	opensbi_entry(gd->arch.boot_hart, (ulong)spl_image->fdt_addr,
		      (ulong)&opensbi_info);
}
