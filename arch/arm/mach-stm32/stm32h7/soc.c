/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv7m_mpu.h>

u32 get_cpu_rev(void)
{
	return 0;
}

int arch_cpu_init(void)
{
	int i;

	struct mpu_region_config stm32_region_config[] = {
		/*
		 * Make all 4GB cacheable & executable. We are overriding it
		 * with next region for any requirement. e.g. below region1,
		 * 2 etc.
		 * In other words, the area not coming in following
		 * regions configuration is the one configured here in region_0
		 * (cacheable & executable).
		 */
		{ 0x00000000, REGION_0, XN_DIS, PRIV_RW_USR_RW,
		O_I_WB_RD_WR_ALLOC, REGION_4GB },

		/* Code area, executable & strongly ordered */
		{ 0xD0000000, REGION_1, XN_EN, PRIV_RW_USR_RW,
		STRONG_ORDER, REGION_8MB },

		/* Device area in all H7 : Not executable */
		{ 0x40000000, REGION_2, XN_EN, PRIV_RW_USR_RW,
		DEVICE_NON_SHARED, REGION_512MB },

		/*
		 * Armv7m fixed configuration: strongly ordered & not
		 * executable, not cacheable
		 */
		{ 0xE0000000, REGION_4, XN_EN, PRIV_RW_USR_RW,
		STRONG_ORDER, REGION_512MB },
	};

	disable_mpu();
	for (i = 0; i < ARRAY_SIZE(stm32_region_config); i++)
		mpu_config(&stm32_region_config[i]);
	enable_mpu();

	return 0;
}

void s_init(void)
{
}
