// SPDX-License-Identifier: GPL-2.0
/*
 * R-Car Gen4 SPL
 *
 * Copyright (C) 2024 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <asm/arch/renesas.h>
#include <asm/io.h>
#include <cpu_func.h>
#include <dm/uclass.h>
#include <dm/util.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <linux/bitops.h>
#include <log.h>
#include <mapmem.h>
#include <spl.h>

#define CNTCR_EN	BIT(0)

#ifdef CONFIG_SPL_BUILD
void board_debug_uart_init(void)
{
}
#endif

static void init_generic_timer(void)
{
	const u32 freq = CONFIG_SYS_CLK_FREQ;

	/* Update memory mapped and register based freqency */
	if (IS_ENABLED(CONFIG_ARM64))
		asm volatile("msr cntfrq_el0, %0" :: "r" (freq));
	else
		asm volatile("mcr p15, 0, %0, c14, c0, 0" :: "r" (freq));

	writel(freq, CNTFID0);

	/* Enable counter */
	setbits_le32(CNTCR_BASE, CNTCR_EN);
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	if (CONFIG_IS_ENABLED(OF_CONTROL)) {
		ret = spl_early_init();
		if (ret) {
			debug("spl_early_init() failed: %d\n", ret);
			hang();
		}
	}

	preloader_console_init();

	ret = uclass_get_device_by_name(UCLASS_NOP, "ram@e6780000", &dev);
	if (ret)
		printf("DBSC5 init failed: %d\n", ret);

	ret = uclass_get_device_by_name(UCLASS_RAM, "ram@ffec0000", &dev);
	if (ret)
		printf("RTVRAM init failed: %d\n", ret);
};

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_SPI;
}

struct legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return map_sysmem(CONFIG_SYS_LOAD_ADDR + offset, 0);
}

#define APMU_BASE 0xe6170000U
#define CL0GRP3_BIT			BIT(3)
#define CL1GRP3_BIT			BIT(7)
#define RTGRP3_BIT			BIT(19)
#define APMU_ACC_ENB_FOR_ARM_CPU	(CL0GRP3_BIT | CL1GRP3_BIT | RTGRP3_BIT)

void s_init(void)
{
	/* Unlock CPG access */
	writel(0x5A5AFFFF, CPGWPR);
	writel(0xA5A50000, CPGWPCR);
	init_generic_timer();

	/* Define for Work Around of APMU */
	writel(0x00ff00ff, APMU_BASE + 0x10);
	writel(0x00ff00ff, APMU_BASE + 0x14);
	writel(0x00ff00ff, APMU_BASE + 0x18);
	writel(0x00ff00ff, APMU_BASE + 0x1c);
	clrbits_le32(APMU_BASE + 0x68, BIT(29));
}

void reset_cpu(void)
{
}
