// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <log.h>
#include <asm/arch/imx-regs.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/syscounter.h>
#include <asm/armv8/mmu.h>
#include <dm/uclass.h>
#include <env.h>
#include <env_internal.h>
#include <errno.h>
#include <fdt_support.h>
#include <linux/bitops.h>
#include <asm/setup.h>
#include <asm/bootm.h>
#include <asm/arch-imx/cpu.h>

DECLARE_GLOBAL_DATA_PTR;

struct rom_api *g_rom_api = (struct rom_api *)0x1980;

#ifdef CONFIG_ENV_IS_IN_MMC
__weak int board_mmc_get_env_dev(int devno)
{
	return devno; }

int mmc_get_env_dev(void)
{
	volatile gd_t *pgd = gd;
	int ret;
	u32 boot;
	u16 boot_type;
	u8 boot_instance;

	ret = g_rom_api->query_boot_infor(QUERY_BT_DEV, &boot,
					  ((uintptr_t)&boot) ^ QUERY_BT_DEV);
	set_gd(pgd);

	if (ret != ROM_API_OKAY) {
		puts("ROMAPI: failure at query_boot_info\n");
		return CONFIG_SYS_MMC_ENV_DEV;
	}

	boot_type = boot >> 16;
	boot_instance = (boot >> 8) & 0xff;

	debug("boot_type %d, instance %d\n", boot_type, boot_instance);

	/* If not boot from sd/mmc, use default value */
	if (boot_type != BOOT_TYPE_SD && boot_type != BOOT_TYPE_MMC)
		return env_get_ulong("mmcdev", 10, CONFIG_SYS_MMC_ENV_DEV);

	return board_mmc_get_env_dev(boot_instance);
}
#endif

u32 get_cpu_rev(void)
{
	return (MXC_CPU_IMX93 << 12) | CHIP_REV_1_0;
}

#define UNLOCK_WORD 0xD928C520 /* unlock word */
#define REFRESH_WORD 0xB480A602 /* refresh word */

static void disable_wdog(void __iomem *wdog_base)
{
	u32 val_cs = readl(wdog_base + 0x00);

	if (!(val_cs & 0x80))
		return;

	/* default is 32bits cmd */
	writel(REFRESH_WORD, (wdog_base + 0x04)); /* Refresh the CNT */

	if (!(val_cs & 0x800)) {
		writel(UNLOCK_WORD, (wdog_base + 0x04));
		while (!(readl(wdog_base + 0x00) & 0x800))
			;
	}
	writel(0x0, (wdog_base + 0x0C)); /* Set WIN to 0 */
	writel(0x400, (wdog_base + 0x08)); /* Set timeout to default 0x400 */
	writel(0x2120, (wdog_base + 0x00)); /* Disable it and set update */

	while (!(readl(wdog_base + 0x00) & 0x400))
		;
}

void init_wdog(void)
{
	u32 src_val;

	disable_wdog((void __iomem *)WDG3_BASE_ADDR);
	disable_wdog((void __iomem *)WDG4_BASE_ADDR);
	disable_wdog((void __iomem *)WDG5_BASE_ADDR);

	src_val = readl(0x54460018); /* reset mask */
	src_val &= ~0x1c;
	writel(src_val, 0x54460018);
}

static struct mm_region imx93_mem_map[] = {
	{
		/* ROM */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x100000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* OCRAM */
		.virt = 0x20480000UL,
		.phys = 0x20480000UL,
		.size = 0xA0000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* AIPS */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Flexible Serial Peripheral Interface */
		.virt = 0x28000000UL,
		.phys = 0x28000000UL,
		.size = 0x30000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* DRAM1 */
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = PHYS_SDRAM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* empty entrie to split table entry 5 if needed when TEEs are used */
		0,
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = imx93_mem_map;

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	mac[0] = 0x1;
	mac[1] = 0x2;
	mac[2] = 0x3;
	mac[3] = 0x4;
	mac[4] = 0x5;
	mac[5] = 0x6;
}

int print_cpuinfo(void)
{
	u32 cpurev;

	cpurev = get_cpu_rev();

	printf("CPU:   i.MX93 rev%d.%d\n", (cpurev & 0x000F0) >> 4, (cpurev & 0x0000F) >> 0);

	return 0;
}

int arch_misc_init(void)
{
	return 0;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	return 0;
}

int arch_cpu_init(void)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		/* Disable wdog */
		init_wdog();

		clock_init();
	}

	return 0;
}

int timer_init(void)
{
#ifdef CONFIG_SPL_BUILD
	struct sctr_regs *sctr = (struct sctr_regs *)SYSCNT_CTRL_BASE_ADDR;
	unsigned long freq = readl(&sctr->cntfid0);

	/* Update with accurate clock frequency */
	asm volatile("msr cntfrq_el0, %0" : : "r" (freq) : "memory");

	clrsetbits_le32(&sctr->cntcr, SC_CNTCR_FREQ0 | SC_CNTCR_FREQ1,
			SC_CNTCR_FREQ0 | SC_CNTCR_ENABLE | SC_CNTCR_HDBG);
#endif

	gd->arch.tbl = 0;
	gd->arch.tbu = 0;

	return 0;
}
