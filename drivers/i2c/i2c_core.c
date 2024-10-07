// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 *
 * (C) Copyright 2012
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Multibus/multiadapter I2C core functions (wrappers)
 */
#include <config.h>
#include <i2c.h>
#include <linker_lists.h>
#include <asm/global_data.h>

struct i2c_adapter *i2c_get_adapter(int index)
{
	struct i2c_adapter *i2c_adap_p = ll_entry_start(struct i2c_adapter,
						i2c);
	int max = ll_entry_count(struct i2c_adapter, i2c);
	int i;

	if (index >= max) {
		printf("Error, wrong i2c adapter %d max %d possible\n",
		       index, max);
		return i2c_adap_p;
	}
	if (index == 0)
		return i2c_adap_p;

	for (i = 0; i < index; i++)
		i2c_adap_p++;

	return i2c_adap_p;
}

DECLARE_GLOBAL_DATA_PTR;

/*
 * i2c_init_bus():
 * ---------------
 *
 * Initializes one bus. Will initialize the parent adapter. No current bus
 * changes, no mux (if any) setup.
 */
static void i2c_init_bus(unsigned int bus_no, int speed, int slaveaddr)
{
	if (bus_no >= CFG_SYS_NUM_I2C_BUSES)
		return;

	I2C_ADAP->init(I2C_ADAP, speed, slaveaddr);

	if (gd->flags & GD_FLG_RELOC) {
		I2C_ADAP->init_done = 1;
		I2C_ADAP->speed = speed;
		I2C_ADAP->slaveaddr = slaveaddr;
	}
}

/* implement possible board specific board init */
__weak void i2c_init_board(void)
{
}

/*
 * i2c_init_all():
 *
 * not longer needed, will deleted. Actual init the SPD_BUS
 * for compatibility.
 * i2c_adap[] must be initialized beforehead with function pointers and
 * data, including speed and slaveaddr.
 */
void i2c_init_all(void)
{
	i2c_init_board();
	i2c_set_bus_num(CONFIG_SYS_SPD_BUS_NUM);
	return;
}

/*
 * i2c_get_bus_num():
 * ------------------
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */
unsigned int i2c_get_bus_num(void)
{
	return gd->cur_i2c_bus;
}

/*
 * i2c_set_bus_num():
 * ------------------
 *
 *  Change the active I2C bus. Subsequent read/write calls will
 *  go to this one. Sets all of the muxes in a proper condition
 *  if that bus is behind muxes.
 *  If previously selected bus is behind the muxes turns off all the
 *  muxes along the path to that bus.
 *
 *	bus - bus index, zero based
 *
 *	Returns: 0 on success, not 0 on failure
 */
int i2c_set_bus_num(unsigned int bus)
{
	int max;

	if ((bus == I2C_BUS) && (I2C_ADAP->init_done > 0))
		return 0;

	max = ll_entry_count(struct i2c_adapter, i2c);
	if (I2C_ADAPTER(bus) >= max) {
		printf("Error, wrong i2c adapter %d max %d possible\n",
		       I2C_ADAPTER(bus), max);
		return -2;
	}

	gd->cur_i2c_bus = bus;
	if (I2C_ADAP->init_done == 0)
		i2c_init_bus(bus, I2C_ADAP->speed, I2C_ADAP->slaveaddr);

	return 0;
}

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uint8_t chip)
{
	return I2C_ADAP->probe(I2C_ADAP, chip);
}

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
int i2c_read(uint8_t chip, unsigned int addr, int alen,
				uint8_t *buffer, int len)
{
	return I2C_ADAP->read(I2C_ADAP, chip, addr, alen, buffer, len);
}

int i2c_write(uint8_t chip, unsigned int addr, int alen,
				uint8_t *buffer, int len)
{
	return I2C_ADAP->write(I2C_ADAP, chip, addr, alen, buffer, len);
}

unsigned int i2c_set_bus_speed(unsigned int speed)
{
	unsigned int ret;

	if (I2C_ADAP->set_bus_speed == NULL)
		return 0;
	ret = I2C_ADAP->set_bus_speed(I2C_ADAP, speed);
	if (gd->flags & GD_FLG_RELOC)
		I2C_ADAP->speed = (ret == 0) ? speed : 0;

	return ret;
}

unsigned int i2c_get_bus_speed(void)
{
	struct i2c_adapter *cur = I2C_ADAP;
	return cur->speed;
}

uint8_t i2c_reg_read(uint8_t addr, uint8_t reg)
{
	uint8_t buf;

	i2c_read(addr, reg, 1, &buf, 1);

#ifdef DEBUG
	printf("%s: bus=%d addr=0x%02x, reg=0x%02x, val=0x%02x\n",
	       __func__, i2c_get_bus_num(), addr, reg, buf);
#endif

	return buf;
}

void i2c_reg_write(uint8_t addr, uint8_t reg, uint8_t val)
{
#ifdef DEBUG
	printf("%s: bus=%d addr=0x%02x, reg=0x%02x, val=0x%02x\n",
	       __func__, i2c_get_bus_num(), addr, reg, val);
#endif

	i2c_write(addr, reg, 1, &val, 1);
}

__weak void i2c_init(int speed, int slaveaddr)
{
	i2c_init_bus(i2c_get_bus_num(), speed, slaveaddr);
}
