/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv7m.h>
#include <asm/arch/stm32.h>

u32 get_cpu_rev(void)
{
	return 0;
}

int arch_cpu_init(void)
{
	/*
		* Configure the memory protection unit (MPU)
		* 0x00000000 - 0xffffffff: Strong-order, Shareable
		* 0xC0000000 - 0xC0800000: Normal, Outer and inner Non-cacheable
	 */

	 /* Disable MPU */
	 writel(0, &V7M_MPU->ctrl);

	 writel(
		 0x00000000 /* address */
		 | 1 << 4	/* VALID */
		 | 0 << 0	/* REGION */
		 , &V7M_MPU->rbar
	 );

	 /* Strong-order, Shareable */
	 /* TEX=000, S=1, C=0, B=0*/
	 writel(
		 (V7M_MPU_RASR_XN_ENABLE
			 | V7M_MPU_RASR_AP_RW_RW
			 | 0x01 << V7M_MPU_RASR_S_SHIFT
			 | 0x00 << V7M_MPU_RASR_TEX_SHIFT
			 | V7M_MPU_RASR_SIZE_4GB
			 | V7M_MPU_RASR_EN)
		 , &V7M_MPU->rasr
	 );

	 writel(
		 0xC0000000 /* address */
		 | 1 << 4	/* VALID */
		 | 1 << 0	/* REGION */
		 , &V7M_MPU->rbar
	 );

	 /* Normal, Outer and inner Non-cacheable */
	 /* TEX=001, S=0, C=0, B=0*/
	 writel(
		 (V7M_MPU_RASR_XN_ENABLE
			 | V7M_MPU_RASR_AP_RW_RW
			 | 0x01 << V7M_MPU_RASR_TEX_SHIFT
			 | 0x01 << V7M_MPU_RASR_B_SHIFT
			 | 0x01 << V7M_MPU_RASR_C_SHIFT
			 | V7M_MPU_RASR_SIZE_8MB
			 | V7M_MPU_RASR_EN)
			 , &V7M_MPU->rasr
	 );

	 /* Enable MPU */
	 writel(V7M_MPU_CTRL_ENABLE | V7M_MPU_CTRL_HFNMIENA, &V7M_MPU->ctrl);

	return 0;
}

void s_init(void)
{
}
