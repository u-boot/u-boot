/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0
 * https://spdx.org/licenses
 */

#ifndef _MARVELL_RTC_H
#define _MARVELL_RTC_H

/* The RTC DRS revision 1.2 indicates that firmware should wait
 * 5us after every register write to the RTC hard macro,
 * so that the required update can occur without holding off the system bus
 */
#define RTC_READ_REG(rtc_base, reg)		readl((rtc_base) + (reg))
#define RTC_WRITE_REG(val, rtc_base, reg)		\
	{ writel((val), (rtc_base) + (reg)); udelay(5); }

#define RTC_NOMINAL_TIMING		0x2000

#define RTC_STATUS_REG_OFFS		0x0
#define RTC_IRQ_1_CONFIG_REG_OFFS	0x4
#define RTC_IRQ_2_CONFIG_REG_OFFS	0x8
#define RTC_TIME_REG_OFFS		0xC
#define RTC_ALARM_1_REG_OFFS		0x10
#define RTC_ALARM_2_REG_OFFS		0x14
#define RTC_CLOCK_CORR_REG_OFFS		0x18
#define RTC_TEST_CONFIG_REG_OFFS	0x1C
#define MV_RTC0_SOC_OFFSET		0x80
#define MV_RTC1_SOC_OFFSET		0x84

#define RTC_WRCLK_PERIOD_OFFS		0
#define RTC_WRCLK_PERIOD_MASK		(0xFFFF << RTC_WRCLK_PERIOD_OFFS)
#define RTC_WRCLK_SETUP_OFFS		16
#define RTC_WRCLK_SETUP_MASK		(0xFFFF << RTC_WRCLK_SETUP_OFFS)

#define RTC_READ_OUTPUT_DELAY_OFFS	0
#define RTC_READ_OUTPUT_DELAY_MASK	(0xFFFF << RTC_READ_OUTPUT_DELAY_OFFS)
#define RTC_WRCLK_CLOCK_HIGH_OFFS	16
#define RTC_WRCLK_CLOCK_HIGH_MASK	(0xFFFF << RTC_WRCLK_CLOCK_HIGH_OFFS)

#define RTC_SZ_STATUS_ALARM1_MASK		0x1
#define RTC_SZ_STATUS_ALARM2_MASK		0x2
#define RTC_SZ_TIMING_RESERVED1_MASK		0xFFFF0000
#define RTC_SZ_INTERRUPT1_INT1AE_MASK		0x1
#define RTC_SZ_INTERRUPT1_RESERVED1_MASK	0xFFFFFFC0
#define RTC_SZ_INTERRUPT2_INT2FE_MASK		0x2
#define RTC_SZ_INTERRUPT2_RESERVED1_MASK	0xFFFFFFC0

struct rtc_unit_config {
	void __iomem *rtc_base;
};

#endif /* _MARVELL_RTC_H */
