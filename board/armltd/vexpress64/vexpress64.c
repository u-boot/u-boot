// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 * Sharma Bhupesh <bhupesh.sharma@freescale.com>
 */
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <init.h>
#include <malloc.h>
#include <errno.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <dm/platform_data/serial_pl01x.h>
#include "pcie.h"
#include <asm/armv8/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pl01x_serial_platdata serial_platdata = {
	.base = V2M_UART0,
	.type = TYPE_PL011,
	.clock = CONFIG_PL011_CLOCK,
};

U_BOOT_DEVICE(vexpress_serials) = {
	.name = "serial_pl01x",
	.platdata = &serial_platdata,
};

static struct mm_region vexpress64_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0xff80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = vexpress64_mem_map;

/* This function gets replaced by platforms supporting PCIe.
 * The replacement function, eg. on Juno, initialises the PCIe bus.
 */
__weak void vexpress64_pcie_init(void)
{
}

int board_init(void)
{
	vexpress64_pcie_init();
	return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
#ifdef PHYS_SDRAM_2
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif

	return 0;
}

#ifdef CONFIG_OF_BOARD
#define JUNO_FLASH_SEC_SIZE	(256 * 1024)
static phys_addr_t find_dtb_in_nor_flash(const char *partname)
{
	phys_addr_t sector = CONFIG_SYS_FLASH_BASE;
	int i;

	for (i = 0;
	     i < CONFIG_SYS_MAX_FLASH_SECT;
	     i++, sector += JUNO_FLASH_SEC_SIZE) {
		int len = strlen(partname) + 1;
		int offs;
		phys_addr_t imginfo;
		u32 reg;

		reg = readl(sector + JUNO_FLASH_SEC_SIZE - 0x04);
                /* This makes up the string "HSLFTOOF" flash footer */
		if (reg != 0x464F4F54U)
			continue;
		reg = readl(sector + JUNO_FLASH_SEC_SIZE - 0x08);
                if (reg != 0x464C5348U)
			continue;

		for (offs = 0; offs < 32; offs += 4, len -= 4) {
			reg = readl(sector + JUNO_FLASH_SEC_SIZE - 0x30 + offs);
			if (strncmp(partname + offs, (char *)&reg,
			            len > 4 ? 4 : len))
				break;

			if (len > 4)
				continue;

			reg = readl(sector + JUNO_FLASH_SEC_SIZE - 0x10);
			imginfo = sector + JUNO_FLASH_SEC_SIZE - 0x30 - reg;
			reg = readl(imginfo + 0x54);

			return CONFIG_SYS_FLASH_BASE +
			       reg * JUNO_FLASH_SEC_SIZE;
		}
	}

	printf("No DTB found\n");

	return ~0;
}

void *board_fdt_blob_setup(void)
{
	phys_addr_t fdt_rom_addr = find_dtb_in_nor_flash(CONFIG_JUNO_DTB_PART);

	if (fdt_rom_addr == ~0UL)
		return NULL;

	return (void *)fdt_rom_addr;
}
#endif

/* Actual reset is done via PSCI. */
void reset_cpu(ulong addr)
{
}

/*
 * Board specific ethernet initialization routine.
 */
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC91111
	rc = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
