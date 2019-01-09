// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <environment.h>
#include <spi.h>

DECLARE_GLOBAL_DATA_PTR;

#define MSCC_GPIO_ALT0		0x54
#define MSCC_GPIO_ALT1		0x58

void external_cs_manage(struct udevice *dev, bool enable)
{
	u32 cs = spi_chip_select(dev);
	/* IF_SI0_OWNER, select the owner of the SI interface
	 * Encoding: 0: SI Slave
	 *           1: SI Boot Master
	 *           2: SI Master Controller
	 */
	if (!enable) {
		writel(ICPU_SW_MODE_SW_PIN_CTRL_MODE |
		       ICPU_SW_MODE_SW_SPI_CS(BIT(cs)), BASE_CFG + ICPU_SW_MODE);
		clrsetbits_le32(BASE_CFG + ICPU_GENERAL_CTRL,
				ICPU_GENERAL_CTRL_IF_SI_OWNER_M,
				ICPU_GENERAL_CTRL_IF_SI_OWNER(2));
	} else {
		writel(0, BASE_CFG + ICPU_SW_MODE);
		clrsetbits_le32(BASE_CFG + ICPU_GENERAL_CTRL,
				ICPU_GENERAL_CTRL_IF_SI_OWNER_M,
				ICPU_GENERAL_CTRL_IF_SI_OWNER(1));
	}
}

void board_debug_uart_init(void)
{
	/* too early for the pinctrl driver, so configure the UART pins here */
	setbits_le32(BASE_DEVCPU_GCB + MSCC_GPIO_ALT0, BIT(6) | BIT(7));
	clrbits_le32(BASE_DEVCPU_GCB + MSCC_GPIO_ALT1, BIT(6) | BIT(7));
}

int board_early_init_r(void)
{
	/* Prepare SPI controller to be used in master mode */
	writel(0, BASE_CFG + ICPU_SW_MODE);
	clrsetbits_le32(BASE_CFG + ICPU_GENERAL_CTRL,
			ICPU_GENERAL_CTRL_IF_SI_OWNER_M,
			ICPU_GENERAL_CTRL_IF_SI_OWNER(2));

	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE;
	return 0;
}
