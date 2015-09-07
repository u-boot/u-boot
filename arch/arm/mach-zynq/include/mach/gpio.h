/*
 * Copyright (c) 2013 Xilinx, Inc.
 * Copyright (c) 2015 DAVE Embedded Systems
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ZYNQ_GPIO_H
#define _ZYNQ_GPIO_H

#define ZYNQ_GPIO_BASE_ADDRESS 0xE000A000

/* Maximum banks */
#define ZYNQ_GPIO_MAX_BANK	4

#define ZYNQ_GPIO_BANK0_NGPIO	32
#define ZYNQ_GPIO_BANK1_NGPIO	22
#define ZYNQ_GPIO_BANK2_NGPIO	32
#define ZYNQ_GPIO_BANK3_NGPIO	32

#define ZYNQ_GPIO_NR_GPIOS	(ZYNQ_GPIO_BANK0_NGPIO + \
				 ZYNQ_GPIO_BANK1_NGPIO + \
				 ZYNQ_GPIO_BANK2_NGPIO + \
				 ZYNQ_GPIO_BANK3_NGPIO)

#define ZYNQ_GPIO_BANK0_PIN_MIN	0
#define ZYNQ_GPIO_BANK0_PIN_MAX	(ZYNQ_GPIO_BANK0_PIN_MIN + \
					ZYNQ_GPIO_BANK0_NGPIO - 1)
#define ZYNQ_GPIO_BANK1_PIN_MIN	(ZYNQ_GPIO_BANK0_PIN_MAX + 1)
#define ZYNQ_GPIO_BANK1_PIN_MAX	(ZYNQ_GPIO_BANK1_PIN_MIN + \
					ZYNQ_GPIO_BANK1_NGPIO - 1)
#define ZYNQ_GPIO_BANK2_PIN_MIN	(ZYNQ_GPIO_BANK1_PIN_MAX + 1)
#define ZYNQ_GPIO_BANK2_PIN_MAX	(ZYNQ_GPIO_BANK2_PIN_MIN + \
					ZYNQ_GPIO_BANK2_NGPIO - 1)
#define ZYNQ_GPIO_BANK3_PIN_MIN	(ZYNQ_GPIO_BANK2_PIN_MAX + 1)
#define ZYNQ_GPIO_BANK3_PIN_MAX	(ZYNQ_GPIO_BANK3_PIN_MIN + \
					ZYNQ_GPIO_BANK3_NGPIO - 1)

/* Register offsets for the GPIO device */
/* LSW Mask & Data -WO */
#define ZYNQ_GPIO_DATA_LSW_OFFSET(BANK)	(0x000 + (8 * BANK))
/* MSW Mask & Data -WO */
#define ZYNQ_GPIO_DATA_MSW_OFFSET(BANK)	(0x004 + (8 * BANK))
/* Data Register-RW */
#define ZYNQ_GPIO_DATA_RO_OFFSET(BANK)	(0x060 + (4 * BANK))
/* Direction mode reg-RW */
#define ZYNQ_GPIO_DIRM_OFFSET(BANK)	(0x204 + (0x40 * BANK))
/* Output enable reg-RW */
#define ZYNQ_GPIO_OUTEN_OFFSET(BANK)	(0x208 + (0x40 * BANK))
/* Interrupt mask reg-RO */
#define ZYNQ_GPIO_INTMASK_OFFSET(BANK)	(0x20C + (0x40 * BANK))
/* Interrupt enable reg-WO */
#define ZYNQ_GPIO_INTEN_OFFSET(BANK)	(0x210 + (0x40 * BANK))
/* Interrupt disable reg-WO */
#define ZYNQ_GPIO_INTDIS_OFFSET(BANK)	(0x214 + (0x40 * BANK))
/* Interrupt status reg-RO */
#define ZYNQ_GPIO_INTSTS_OFFSET(BANK)	(0x218 + (0x40 * BANK))
/* Interrupt type reg-RW */
#define ZYNQ_GPIO_INTTYPE_OFFSET(BANK)	(0x21C + (0x40 * BANK))
/* Interrupt polarity reg-RW */
#define ZYNQ_GPIO_INTPOL_OFFSET(BANK)	(0x220 + (0x40 * BANK))
/* Interrupt on any, reg-RW */
#define ZYNQ_GPIO_INTANY_OFFSET(BANK)	(0x224 + (0x40 * BANK))

/* Disable all interrupts mask */
#define ZYNQ_GPIO_IXR_DISABLE_ALL	0xFFFFFFFF

/* Mid pin number of a bank */
#define ZYNQ_GPIO_MID_PIN_NUM 16

/* GPIO upper 16 bit mask */
#define ZYNQ_GPIO_UPPER_MASK 0xFFFF0000

#endif /* _ZYNQ_GPIO_H */
