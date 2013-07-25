/*
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_PMIC_H__
#define __FSL_PMIC_H__

/*
 * The registers of different PMIC has the same meaning
 * but the bit positions of the fields can differ or
 * some fields has a meaning only on some devices.
 * You have to check with the internal SPI bitmap
 * (see Freescale Documentation) to set the registers
 * for the device you are using
 */
enum {
	REG_INT_STATUS0 = 0,
	REG_INT_MASK0,
	REG_INT_SENSE0,
	REG_INT_STATUS1,
	REG_INT_MASK1,
	REG_INT_SENSE1,
	REG_PU_MODE_S,
	REG_IDENTIFICATION,
	REG_UNUSED0,
	REG_ACC0,
	REG_ACC1,		/*10 */
	REG_UNUSED1,
	REG_UNUSED2,
	REG_POWER_CTL0,
	REG_POWER_CTL1,
	REG_POWER_CTL2,
	REG_REGEN_ASSIGN,
	REG_UNUSED3,
	REG_MEM_A,
	REG_MEM_B,
	REG_RTC_TIME,		/*20 */
	REG_RTC_ALARM,
	REG_RTC_DAY,
	REG_RTC_DAY_ALARM,
	REG_SW_0,
	REG_SW_1,
	REG_SW_2,
	REG_SW_3,
	REG_SW_4,
	REG_SW_5,
	REG_SETTING_0,		/*30 */
	REG_SETTING_1,
	REG_MODE_0,
	REG_MODE_1,
	REG_POWER_MISC,
	REG_UNUSED4,
	REG_UNUSED5,
	REG_UNUSED6,
	REG_UNUSED7,
	REG_UNUSED8,
	REG_UNUSED9,		/*40 */
	REG_UNUSED10,
	REG_UNUSED11,
	REG_ADC0,
	REG_ADC1,
	REG_ADC2,
	REG_ADC3,
	REG_ADC4,
	REG_CHARGE,
	REG_USB0,
	REG_USB1,		/*50 */
	REG_LED_CTL0,
	REG_LED_CTL1,
	REG_LED_CTL2,
	REG_LED_CTL3,
	REG_UNUSED12,
	REG_UNUSED13,
	REG_TRIM0,
	REG_TRIM1,
	REG_TEST0,
	REG_TEST1,		/*60 */
	REG_TEST2,
	REG_TEST3,
	REG_TEST4,
	PMIC_NUM_OF_REGS,
};

/* REG_POWER_MISC */
#define GPO1EN		(1 << 6)
#define GPO1STBY	(1 << 7)
#define GPO2EN		(1 << 8)
#define GPO2STBY	(1 << 9)
#define GPO3EN		(1 << 10)
#define GPO3STBY	(1 << 11)
#define GPO4EN		(1 << 12)
#define GPO4STBY	(1 << 13)
#define PWGT1SPIEN	(1 << 15)
#define PWGT2SPIEN	(1 << 16)
#define PWUP		(1 << 21)

/* Power Control 0 */
#define COINCHEN	(1 << 23)
#define BATTDETEN	(1 << 19)

/* Interrupt status 1 */
#define RTCRSTI		(1 << 7)

/* MC34708 Definitions */
#define SWx_VOLT_MASK_MC34708	0x3F
#define SWx_1_250V_MC34708	0x30
#define SWx_1_300V_MC34708	0x34
#define TIMER_MASK_MC34708	0x300
#define TIMER_4S_MC34708	0x100
#define VUSBSEL_MC34708		(1 << 2)
#define VUSBEN_MC34708		(1 << 3)
#define SWBST_CTRL		31
#define SWBST_AUTO		0x8

#endif
