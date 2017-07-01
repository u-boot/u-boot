/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv7m_mpu.h>
#include <asm/arch/stm32.h>

u32 get_cpu_rev(void)
{
	return 0;
}

int arch_cpu_init(void)
{
	int i;

	struct mpu_region_config stm32_region_config[] = {
		{ 0x00000000, REGION_0, XN_DIS, PRIV_RW_USR_RW,
		O_I_WB_RD_WR_ALLOC, REGION_4GB },

		{ 0x00000000, REGION_1, XN_DIS, PRIV_RW_USR_RW,
		STRONG_ORDER, REGION_512MB },

		{ 0x40000000, REGION_2, XN_EN, PRIV_RW_USR_RW,
		DEVICE_NON_SHARED, REGION_512MB },

		{ 0xA0000000, REGION_3, XN_EN, PRIV_RW_USR_RW,
		DEVICE_NON_SHARED, REGION_512MB },

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
