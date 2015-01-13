/*
 * Copyright 2014 - Hans de Goede <hdegoede@redhat.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _SUNXI_I2C_H_
#define _SUNXI_I2C_H_

#include <asm/arch/cpu.h>

#define CONFIG_I2C_MVTWSI_BASE	SUNXI_TWI0_BASE
/* This is abp0-clk on sun4i/5i/7i / abp1-clk on sun6i/sun8i which is 24MHz */
#define CONFIG_SYS_TCLK		24000000

#endif
