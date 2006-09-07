/*
 * (C) Copyright 2006 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2003,Motorola Inc.
 * Xianghua Xiao <x.xiao@motorola.com>
 * Adapted for Motorola 85xx chip.
 *
 * (C) Copyright 2003
 * Gleb Natapov <gnatapov@mrv.com>
 * Some bits are taken from linux driver writen by adrian@humboldt.co.uk
 *
 * Hardware I2C driver for MPC107 PCI bridge.
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
 * Change log:
 *
 * 20050101: Eran Liberty (liberty@freescale.com)
 *           Initial file creating (porting from 85XX & 8260)
 * 20060601: Dave Liu (daveliu@freescale.com)
 *           Unified variable names for mpc83xx
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#ifdef CONFIG_HARD_I2C
#include <i2c.h>
#include <asm/i2c.h>

DECLARE_GLOBAL_DATA_PTR;

/* Three I2C bus speeds are supported here (50kHz, 100kHz
 * and 400kHz).  It should be easy to add more.  Note that
 * the maximum bus speed for I2C bus 1 is CSB/3, while I2C
 * bus 2 can go as high as CSB.
 * Typical values for CSB are 266MHz and 200MHz.  */

                                                                      /* 50kH  100kHz  400kHz */
static const uchar speed_map_266[][3] =
                                                                      {{0x2e,         0x2a,   0x20},  /* base 88MHz */
                                                                       {0x34,         0x30,   0x28}}; /* base 266 MHz */

static const uchar speed_map_200[][3] =
                                                                      {{0x2c,         0x28,   0x20},  /* base 66 MHz */
                                                                       {0x33,         0x2f,   0x26}}; /* base 200 MHz */

/* Initialize the bus pointer to whatever one the SPD EEPROM is on.
 * Default is bus 1.  This is necessary because the DDR initialization
 * runs from ROM, and we can't switch buses because we can't modify
 * the i2c_dev variable.  Everything gets straightened out once i2c_init
 * is called from RAM.  */

#if defined CFG_SPD_BUS_NUM
static i2c_t *i2c_dev = CFG_SPD_BUS_NUM;
#else
static i2c_t *i2c_dev = I2C_1;
#endif

static uchar busNum = I2C_BUS_1 ;
static int bus_speed[2] = {0, 0};

static int set_speed(int speed)
{
      uchar value;
      const uchar *spdPtr;

      /* Global data contains maximum I2C bus 1 speed, which is CSB/3 */
      if(gd->i2c_clk == 88000000)
      {
              spdPtr = speed_map_266[busNum];
      }
      else if(gd->i2c_clk == 66000000)
      {
              spdPtr = speed_map_200[busNum];
      }
      else
      {
              printf("Max I2C bus speed %d not supported\n", gd->i2c_clk);
              return -1;
      }

      switch(speed)
      {
              case 50000:
                      value   = *(spdPtr + 0);
                      break;
              case 100000:
                      value   = *(spdPtr + 1);
                      break;
              case 400000:
                      value   = *(spdPtr + 2);
                      break;
              default:
                      printf("I2C bus speed %d not supported\n", speed);
                      return -2;
      }
      /* set clock */
      writeb(value, &i2c_dev->fdr);
      bus_speed[busNum] = speed;
      return 0;
}


static void _i2c_init(int speed, int slaveadd)
{
	/* stop I2C controller */
	writeb(0x00 , &i2c_dev->cr);

	/* set clock */
	writeb(speed, &i2c_dev->fdr);

	/* set default filter */
	writeb(0x10,&i2c_dev->dfsrr);

	/* write slave address */
	writeb(slaveadd, &i2c_dev->adr);

	/* clear status register */
	writeb(0x00, &i2c_dev->sr);

	/* start I2C controller */
	writeb(I2C_CR_MEN, &i2c_dev->cr);
}

void i2c_init(int speed, int slaveadd)
{
      /* Set both interfaces to the same speed and slave address */
      /* Note: This function gets called twice - before and after
       * relocation to RAM.  The first time it's called, we are unable
       * to change buses, so whichever one 'i2c_dev' was initialized to
       * gets set twice.  When run from RAM both buses get set properly */

      i2c_set_bus_num(I2C_BUS_1);
      _i2c_init(speed, slaveadd);
#ifdef        CFG_I2C2_OFFSET
      i2c_set_bus_num(I2C_BUS_2);
      _i2c_init(speed, slaveadd);
      i2c_set_bus_num(I2C_BUS_1);
#endif        /* CFG_I2C2_OFFSET */
}

static __inline__ int
i2c_wait4bus (void)
{
	ulong timeval = get_timer (0);
	while (readb(&i2c_dev->sr) & I2C_SR_MBB) {
		if (get_timer (timeval) > I2C_TIMEOUT) {
			return -1;
		}
	}
	return 0;
}

static __inline__ int
i2c_wait (int write)
{
	u32 csr;
	ulong timeval = get_timer(0);
	do {
		csr = readb(&i2c_dev->sr);

		if (!(csr & I2C_SR_MIF))
			continue;

		writeb(0x0, &i2c_dev->sr);

		if (csr & I2C_SR_MAL) {
			debug("i2c_wait: MAL\n");
			return -1;
		}

		if (!(csr & I2C_SR_MCF))	{
			debug("i2c_wait: unfinished\n");
			return -1;
		}

		if (write == I2C_WRITE && (csr & I2C_SR_RXAK)) {
			debug("i2c_wait: No RXACK\n");
			return -1;
		}

		return 0;
	} while (get_timer (timeval) < I2C_TIMEOUT);

	debug("i2c_wait: timed out\n");
	return -1;
}

static __inline__ int
i2c_write_addr (u8 dev, u8 dir, int rsta)
{
	writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX |
	       (rsta?I2C_CR_RSTA:0),
	       &i2c_dev->cr);

	writeb((dev << 1) | dir, &i2c_dev->dr);

	if (i2c_wait (I2C_WRITE) < 0)
		return 0;
	return 1;
}

static __inline__ int
__i2c_write (u8 *data, int length)
{
	int i;

	writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX,
	       &i2c_dev->cr);

	for (i=0; i < length; i++) {
		writeb(data[i], &i2c_dev->dr);

		if (i2c_wait (I2C_WRITE) < 0)
			break;
	}
	return i;
}

static __inline__ int
__i2c_read (u8 *data, int length)
{
	int i;

	writeb(I2C_CR_MEN | I2C_CR_MSTA |
	       ((length == 1) ? I2C_CR_TXAK : 0),
	       &i2c_dev->cr);

	/* dummy read */
	readb(&i2c_dev->dr);

	for (i=0; i < length; i++) {
		if (i2c_wait (I2C_READ) < 0)
			break;

		/* Generate ack on last next to last byte */
		if (i == length - 2)
			writeb(I2C_CR_MEN | I2C_CR_MSTA |
			       I2C_CR_TXAK,
			       &i2c_dev->cr);

		/* Generate stop on last byte */
		if (i == length - 1)
			writeb(I2C_CR_MEN | I2C_CR_TXAK, &i2c_dev->cr);

		data[i] = readb(&i2c_dev->dr);
	}
	return i;
}

int
i2c_read (u8 dev, uint addr, int alen, u8 *data, int length)
{
	int i = 0;
	u8 *a = (u8*)&addr;

	if (i2c_wait4bus () < 0)
		goto exit;

	if (i2c_write_addr (dev, I2C_WRITE, 0) == 0)
		goto exit;

	if (__i2c_write (&a[4 - alen], alen) != alen)
		goto exit;

	if (i2c_write_addr (dev, I2C_READ, 1) == 0)
		goto exit;

	i = __i2c_read (data, length);

 exit:
	writeb(I2C_CR_MEN, &i2c_dev->cr);
	return !(i == length);
}

int
i2c_write (u8 dev, uint addr, int alen, u8 *data, int length)
{
	int i = 0;
	u8 *a = (u8*)&addr;

	if (i2c_wait4bus () < 0)
		goto exit;

	if (i2c_write_addr (dev, I2C_WRITE, 0) == 0)
		goto exit;

	if (__i2c_write (&a[4 - alen], alen) != alen)
		goto exit;

	i = __i2c_write (data, length);

 exit:
	writeb(I2C_CR_MEN, &i2c_dev->cr);
	return !(i == length);
}

int i2c_probe (uchar chip)
{
	int tmp;

	/*
	 * Try to read the first location of the chip.  The underlying
	 * driver doesn't appear to support sending just the chip address
	 * and looking for an <ACK> back.
	 */
	udelay(10000);
	return i2c_read (chip, 0, 1, (uchar *)&tmp, 1);
}

uchar i2c_reg_read (uchar i2c_addr, uchar reg)
{
	uchar buf[1];

	i2c_read (i2c_addr, reg, 1, buf, 1);

	return (buf[0]);
}

void i2c_reg_write (uchar i2c_addr, uchar reg, uchar val)
{
	i2c_write (i2c_addr, reg, 1, &val, 1);
}

int 	i2c_set_bus_num(uchar bus)
{
	if(bus == I2C_BUS_1)
	{
		i2c_dev = I2C_1;
	}
#ifdef	CFG_I2C2_OFFSET
	else if(bus == I2C_BUS_2)
	{
		i2c_dev = I2C_2;
	}
#endif	/* CFG_I2C2_OFFSET */
	else
	{
		return -1;
	}
	busNum = bus;
	return 0;
}

int 	i2c_set_bus_speed(int speed)
{
	return set_speed(speed);
}

uchar i2c_get_bus_num(void)
{
	return busNum;
}

int 	i2c_get_bus_speed(void)
{
	return bus_speed[busNum];
}
#endif /* CONFIG_HARD_I2C */
