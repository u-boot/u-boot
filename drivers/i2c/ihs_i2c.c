/*
 * (C) Copyright 2013
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <gdsys_fpga.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	I2CINT_ERROR_EV = 1 << 13,
	I2CINT_TRANSMIT_EV = 1 << 14,
	I2CINT_RECEIVE_EV = 1 << 15,
};

enum {
	I2CMB_WRITE = 1 << 10,
	I2CMB_2BYTE = 1 << 11,
	I2CMB_HOLD_BUS = 1 << 13,
	I2CMB_NATIVE = 2 << 14,
};

static int wait_for_int(bool read)
{
	u16 val;
	unsigned int ctr = 0;

	FPGA_GET_REG(I2C_ADAP_HWNR, i2c.interrupt_status, &val);
	while (!(val & (I2CINT_ERROR_EV
	       | (read ? I2CINT_RECEIVE_EV : I2CINT_TRANSMIT_EV)))) {
		udelay(10);
		if (ctr++ > 5000) {
			return 1;
		}
		FPGA_GET_REG(I2C_ADAP_HWNR, i2c.interrupt_status, &val);
	}

	return (val & I2CINT_ERROR_EV) ? 1 : 0;
}

static int ihs_i2c_transfer(uchar chip, uchar *buffer, int len, bool read,
			    bool is_last)
{
	u16 val;

	FPGA_SET_REG(I2C_ADAP_HWNR, i2c.interrupt_status, I2CINT_ERROR_EV
		     | I2CINT_RECEIVE_EV | I2CINT_TRANSMIT_EV);
	FPGA_GET_REG(I2C_ADAP_HWNR, i2c.interrupt_status, &val);

	if (!read && len) {
		val = buffer[0];

		if (len > 1)
			val |= buffer[1] << 8;
		FPGA_SET_REG(I2C_ADAP_HWNR, i2c.write_mailbox_ext, val);
	}

	FPGA_SET_REG(I2C_ADAP_HWNR, i2c.write_mailbox,
		     I2CMB_NATIVE
		     | (read ? 0 : I2CMB_WRITE)
		     | (chip << 1)
		     | ((len > 1) ? I2CMB_2BYTE : 0)
		     | (is_last ? 0 : I2CMB_HOLD_BUS));

	if (wait_for_int(read))
		return 1;

	if (read) {
		FPGA_GET_REG(I2C_ADAP_HWNR, i2c.read_mailbox_ext, &val);
		buffer[0] = val & 0xff;
		if (len > 1)
			buffer[1] = val >> 8;
	}

	return 0;
}

static int ihs_i2c_address(uchar chip, uint addr, int alen, bool hold_bus)
{
	int shift = (alen-1) * 8;

	while (alen) {
		int transfer = MIN(alen, 2);
		uchar buf[2];
		bool is_last = alen <= transfer;

		buf[0] = addr >> shift;
		if (alen > 1)
			buf[1] = addr >> (shift - 8);

		if (ihs_i2c_transfer(chip, buf, transfer, false,
				     hold_bus ? false : is_last))
			return 1;

		shift -= 16;
		alen -= transfer;
	}

	return 0;
}

static int ihs_i2c_access(struct i2c_adapter *adap, uchar chip, uint addr,
			  int alen, uchar *buffer, int len, bool read)
{
	if (len <= 0)
		return 1;

	if (ihs_i2c_address(chip, addr, alen, !read))
		return 1;

	while (len) {
		int transfer = MIN(len, 2);

		if (ihs_i2c_transfer(chip, buffer, transfer, read,
				     len <= transfer))
			return 1;

		buffer += transfer;
		addr += transfer;
		len -= transfer;
	}

	return 0;
}


static void ihs_i2c_init(struct i2c_adapter *adap, int speed, int slaveaddr)
{
#ifdef CONFIG_SYS_I2C_INIT_BOARD
	/*
	 * Call board specific i2c bus reset routine before accessing the
	 * environment, which might be in a chip on that bus. For details
	 * about this problem see doc/I2C_Edge_Conditions.
	 */
	i2c_init_board();
#endif
}

static int ihs_i2c_probe(struct i2c_adapter *adap, uchar chip)
{
	uchar buffer[2];

	if (ihs_i2c_transfer(chip, buffer, 0, true, true))
		return 1;

	return 0;
}

static int ihs_i2c_read(struct i2c_adapter *adap, uchar chip, uint addr,
			int alen, uchar *buffer, int len)
{
	return ihs_i2c_access(adap, chip, addr, alen, buffer, len, true);
}

static int ihs_i2c_write(struct i2c_adapter *adap, uchar chip, uint addr,
			 int alen, uchar *buffer, int len)
{
	return ihs_i2c_access(adap, chip, addr, alen, buffer, len, false);
}

static unsigned int ihs_i2c_set_bus_speed(struct i2c_adapter *adap,
					     unsigned int speed)
{
	if (speed != adap->speed)
		return 1;
	return speed;
}

/*
 * Register IHS i2c adapters
 */
#ifdef CONFIG_SYS_I2C_IHS_CH0
U_BOOT_I2C_ADAP_COMPLETE(ihs0, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_0,
			 CONFIG_SYS_I2C_IHS_SLAVE_0, 0)
#endif
#ifdef CONFIG_SYS_I2C_IHS_CH1
U_BOOT_I2C_ADAP_COMPLETE(ihs1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_1, 1)
#endif
#ifdef CONFIG_SYS_I2C_IHS_CH2
U_BOOT_I2C_ADAP_COMPLETE(ihs2, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_2,
			 CONFIG_SYS_I2C_IHS_SLAVE_2, 2)
#endif
#ifdef CONFIG_SYS_I2C_IHS_CH3
U_BOOT_I2C_ADAP_COMPLETE(ihs3, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_3,
			 CONFIG_SYS_I2C_IHS_SLAVE_3, 3)
#endif
