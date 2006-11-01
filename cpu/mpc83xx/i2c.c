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

/* Initialize the bus pointer to whatever one the SPD EEPROM is on.
 * Default is bus 1.  This is necessary because the DDR initialization
 * runs from ROM, and we can't switch buses because we can't modify
 * the i2c_dev variable.  Everything gets straightened out once i2c_init
 * is called from RAM.  */

#ifndef CFG_SPD_BUS_NUM
#define CFG_SPD_BUS_NUM 1
#endif

static unsigned int i2c_bus_num = CFG_SPD_BUS_NUM;

#if CFG_SPD_BUS_NUM == 1
static volatile i2c_t *i2c_dev = I2C_1;
#else
static volatile i2c_t *i2c_dev = I2C_2;
#endif

static int i2c_bus_speed[2] = {0, 0};

/*
 * Map the frequency divider to the FDR.  This data is taken from table 17-5
 * of the MPC8349EA reference manual, with duplicates removed.
 */
static struct {
    unsigned int divider;
    u8 fdr;
} i2c_speed_map[] =
{
	{0, 0x20},
	{256, 0x20},
	{288, 0x21},
	{320, 0x22},
	{352, 0x23},
	{384, 0x24},
	{416, 0x01},
	{448, 0x25},
	{480, 0x02},
	{512, 0x26},
	{576, 0x27},
	{640, 0x28},
	{704, 0x05},
	{768, 0x29},
	{832, 0x06},
	{896, 0x2A},
	{1024, 0x2B},
	{1152, 0x08},
	{1280, 0x2C},
	{1536, 0x2D},
	{1792, 0x2E},
	{1920, 0x0B},
	{2048, 0x2F},
	{2304, 0x0C},
	{2560, 0x30},
	{3072, 0x31},
	{3584, 0x32},
	{3840, 0x0F},
	{4096, 0x33},
	{4608, 0x10},
	{5120, 0x34},
	{6144, 0x35},
	{7168, 0x36},
	{7680, 0x13},
	{8192, 0x37},
	{9216, 0x14},
	{10240, 0x38},
	{12288, 0x39},
	{14336, 0x3A},
	{15360, 0x17},
	{16384, 0x3B},
	{18432, 0x18},
	{20480, 0x3C},
	{24576, 0x3D},
	{28672, 0x3E},
	{30720, 0x1B},
	{32768, 0x3F},
	{36864, 0x1C},
	{40960, 0x1D},
	{49152, 0x1E},
	{61440, 0x1F},
	{-1, 0x1F}
};

#define NUM_I2C_SPEEDS (sizeof(i2c_speed_map) / sizeof(i2c_speed_map[0]))

static int set_speed(unsigned int speed)
{
	unsigned long i2c_clk;
	unsigned int divider, i;
	u8 fdr = 0x3F;

	i2c_clk = (i2c_bus_num == 2) ? gd->i2c2_clk : gd->i2c1_clk;

	divider = i2c_clk / speed;

	/* Scan i2c_speed_map[] for the closest matching divider.*/

	for (i = 0; i < NUM_I2C_SPEEDS-1; i++) {
		/* Locate our divider in between two entries in i2c_speed_map[] */
		if ((divider >= i2c_speed_map[i].divider) &&
		    (divider <= i2c_speed_map[i+1].divider)) {
			/* Which one is closer? */
			if ((divider - i2c_speed_map[i].divider) < (i2c_speed_map[i+1].divider - divider)) {
				fdr = i2c_speed_map[i].fdr;
			} else {
				fdr = i2c_speed_map[i+1].fdr;
			}
			break;
		}
	}

	writeb(fdr, &i2c_dev->fdr);
	i2c_bus_speed[i2c_bus_num - 1] = speed;

	return 0;
}


static void _i2c_init(int speed, int slaveadd)
{
	/* stop I2C controller */
	writeb(0x00 , &i2c_dev->cr);

	/* set clock */
	set_speed(speed);

	/* set default filter */
	writeb(IC2_FDR,&i2c_dev->dfsrr);

	/* write slave address */
	writeb(slaveadd, &i2c_dev->adr);

	/* clear status register */
	writeb(I2C_CR_MTX, &i2c_dev->sr);

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

	i2c_set_bus_num(1);
	_i2c_init(speed, slaveadd);
#ifdef	CFG_I2C2_OFFSET
	i2c_set_bus_num(2);
	_i2c_init(speed, slaveadd);
	i2c_set_bus_num(1);
#endif	/* CFG_I2C2_OFFSET */
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

int i2c_set_bus_num(unsigned int bus)
{
	if(bus == 1)
	{
		i2c_dev = I2C_1;
	}
#ifdef	CFG_I2C2_OFFSET
	else if(bus == 2)
	{
		i2c_dev = I2C_2;
	}
#endif
	else
	{
		return -1;
	}
	i2c_bus_num = bus;
	return 0;
}

int i2c_set_bus_speed(unsigned int speed)
{
	return set_speed(speed);
}

unsigned int i2c_get_bus_num(void)
{
	return i2c_bus_num;
}

unsigned int i2c_get_bus_speed(void)
{
	return i2c_bus_speed[i2c_bus_num - 1];
}
#endif /* CONFIG_HARD_I2C */
