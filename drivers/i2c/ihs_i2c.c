/*
 * (C) Copyright 2013
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * NOTE: This driver should be converted to driver model before June 2017.
 * Please see doc/driver-model/i2c-howto.txt for instructions.
 */

#include <common.h>
#include <i2c.h>
#include <gdsys_fpga.h>
#include <asm/unaligned.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_I2C_IHS_DUAL
#define I2C_SET_REG(fld, val) \
	do { \
		if (I2C_ADAP_HWNR & 0x10) \
			FPGA_SET_REG(I2C_ADAP_HWNR & 0xf, i2c1.fld, val); \
		else \
			FPGA_SET_REG(I2C_ADAP_HWNR, i2c0.fld, val); \
	} while (0)
#else
#define I2C_SET_REG(fld, val) \
		FPGA_SET_REG(I2C_ADAP_HWNR, i2c0.fld, val)
#endif

#ifdef CONFIG_SYS_I2C_IHS_DUAL
#define I2C_GET_REG(fld, val) \
	do {					\
		if (I2C_ADAP_HWNR & 0x10) \
			FPGA_GET_REG(I2C_ADAP_HWNR & 0xf, i2c1.fld, val); \
		else \
			FPGA_GET_REG(I2C_ADAP_HWNR, i2c0.fld, val); \
	} while (0)
#else
#define I2C_GET_REG(fld, val) \
		FPGA_GET_REG(I2C_ADAP_HWNR, i2c0.fld, val)
#endif

enum {
	I2CINT_ERROR_EV = BIT(13),
	I2CINT_TRANSMIT_EV = BIT(14),
	I2CINT_RECEIVE_EV = BIT(15),
};

enum {
	I2CMB_READ = 0 << 10,
	I2CMB_WRITE = 1 << 10,
	I2CMB_1BYTE = 0 << 11,
	I2CMB_2BYTE = 1 << 11,
	I2CMB_DONT_HOLD_BUS = 0 << 13,
	I2CMB_HOLD_BUS = 1 << 13,
	I2CMB_NATIVE = 2 << 14,
};

enum {
	I2COP_WRITE = 0,
	I2COP_READ = 1,
};

static int wait_for_int(bool read)
{
	u16 val;
	uint ctr = 0;

	I2C_GET_REG(interrupt_status, &val);
	/* Wait until error or receive/transmit interrupt was raised */
	while (!(val & (I2CINT_ERROR_EV
	       | (read ? I2CINT_RECEIVE_EV : I2CINT_TRANSMIT_EV)))) {
		udelay(10);
		if (ctr++ > 5000)
			return 1;
		I2C_GET_REG(interrupt_status, &val);
	}

	return (val & I2CINT_ERROR_EV) ? 1 : 0;
}

static int ihs_i2c_transfer(uchar chip, uchar *buffer, int len, bool read,
			    bool is_last)
{
	u16 val;

	/* Clear interrupt status */
	I2C_SET_REG(interrupt_status, I2CINT_ERROR_EV
		     | I2CINT_RECEIVE_EV | I2CINT_TRANSMIT_EV);
	I2C_GET_REG(interrupt_status, &val);

	/* If we want to write and have data, write the bytes to the mailbox */
	if (!read && len) {
		val = buffer[0];

		if (len > 1)
			val |= buffer[1] << 8;
		I2C_SET_REG(write_mailbox_ext, val);
	}

	I2C_SET_REG(write_mailbox,
		    I2CMB_NATIVE
		    | (read ? 0 : I2CMB_WRITE)
		    | (chip << 1)
		    | ((len > 1) ? I2CMB_2BYTE : 0)
		    | (is_last ? 0 : I2CMB_HOLD_BUS));

	if (wait_for_int(read))
		return 1;

	/* If we want to read, get the bytes from the mailbox */
	if (read) {
		I2C_GET_REG(read_mailbox_ext, &val);
		buffer[0] = val & 0xff;
		if (len > 1)
			buffer[1] = val >> 8;
	}

	return 0;
}

static int ihs_i2c_address(uchar chip, u8 *addr, int alen, bool hold_bus)
{
	while (alen) {
		int transfer = min(alen, 2);
		bool is_last = alen <= transfer;

		if (ihs_i2c_transfer(chip, addr, transfer, I2COP_WRITE,
				     hold_bus ? false : is_last))
			return 1;

		alen -= transfer;
	}

	return 0;
}

static int ihs_i2c_access(struct i2c_adapter *adap, uchar chip, u8 *addr,
			  int alen, uchar *buffer, int len, int read)
{
	/* Don't hold the bus if length of data to send/receive is zero */
	if (len <= 0 || ihs_i2c_address(chip, addr, alen, len))
		return 1;

	while (len) {
		int transfer = min(len, 2);
		bool is_last = len <= transfer;

		if (ihs_i2c_transfer(chip, buffer, transfer, read,
				     is_last))
			return 2;

		buffer += transfer;
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

	if (ihs_i2c_transfer(chip, buffer, 0, I2COP_READ, true))
		return 1;

	return 0;
}

static int ihs_i2c_read(struct i2c_adapter *adap, uchar chip, uint addr,
			int alen, uchar *buffer, int len)
{
	u8 addr_bytes[4];

	put_unaligned_le32(addr, addr_bytes);

	return ihs_i2c_access(adap, chip, addr_bytes, alen, buffer, len,
			      I2COP_READ);
}

static int ihs_i2c_write(struct i2c_adapter *adap, uchar chip, uint addr,
			 int alen, uchar *buffer, int len)
{
	u8 addr_bytes[4];

	put_unaligned_le32(addr, addr_bytes);

	return ihs_i2c_access(adap, chip, addr_bytes, alen, buffer, len,
			      I2COP_WRITE);
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
#ifdef CONFIG_SYS_I2C_IHS_DUAL
U_BOOT_I2C_ADAP_COMPLETE(ihs0_1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_0_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_0_1, 16)
#endif
#endif
#ifdef CONFIG_SYS_I2C_IHS_CH1
U_BOOT_I2C_ADAP_COMPLETE(ihs1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_1, 1)
#ifdef CONFIG_SYS_I2C_IHS_DUAL
U_BOOT_I2C_ADAP_COMPLETE(ihs1_1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_1_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_1_1, 17)
#endif
#endif
#ifdef CONFIG_SYS_I2C_IHS_CH2
U_BOOT_I2C_ADAP_COMPLETE(ihs2, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_2,
			 CONFIG_SYS_I2C_IHS_SLAVE_2, 2)
#ifdef CONFIG_SYS_I2C_IHS_DUAL
U_BOOT_I2C_ADAP_COMPLETE(ihs2_1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_2_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_2_1, 18)
#endif
#endif
#ifdef CONFIG_SYS_I2C_IHS_CH3
U_BOOT_I2C_ADAP_COMPLETE(ihs3, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_3,
			 CONFIG_SYS_I2C_IHS_SLAVE_3, 3)
#ifdef CONFIG_SYS_I2C_IHS_DUAL
U_BOOT_I2C_ADAP_COMPLETE(ihs3_1, ihs_i2c_init, ihs_i2c_probe,
			 ihs_i2c_read, ihs_i2c_write,
			 ihs_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_IHS_SPEED_3_1,
			 CONFIG_SYS_I2C_IHS_SLAVE_3_1, 19)
#endif
#endif
