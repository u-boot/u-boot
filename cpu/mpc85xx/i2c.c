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
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#ifdef CONFIG_HARD_I2C
#include <i2c.h>

#define TIMEOUT (CFG_HZ/4)

#define I2C_Addr ((u8 *)(CFG_CCSRBAR + 0x3000))

#define I2CADR  &I2C_Addr[0]
#define I2CFDR  &I2C_Addr[4]
#define I2CCCR  &I2C_Addr[8]
#define I2CCSR  &I2C_Addr[12]
#define I2CCDR  &I2C_Addr[16]
#define I2CDFSRR &I2C_Addr[20]

#define I2C_READ  1
#define I2C_WRITE 0

void
i2c_init(int speed, int slaveadd)
{
	/* stop I2C controller */
	writeb(0x0, I2CCCR);

	/* set clock */
	writeb(0x3f, I2CFDR);

	/* set default filter */
	writeb(0x10,I2CDFSRR);

	/* write slave address */
	writeb(slaveadd, I2CADR);

	/* clear status register */
	writeb(0x0, I2CCSR);

	/* start I2C controller */
	writeb(MPC85xx_I2CCR_MEN, I2CCCR);
}

static __inline__ int
i2c_wait4bus (void)
{
	ulong timeval = get_timer (0);

	while (readb(I2CCSR) & MPC85xx_I2CSR_MBB) {
		if (get_timer (timeval) > TIMEOUT) {
			return -1;
		}
	}

  return 0;
}

static __inline__ int
i2c_wait (int write)
{
	u32 csr;
	ulong timeval = get_timer (0);

	do {
		csr = readb(I2CCSR);

		if (!(csr & MPC85xx_I2CSR_MIF))
			continue;

		writeb(0x0, I2CCSR);

		if (csr & MPC85xx_I2CSR_MAL) {
			debug("i2c_wait: MAL\n");
			return -1;
		}

		if (!(csr & MPC85xx_I2CSR_MCF))	{
			debug("i2c_wait: unfinished\n");
			return -1;
		}

		if (write == I2C_WRITE && (csr & MPC85xx_I2CSR_RXAK)) {
			debug("i2c_wait: No RXACK\n");
			return -1;
		}

		return 0;
	} while (get_timer (timeval) < TIMEOUT);

	debug("i2c_wait: timed out\n");
	return -1;
}

static __inline__ int
i2c_write_addr (u8 dev, u8 dir, int rsta)
{
	writeb(MPC85xx_I2CCR_MEN | MPC85xx_I2CCR_MSTA | MPC85xx_I2CCR_MTX |
	       (rsta?MPC85xx_I2CCR_RSTA:0),
	       I2CCCR);

	writeb((dev << 1) | dir, I2CCDR);

	if (i2c_wait (I2C_WRITE) < 0)
		return 0;

	return 1;
}

static __inline__ int
__i2c_write (u8 *data, int length)
{
	int i;

	writeb(MPC85xx_I2CCR_MEN | MPC85xx_I2CCR_MSTA | MPC85xx_I2CCR_MTX,
	       I2CCCR);

	for (i=0; i < length; i++) {
		writeb(data[i], I2CCDR);

		if (i2c_wait (I2C_WRITE) < 0)
			break;
	}

	return i;
}

static __inline__ int
__i2c_read (u8 *data, int length)
{
	int i;

	writeb(MPC85xx_I2CCR_MEN | MPC85xx_I2CCR_MSTA |
	       ((length == 1) ? MPC85xx_I2CCR_TXAK : 0),
	       I2CCCR);

	/* dummy read */
	readb(I2CCDR);

	for (i=0; i < length; i++) {
		if (i2c_wait (I2C_READ) < 0)
			break;

		/* Generate ack on last next to last byte */
		if (i == length - 2)
			writeb(MPC85xx_I2CCR_MEN | MPC85xx_I2CCR_MSTA |
			       MPC85xx_I2CCR_TXAK,
			       I2CCCR);

		/* Generate stop on last byte */
		if (i == length - 1)
			writeb(MPC85xx_I2CCR_MEN | MPC85xx_I2CCR_TXAK, I2CCCR);

		data[i] = readb(I2CCDR);
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
	writeb(MPC85xx_I2CCR_MEN, I2CCCR);

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
	writeb(MPC85xx_I2CCR_MEN, I2CCCR);

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
