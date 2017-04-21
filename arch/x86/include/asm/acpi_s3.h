/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ACPI_S3_H__
#define __ASM_ACPI_S3_H__

/* PM1_STATUS register */
#define WAK_STS		(1 << 15)
#define PCIEXPWAK_STS	(1 << 14)
#define RTC_STS		(1 << 10)
#define SLPBTN_STS	(1 << 9)
#define PWRBTN_STS	(1 << 8)
#define GBL_STS		(1 << 5)
#define BM_STS		(1 << 4)
#define TMR_STS		(1 << 0)

/* PM1_CNT register */
#define SLP_EN		(1 << 13)
#define SLP_TYP_SHIFT	10
#define SLP_TYP		(7 << SLP_TYP_SHIFT)
#define SLP_TYP_S0	0
#define SLP_TYP_S1	1
#define SLP_TYP_S3	5
#define SLP_TYP_S4	6
#define SLP_TYP_S5	7

enum acpi_sleep_state {
	ACPI_S0,
	ACPI_S1,
	ACPI_S2,
	ACPI_S3,
	ACPI_S4,
	ACPI_S5,
};

/**
 * acpi_sleep_from_pm1() - get ACPI-defined sleep state from PM1_CNT register
 *
 * @pm1_cnt:	PM1_CNT register value
 * @return:	ACPI-defined sleep state if given valid PM1_CNT register value,
 *		-EINVAL otherwise.
 */
static inline enum acpi_sleep_state acpi_sleep_from_pm1(u32 pm1_cnt)
{
	switch ((pm1_cnt & SLP_TYP) >> SLP_TYP_SHIFT) {
	case SLP_TYP_S0:
		return ACPI_S0;
	case SLP_TYP_S1:
		return ACPI_S1;
	case SLP_TYP_S3:
		return ACPI_S3;
	case SLP_TYP_S4:
		return ACPI_S4;
	case SLP_TYP_S5:
		return ACPI_S5;
	}

	return -EINVAL;
}

#endif /* __ASM_ACPI_S3_H__ */
