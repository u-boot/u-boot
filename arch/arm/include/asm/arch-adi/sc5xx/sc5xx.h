/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */
#ifndef ARCH_ADI_SC5XX_SC5XX_H
#define ARCH_ADI_SC5XX_SC5XX_H

#include <linux/types.h>

#define TWI0_CLKDIV                 0x31001400    // TWI0 SCL Clock Divider Register
#define TWI1_CLKDIV                 0x31001500    // TWI1 SCL Clock Divider Register
#define TWI2_CLKDIV                 0x31001600    // TWI2 SCL Clock Divider Register

const char *sc5xx_get_boot_mode(u32 *bmode);
void sc5xx_enable_rgmii(void);

void sc5xx_enable_ns_sharc_access(uintptr_t securec0_base);
void sc5xx_disable_spu0(uintptr_t spu0_start, uintptr_t spu0_end);
void sc5xx_enable_pmu(void);

/**
 * Per-SoC init function to be used to initialize hw-specific things. Examples:
 * enable PMU on armv7, enable coresight timer on armv8, etc.
 */
void sc5xx_soc_init(void);

/*
 * Reconfigure SPI memory map region for OSPI use. The adi-spi3 driver
 * does not use the memory map, while the OSPI driver requires it. Only
 * available on sc59x and sc59x-64
 */
void sc59x_remap_ospi(void);

#endif
