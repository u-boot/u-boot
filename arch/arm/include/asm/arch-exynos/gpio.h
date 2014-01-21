/*
 * (C) Copyright 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_GPIO_H
#define __ASM_ARCH_GPIO_H

#ifndef __ASSEMBLY__
struct s5p_gpio_bank {
	unsigned int	con;
	unsigned int	dat;
	unsigned int	pull;
	unsigned int	drv;
	unsigned int	pdn_con;
	unsigned int	pdn_pull;
	unsigned char	res1[8];
};

struct exynos4_gpio_part1 {
	struct s5p_gpio_bank a0;
	struct s5p_gpio_bank a1;
	struct s5p_gpio_bank b;
	struct s5p_gpio_bank c0;
	struct s5p_gpio_bank c1;
	struct s5p_gpio_bank d0;
	struct s5p_gpio_bank d1;
	struct s5p_gpio_bank e0;
	struct s5p_gpio_bank e1;
	struct s5p_gpio_bank e2;
	struct s5p_gpio_bank e3;
	struct s5p_gpio_bank e4;
	struct s5p_gpio_bank f0;
	struct s5p_gpio_bank f1;
	struct s5p_gpio_bank f2;
	struct s5p_gpio_bank f3;
};

struct exynos4_gpio_part2 {
	struct s5p_gpio_bank j0;
	struct s5p_gpio_bank j1;
	struct s5p_gpio_bank k0;
	struct s5p_gpio_bank k1;
	struct s5p_gpio_bank k2;
	struct s5p_gpio_bank k3;
	struct s5p_gpio_bank l0;
	struct s5p_gpio_bank l1;
	struct s5p_gpio_bank l2;
	struct s5p_gpio_bank y0;
	struct s5p_gpio_bank y1;
	struct s5p_gpio_bank y2;
	struct s5p_gpio_bank y3;
	struct s5p_gpio_bank y4;
	struct s5p_gpio_bank y5;
	struct s5p_gpio_bank y6;
	struct s5p_gpio_bank res1[80];
	struct s5p_gpio_bank x0;
	struct s5p_gpio_bank x1;
	struct s5p_gpio_bank x2;
	struct s5p_gpio_bank x3;
};

struct exynos4_gpio_part3 {
	struct s5p_gpio_bank z;
};

struct exynos4x12_gpio_part1 {
	struct s5p_gpio_bank a0;
	struct s5p_gpio_bank a1;
	struct s5p_gpio_bank b;
	struct s5p_gpio_bank c0;
	struct s5p_gpio_bank c1;
	struct s5p_gpio_bank d0;
	struct s5p_gpio_bank d1;
	struct s5p_gpio_bank res1[0x5];
	struct s5p_gpio_bank f0;
	struct s5p_gpio_bank f1;
	struct s5p_gpio_bank f2;
	struct s5p_gpio_bank f3;
	struct s5p_gpio_bank res2[0x2];
	struct s5p_gpio_bank j0;
	struct s5p_gpio_bank j1;
};

struct exynos4x12_gpio_part2 {
	struct s5p_gpio_bank res1[0x2];
	struct s5p_gpio_bank k0;
	struct s5p_gpio_bank k1;
	struct s5p_gpio_bank k2;
	struct s5p_gpio_bank k3;
	struct s5p_gpio_bank l0;
	struct s5p_gpio_bank l1;
	struct s5p_gpio_bank l2;
	struct s5p_gpio_bank y0;
	struct s5p_gpio_bank y1;
	struct s5p_gpio_bank y2;
	struct s5p_gpio_bank y3;
	struct s5p_gpio_bank y4;
	struct s5p_gpio_bank y5;
	struct s5p_gpio_bank y6;
	struct s5p_gpio_bank res2[0x3];
	struct s5p_gpio_bank m0;
	struct s5p_gpio_bank m1;
	struct s5p_gpio_bank m2;
	struct s5p_gpio_bank m3;
	struct s5p_gpio_bank m4;
	struct s5p_gpio_bank res3[0x48];
	struct s5p_gpio_bank x0;
	struct s5p_gpio_bank x1;
	struct s5p_gpio_bank x2;
	struct s5p_gpio_bank x3;
};

struct exynos4x12_gpio_part3 {
	struct s5p_gpio_bank z;
};

struct exynos4x12_gpio_part4 {
	struct s5p_gpio_bank v0;
	struct s5p_gpio_bank v1;
	struct s5p_gpio_bank res1[0x1];
	struct s5p_gpio_bank v2;
	struct s5p_gpio_bank v3;
	struct s5p_gpio_bank res2[0x1];
	struct s5p_gpio_bank v4;
};

struct exynos5420_gpio_part1 {
	struct s5p_gpio_bank a0;
	struct s5p_gpio_bank a1;
	struct s5p_gpio_bank a2;
	struct s5p_gpio_bank b0;
	struct s5p_gpio_bank b1;
	struct s5p_gpio_bank b2;
	struct s5p_gpio_bank b3;
	struct s5p_gpio_bank b4;
	struct s5p_gpio_bank h0;
};

struct exynos5420_gpio_part2 {
	struct s5p_gpio_bank y7; /* 0x1340_0000 */
	struct s5p_gpio_bank res[0x5f]; /*  */
	struct s5p_gpio_bank x0; /* 0x1340_0C00 */
	struct s5p_gpio_bank x1; /* 0x1340_0C20 */
	struct s5p_gpio_bank x2; /* 0x1340_0C40 */
	struct s5p_gpio_bank x3; /* 0x1340_0C60 */
};

struct exynos5420_gpio_part3 {
	struct s5p_gpio_bank c0;
	struct s5p_gpio_bank c1;
	struct s5p_gpio_bank c2;
	struct s5p_gpio_bank c3;
	struct s5p_gpio_bank c4;
	struct s5p_gpio_bank d1;
	struct s5p_gpio_bank y0;
	struct s5p_gpio_bank y1;
	struct s5p_gpio_bank y2;
	struct s5p_gpio_bank y3;
	struct s5p_gpio_bank y4;
	struct s5p_gpio_bank y5;
	struct s5p_gpio_bank y6;
};

struct exynos5420_gpio_part4 {
	struct s5p_gpio_bank e0; /* 0x1400_0000 */
	struct s5p_gpio_bank e1; /* 0x1400_0020 */
	struct s5p_gpio_bank f0; /* 0x1400_0040 */
	struct s5p_gpio_bank f1; /* 0x1400_0060 */
	struct s5p_gpio_bank g0; /* 0x1400_0080 */
	struct s5p_gpio_bank g1; /* 0x1400_00A0 */
	struct s5p_gpio_bank g2; /* 0x1400_00C0 */
	struct s5p_gpio_bank j4; /* 0x1400_00E0 */
};

struct exynos5420_gpio_part5 {
	struct s5p_gpio_bank z0; /* 0x0386_0000 */
};

struct exynos5_gpio_part1 {
	struct s5p_gpio_bank a0;
	struct s5p_gpio_bank a1;
	struct s5p_gpio_bank a2;
	struct s5p_gpio_bank b0;
	struct s5p_gpio_bank b1;
	struct s5p_gpio_bank b2;
	struct s5p_gpio_bank b3;
	struct s5p_gpio_bank c0;
	struct s5p_gpio_bank c1;
	struct s5p_gpio_bank c2;
	struct s5p_gpio_bank c3;
	struct s5p_gpio_bank d0;
	struct s5p_gpio_bank d1;
	struct s5p_gpio_bank y0;
	struct s5p_gpio_bank y1;
	struct s5p_gpio_bank y2;
	struct s5p_gpio_bank y3;
	struct s5p_gpio_bank y4;
	struct s5p_gpio_bank y5;
	struct s5p_gpio_bank y6;
	struct s5p_gpio_bank res1[0x3];
	struct s5p_gpio_bank c4;
	struct s5p_gpio_bank res2[0x48];
	struct s5p_gpio_bank x0;
	struct s5p_gpio_bank x1;
	struct s5p_gpio_bank x2;
	struct s5p_gpio_bank x3;
};

struct exynos5_gpio_part2 {
	struct s5p_gpio_bank e0;
	struct s5p_gpio_bank e1;
	struct s5p_gpio_bank f0;
	struct s5p_gpio_bank f1;
	struct s5p_gpio_bank g0;
	struct s5p_gpio_bank g1;
	struct s5p_gpio_bank g2;
	struct s5p_gpio_bank h0;
	struct s5p_gpio_bank h1;
};

struct exynos5_gpio_part3 {
	struct s5p_gpio_bank v0;
	struct s5p_gpio_bank v1;
	struct s5p_gpio_bank res1[0x1];
	struct s5p_gpio_bank v2;
	struct s5p_gpio_bank v3;
	struct s5p_gpio_bank res2[0x1];
	struct s5p_gpio_bank v4;
};

struct exynos5_gpio_part4 {
	struct s5p_gpio_bank z;
};

/* functions */
void s5p_gpio_cfg_pin(struct s5p_gpio_bank *bank, int gpio, int cfg);
void s5p_gpio_direction_output(struct s5p_gpio_bank *bank, int gpio, int en);
void s5p_gpio_direction_input(struct s5p_gpio_bank *bank, int gpio);
void s5p_gpio_set_value(struct s5p_gpio_bank *bank, int gpio, int en);
unsigned int s5p_gpio_get_value(struct s5p_gpio_bank *bank, int gpio);
void s5p_gpio_set_pull(struct s5p_gpio_bank *bank, int gpio, int mode);
void s5p_gpio_set_drv(struct s5p_gpio_bank *bank, int gpio, int mode);
void s5p_gpio_set_rate(struct s5p_gpio_bank *bank, int gpio, int mode);

/* GPIO pins per bank  */
#define GPIO_PER_BANK 8

#define exynos4_gpio_part1_get_nr(bank, pin) \
	((((((unsigned int) &(((struct exynos4_gpio_part1 *) \
			       EXYNOS4_GPIO_PART1_BASE)->bank)) \
	    - EXYNOS4_GPIO_PART1_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin)

#define EXYNOS4_GPIO_PART1_MAX ((sizeof(struct exynos4_gpio_part1) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos4_gpio_part2_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos4_gpio_part2 *) \
				EXYNOS4_GPIO_PART2_BASE)->bank)) \
	    - EXYNOS4_GPIO_PART2_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS4_GPIO_PART1_MAX)

#define exynos4x12_gpio_part1_get_nr(bank, pin) \
	((((((unsigned int) &(((struct exynos4x12_gpio_part1 *) \
			       EXYNOS4X12_GPIO_PART1_BASE)->bank)) \
	    - EXYNOS4X12_GPIO_PART1_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin)

#define EXYNOS4X12_GPIO_PART1_MAX ((sizeof(struct exynos4x12_gpio_part1) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos4x12_gpio_part2_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos4x12_gpio_part2 *) \
				EXYNOS4X12_GPIO_PART2_BASE)->bank)) \
	    - EXYNOS4X12_GPIO_PART2_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS4X12_GPIO_PART1_MAX)

#define EXYNOS4X12_GPIO_PART2_MAX ((sizeof(struct exynos4x12_gpio_part2) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos4x12_gpio_part3_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos4x12_gpio_part3 *) \
				EXYNOS4X12_GPIO_PART3_BASE)->bank)) \
	    - EXYNOS4X12_GPIO_PART3_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS4X12_GPIO_PART2_MAX)

#define exynos5_gpio_part1_get_nr(bank, pin) \
	((((((unsigned int) &(((struct exynos5_gpio_part1 *) \
			       EXYNOS5_GPIO_PART1_BASE)->bank)) \
	    - EXYNOS5_GPIO_PART1_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin)

#define EXYNOS5_GPIO_PART1_MAX ((sizeof(struct exynos5_gpio_part1) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos5_gpio_part2_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos5_gpio_part2 *) \
				EXYNOS5_GPIO_PART2_BASE)->bank)) \
	    - EXYNOS5_GPIO_PART2_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS5_GPIO_PART1_MAX)

#define EXYNOS5_GPIO_PART2_MAX ((sizeof(struct exynos5_gpio_part2) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos5_gpio_part3_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos5_gpio_part3 *) \
				EXYNOS5_GPIO_PART3_BASE)->bank)) \
	    - EXYNOS5_GPIO_PART3_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS5_GPIO_PART2_MAX)


/* EXYNOS5420 */
#define exynos5420_gpio_part1_get_nr(bank, pin) \
	((((((unsigned int) &(((struct exynos5420_gpio_part1 *)\
			       EXYNOS5420_GPIO_PART1_BASE)->bank)) \
	    - EXYNOS5420_GPIO_PART1_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin)

#define EXYNOS5420_GPIO_PART1_MAX ((sizeof(struct exynos5420_gpio_part1) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos5420_gpio_part2_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos5420_gpio_part2 *)\
				EXYNOS5420_GPIO_PART2_BASE)->bank)) \
	    - EXYNOS5420_GPIO_PART2_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS5420_GPIO_PART1_MAX)

#define EXYNOS5420_GPIO_PART2_MAX ((sizeof(struct exynos5420_gpio_part2) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos5420_gpio_part3_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos5420_gpio_part3 *)\
				EXYNOS5420_GPIO_PART3_BASE)->bank)) \
	    - EXYNOS5420_GPIO_PART3_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS5420_GPIO_PART2_MAX)

#define EXYNOS5420_GPIO_PART3_MAX ((sizeof(struct exynos5420_gpio_part3) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define exynos5420_gpio_part4_get_nr(bank, pin) \
	(((((((unsigned int) &(((struct exynos5420_gpio_part4 *)\
				EXYNOS5420_GPIO_PART4_BASE)->bank)) \
	    - EXYNOS5420_GPIO_PART4_BASE) / sizeof(struct s5p_gpio_bank)) \
	  * GPIO_PER_BANK) + pin) + EXYNOS5420_GPIO_PART3_MAX)

#define EXYNOS5420_GPIO_PART4_MAX ((sizeof(struct exynos5420_gpio_part4) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

#define EXYNOS5420_GPIO_PART5_MAX ((sizeof(struct exynos5420_gpio_part5) \
			    / sizeof(struct s5p_gpio_bank)) * GPIO_PER_BANK)

static inline unsigned int s5p_gpio_base(int nr)
{
	if (cpu_is_exynos5()) {
		if (proid_is_exynos5420()) {
			if (nr < EXYNOS5420_GPIO_PART1_MAX)
				return EXYNOS5420_GPIO_PART1_BASE;
			else if (nr < EXYNOS5420_GPIO_PART2_MAX)
				return EXYNOS5420_GPIO_PART2_BASE;
			else if (nr < EXYNOS5420_GPIO_PART3_MAX)
				return EXYNOS5420_GPIO_PART3_BASE;
			else
				return EXYNOS5420_GPIO_PART4_BASE;
		} else {
			if (nr < EXYNOS5_GPIO_PART1_MAX)
				return EXYNOS5_GPIO_PART1_BASE;
			else if (nr < EXYNOS5_GPIO_PART2_MAX)
				return EXYNOS5_GPIO_PART2_BASE;
			else
				return EXYNOS5_GPIO_PART3_BASE;
		}
	} else if (cpu_is_exynos4()) {
		if (nr < EXYNOS4_GPIO_PART1_MAX)
			return EXYNOS4_GPIO_PART1_BASE;
		else
			return EXYNOS4_GPIO_PART2_BASE;
	}

	return 0;
}

static inline unsigned int s5p_gpio_part_max(int nr)
{
	if (cpu_is_exynos5()) {
		if (proid_is_exynos5420()) {
			if (nr < EXYNOS5420_GPIO_PART1_MAX)
				return 0;
			else if (nr < EXYNOS5420_GPIO_PART2_MAX)
				return EXYNOS5420_GPIO_PART1_MAX;
			else if (nr < EXYNOS5420_GPIO_PART3_MAX)
				return EXYNOS5420_GPIO_PART2_MAX;
			else if (nr < EXYNOS5420_GPIO_PART4_MAX)
				return EXYNOS5420_GPIO_PART3_MAX;
			else
				return EXYNOS5420_GPIO_PART4_MAX;
		} else {
			if (nr < EXYNOS5_GPIO_PART1_MAX)
				return 0;
			else if (nr < EXYNOS5_GPIO_PART2_MAX)
				return EXYNOS5_GPIO_PART1_MAX;
			else
				return EXYNOS5_GPIO_PART2_MAX;
		}
	} else if (cpu_is_exynos4()) {
		if (proid_is_exynos4412()) {
			if (nr < EXYNOS4X12_GPIO_PART1_MAX)
				return 0;
			else if (nr < EXYNOS4X12_GPIO_PART2_MAX)
				return EXYNOS4X12_GPIO_PART1_MAX;
			else
				return EXYNOS4X12_GPIO_PART2_MAX;
		} else {
			if (nr < EXYNOS4_GPIO_PART1_MAX)
				return 0;
			else
				return EXYNOS4_GPIO_PART1_MAX;
		}
	}

	return 0;
}
#endif

/* Pin configurations */
#define GPIO_INPUT	0x0
#define GPIO_OUTPUT	0x1
#define GPIO_IRQ	0xf
#define GPIO_FUNC(x)	(x)

/* Pull mode */
#define GPIO_PULL_NONE	0x0
#define GPIO_PULL_DOWN	0x1
#define GPIO_PULL_UP	0x3

/* Drive Strength level */
#define GPIO_DRV_1X	0x0
#define GPIO_DRV_3X	0x1
#define GPIO_DRV_2X	0x2
#define GPIO_DRV_4X	0x3
#define GPIO_DRV_FAST	0x0
#define GPIO_DRV_SLOW	0x1
#endif
