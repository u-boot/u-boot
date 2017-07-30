/*
 * (C) Copyright 2017
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/bitops.h>
#include <asm/armv7m.h>
#include <asm/armv7m_mpu.h>
#include <asm/io.h>

#define V7M_MPU_CTRL_ENABLE		(1 << 0)
#define V7M_MPU_CTRL_DISABLE		(0 << 0)
#define V7M_MPU_CTRL_HFNMIENA		(1 << 1)
#define VALID_REGION			(1 << 4)

#define ENABLE_REGION			(1 << 0)

#define AP_SHIFT			24
#define XN_SHIFT			28
#define TEX_SHIFT			19
#define S_SHIFT				18
#define C_SHIFT				17
#define B_SHIFT				16
#define REGION_SIZE_SHIFT		1

#define CACHEABLE			(1 << C_SHIFT)
#define BUFFERABLE			(1 << B_SHIFT)
#define SHAREABLE			(1 << S_SHIFT)

void disable_mpu(void)
{
	writel(0, &V7M_MPU->ctrl);
}

void enable_mpu(void)
{
	writel(V7M_MPU_CTRL_ENABLE | V7M_MPU_CTRL_HFNMIENA, &V7M_MPU->ctrl);

	/* Make sure new mpu config is effective for next memory access */
	dsb();
	isb();	/* Make sure instruction stream sees it */
}

void mpu_config(struct mpu_region_config *reg_config)
{
	uint32_t attr;

	switch (reg_config->mr_attr) {
	case STRONG_ORDER:
		attr = SHAREABLE;
		break;
	case SHARED_WRITE_BUFFERED:
		attr = BUFFERABLE;
		break;
	case O_I_WT_NO_WR_ALLOC:
		attr = CACHEABLE;
		break;
	case O_I_WB_NO_WR_ALLOC:
		attr = CACHEABLE | BUFFERABLE;
		break;
	case O_I_NON_CACHEABLE:
		attr = 1 << TEX_SHIFT;
		break;
	case O_I_WB_RD_WR_ALLOC:
		attr = (1 << TEX_SHIFT) | CACHEABLE | BUFFERABLE;
		break;
	case DEVICE_NON_SHARED:
		attr = (2 << TEX_SHIFT) | BUFFERABLE;
		break;
	default:
		attr = 0; /* strongly ordered */
		break;
	};

	writel(reg_config->start_addr | VALID_REGION | reg_config->region_no,
	       &V7M_MPU->rbar);

	writel(reg_config->xn << XN_SHIFT | reg_config->ap << AP_SHIFT | attr
		| reg_config->reg_size << REGION_SIZE_SHIFT | ENABLE_REGION
	       , &V7M_MPU->rasr);
}
