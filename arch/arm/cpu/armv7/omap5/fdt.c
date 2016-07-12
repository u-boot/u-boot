/*
 * Copyright 2016 Texas Instruments, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <malloc.h>

#include <asm/omap_common.h>
#include <asm/arch-omap5/sys_proto.h>

#ifdef CONFIG_TI_SECURE_DEVICE

/* Give zero values if not already defined */
#ifndef TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ
#define TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ (0)
#endif
#ifndef CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ
#define CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ (0)
#endif

static u32 hs_irq_skip[] = {
	8,	/* Secure violation reporting interrupt */
	15,	/* One interrupt for SDMA by secure world */
	118	/* One interrupt for Crypto DMA by secure world */
};

static int ft_hs_fixup_crossbar(void *fdt, bd_t *bd)
{
	const char *path;
	int offs;
	int ret;
	int len, i, old_cnt, new_cnt;
	u32 *temp;
	const u32 *p_data;

	/*
	 * Increase the size of the fdt
	 * so we have some breathing room
	 */
	ret = fdt_increase_size(fdt, 512);
	if (ret < 0) {
		printf("Could not increase size of device tree: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	/* Reserve IRQs that are used/needed by secure world */
	path = "/ocp/crossbar";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		debug("Node %s not found.\n", path);
		return 0;
	}

	/* Get current entries */
	p_data = fdt_getprop(fdt, offs, "ti,irqs-skip", &len);
	if (p_data)
		old_cnt = len / sizeof(u32);
	else
		old_cnt = 0;

	new_cnt = sizeof(hs_irq_skip) /
				sizeof(hs_irq_skip[0]);

	/* Create new/updated skip list for HS parts */
	temp = malloc(sizeof(u32) * (old_cnt + new_cnt));
	for (i = 0; i < new_cnt; i++)
		temp[i] = cpu_to_fdt32(hs_irq_skip[i]);
	for (i = 0; i < old_cnt; i++)
		temp[i + new_cnt] = p_data[i];

	/* Blow away old data and set new data */
	fdt_delprop(fdt, offs, "ti,irqs-skip");
	ret = fdt_setprop(fdt, offs, "ti,irqs-skip",
			  temp,
			  (old_cnt + new_cnt) * sizeof(u32));
	free(temp);

	/* Check if the update worked */
	if (ret < 0) {
		printf("Could not add ti,irqs-skip property to node %s: %s\n",
		       path, fdt_strerror(ret));
		return ret;
	}

	return 0;
}

static int ft_hs_disable_rng(void *fdt, bd_t *bd)
{
	const char *path;
	int offs;
	int ret;

	/* Make HW RNG reserved for secure world use */
	path = "/ocp/rng";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		debug("Node %s not found.\n", path);
		return 0;
	}
	ret = fdt_setprop_string(fdt, offs,
				 "status", "disabled");
	if (ret < 0) {
		printf("Could not add status property to node %s: %s\n",
		       path, fdt_strerror(ret));
		return ret;
	}
	return 0;
}

#if ((TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ != 0) || \
    (CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ != 0))
static int ft_hs_fixup_sram(void *fdt, bd_t *bd)
{
	const char *path;
	int offs;
	int ret;
	u32 temp[2];

	/*
	 * Update SRAM reservations on secure devices. The OCMC RAM
	 * is always reserved for secure use from the start of that
	 * memory region
	 */
	path = "/ocp/ocmcram@40300000/sram-hs";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		debug("Node %s not found.\n", path);
		return 0;
	}

	/* relative start offset */
	temp[0] = cpu_to_fdt32(0);
	/* reservation size */
	temp[1] = cpu_to_fdt32(max(TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ,
				   CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ));
	fdt_delprop(fdt, offs, "reg");
	ret = fdt_setprop(fdt, offs, "reg", temp, 2 * sizeof(u32));
	if (ret < 0) {
		printf("Could not add reg property to node %s: %s\n",
		       path, fdt_strerror(ret));
		return ret;
	}

	return 0;
}
#else
static int ft_hs_fixup_sram(void *fdt, bd_t *bd) { return 0; }
#endif

static void ft_hs_fixups(void *fdt, bd_t *bd)
{
	/* Check we are running on an HS/EMU device type */
	if (GP_DEVICE != get_device_type()) {
		if ((ft_hs_fixup_crossbar(fdt, bd) == 0) &&
		    (ft_hs_disable_rng(fdt, bd) == 0) &&
		    (ft_hs_fixup_sram(fdt, bd) == 0))
			return;
	} else {
		printf("ERROR: Incorrect device type (GP) detected!");
	}
	/* Fixup failed or wrong device type */
	hang();
}
#else
static void ft_hs_fixups(void *fdt, bd_t *bd)
{
}
#endif

/*
 * Place for general cpu/SoC FDT fixups. Board specific
 * fixups should remain in the board files which is where
 * this function should be called from.
 */
void ft_cpu_setup(void *fdt, bd_t *bd)
{
	ft_hs_fixups(fdt, bd);
}
