/*
 * SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2021 Arm Limited
 */

#ifndef _ASM_ARMV8_MPU_H_
#define _ASM_ARMV8_MPU_H_

#include <asm/armv8/mmu.h>
#include <asm/barriers.h>
#include <linux/stringify.h>

#define PRSELR_EL2		S3_4_c6_c2_1
#define PRBAR_EL2		S3_4_c6_c8_0
#define PRLAR_EL2		S3_4_c6_c8_1
#define MPUIR_EL2		S3_4_c0_c0_4

#define PRBAR_ADDRESS(addr)	((addr) & ~(0x3fULL))

/* Access permissions */
#define PRBAR_AP(val)		(((val) & 0x3) << 2)
#define PRBAR_AP_RW_HYP		PRBAR_AP(0x0)
#define PRBAR_AP_RW_ANY		PRBAR_AP(0x1)
#define PRBAR_AP_RO_HYP		PRBAR_AP(0x2)
#define PRBAR_AP_RO_ANY		PRBAR_AP(0x3)

/* Shareability */
#define PRBAR_SH(val)		(((val) & 0x3) << 4)
#define PRBAR_NON_SH		PRBAR_SH(0x0)
#define PRBAR_OUTER_SH		PRBAR_SH(0x2)
#define PRBAR_INNER_SH		PRBAR_SH(0x3)

/* Memory attribute (MAIR idx) */
#define PRLAR_ATTRIDX(val)	(((val) & 0x7) << 1)
#define PRLAR_EN_BIT		(0x1)
#define PRLAR_ADDRESS(addr)	((addr) & ~(0x3fULL))

#ifndef __ASSEMBLY__

static inline void setup_el2_mpu_region(uint8_t region, uint64_t base, uint64_t limit)
{
	asm volatile("msr " __stringify(PRSELR_EL2) ", %0" : : "r" (region));
	isb();
	asm volatile("msr " __stringify(PRBAR_EL2) ", %0" : : "r" (base));
	asm volatile("msr " __stringify(PRLAR_EL2) ", %0" : : "r" (limit));
	dsb();
	isb();
}

#endif

struct mpu_region {
	u64 start;
	u64 end;
	u64 attrs;
};

extern struct mpu_region *mpu_mem_map;

#endif /* _ASM_ARMV8_MPU_H_ */
