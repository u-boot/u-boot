// SPDX-License-Identifier: GPL-2.0+
/*
 * CAAM RNG self test for the i.MX6 chips whose boot ROM gets it wrong
 *
 * Copyright 2018, 2021 NXP
 * Copyright (C) 2026 Stefan Bigler, Securiton AG
 * Copyright (C) 2026 Heiko Schocher, Nabla Software Engineering <hs@nabladev.com>
 */

#include <cpu_func.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <vsprintf.h>
#include <asm/arch/sys_proto.h>
#include <linux/kernel.h>
#include "jr.h"

/* Number of words in the descriptor below */
#define RNG_DSC_SIZE		0x36

/* Size of the known answer the descriptor produces */
#define RNG_KAT_SIZE		32

/*
 * Descriptor of the self test, taken from the NXP tree. The word five
 * from the end holds the address the result is written to and is filled
 * in before the descriptor is submitted.
 */
#define RNG_DSC_RESULT_IDX	(RNG_DSC_SIZE - 5)

static const u32 rng_dsc[RNG_DSC_SIZE] = {
	0xb0800036, 0x04800010, 0x3c85a15b, 0x50a9d0b1,
	0x71a09fee, 0x2eecf20b, 0x02800020, 0xb267292e,
	0x85bf712d, 0xe85ff43a, 0xa716b7fb, 0xc40bb528,
	0x27b6f564, 0x8821cb5d, 0x9b5f6c26, 0x12a00020,
	0x0a20de17, 0x6529357e, 0x316277ab, 0x2846254e,
	0x34d23ba5, 0x6f5e9c32, 0x7abdc1bb, 0x0197a385,
	0x82500405, 0xa2000001, 0x10880004, 0x00000005,
	0x12820004, 0x00000020, 0x82500001, 0xa2000001,
	0x10880004, 0x40000045, 0x02800020, 0x8f389cc7,
	0xe7f7cbb0, 0x6bf2073d, 0xfc380b6d, 0xb22e9d1a,
	0xee64fcb7, 0xa2b48d49, 0xdf9bc3a4, 0x82500009,
	0xa2000001, 0x10880004, 0x00000005, 0x82500001,
	0x60340020, 0xFFFFFFFF, 0xa2000001, 0x10880004,
	0x00000005, 0x8250000d
};

static const u8 rng_result[RNG_KAT_SIZE] = {
	0x3a, 0xfe, 0x2c, 0x87, 0xcc, 0xb6, 0x44, 0x49,
	0x19, 0x16, 0x9a, 0x74, 0xa1, 0x31, 0x8b, 0xef,
	0xf4, 0x86, 0x0b, 0xb9, 0x5e, 0xee, 0xae, 0x91,
	0x92, 0xf4, 0xa9, 0x8f, 0xb0, 0x37, 0x18, 0xa4
};

/**
 * rng_self_test_jobdesc() - build the self test descriptor
 *
 * @desc:	descriptor to fill in, RNG_DSC_SIZE words
 * @result:	where the CAAM is to deposit the generated data
 */
static void rng_self_test_jobdesc(u32 *desc, u8 *result)
{
	uint i;

	for (i = 0; i < RNG_DSC_SIZE; i++)
		desc[i] = rng_dsc[i];

	desc[RNG_DSC_RESULT_IDX] = (uintptr_t)result;
}

/**
 * rng_test() - run the self test once and compare against the known answer
 *
 * A failure means the RNG cannot be trusted, so the board is stopped
 * rather than left to carry on with keys derived from it.
 */
static void rng_test(void)
{
	u8 *result;
	u32 *desc;
	uint size;
	int ret, i;

	result = memalign(ARCH_DMA_MINALIGN, RNG_KAT_SIZE);
	if (!result) {
		puts("RNG:   not enough memory for the result\n");
		return;
	}

	desc = memalign(ARCH_DMA_MINALIGN, sizeof(u32) * RNG_DSC_SIZE);
	if (!desc) {
		puts("RNG:   not enough memory for the descriptor\n");
		free(result);
		return;
	}

	rng_self_test_jobdesc(desc, result);

	size = roundup(sizeof(u32) * RNG_DSC_SIZE, ARCH_DMA_MINALIGN);
	flush_dcache_range((ulong)desc, (ulong)desc + size);
	size = roundup(RNG_KAT_SIZE, ARCH_DMA_MINALIGN);
	flush_dcache_range((ulong)result, (ulong)result + size);

	ret = run_descriptor_jr(desc);
	if (ret) {
		printf("RNG:   self test descriptor failed: %d\n", ret);
		goto err;
	}

	invalidate_dcache_range((ulong)result, (ulong)result + size);

	for (i = 0; i < RNG_KAT_SIZE; i++)
		debug("%02x", result[i]);
	debug("\n");

	if (memcmp(result, rng_result, RNG_KAT_SIZE))
		goto err;

	free(desc);
	free(result);
	puts("RNG:   self test passed\n");

	return;

err:
	free(desc);
	free(result);
	panic("RNG:   self test failed\n");
}

/**
 * rng_self_test_needed() - is this chip one of the affected ones?
 *
 * Chip and revision both have to match one of the pairs that NXP names
 * as impacted, and no other revision of those chips is on that list.
 */
static bool rng_self_test_needed(void)
{
	if (is_cpu_type(MXC_CPU_MX6QP))
		return soc_rev() == CHIP_REV_1_1;

	if (is_cpu_type(MXC_CPU_MX6Q))
		return soc_rev() == CHIP_REV_1_6;

	if (is_cpu_type(MXC_CPU_MX6SOLO) || is_cpu_type(MXC_CPU_MX6DL) ||
	    is_cpu_type(MXC_CPU_MX6SX))
		return soc_rev() == CHIP_REV_1_4;

	return false;
}

void rng_self_test(void)
{
	if (rng_self_test_needed())
		rng_test();
}
