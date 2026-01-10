/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025-2026 RISCStar Ltd.
 */

#ifndef __SPACEMIT_P1_H_
#define __SPACEMIT_P1_H_

#define P1_MAX_REGS			0xA8

#define P1_REG_ID			0x0

#define P1_ID				0x2

#define P1_REG_BUCK1_CTRL		0x47
#define P1_REG_BUCK2_CTRL		0x4a
#define P1_REG_BUCK3_CTRL		0x4d
#define P1_REG_BUCK4_CTRL		0x50
#define P1_REG_BUCK5_CTRL		0x53
#define P1_REG_BUCK6_CTRL		0x56

#define P1_REG_BUCK1_VSEL		0x48
#define P1_REG_BUCK2_VSEL		0x4b
#define P1_REG_BUCK3_VSEL		0x4e
#define P1_REG_BUCK4_VSEL		0x51
#define P1_REG_BUCK5_VSEL		0x54
#define P1_REG_BUCK6_VSEL		0x57

#define P1_REG_BUCK1_SVSEL		0x49
#define P1_REG_BUCK2_SVSEL		0x4c
#define P1_REG_BUCK3_SVSEL		0x4f
#define P1_REG_BUCK4_SVSEL		0x52
#define P1_REG_BUCK5_SVSEL		0x55
#define P1_REG_BUCK6_SVSEL		0x58

#define P1_BUCK_CTRL(x)			(0x47 + ((x) - 1) * 3)
#define P1_BUCK_VSEL(x)			(0x48 + ((x) - 1) * 3)
#define P1_BUCK_SVSEL(x)		(0x49 + ((x) - 1) * 3)

#define BUCK_VSEL_MASK			0xff
#define BUCK_EN_MASK			0x1
#define BUCK_SVSEL_MASK			0xff

#define P1_REG_ALDO1_CTRL		0x5b
#define P1_REG_ALDO2_CTRL		0x5e
#define P1_REG_ALDO3_CTRL		0x61
#define P1_REG_ALDO4_CTRL		0x64

#define P1_REG_ALDO1_VOLT		0x5c
#define P1_REG_ALDO2_VOLT		0x5f
#define P1_REG_ALDO3_VOLT		0x62
#define P1_REG_ALDO4_VOLT		0x65

#define P1_REG_ALDO1_SVOLT		0x5d
#define P1_REG_ALDO2_SVOLT		0x60
#define P1_REG_ALDO3_SVOLT		0x63
#define P1_REG_ALDO4_SVOLT		0x66

#define P1_ALDO_CTRL(x)			(0x5b + ((x) - 1) * 3)
#define P1_ALDO_VOLT(x)			(0x5c + ((x) - 1) * 3)
#define P1_ALDO_SVOLT(x)		(0x5d + ((x) - 1) * 3)

#define ALDO_SVSEL_MASK			0x7f
#define ALDO_EN_MASK			0x1
#define ALDO_VSEL_MASK			0x7f

#define P1_REG_DLDO1_CTRL		0x67
#define P1_REG_DLDO2_CTRL		0x6a
#define P1_REG_DLDO3_CTRL		0x6d
#define P1_REG_DLDO4_CTRL		0x70
#define P1_REG_DLDO5_CTRL		0x73
#define P1_REG_DLDO6_CTRL		0x76
#define P1_REG_DLDO7_CTRL		0x79

#define P1_REG_DLDO1_VOLT		0x68
#define P1_REG_DLDO2_VOLT		0x6b
#define P1_REG_DLDO3_VOLT		0x6e
#define P1_REG_DLDO4_VOLT		0x71
#define P1_REG_DLDO5_VOLT		0x74
#define P1_REG_DLDO6_VOLT		0x77
#define P1_REG_DLDO7_VOLT		0x7a

#define P1_REG_DLDO1_SVOLT		0x69
#define P1_REG_DLDO2_SVOLT		0x6c
#define P1_REG_DLDO3_SVOLT		0x6f
#define P1_REG_DLDO4_SVOLT		0x72
#define P1_REG_DLDO5_SVOLT		0x75
#define P1_REG_DLDO6_SVOLT		0x78
#define P1_REG_DLDO7_SVOLT		0x7b

#define P1_DLDO_CTRL(x)			(0x67 + ((x) - 1) * 3)
#define P1_DLDO_VOLT(x)			(0x68 + ((x) - 1) * 3)
#define P1_DLDO_SVOLT(x)		(0x69 + ((x) - 1) * 3)

#define DLDO_SVSEL_MASK			0x7f
#define DLDO_EN_MASK			0x1
#define DLDO_VSEL_MASK			0x7f

#define P1_REG_SWITCH_CTRL		0x59
#define P1_SWTICH_EN_MASK		0x1

#define P1_REG_SWITCH_PWRKEY_EVENT_CTRL	0x97
#define P1_SWITCH_PWRKEY_EVENT_EN_MSK	0xf

#define P1_REG_SWITCH_PWRKEY_INIT_CTRL	0x9e
#define P1_SWITCH_PWRKEY_INT_EN_MSK	0xf

/* Watchdog Timer Registers */
#define P1_WDT_CTRL			0x44
#define P1_PWR_CTRL0			0x7C
#define P1_PWR_CTRL2			0x7E
#define P1_PWR_CTRL2_MSK		0xff

/* Watchdog Timer Control Bits */
#define P1_WDT_CLEAR_STATUS		0x1
#define P1_SW_RST			0x2
#define P1_WDT_RESET_ENABLE		0x80
#define P1_WDT_ENABLE			0x8
#define P1_WDT_TIMEOUT_1S		0x0
#define P1_WDT_TIMEOUT_4S		0x1
#define P1_WDT_TIMEOUT_8S		0x2
#define P1_WDT_TIMEOUT_16S		0x3

#define P1_RTC_TICK_CTRL		0x1d
#define P1_RTC_TICK_CTRL_MSK		0x7f

#define P1_RTC_TICK_EVENT		0x92
#define P1_RTC_TICK_EVENT_MSK		0x3f

#define P1_RTC_TICK_IRQ			0x99
#define P1_RTC_TICK_IRQ_MSK		0x3f

#define P1_REG_ALIVE			0xab
#define P1_ALIVE_MSK			0x7
#define SYS_REBOOT_FLAG_BIT		0x2

/* SWITCH ID */
enum {
	P1_ID_SWITCH1,
	P1_ID_SWITCH1_PWRKEY_EVENT,
	P1_ID_SWITCH1_PWRKEY_INT,
	P1_ID_SWITCH_RTC_TICK_CTRL,
	P1_ID_SWITCH_RTC_TICK_EVENT,
	P1_ID_SWITCH_RTC_TCK_IRQ,
	P1_ID_SWITCH_POWER_DOWN,
	P1_ID_SWITCH_CHARGING_FLAG,
};

/* POWERKEY events */
enum {
	PWRKEY_RISING_EVENT = 1,
	PWRKEY_FAILING_EVENT = 2,
	PWRKEY_SHORT_PRESS_EVENT = 4,
	PWRKEY_LONG_PRESS_EVENT = 8,
};

#define P1_BUCK_DRIVER			"p1_buck"
#define P1_LDO_DRIVER			"p1_ldo"
#define P1_SWITCH_DRIVER		"p1_switch"
#define P1_WDT_DRIVER			"p1_wdt"

#endif /* __SPACEMIT_P1_H_ */
