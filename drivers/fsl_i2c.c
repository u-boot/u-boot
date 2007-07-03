/*
 * Copyright 2006 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

#ifdef CONFIG_FSL_I2C
#ifdef CONFIG_HARD_I2C

#include <command.h>
#include <i2c.h>		/* Functional interface */

#include <asm/io.h>
#include <asm/fsl_i2c.h>	/* HW definitions */

#define I2C_TIMEOUT	(CFG_HZ / 4)

#define I2C_READ_BIT  1
#define I2C_WRITE_BIT 0

/* Initialize the bus pointer to whatever one the SPD EEPROM is on.
 * Default is bus 0.  This is necessary because the DDR initialization
 * runs from ROM, and we can't switch buses because we can't modify
 * the global variables.
 */
#ifdef CFG_SPD_BUS_NUM
static unsigned int i2c_bus_num __attribute__ ((section ("data"))) = CFG_SPD_BUS_NUM;
#else
static unsigned int i2c_bus_num __attribute__ ((section ("data"))) = 0;
#endif

static volatile struct fsl_i2c *i2c_dev[2] = {
	(struct fsl_i2c *) (CFG_IMMR + CFG_I2C_OFFSET),
#ifdef CFG_I2C2_OFFSET
	(struct fsl_i2c *) (CFG_IMMR + CFG_I2C2_OFFSET)
#endif
};

void
i2c_init(int speed, int slaveadd)
{
	volatile struct fsl_i2c *dev;

	dev = (struct fsl_i2c *) (CFG_IMMR + CFG_I2C_OFFSET);

	writeb(0, &dev->cr);			/* stop I2C controller */
	udelay(5);				/* let it shutdown in peace */
	writeb(0x3F, &dev->fdr);		/* set bus speed */
	writeb(0x3F, &dev->dfsrr);		/* set default filter */
	writeb(slaveadd << 1, &dev->adr);	/* write slave address */
	writeb(0x0, &dev->sr);			/* clear status register */
	writeb(I2C_CR_MEN, &dev->cr);		/* start I2C controller */

#ifdef	CFG_I2C2_OFFSET
	dev = (struct fsl_i2c *) (CFG_IMMR + CFG_I2C2_OFFSET);

	writeb(0, &dev->cr);			/* stop I2C controller */
	udelay(5);				/* let it shutdown in peace */
	writeb(0x3F, &dev->fdr);		/* set bus speed */
	writeb(0x3F, &dev->dfsrr);		/* set default filter */
	writeb(slaveadd << 1, &dev->adr);	/* write slave address */
	writeb(0x0, &dev->sr);			/* clear status register */
	writeb(I2C_CR_MEN, &dev->cr);		/* start I2C controller */
#endif	/* CFG_I2C2_OFFSET */
}

static __inline__ int
i2c_wait4bus(void)
{
	ulong timeval = get_timer(0);

	while (readb(&i2c_dev[i2c_bus_num]->sr) & I2C_SR_MBB) {
		if (get_timer(timeval) > I2C_TIMEOUT) {
			return -1;
		}
	}

	return 0;
}

static __inline__ int
i2c_wait(int write)
{
	u32 csr;
	ulong timeval = get_timer(0);

	do {
		csr = readb(&i2c_dev[i2c_bus_num]->sr);
		if (!(csr & I2C_SR_MIF))
			continue;

		writeb(0x0, &i2c_dev[i2c_bus_num]->sr);

		if (csr & I2C_SR_MAL) {
			debug("i2c_wait: MAL\n");
			return -1;
		}

		if (!(csr & I2C_SR_MCF))	{
			debug("i2c_wait: unfinished\n");
			return -1;
		}

		if (write == I2C_WRITE_BIT && (csr & I2C_SR_RXAK)) {
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
	writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX
	       | (rsta ? I2C_CR_RSTA : 0),
	       &i2c_dev[i2c_bus_num]->cr);

	writeb((dev << 1) | dir, &i2c_dev[i2c_bus_num]->dr);

	if (i2c_wait(I2C_WRITE_BIT) < 0)
		return 0;

	return 1;
}

static __inline__ int
__i2c_write(u8 *data, int length)
{
	int i;

	writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX,
	       &i2c_dev[i2c_bus_num]->cr);

	for (i = 0; i < length; i++) {
		writeb(data[i], &i2c_dev[i2c_bus_num]->dr);

		if (i2c_wait(I2C_WRITE_BIT) < 0)
			break;
	}

	return i;
}

static __inline__ int
__i2c_read(u8 *data, int length)
{
	int i;

	writeb(I2C_CR_MEN | I2C_CR_MSTA | ((length == 1) ? I2C_CR_TXAK : 0),
	       &i2c_dev[i2c_bus_num]->cr);

	/* dummy read */
	readb(&i2c_dev[i2c_bus_num]->dr);

	for (i = 0; i < length; i++) {
		if (i2c_wait(I2C_READ_BIT) < 0)
			break;

		/* Generate ack on last next to last byte */
		if (i == length - 2)
			writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_TXAK,
			       &i2c_dev[i2c_bus_num]->cr);

		/* Generate stop on last byte */
		if (i == length - 1)
			writeb(I2C_CR_MEN | I2C_CR_TXAK, &i2c_dev[i2c_bus_num]->cr);

		data[i] = readb(&i2c_dev[i2c_bus_num]->dr);
	}

	return i;
}

int
i2c_read(u8 dev, uint addr, int alen, u8 *data, int length)
{
	int i = -1; /* signal error */
	u8 *a = (u8*)&addr;

	if (i2c_wait4bus() >= 0
	    && i2c_write_addr(dev, I2C_WRITE_BIT, 0) != 0
	    && __i2c_write(&a[4 - alen], alen) == alen)
		i = 0; /* No error so far */

	if (length
	    && i2c_write_addr(dev, I2C_READ_BIT, 1) != 0)
		i = __i2c_read(data, length);

	writeb(I2C_CR_MEN, &i2c_dev[i2c_bus_num]->cr);

	if (i == length)
	    return 0;

	return -1;
}

int
i2c_write(u8 dev, uint addr, int alen, u8 *data, int length)
{
	int i = -1; /* signal error */
	u8 *a = (u8*)&addr;

	if (i2c_wait4bus() >= 0
	    && i2c_write_addr(dev, I2C_WRITE_BIT, 0) != 0
	    && __i2c_write(&a[4 - alen], alen) == alen) {
		i = __i2c_write(data, length);
	}

	writeb(I2C_CR_MEN, &i2c_dev[i2c_bus_num]->cr);

	if (i == length)
	    return 0;

	return -1;
}

int
i2c_probe(uchar chip)
{
	/* For unknow reason the controller will ACK when
	 * probing for a slave with the same address, so skip
	 * it.
	 */
	if (chip == (readb(&i2c_dev[i2c_bus_num]->adr) >> 1))
		return -1;

	return i2c_read(chip, 0, 0, NULL, 0);
}

uchar
i2c_reg_read(uchar i2c_addr, uchar reg)
{
	uchar buf[1];

	i2c_read(i2c_addr, reg, 1, buf, 1);

	return buf[0];
}

void
i2c_reg_write(uchar i2c_addr, uchar reg, uchar val)
{
	i2c_write(i2c_addr, reg, 1, &val, 1);
}

int i2c_set_bus_num(unsigned int bus)
{
#ifdef CFG_I2C2_OFFSET
	if (bus > 1) {
#else
	if (bus > 0) {
#endif
		return -1;
	}

	i2c_bus_num = bus;

	return 0;
}

int i2c_set_bus_speed(unsigned int speed)
{
	return -1;
}

unsigned int i2c_get_bus_num(void)
{
	return i2c_bus_num;
}

unsigned int i2c_get_bus_speed(void)
{
	return 0;
}
#endif /* CONFIG_HARD_I2C */
#endif /* CONFIG_FSL_I2C */
