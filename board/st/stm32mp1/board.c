// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <power/pmic.h>
#include <power/stpmic1.h>

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
#if (CONFIG_DEBUG_UART_BASE == STM32_UART4_BASE)

#define RCC_MP_APB1ENSETR (STM32_RCC_BASE + 0x0A00)
#define RCC_MP_AHB4ENSETR (STM32_RCC_BASE + 0x0A28)

	/* UART4 clock enable */
	setbits_le32(RCC_MP_APB1ENSETR, BIT(16));

#define GPIOG_BASE 0x50008000
	/* GPIOG clock enable */
	writel(BIT(6), RCC_MP_AHB4ENSETR);
	/* GPIO configuration for EVAL board
	 * => Uart4 TX = G11
	 */
	writel(0xffbfffff, GPIOG_BASE + 0x00);
	writel(0x00006000, GPIOG_BASE + 0x24);
#else

#error("CONFIG_DEBUG_UART_BASE: not supported value")

#endif
}
#endif

#ifdef CONFIG_PMIC_STPMIC1
int board_ddr_power_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(pmic_stpmic1), &dev);
	if (ret)
		/* No PMIC on board */
		return 0;

	/* VTT = Set LDO3 to sync mode */
	ret = pmic_reg_read(dev, STPMIC1_LDOX_CTRL_REG(STPMIC1_LDO3));
	if (ret < 0)
		return ret;

	ret &= ~STPMIC1_LDO3_MODE;
	ret &= ~STPMIC1_LDO12356_OUTPUT_MASK;
	ret |= STPMIC1_LDO3_DDR_SEL << STPMIC1_LDO12356_OUTPUT_SHIFT;

	ret = pmic_reg_write(dev, STPMIC1_LDOX_CTRL_REG(STPMIC1_LDO3),
			     ret);
	if (ret < 0)
		return ret;

	/* VDD_DDR = Set BUCK2 to 1.35V */
	ret = pmic_clrsetbits(dev,
			      STPMIC1_BUCKX_CTRL_REG(STPMIC1_BUCK2),
			      STPMIC1_BUCK_OUTPUT_MASK,
			      STPMIC1_BUCK2_1350000V);
	if (ret < 0)
		return ret;

	/* Enable VDD_DDR = BUCK2 */
	ret = pmic_clrsetbits(dev,
			      STPMIC1_BUCKX_CTRL_REG(STPMIC1_BUCK2),
			      STPMIC1_BUCK_EN, STPMIC1_BUCK_EN);
	if (ret < 0)
		return ret;

	mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

	/* Enable VREF */
	ret = pmic_clrsetbits(dev, STPMIC1_VREF_CTRL_REG,
			      STPMIC1_VREF_EN, STPMIC1_VREF_EN);
	if (ret < 0)
		return ret;

	mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

	/* Enable LDO3 */
	ret = pmic_clrsetbits(dev,
			      STPMIC1_LDOX_CTRL_REG(STPMIC1_LDO3),
			      STPMIC1_LDO_EN, STPMIC1_LDO_EN);
	if (ret < 0)
		return ret;

	mdelay(STPMIC1_DEFAULT_START_UP_DELAY_MS);

	return 0;
}
#endif
