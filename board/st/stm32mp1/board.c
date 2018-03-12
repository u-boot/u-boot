/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier:	GPL-2.0+	BSD-3-Clause
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <power/pmic.h>
#include <power/stpmu1.h>

#ifdef CONFIG_PMIC_STPMU1
int board_ddr_power_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(pmic_stpmu1), &dev);
	if (ret)
		/* No PMIC on board */
		return 0;

	/* Set LDO3 to sync mode */
	ret = pmic_reg_read(dev, STPMU1_LDOX_CTRL_REG(STPMU1_LDO3));
	if (ret < 0)
		return ret;

	ret &= ~STPMU1_LDO3_MODE;
	ret &= ~STPMU1_LDO12356_OUTPUT_MASK;
	ret |= STPMU1_LDO3_DDR_SEL << STPMU1_LDO12356_OUTPUT_SHIFT;

	ret = pmic_reg_write(dev, STPMU1_LDOX_CTRL_REG(STPMU1_LDO3),
			     ret);
	if (ret < 0)
		return ret;

	/* Set BUCK2 to 1.35V */
	ret = pmic_clrsetbits(dev,
			      STPMU1_BUCKX_CTRL_REG(STPMU1_BUCK2),
			      STPMU1_BUCK_OUTPUT_MASK,
			      STPMU1_BUCK2_1350000V);
	if (ret < 0)
		return ret;

	/* Enable BUCK2 and VREF */
	ret = pmic_clrsetbits(dev,
			      STPMU1_BUCKX_CTRL_REG(STPMU1_BUCK2),
			      STPMU1_BUCK_EN, STPMU1_BUCK_EN);
	if (ret < 0)
		return ret;

	mdelay(STPMU1_DEFAULT_START_UP_DELAY_MS);

	ret = pmic_clrsetbits(dev, STPMU1_VREF_CTRL_REG,
			      STPMU1_VREF_EN, STPMU1_VREF_EN);
	if (ret < 0)
		return ret;

	mdelay(STPMU1_DEFAULT_START_UP_DELAY_MS);

	/* Enable LDO3 */
	ret = pmic_clrsetbits(dev,
			      STPMU1_LDOX_CTRL_REG(STPMU1_LDO3),
			      STPMU1_LDO_EN, STPMU1_LDO_EN);
	if (ret < 0)
		return ret;

	mdelay(STPMU1_DEFAULT_START_UP_DELAY_MS);

	return 0;
}
#endif
