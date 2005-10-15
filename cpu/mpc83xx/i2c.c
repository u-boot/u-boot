/*
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
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#ifdef CONFIG_HARD_I2C
#include <i2c.h>
#include <asm/i2c.h>

#if defined(CONFIG_MPC8349ADS) || defined(CONFIG_TQM834X)
i2c_t * mpc8349_i2c = (i2c_t*)(CFG_IMMRBAR + CFG_I2C_OFFSET);
#endif

void
i2c_init(int speed, int slaveadd)
{
	/* stop I2C controller */
	writeb(0x00 , &I2C->cr);

	/* set clock */
	writeb(0x3f, &I2C->fdr);

	/* set default filter */
	writeb(0x10,&I2C->dfsrr);

	/* write slave address */
	writeb(slaveadd, &I2C->adr);

	/* clear status register */
	writeb(0x00, &I2C->sr);

	/* start I2C controller */
	writeb(I2C_CR_MEN, &I2C->cr);
}

static __inline__ int
i2c_wait4bus (void)
{
	ulong timeval = get_timer (0);
	while (readb(&I2C->sr) & I2C_SR_MBB) {
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
		csr = readb(&I2C->sr);

		if (!(csr & I2C_SR_MIF))
			continue;

		writeb(0x0, &I2C->sr);

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
	       &I2C->cr);

	writeb((dev << 1) | dir, &I2C->dr);

	if (i2c_wait (I2C_WRITE) < 0)
		return 0;
	return 1;
}

static __inline__ int
__i2c_write (u8 *data, int length)
{
	int i;

	writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX,
	       &I2C->cr);

	for (i=0; i < length; i++) {
		writeb(data[i], &I2C->dr);

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
	       &I2C->cr);

	/* dummy read */
	readb(&I2C->dr);

	for (i=0; i < length; i++) {
		if (i2c_wait (I2C_READ) < 0)
			break;

		/* Generate ack on last next to last byte */
		if (i == length - 2)
			writeb(I2C_CR_MEN | I2C_CR_MSTA |
			       I2C_CR_TXAK,
			       &I2C->cr);

		/* Generate stop on last byte */
		if (i == length - 1)
			writeb(I2C_CR_MEN | I2C_CR_TXAK, &I2C->cr);

		data[i] = readb(&I2C->dr);
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
	writeb(I2C_CR_MEN, &I2C->cr);
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
	writeb(I2C_CR_MEN, &I2C->cr);
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

#endif /* CONFIG_HARD_I2C */
