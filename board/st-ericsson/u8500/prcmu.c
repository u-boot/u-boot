/*
 * Copyright (C) 2009 ST-Ericsson SA
 *
 * Adapted from the Linux version:
 * Author: Kumar Sanghvi <kumar.sanghvi@stericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
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

#include "prcmu-fw.h"

/* CPU mailbox registers */
#define PRCM_MBOX_CPU_VAL (U8500_PRCMU_BASE + 0x0fc)
#define PRCM_MBOX_CPU_SET (U8500_PRCMU_BASE + 0x100)
#define PRCM_MBOX_CPU_CLR (U8500_PRCMU_BASE + 0x104)

static int prcmu_is_ready(void)
{
	int ready = readb(PRCM_XP70_CUR_PWR_STATE) == AP_EXECUTE;
	if (!ready)
		printf("PRCMU firmware not ready\n");
	return ready;
}

static int _wait_for_req_complete(int num)
{
	int timeout = 1000;

	/* checking any already on-going transaction */
	while ((readl(PRCM_MBOX_CPU_VAL) & (1 << num)) && timeout--)
		;

	timeout = 1000;

	/* Set an interrupt to XP70 */
	writel(1 << num, PRCM_MBOX_CPU_SET);

	while ((readl(PRCM_MBOX_CPU_VAL) & (1 << num)) && timeout--)
		;

	if (!timeout) {
		printf("PRCMU operation timed out\n");
		return -1;
	}

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

	if (!prcmu_is_ready())
		return -1;

	debug("\nprcmu_4500_i2c_read:bank=%x;reg=%x;\n",
			reg, slave);

	/* prepare the data for mailbox 5 */
	writeb((reg << 1) | I2CREAD, PRCM_REQ_MB5_I2COPTYPE_REG);
	writeb((1 << 3) | 0x0, PRCM_REQ_MB5_BIT_FIELDS);
	writeb(slave, PRCM_REQ_MB5_I2CSLAVE);
	writeb(0, PRCM_REQ_MB5_I2CVAL);

	_wait_for_req_complete(REQ_MB5);

	/* retrieve values */
	debug("ack-mb5:transfer status = %x\n",
			readb(PRCM_ACK_MB5_STATUS));
	debug("ack-mb5:reg bank = %x\n", readb(PRCM_ACK_MB5) >> 1);
	debug("ack-mb5:slave_add = %x\n",
			readb(PRCM_ACK_MB5_SLAVE));
	debug("ack-mb5:reg_val = %d\n", readb(PRCM_ACK_MB5_VAL));

	i2c_status = readb(PRCM_ACK_MB5_STATUS);
	i2c_val = readb(PRCM_ACK_MB5_VAL);

	if (i2c_status == I2C_RD_OK)
		return i2c_val;
	else {

		printf("prcmu_i2c_read:read return status= %d\n",
				i2c_status);
		return -1;
	}

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

	if (!prcmu_is_ready())
		return -1;

	debug("\nprcmu_4500_i2c_write:bank=%x;reg=%x;\n",
			reg, slave);

	/* prepare the data for mailbox 5 */
	writeb((reg << 1) | I2CWRITE, PRCM_REQ_MB5_I2COPTYPE_REG);
	writeb((1 << 3) | 0x0, PRCM_REQ_MB5_BIT_FIELDS);
	writeb(slave, PRCM_REQ_MB5_I2CSLAVE);
	writeb(reg_data, PRCM_REQ_MB5_I2CVAL);

	debug("\ncpu_is_u8500v11\n");
	_wait_for_req_complete(REQ_MB5);

	/* retrieve values */
	debug("ack-mb5:transfer status = %x\n",
			readb(PRCM_ACK_MB5_STATUS));
	debug("ack-mb5:reg bank = %x\n", readb(PRCM_ACK_MB5) >> 1);
	debug("ack-mb5:slave_add = %x\n",
			readb(PRCM_ACK_MB5_SLAVE));
	debug("ack-mb5:reg_val = %d\n", readb(PRCM_ACK_MB5_VAL));

	i2c_status = readb(PRCM_ACK_MB5_STATUS);
	debug("\ni2c_status = %x\n", i2c_status);
	if (i2c_status == I2C_WR_OK)
		return 0;
	else {
		printf("ape-i2c: i2c_status : 0x%x\n", i2c_status);
		return -1;
	}
}
