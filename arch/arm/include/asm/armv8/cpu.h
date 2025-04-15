/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#define MIDR_PARTNUM_CORTEX_A35		0xD04
#define MIDR_PARTNUM_CORTEX_A53		0xD03
#define MIDR_PARTNUM_CORTEX_A55		0xD05
#define MIDR_PARTNUM_CORTEX_A57		0xD07
#define MIDR_PARTNUM_CORTEX_A72		0xD08
#define MIDR_PARTNUM_CORTEX_A73		0xD09
#define MIDR_PARTNUM_CORTEX_A75		0xD0A
#define MIDR_PARTNUM_CORTEX_A76		0xD0B
#define MIDR_PARTNUM_SHIFT		0x4
#define MIDR_PARTNUM_MASK		(0xFFF << MIDR_PARTNUM_SHIFT)

static inline unsigned int read_midr(void)
{
	unsigned long val;

	asm volatile("mrs %0, midr_el1" : "=r" (val));

	return val;
}

#define is_cortex_a(__n)					\
	static inline int is_cortex_a##__n(void)		\
	{							\
		unsigned int midr = read_midr();		\
		midr &= MIDR_PARTNUM_MASK;			\
		midr >>= MIDR_PARTNUM_SHIFT;			\
		return midr == MIDR_PARTNUM_CORTEX_A##__n;	\
	}

is_cortex_a(35)
is_cortex_a(53)
is_cortex_a(55)
is_cortex_a(57)
is_cortex_a(72)
is_cortex_a(73)
is_cortex_a(75)
is_cortex_a(76)
