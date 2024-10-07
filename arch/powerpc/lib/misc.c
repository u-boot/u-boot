// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 Linaro Ltd.
 */

#include <image.h>
#include <init.h>
#include <lmb.h>

#include <asm/mp.h>
#include <dm/device.h>
#include <dm/uclass.h>

#ifndef CFG_SYS_LINUX_LOWMEM_MAX_SIZE
#define CFG_SYS_LINUX_LOWMEM_MAX_SIZE	(768 * 1024 * 1024)
#endif

int arch_misc_init(void)
{
	if (CONFIG_IS_ENABLED(CMD_BOOTM)) {
		phys_size_t bootm_size;
		ulong size, bootmap_base;

		bootmap_base = env_get_bootm_low();
		bootm_size = env_get_bootm_size();

#ifdef DEBUG
		if (((u64)bootmap_base + bootm_size) >
		    (CFG_SYS_SDRAM_BASE + (u64)gd->ram_size))
			puts("WARNING: bootm_low + bootm_size exceed total memory\n");
		if ((bootmap_base + bootm_size) > get_effective_memsize())
			puts("WARNING: bootm_low + bootm_size exceed eff. memory\n");
#endif

		size = min(bootm_size, get_effective_memsize());
		size = min(size, (ulong)CFG_SYS_LINUX_LOWMEM_MAX_SIZE);

		if (size < bootm_size) {
			ulong base = bootmap_base + size;

			printf("WARNING: adjusting available memory from 0x%lx to 0x%llx\n",
			       size, (unsigned long long)bootm_size);
			lmb_reserve(base, bootm_size - size);
		}

#ifdef CONFIG_MP
		cpu_mp_lmb_reserve();
#endif
	}

	if (IS_ENABLED(CONFIG_FSL_CAAM)) {
		struct udevice *dev;
		int ret;

		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(caam_jr), &dev);
		if (ret)
			printf("Failed to initialize caam_jr: %d\n", ret);
	}

	return 0;
}
