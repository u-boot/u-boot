/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 *
 * Authors:  Chunhe Lan <b25806@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __BCSR_H_
#define __BCSR_H_

#include <common.h>

/*
 * BCSR Bit definitions
	* BCSR 15 *
	0	device insertion oriention
	1	stack processor present
	2	power supply shut down/normal operation
	3	I2C bus0 drive enable
	4	reserved
	5:7	I2C bus0 select
				5 - I2C_BUS_0_SS0
				6 - I2C_BUS_0_SS1
				7 - I2C_BUS_0_SS2
*/

/* BCSR register base address is 0xFX000020 */
#define BCSR_BASE_REG_OFFSET	0x20
#define BCSR_ACCESS_REG_ADDR	(CONFIG_SYS_BCSR_BASE + BCSR_BASE_REG_OFFSET)

#define BCSR15_DEV_INS_ORI	0x80
#define BCSR15_STACK_PRO_PRE	0x40
#define BCSR15_POWER_SUPPLY	0x20
#define BCSR15_I2C_BUS0_EN	0x10
#define BCSR15_I2C_BUS0_SEG0	0x00
#define BCSR15_I2C_BUS0_SEG1	0x04
#define BCSR15_I2C_BUS0_SEG2	0x02
#define BCSR15_I2C_BUS0_SEG3	0x06
#define BCSR15_I2C_BUS0_SEG4	0x01
#define BCSR15_I2C_BUS0_SEG5	0x05
#define BCSR15_I2C_BUS0_SEG6	0x03
#define BCSR15_I2C_BUS0_SEG7	0x07
#define BCSR15_I2C_BUS0_SEG_CLR	0x07
#define BCSR19_SGMII_SEL_L	0x01

/*BCSR Utils functions*/
void fixup_i2c_bus0_sel_seg0(void);
#endif  /* __BCSR_H_ */
