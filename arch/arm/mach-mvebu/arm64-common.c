// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>
#include <power/regulator.h>
#include <mach/fw_info.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Not all memory is mapped in the MMU. So we need to restrict the
 * memory size so that U-Boot does not try to access it. Also, the
 * internal registers are located at 0xf000.0000 - 0xffff.ffff.
 * Currently only 2GiB are mapped for system memory. This is what
 * we pass to the U-Boot subsystem here.
 */
#define USABLE_RAM_SIZE		0x80000000

ulong board_get_usable_ram_top(ulong total_size)
{
	if (gd->ram_size > USABLE_RAM_SIZE)
		return USABLE_RAM_SIZE;

	return gd->ram_size;
}

/*
 * On ARMv8, MBus is not configured in U-Boot. To enable compilation
 * of the already implemented drivers, lets add a dummy version of
 * this function so that linking does not fail.
 */
const struct mbus_dram_target_info *mvebu_mbus_dram_info(void)
{
	return NULL;
}

/* DRAM init code ... */

#define MV_SIP_DRAM_SIZE	0x82000010

static u64 a8k_dram_scan_ap_sz(void)
{
	struct pt_regs pregs;

	pregs.regs[0] = MV_SIP_DRAM_SIZE;
	pregs.regs[1] = SOC_REGS_PHY_BASE;
	smc_call(&pregs);

	return pregs.regs[0];
}

static void a8k_dram_init_banksize(void)
{
	/*
	 * The firmware (ATF) leaves a 1G whole above the 3G mark for IO
	 * devices. Higher RAM is mapped at 4G.
	 *
	 * Config 2 DRAM banks:
	 * Bank 0 - max size 4G - 1G
	 * Bank 1 - ram size - 4G + 1G
	 */
	phys_size_t max_bank0_size = SZ_4G - SZ_1G;

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	if (gd->ram_size <= max_bank0_size) {
		gd->bd->bi_dram[0].size = gd->ram_size;
		return;
	}

	gd->bd->bi_dram[0].size = max_bank0_size;
	if (CONFIG_NR_DRAM_BANKS > 1) {
		gd->bd->bi_dram[1].start = SZ_4G;
		gd->bd->bi_dram[1].size = gd->ram_size - max_bank0_size;
	}
}

static u64 a3700_dram_scan_ap_sz(void)
{
	struct pt_regs pregs;

	pregs.regs[0] = MV_SIP_DRAM_SIZE;
	smc_call(&pregs);

	return pregs.regs[0];
}

static void a3700_dram_init_banksize(void)
{
	/* If ddr size is below 2GB there is only one ddr bank used */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	if (gd->ram_size <= SZ_2G) {
		gd->bd->bi_dram[0].size = gd->ram_size;
		return;
	}

	/*
	 * If ddr size is above 2GB there is only one case 4GB but the firmware
	 * uses 4 decoding windows for describing it in way reflected below.
	 */
	gd->bd->bi_dram[0].size = SZ_2G;
	gd->bd->bi_dram[1].start = SZ_2G;
	gd->bd->bi_dram[1].size = SZ_1G;
	gd->bd->bi_dram[2].start = SZ_2G + SZ_1G;
	gd->bd->bi_dram[2].size = SZ_256M;
	gd->bd->bi_dram[3].start = 0xe0000000;
	gd->bd->bi_dram[3].size = SZ_128M;
}

__weak int dram_init_banksize(void)
{
	if (CONFIG_IS_ENABLED(ARMADA_8K))
		a8k_dram_init_banksize();
	else if (CONFIG_IS_ENABLED(ARMADA_3700))
		a3700_dram_init_banksize();
	else
		fdtdec_setup_memory_banksize();

	return 0;
}

__weak int dram_init(void)
{
	if (CONFIG_IS_ENABLED(ARMADA_8K))
		gd->ram_size = a8k_dram_scan_ap_sz();
	else if (CONFIG_IS_ENABLED(ARMADA_3700))
		gd->ram_size = a3700_dram_scan_ap_sz();

	if (gd->ram_size != 0)
		return 0;

	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int arch_cpu_init(void)
{
	/* Nothing to do (yet) */
	return 0;
}

int arch_early_init_r(void)
{
	struct udevice *dev;
	int ret;
	int i;

	printf("Running in RAM - U-Boot at: 0x%08lx\n", gd->relocaddr);
	printf("                 Env at:    0x%08lx\n", gd->env_addr);
	/*
	 * Loop over all MISC uclass drivers to call the comphy code
	 * and init all CP110 devices enabled in the DT
	 */
	i = 0;
	while (1) {
		/* Call the comphy code via the MISC uclass driver */
		ret = uclass_get_device(UCLASS_MISC, i++, &dev);

		/* We're done, once no further CP110 device is found */
		if (ret)
			break;
	}

	i = 0;
	while (1) {
		/* Call the pinctrl code via the PINCTRL uclass driver */
		ret = uclass_get_device(UCLASS_PINCTRL, i++, &dev);

		/* We're done, once no further CP110 device is found */
		if (ret)
			break;
	}

	/* Cause the SATA device to do its early init */
	uclass_first_device(UCLASS_AHCI, &dev);

#ifdef CONFIG_DM_PCI
	/* Trigger PCIe devices detection */
	pci_init();
#endif

	return 0;
}

void plat_do_sync(void)
{
	u32 far, el;

	el = current_el();

	if (el == 1)
		asm volatile("mrs %0, far_el1" : "=r" (far));
	else if (el == 2)
		asm volatile("mrs %0, far_el2" : "=r" (far));
	else
		asm volatile("mrs %0, far_el3" : "=r" (far));

	if (far >= ATF_REGION_START && far <= ATF_REGION_END) {
		pr_err("\n\tAttempt to access RT service or TEE region (addr: 0x%x, el%d)\n",
		       far, el);
		pr_err("\tDo not use address range 0x%x-0x%x\n\n",
		       ATF_REGION_START, ATF_REGION_END);
	}
}
