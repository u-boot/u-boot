// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2024 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#include <asm/io.h>
#include <asm/armv8/mmu.h>
#include <asm/arch-adi/sc5xx/sc5xx.h>
#include <asm/arch-adi/sc5xx/spl.h>

#define REG_TSGENWR0_CNTCR 0x310AE000
#define REG_PADS0_PCFG0 0x31004604
#define REG_RCU0_BCODE 0x3108C028

#define REG_SPU0_SECUREP_START 0x3108BA00
#define REG_SPU0_WP_START 0x3108B400
#define REG_SPU0_SECUREC0 0x3108B980

#define REG_SCB5_SPI2_OSPI_REMAP 0x30400000
#define BITM_SCB5_SPI2_OSPI_REMAP_REMAP 0x00000003
#define ENUM_SCB5_SPI2_OSPI_REMAP_OSPI0 0x00000001

static struct mm_region sc598_mem_map[] = {
	{
		/* Peripherals */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* DDR */
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = sc598_mem_map;

adi_rom_boot_fn adi_rom_boot = (adi_rom_boot_fn)0x000000e4;

void sc5xx_enable_rgmii(void)
{
	writel((readl(REG_PADS0_PCFG0) | 0xc), REG_PADS0_PCFG0);

	// Set dw for little endian operation as well
	writel(readl(REG_PADS0_PCFG0) & ~(1 << 19), REG_PADS0_PCFG0);
	writel(readl(REG_PADS0_PCFG0) & ~(1 << 20), REG_PADS0_PCFG0);
}

void sc59x_remap_ospi(void)
{
	clrsetbits_le32(REG_SCB5_SPI2_OSPI_REMAP,
			BITM_SCB5_SPI2_OSPI_REMAP_REMAP,
			ENUM_SCB5_SPI2_OSPI_REMAP_OSPI0);
}

/**
 * SPU/SMPU configuration is the default for permissive access from non-secure
 * EL1. If TFA and OPTEE are configured, they run *after* this code, as the
 * current boot flow is SPL -> TFA -> OPTEE -> Proper -> Linux, and will
 * be expected to configure peripheral security correctly. If they are not
 * configured, then this permissive setting will allow Linux (which always
 * runs in NS EL1) to control all access to these peripherals. Without it,
 * the peripherals would simply be unavailable in a non-security build,
 * which is not OK.
 */
void sc5xx_soc_init(void)
{
	phys_addr_t smpus[] = {
		0x31007800, //SMPU0
		0x31083800, //SMPU2
		0x31084800, //SMPU3
		0x31085800, //SMPU4
		0x31086800, //SMPU5
		0x31087800, //SMPU6
		0x310A0800, //SMPU9
		0x310A1800, //SMPU11
		0x31012800, //SMPU12
	};
	size_t i;

	// Enable coresight timer
	writel(1, REG_TSGENWR0_CNTCR);

	//Do not rerun preboot routine --
	// Without this, hardware resets triggered by RCU0_CTL:SYSRST
	// lead to a deadlock somewhere in the boot ROM
	writel(0x200, REG_RCU0_BCODE);

	/* Alter outstanding transactions property of A55*/
	writel(0x1, 0x30643108); /* SCB6 A55 M0 Ib.fn Mod */
	isb();

	/* configure DDR prefetch behavior, per ADI */
	writel(0x1, 0x31076000);

	/* configure smart mode, per ADI */
	writel(0x1307, 0x31076004);

	// Disable SPU and SPU WP registers
	sc5xx_disable_spu0(REG_SPU0_SECUREP_START, REG_SPU0_SECUREP_START + 4*213);
	sc5xx_disable_spu0(REG_SPU0_WP_START, REG_SPU0_WP_START + 4*213);

	/* configure smpus permissively */
	for (i = 0; i < ARRAY_SIZE(smpus); ++i)
		writel(0x500, smpus[i]);

	sc5xx_enable_ns_sharc_access(REG_SPU0_SECUREC0);
}
