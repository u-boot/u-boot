// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 */

#include <linux/types.h>
#include <common.h>
#include <asm/io.h>
#include <asm/bootm.h>
#include <mach/sdhci.h>
#include <mach/timer.h>
#include <mmc.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

#define BCMSTB_DATA_SECTION __attribute__((section(".data")))

struct bcmstb_boot_parameters bcmstb_boot_parameters BCMSTB_DATA_SECTION;

phys_addr_t prior_stage_fdt_address BCMSTB_DATA_SECTION;

union reg_value_union {
	const char *data;
	const phys_addr_t *address;
};

int board_init(void)
{
	return 0;
}

u32 get_board_rev(void)
{
	return 0;
}

void reset_cpu(ulong ignored)
{
}

int print_cpuinfo(void)
{
	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	/*
	 * On this SoC, U-Boot is running as an ELF file.  Change the
	 * relocation address to CONFIG_SYS_TEXT_BASE, so that in
	 * setup_reloc, gd->reloc_off works out to 0, effectively
	 * disabling relocation.  Otherwise U-Boot hangs in the setup
	 * instructions just before relocate_code in
	 * arch/arm/lib/crt0.S.
	 */
	gd->relocaddr = CONFIG_SYS_TEXT_BASE;

	return 0;
}

void enable_caches(void)
{
	/*
	 * This port assumes that the prior stage bootloader has
	 * enabled I-cache and D-cache already.  Implementing this
	 * function silences the warning in the default function.
	 */
}

static const phys_addr_t bcmstb_sdhci_address(u32 alias_index)
{
	int node = 0;
	int ret = 0;
	char sdhci[16] = { 0 };
	const void *fdt = gd->fdt_blob;
	const char *path = NULL;
	struct fdt_resource resource = { 0 };

	if (!fdt) {
		printf("%s: Invalid gd->fdt_blob\n", __func__);
		return 0;
	}

	node = fdt_path_offset(fdt, "/aliases");
	if (node < 0) {
		printf("%s: Failed to find /aliases node\n", __func__);
		return 0;
	}

	sprintf(sdhci, "sdhci%d", alias_index);
	path = fdt_getprop(fdt, node, sdhci, NULL);
	if (!path) {
		printf("%s: Failed to find alias for %s\n", __func__, sdhci);
		return 0;
	}

	node = fdt_path_offset(fdt, path);
	if (node < 0) {
		printf("%s: Failed to resolve BCMSTB SDHCI alias\n", __func__);
		return 0;
	}

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "host", &resource);
	if (ret) {
		printf("%s: Failed to read BCMSTB SDHCI host resource\n",
		       __func__);
		return 0;
	}

	return resource.start;
}

int board_mmc_init(bd_t *bis)
{
	phys_addr_t sdhci_base_address = 0;

	sdhci_base_address = bcmstb_sdhci_address(CONFIG_BCMSTB_SDHCI_INDEX);

	if (!sdhci_base_address) {
		sdhci_base_address = BCMSTB_SDHCI_BASE;
		printf("%s: Assuming BCMSTB SDHCI address: 0x%p\n",
		       __func__, (void *)sdhci_base_address);
	}

	debug("BCMSTB SDHCI base address: 0x%p\n", (void *)sdhci_base_address);

	bcmstb_sdhci_init(sdhci_base_address);

	return 0;
}

int timer_init(void)
{
	gd->arch.timer_rate_hz = readl(BCMSTB_TIMER_FREQUENCY);

	return 0;
}

ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}

uint64_t get_ticks(void)
{
	gd->timebase_h = readl(BCMSTB_TIMER_HIGH);
	gd->timebase_l = readl(BCMSTB_TIMER_LOW);

	return ((uint64_t)gd->timebase_h << 32) | gd->timebase_l;
}

int board_late_init(void)
{
	debug("Arguments from prior stage bootloader:\n");
	debug("General Purpose Register 0: 0x%x\n", bcmstb_boot_parameters.r0);
	debug("General Purpose Register 1: 0x%x\n", bcmstb_boot_parameters.r1);
	debug("General Purpose Register 2: 0x%x\n", bcmstb_boot_parameters.r2);
	debug("General Purpose Register 3: 0x%x\n", bcmstb_boot_parameters.r3);
	debug("Stack Pointer Register:     0x%x\n", bcmstb_boot_parameters.sp);
	debug("Link Register:              0x%x\n", bcmstb_boot_parameters.lr);
	debug("Assuming timer frequency register at: 0x%p\n",
	      (void *)BCMSTB_TIMER_FREQUENCY);
	debug("Read timer frequency (in Hz): %ld\n", gd->arch.timer_rate_hz);
	debug("Prior stage provided DTB at: 0x%p\n",
	      (void *)prior_stage_fdt_address);

	/*
	 * Set fdtcontroladdr in the environment so that scripts can
	 * refer to it, for example, to reuse it for fdtaddr.
	 */
	env_set_hex("fdtcontroladdr", prior_stage_fdt_address);

	/*
	 * Do not set machid to the machine identifier value provided
	 * by the prior stage bootloader (bcmstb_boot_parameters.r1)
	 * because we're using a device tree to boot Linux.
	 */

	return 0;
}
