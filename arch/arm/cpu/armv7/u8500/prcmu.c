/*
 * Copyright (C) 2009 ST-Ericsson SA
 *
 * Adapted from the Linux version:
 * Author: Kumar Sanghvi <kumar.sanghvi@stericsson.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * NOTE: This currently does not support the I2C workaround access method.
 */

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/types.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/prcmu.h>

/* CPU mailbox registers */
#define PRCMU_I2C_WRITE(slave)  \
	(((slave) << 1) | I2CWRITE | (1 << 6))
#define PRCMU_I2C_READ(slave) \
	(((slave) << 1) | I2CREAD | (1 << 6))

#define I2C_MBOX_BIT    (1 << 5)

static int prcmu_is_ready(void)
{
	int ready = readb(PRCM_XP70_CUR_PWR_STATE) == AP_EXECUTE;
	if (!ready)
		printf("PRCMU firmware not ready\n");
	return ready;
}

static int wait_for_i2c_mbx_rdy(void)
{
	int timeout = 10000;

	if (readl(PRCM_ARM_IT1_VAL) & I2C_MBOX_BIT) {
		printf("prcmu: warning i2c mailbox was not acked\n");
		/* clear mailbox 5 ack irq */
		writel(I2C_MBOX_BIT, PRCM_ARM_IT1_CLEAR);
	}

	/* check any already on-going transaction */
	while ((readl(PRCM_MBOX_CPU_VAL) & I2C_MBOX_BIT) && timeout)
		timeout--;

	if (timeout == 0)
		return -1;

	return 0;
}

static int wait_for_i2c_req_done(void)
{
	int timeout = 10000;

	/* Set an interrupt to XP70 */
	writel(I2C_MBOX_BIT, PRCM_MBOX_CPU_SET);

	/* wait for mailbox 5 (i2c) ack */
	while (!(readl(PRCM_ARM_IT1_VAL) & I2C_MBOX_BIT) && timeout)
		timeout--;

	if (timeout == 0)
		return -1;

	return 0;
}

/**
 * prcmu_i2c_read - PRCMU - 4500 communication using PRCMU I2C
 * @reg: - db8500 register bank to be accessed
 * @slave:  - db8500 register to be accessed
 * Returns: ACK_MB5  value containing the status
 */
int prcmu_i2c_read(u8 reg, u16 slave)
{
	uint8_t i2c_status;
	uint8_t i2c_val;
	int ret;

	if (!prcmu_is_ready())
		return -1;

	debug("\nprcmu_4500_i2c_read:bank=%x;reg=%x;\n",
			reg, slave);

	ret = wait_for_i2c_mbx_rdy();
	if (ret) {
		printf("prcmu_i2c_read: mailbox became not ready\n");
		return ret;
	}

	/* prepare the data for mailbox 5 */
	writeb(PRCMU_I2C_READ(reg), PRCM_REQ_MB5_I2COPTYPE_REG);
	writeb((1 << 3) | 0x0, PRCM_REQ_MB5_BIT_FIELDS);
	writeb(slave, PRCM_REQ_MB5_I2CSLAVE);
	writeb(0, PRCM_REQ_MB5_I2CVAL);

	ret = wait_for_i2c_req_done();
	if (ret) {
		printf("prcmu_i2c_read: mailbox request timed out\n");
		return ret;
	}

	/* retrieve values */
	debug("ack-mb5:transfer status = %x\n",
			readb(PRCM_ACK_MB5_STATUS));
	debug("ack-mb5:reg bank = %x\n", readb(PRCM_ACK_MB5) >> 1);
	debug("ack-mb5:slave_add = %x\n",
			readb(PRCM_ACK_MB5_SLAVE));
	debug("ack-mb5:reg_val = %d\n", readb(PRCM_ACK_MB5_VAL));

	i2c_status = readb(PRCM_ACK_MB5_STATUS);
	i2c_val = readb(PRCM_ACK_MB5_VAL);
	/* clear mailbox 5 ack irq */
	writel(I2C_MBOX_BIT, PRCM_ARM_IT1_CLEAR);

	if (i2c_status == I2C_RD_OK)
		return i2c_val;

	printf("prcmu_i2c_read:read return status= %d\n", i2c_status);
	return -1;
}

/**
 * prcmu_i2c_write - PRCMU-db8500 communication using PRCMU I2C
 * @reg: - db8500 register bank to be accessed
 * @slave:  - db800 register to be written to
 * @reg_data: - the data to write
 * Returns: ACK_MB5 value containing the status
 */
int prcmu_i2c_write(u8 reg, u16 slave, u8 reg_data)
{
	uint8_t i2c_status;
	int ret;

	if (!prcmu_is_ready())
		return -1;

	debug("\nprcmu_4500_i2c_write:bank=%x;reg=%x;\n",
			reg, slave);

	ret = wait_for_i2c_mbx_rdy();
	if (ret) {
		printf("prcmu_i2c_write: mailbox became not ready\n");
		return ret;
	}

	/* prepare the data for mailbox 5 */
	writeb(PRCMU_I2C_WRITE(reg), PRCM_REQ_MB5_I2COPTYPE_REG);
	writeb((1 << 3) | 0x0, PRCM_REQ_MB5_BIT_FIELDS);
	writeb(slave, PRCM_REQ_MB5_I2CSLAVE);
	writeb(reg_data, PRCM_REQ_MB5_I2CVAL);

	ret = wait_for_i2c_req_done();
	if (ret) {
		printf("prcmu_i2c_write: mailbox request timed out\n");
		return ret;
	}

	/* retrieve values */
	debug("ack-mb5:transfer status = %x\n",
			readb(PRCM_ACK_MB5_STATUS));
	debug("ack-mb5:reg bank = %x\n", readb(PRCM_ACK_MB5) >> 1);
	debug("ack-mb5:slave_add = %x\n",
			readb(PRCM_ACK_MB5_SLAVE));
	debug("ack-mb5:reg_val = %d\n", readb(PRCM_ACK_MB5_VAL));

	i2c_status = readb(PRCM_ACK_MB5_STATUS);
	debug("\ni2c_status = %x\n", i2c_status);
	/* clear mailbox 5 ack irq */
	writel(I2C_MBOX_BIT, PRCM_ARM_IT1_CLEAR);

	if (i2c_status == I2C_WR_OK)
		return 0;

	printf("%s: i2c_status : 0x%x\n", __func__, i2c_status);
	return -1;
}

void u8500_prcmu_enable(u32 *reg)
{
	writel(readl(reg) | (1 << 8), reg);
}

void db8500_prcmu_init(void)
{
	/* Enable timers */
	writel(1 << 17, PRCM_TCR);

	u8500_prcmu_enable((u32 *)PRCM_PER1CLK_MGT_REG);
	u8500_prcmu_enable((u32 *)PRCM_PER2CLK_MGT_REG);
	u8500_prcmu_enable((u32 *)PRCM_PER3CLK_MGT_REG);
	/* PER4CLK does not exist */
	u8500_prcmu_enable((u32 *)PRCM_PER5CLK_MGT_REG);
	u8500_prcmu_enable((u32 *)PRCM_PER6CLK_MGT_REG);
	/* Only exists in ED but is always ok to write to */
	u8500_prcmu_enable((u32 *)PRCM_PER7CLK_MGT_REG);

	u8500_prcmu_enable((u32 *)PRCM_UARTCLK_MGT_REG);
	u8500_prcmu_enable((u32 *)PRCM_I2CCLK_MGT_REG);

	u8500_prcmu_enable((u32 *)PRCM_SDMMCCLK_MGT_REG);

	/* Clean up the mailbox interrupts after pre-u-boot code. */
	writel(I2C_MBOX_BIT, PRCM_ARM_IT1_CLEAR);
}
