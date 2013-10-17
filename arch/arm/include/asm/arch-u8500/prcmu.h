/*
 * Copyright (C) 2009 ST-Ericsson SA
 *
 * Copied from the Linux version:
 * Author: Kumar Sanghvi <kumar.sanghvi@stericsson.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __MACH_PRCMU_FW_V1_H
#define __MACH_PRCMU_FW_V1_H

#define AP_EXECUTE	2
#define I2CREAD		1
#define I2C_WR_OK	1
#define I2C_RD_OK	2
#define I2CWRITE	0

#define PRCMU_BASE			U8500_PRCMU_BASE
#define PRCMU_BASE_TCDM			U8500_PRCMU_TCDM_BASE
#define PRCM_UARTCLK_MGT_REG		(PRCMU_BASE + 0x018)
#define PRCM_MSPCLK_MGT_REG		(PRCMU_BASE + 0x01C)
#define PRCM_I2CCLK_MGT_REG		(PRCMU_BASE + 0x020)
#define PRCM_SDMMCCLK_MGT_REG		(PRCMU_BASE + 0x024)
#define PRCM_PER1CLK_MGT_REG		(PRCMU_BASE + 0x02C)
#define PRCM_PER2CLK_MGT_REG		(PRCMU_BASE + 0x030)
#define PRCM_PER3CLK_MGT_REG		(PRCMU_BASE + 0x034)
#define PRCM_PER5CLK_MGT_REG		(PRCMU_BASE + 0x038)
#define PRCM_PER6CLK_MGT_REG		(PRCMU_BASE + 0x03C)
#define PRCM_PER7CLK_MGT_REG		(PRCMU_BASE + 0x040)
#define PRCM_MBOX_CPU_VAL		(PRCMU_BASE + 0x0FC)
#define PRCM_MBOX_CPU_SET		(PRCMU_BASE + 0x100)

#define PRCM_ARM_IT1_CLEAR		(PRCMU_BASE + 0x48C)
#define PRCM_ARM_IT1_VAL		(PRCMU_BASE + 0x494)
#define PRCM_TCR			(PRCMU_BASE + 0x1C8)
#define PRCM_REQ_MB5			(PRCMU_BASE_TCDM + 0xE44)
#define PRCM_ACK_MB5			(PRCMU_BASE_TCDM + 0xDF4)
#define PRCM_XP70_CUR_PWR_STATE		(PRCMU_BASE_TCDM + 0xFFC)
/* Mailbox 5 Requests */
#define PRCM_REQ_MB5_I2COPTYPE_REG	(PRCM_REQ_MB5 + 0x0)
#define PRCM_REQ_MB5_BIT_FIELDS		(PRCM_REQ_MB5 + 0x1)
#define PRCM_REQ_MB5_I2CSLAVE		(PRCM_REQ_MB5 + 0x2)
#define PRCM_REQ_MB5_I2CVAL		(PRCM_REQ_MB5 + 0x3)

/* Mailbox 5 ACKs */
#define PRCM_ACK_MB5_STATUS	(PRCM_ACK_MB5 + 0x1)
#define PRCM_ACK_MB5_SLAVE	(PRCM_ACK_MB5 + 0x2)
#define PRCM_ACK_MB5_VAL	(PRCM_ACK_MB5 + 0x3)

#define LOW_POWER_WAKEUP	1
#define EXE_WAKEUP		0

#define REQ_MB5			5

#define ab8500_read	prcmu_i2c_read
#define ab8500_write	prcmu_i2c_write

int prcmu_i2c_read(u8 reg, u16 slave);
int prcmu_i2c_write(u8 reg, u16 slave, u8 reg_data);

void u8500_prcmu_enable(u32 *reg);
void db8500_prcmu_init(void);

#endif /* __MACH_PRCMU_FW_V1_H */
