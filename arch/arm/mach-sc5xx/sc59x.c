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
#include <asm/arch-adi/sc5xx/sc5xx.h>
#include <asm/arch-adi/sc5xx/spl.h>

#define REG_SPU0_SECUREC0 0x3108B980
#define REG_PADS0_PCFG0 0x31004604
#define REG_SPU0_SECUREP_START 0x3108BA00
#define REG_SPU0_SECUREP_END 0x3108BD24

#define REG_SCB5_SPI2_OSPI_REMAP        0x30400000
#define BITM_SCB5_SPI2_OSPI_REMAP_REMAP 0x00000003
#define ENUM_SCB5_SPI2_OSPI_REMAP_OSPI0 0x00000001

adi_rom_boot_fn adi_rom_boot = (adi_rom_boot_fn)0x000000e9;

void sc5xx_enable_rgmii(void)
{
	writel((readl(REG_PADS0_PCFG0) | 0xc), REG_PADS0_PCFG0);
}

void sc59x_remap_ospi(void)
{
	clrsetbits_le32(REG_SCB5_SPI2_OSPI_REMAP,
			BITM_SCB5_SPI2_OSPI_REMAP_REMAP,
			ENUM_SCB5_SPI2_OSPI_REMAP_OSPI0);
}

void sc5xx_soc_init(void)
{
	sc5xx_enable_ns_sharc_access(REG_SPU0_SECUREC0);
	sc5xx_disable_spu0(REG_SPU0_SECUREP_START, REG_SPU0_SECUREP_END);
	sc5xx_enable_pmu();
}
