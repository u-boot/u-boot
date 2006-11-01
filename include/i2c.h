/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * The original I2C interface was
 *   (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
 *   AIRVENT SAM s.p.a - RIMINI(ITALY)
 * but has been changed substantially.
 */

#ifndef _I2C_H_
#define _I2C_H_

/*
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 * The implementation MUST NOT use static or global variables if the
 * I2C routines are used to read SDRAM configuration information
 * because this is done before the memories are initialized. Limited
 * use of stack-based variables are OK (the initial stack size is
 * limited).
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 */

/*
 * Configuration items.
 */
#define I2C_RXTX_LEN	128	/* maximum tx/rx buffer length */

/*
 * Initialization, must be called once on start up, may be called
 * repeatedly to change the speed and slave addresses.
 */
void i2c_init(int speed, int slaveaddr);
#ifdef CFG_I2C_INIT_BOARD
void i2c_init_board(void);
#endif

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uchar chip);

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);

/*
 * Utility routines to read/write registers.
 */
uchar i2c_reg_read (uchar chip, uchar reg);
void  i2c_reg_write(uchar chip, uchar reg, uchar val);

/*
 * Functions for setting the current I2C bus and its speed
 */

/*
 * i2c_set_bus_num:
 *
 *  Change the active I2C bus.  Subsequent read/write calls will
 *  go to this one.
 *
 * 	bus - bus index, zero based
 *
 * 	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_num(unsigned int bus);

/*
 * i2c_get_bus_num:
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */

unsigned int i2c_get_bus_num(void);

/*
 * i2c_set_bus_speed:
 *
 *  Change the speed of the active I2C bus
 *
 * 	speed - bus speed in Hz
 *
 * 	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_speed(unsigned int);

/*
 * i2c_get_bus_speed:
 *
 *  Returns speed of currently active I2C bus in Hz
 */

unsigned int i2c_get_bus_speed(void);

#endif	/* _I2C_H_ */
