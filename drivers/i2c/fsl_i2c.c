/*
 * Copyright 2006,2009 Freescale Semiconductor, Inc.
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

#ifdef CONFIG_HARD_I2C

#include <command.h>
#include <i2c.h>		/* Functional interface */

#include <asm/io.h>
#include <asm/fsl_i2c.h>	/* HW definitions */

/* The maximum number of microseconds we will wait until another master has
 * released the bus.  If not defined in the board header file, then use a
 * generic value.
 */
#ifndef CONFIG_I2C_MBB_TIMEOUT
#define CONFIG_I2C_MBB_TIMEOUT	100000
#endif

/* The maximum number of microseconds we will wait for a read or write
 * operation to complete.  If not defined in the board header file, then use a
 * generic value.
 */
#ifndef CONFIG_I2C_TIMEOUT
#define CONFIG_I2C_TIMEOUT	10000
#endif

#define I2C_READ_BIT  1
#define I2C_WRITE_BIT 0

DECLARE_GLOBAL_DATA_PTR;

/* Initialize the bus pointer to whatever one the SPD EEPROM is on.
 * Default is bus 0.  This is necessary because the DDR initialization
 * runs from ROM, and we can't switch buses because we can't modify
 * the global variables.
 */
#ifndef CONFIG_SYS_SPD_BUS_NUM
#define CONFIG_SYS_SPD_BUS_NUM 0
#endif
static unsigned int i2c_bus_num __attribute__ ((section (".data"))) = CONFIG_SYS_SPD_BUS_NUM;
#if defined(CONFIG_I2C_MUX)
static unsigned int i2c_bus_num_mux __attribute__ ((section ("data"))) = 0;
#endif

static unsigned int i2c_bus_speed[2] = {CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SPEED};

static const struct fsl_i2c *i2c_dev[2] = {
	(struct fsl_i2c *) (CONFIG_SYS_IMMR + CONFIG_SYS_I2C_OFFSET),
#ifdef CONFIG_SYS_I2C2_OFFSET
	(struct fsl_i2c *) (CONFIG_SYS_IMMR + CONFIG_SYS_I2C2_OFFSET)
#endif
};

/* I2C speed map for a DFSR value of 1 */

/*
 * Map I2C frequency dividers to FDR and DFSR values
 *
 * This structure is used to define the elements of a table that maps I2C
 * frequency divider (I2C clock rate divided by I2C bus speed) to a value to be
 * programmed into the Frequency Divider Ratio (FDR) and Digital Filter
 * Sampling Rate (DFSR) registers.
 *
 * The actual table should be defined in the board file, and it must be called
 * fsl_i2c_speed_map[].
 *
 * The last entry of the table must have a value of {-1, X}, where X is same
 * FDR/DFSR values as the second-to-last entry.  This guarantees that any
 * search through the array will always find a match.
 *
 * The values of the divider must be in increasing numerical order, i.e.
 * fsl_i2c_speed_map[x+1].divider > fsl_i2c_speed_map[x].divider.
 *
 * For this table, the values are based on a value of 1 for the DFSR
 * register.  See the application note AN2919 "Determining the I2C Frequency
 * Divider Ratio for SCL"
 *
 * ColdFire I2C frequency dividers for FDR values are different from
 * PowerPC. The protocol to use the I2C module is still the same.
 * A different table is defined and are based on MCF5xxx user manual.
 *
 */
static const struct {
	unsigned short divider;
	u8 fdr;
} fsl_i2c_speed_map[] = {
#ifdef __M68K__
	{20, 32}, {22, 33}, {24, 34}, {26, 35},
	{28, 0}, {28, 36}, {30, 1}, {32, 37},
	{34, 2}, {36, 38}, {40, 3}, {40, 39},
	{44, 4}, {48, 5}, {48, 40}, {56, 6},
	{56, 41}, {64, 42}, {68, 7}, {72, 43},
	{80, 8}, {80, 44}, {88, 9}, {96, 41},
	{104, 10}, {112, 42}, {128, 11}, {128, 43},
	{144, 12}, {160, 13}, {160, 48}, {192, 14},
	{192, 49}, {224, 50}, {240, 15}, {256, 51},
	{288, 16}, {320, 17}, {320, 52}, {384, 18},
	{384, 53}, {448, 54}, {480, 19}, {512, 55},
	{576, 20}, {640, 21}, {640, 56}, {768, 22},
	{768, 57}, {960, 23}, {896, 58}, {1024, 59},
	{1152, 24}, {1280, 25}, {1280, 60}, {1536, 26},
	{1536, 61}, {1792, 62}, {1920, 27}, {2048, 63},
	{2304, 28}, {2560, 29}, {3072, 30}, {3840, 31},
	{-1, 31}
#endif
};

/**
 * Set the I2C bus speed for a given I2C device
 *
 * @param dev: the I2C device
 * @i2c_clk: I2C bus clock frequency
 * @speed: the desired speed of the bus
 *
 * The I2C device must be stopped before calling this function.
 *
 * The return value is the actual bus speed that is set.
 */
static unsigned int set_i2c_bus_speed(const struct fsl_i2c *dev,
	unsigned int i2c_clk, unsigned int speed)
{
	unsigned short divider = min(i2c_clk / speed, (unsigned short) -1);

	/*
	 * We want to choose an FDR/DFSR that generates an I2C bus speed that
	 * is equal to or lower than the requested speed.  That means that we
	 * want the first divider that is equal to or greater than the
	 * calculated divider.
	 */
#ifdef __PPC__
	u8 dfsr, fdr = 0x31; /* Default if no FDR found */
	/* a, b and dfsr matches identifiers A,B and C respectively in AN2919 */
	unsigned short a, b, ga, gb;
	unsigned long c_div, est_div;

#ifdef CONFIG_FSL_I2C_CUSTOM_DFSR
	dfsr = CONFIG_FSL_I2C_CUSTOM_DFSR;
#else
	/* Condition 1: dfsr <= 50/T */
	dfsr = (5 * (i2c_clk / 1000)) / 100000;
#endif
#ifdef CONFIG_FSL_I2C_CUSTOM_FDR
	fdr = CONFIG_FSL_I2C_CUSTOM_FDR;
	speed = i2c_clk / divider; /* Fake something */
#else
	debug("Requested speed:%d, i2c_clk:%d\n", speed, i2c_clk);
	if (!dfsr)
		dfsr = 1;

	est_div = ~0;
	for (ga = 0x4, a = 10; a <= 30; ga++, a += 2) {
		for (gb = 0; gb < 8; gb++) {
			b = 16 << gb;
			c_div = b * (a + ((3*dfsr)/b)*2);
			if ((c_div > divider) && (c_div < est_div)) {
				unsigned short bin_gb, bin_ga;

				est_div = c_div;
				bin_gb = gb << 2;
				bin_ga = (ga & 0x3) | ((ga & 0x4) << 3);
				fdr = bin_gb | bin_ga;
				speed = i2c_clk / est_div;
				debug("FDR:0x%.2x, div:%ld, ga:0x%x, gb:0x%x, "
				      "a:%d, b:%d, speed:%d\n",
				      fdr, est_div, ga, gb, a, b, speed);
				/* Condition 2 not accounted for */
				debug("Tr <= %d ns\n",
				      (b - 3 * dfsr) * 1000000 /
				      (i2c_clk / 1000));
			}
		}
		if (a == 20)
			a += 2;
		if (a == 24)
			a += 4;
	}
	debug("divider:%d, est_div:%ld, DFSR:%d\n", divider, est_div, dfsr);
	debug("FDR:0x%.2x, speed:%d\n", fdr, speed);
#endif
	writeb(dfsr, &dev->dfsrr);	/* set default filter */
	writeb(fdr, &dev->fdr);		/* set bus speed */
#else
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(fsl_i2c_speed_map); i++)
		if (fsl_i2c_speed_map[i].divider >= divider) {
			u8 fdr;

			fdr = fsl_i2c_speed_map[i].fdr;
			speed = i2c_clk / fsl_i2c_speed_map[i].divider;
			writeb(fdr, &dev->fdr);		/* set bus speed */

			break;
		}
#endif
	return speed;
}

void
i2c_init(int speed, int slaveadd)
{
	struct fsl_i2c *dev;
	unsigned int temp;

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	/* call board specific i2c bus reset routine before accessing the   */
	/* environment, which might be in a chip on that bus. For details   */
	/* about this problem see doc/I2C_Edge_Conditions.                  */
	i2c_init_board();
#endif
	dev = (struct fsl_i2c *) (CONFIG_SYS_IMMR + CONFIG_SYS_I2C_OFFSET);

	writeb(0, &dev->cr);			/* stop I2C controller */
	udelay(5);				/* let it shutdown in peace */
	temp = set_i2c_bus_speed(dev, gd->i2c1_clk, speed);
	if (gd->flags & GD_FLG_RELOC)
		i2c_bus_speed[0] = temp;
	writeb(slaveadd << 1, &dev->adr);	/* write slave address */
	writeb(0x0, &dev->sr);			/* clear status register */
	writeb(I2C_CR_MEN, &dev->cr);		/* start I2C controller */

#ifdef	CONFIG_SYS_I2C2_OFFSET
	dev = (struct fsl_i2c *) (CONFIG_SYS_IMMR + CONFIG_SYS_I2C2_OFFSET);

	writeb(0, &dev->cr);			/* stop I2C controller */
	udelay(5);				/* let it shutdown in peace */
	temp = set_i2c_bus_speed(dev, gd->i2c2_clk, speed);
	if (gd->flags & GD_FLG_RELOC)
		i2c_bus_speed[1] = temp;
	writeb(slaveadd << 1, &dev->adr);	/* write slave address */
	writeb(0x0, &dev->sr);			/* clear status register */
	writeb(I2C_CR_MEN, &dev->cr);		/* start I2C controller */
#endif
}

static int
i2c_wait4bus(void)
{
	unsigned long long timeval = get_ticks();
	const unsigned long long timeout = usec2ticks(CONFIG_I2C_MBB_TIMEOUT);

	while (readb(&i2c_dev[i2c_bus_num]->sr) & I2C_SR_MBB) {
		if ((get_ticks() - timeval) > timeout)
			return -1;
	}

	return 0;
}

static __inline__ int
i2c_wait(int write)
{
	u32 csr;
	unsigned long long timeval = get_ticks();
	const unsigned long long timeout = usec2ticks(CONFIG_I2C_TIMEOUT);

	do {
		csr = readb(&i2c_dev[i2c_bus_num]->sr);
		if (!(csr & I2C_SR_MIF))
			continue;
		/* Read again to allow register to stabilise */
		csr = readb(&i2c_dev[i2c_bus_num]->sr);

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
	} while ((get_ticks() - timeval) < timeout);

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

		/* Do not generate stop on last byte */
		if (i == length - 1)
			writeb(I2C_CR_MEN | I2C_CR_MSTA | I2C_CR_MTX,
			       &i2c_dev[i2c_bus_num]->cr);

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

	if (i2c_wait4bus()) /* Wait until STOP */
		debug("i2c_read: wait4bus timed out\n");

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
	if (i2c_wait4bus()) /* Wait until STOP */
		debug("i2c_write: wait4bus timed out\n");

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

int i2c_set_bus_num(unsigned int bus)
{
#if defined(CONFIG_I2C_MUX)
	if (bus < CONFIG_SYS_MAX_I2C_BUS) {
		i2c_bus_num = bus;
	} else {
		int	ret;

		ret = i2x_mux_select_mux(bus);
		if (ret)
			return ret;
		i2c_bus_num = 0;
	}
	i2c_bus_num_mux = bus;
#else
#ifdef CONFIG_SYS_I2C2_OFFSET
	if (bus > 1) {
#else
	if (bus > 0) {
#endif
		return -1;
	}

	i2c_bus_num = bus;
#endif
	return 0;
}

int i2c_set_bus_speed(unsigned int speed)
{
	unsigned int i2c_clk = (i2c_bus_num == 1) ? gd->i2c2_clk : gd->i2c1_clk;

	writeb(0, &i2c_dev[i2c_bus_num]->cr);		/* stop controller */
	i2c_bus_speed[i2c_bus_num] =
		set_i2c_bus_speed(i2c_dev[i2c_bus_num], i2c_clk, speed);
	writeb(I2C_CR_MEN, &i2c_dev[i2c_bus_num]->cr);	/* start controller */

	return 0;
}

unsigned int i2c_get_bus_num(void)
{
#if defined(CONFIG_I2C_MUX)
	return i2c_bus_num_mux;
#else
	return i2c_bus_num;
#endif
}

unsigned int i2c_get_bus_speed(void)
{
	return i2c_bus_speed[i2c_bus_num];
}

#endif /* CONFIG_HARD_I2C */
