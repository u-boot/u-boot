/*
 * Basic I2C functions
 *
 * Copyright (c) 2004 Texas Instruments
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author: Jian Zhang jzhang@ti.com, Texas Instruments
 *
 * Copyright (c) 2003 Wolfgang Denk, wd@denx.de
 * Rewritten to fit into the current U-Boot framework
 *
 * Adapted for OMAP2420 I2C, r-woodruff2@ti.com
 *
 */

#include <common.h>

#include <asm/arch/i2c.h>
#include <asm/io.h>

#include "omap24xx_i2c.h"

DECLARE_GLOBAL_DATA_PTR;

#define I2C_STAT_TIMEO	(1 << 31)
#define I2C_TIMEOUT	10

static u32 wait_for_bb(void);
static u32 wait_for_status_mask(u16 mask);
static void flush_fifo(void);

/*
 * For SPL boot some boards need i2c before SDRAM is initialised so force
 * variables to live in SRAM
 */
static struct i2c __attribute__((section (".data"))) *i2c_base =
					(struct i2c *)I2C_DEFAULT_BASE;
static unsigned int __attribute__((section (".data"))) bus_initialized[I2C_BUS_MAX] =
					{ [0 ... (I2C_BUS_MAX-1)] = 0 };
static unsigned int __attribute__((section (".data"))) current_bus = 0;

void i2c_init(int speed, int slaveadd)
{
	int psc, fsscll, fssclh;
	int hsscll = 0, hssclh = 0;
	u32 scll, sclh;

	/* Only handle standard, fast and high speeds */
	if ((speed != OMAP_I2C_STANDARD) &&
	    (speed != OMAP_I2C_FAST_MODE) &&
	    (speed != OMAP_I2C_HIGH_SPEED)) {
		printf("Error : I2C unsupported speed %d\n", speed);
		return;
	}

	psc = I2C_IP_CLK / I2C_INTERNAL_SAMPLING_CLK;
	psc -= 1;
	if (psc < I2C_PSC_MIN) {
		printf("Error : I2C unsupported prescalar %d\n", psc);
		return;
	}

	if (speed == OMAP_I2C_HIGH_SPEED) {
		/* High speed */

		/* For first phase of HS mode */
		fsscll = fssclh = I2C_INTERNAL_SAMPLING_CLK /
			(2 * OMAP_I2C_FAST_MODE);

		fsscll -= I2C_HIGHSPEED_PHASE_ONE_SCLL_TRIM;
		fssclh -= I2C_HIGHSPEED_PHASE_ONE_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			puts("Error : I2C initializing first phase clock\n");
			return;
		}

		/* For second phase of HS mode */
		hsscll = hssclh = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		hsscll -= I2C_HIGHSPEED_PHASE_TWO_SCLL_TRIM;
		hssclh -= I2C_HIGHSPEED_PHASE_TWO_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			puts("Error : I2C initializing second phase clock\n");
			return;
		}

		scll = (unsigned int)hsscll << 8 | (unsigned int)fsscll;
		sclh = (unsigned int)hssclh << 8 | (unsigned int)fssclh;

	} else {
		/* Standard and fast speed */
		fsscll = fssclh = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		fsscll -= I2C_FASTSPEED_SCLL_TRIM;
		fssclh -= I2C_FASTSPEED_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			puts("Error : I2C initializing clock\n");
			return;
		}

		scll = (unsigned int)fsscll;
		sclh = (unsigned int)fssclh;
	}

	if (gd->flags & GD_FLG_RELOC)
		bus_initialized[current_bus] = 1;

	if (readw(&i2c_base->con) & I2C_CON_EN) {
		writew(0, &i2c_base->con);
		udelay(50000);
	}

	writew(psc, &i2c_base->psc);
	writew(scll, &i2c_base->scll);
	writew(sclh, &i2c_base->sclh);

	/* own address */
	writew(slaveadd, &i2c_base->oa);
	writew(I2C_CON_EN, &i2c_base->con);

	/* have to enable intrrupts or OMAP i2c module doesn't work */
	writew(I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
		I2C_IE_NACK_IE | I2C_IE_AL_IE, &i2c_base->ie);
	udelay(1000);
	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	writew(0, &i2c_base->cnt);
}

static void flush_fifo(void)
{	u16 stat;

	/* note: if you try and read data when its not there or ready
	 * you get a bus error
	 */
	while (1) {
		stat = readw(&i2c_base->stat);
		if (stat == I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || \
	defined(CONFIG_OMAP44XX) || defined(CONFIG_AM33XX)
			readb(&i2c_base->data);
#else
			readw(&i2c_base->data);
#endif
			writew(I2C_STAT_RRDY, &i2c_base->stat);
			udelay(1000);
		} else
			break;
	}
}

int i2c_probe(uchar chip)
{
	u32 status;
	int res = 1; /* default = fail */

	if (chip == readw(&i2c_base->oa))
		return res;

	/* wait until bus not busy */
	status = wait_for_bb();
	/* exit on BUS busy */
	if (status & I2C_STAT_TIMEO)
		return res;

	/* try to write one byte */
	writew(1, &i2c_base->cnt);
	/* set slave address */
	writew(chip, &i2c_base->sa);
	/* stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT
			| I2C_CON_STP, &i2c_base->con);
	/* enough delay for the NACK bit set */
	udelay(9000);

	if (!(readw(&i2c_base->stat) & I2C_STAT_NACK)) {
		res = 0;      /* success case */
		flush_fifo();
		writew(0xFFFF, &i2c_base->stat);
	} else {
		/* failure, clear sources*/
		writew(0xFFFF, &i2c_base->stat);
		/* finish up xfer */
		writew(readw(&i2c_base->con) | I2C_CON_STP, &i2c_base->con);
		status = wait_for_bb();
		/* exit on BUS busy */
		if (status & I2C_STAT_TIMEO)
			return res;
	}
	flush_fifo();
	/* don't allow any more data in... we don't want it. */
	writew(0, &i2c_base->cnt);
	writew(0xFFFF, &i2c_base->stat);
	return res;
}

int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int i2c_error = 0, i;
	u32 status;

	if ((alen > 2) || (alen < 0))
		return 1;

	if (alen < 2) {
		if (addr + len > 256)
			return 1;
	} else if (addr + len > 0xFFFF) {
		return 1;
	}

	/* wait until bus not busy */
	status = wait_for_bb();

	/* exit on BUS busy */
	if (status & I2C_STAT_TIMEO)
		return 1;

	writew((alen & 0xFF), &i2c_base->cnt);
	/* set slave address */
	writew(chip, &i2c_base->sa);
	/* Clear the Tx & Rx FIFOs */
	writew((readw(&i2c_base->buf) | I2C_RXFIFO_CLEAR |
		I2C_TXFIFO_CLEAR), &i2c_base->buf);
	/* no stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_TRX |
		I2C_CON_STT, &i2c_base->con);

	/* wait for Transmit ready condition */
	status = wait_for_status_mask(I2C_STAT_XRDY | I2C_STAT_NACK);

	if (status & (I2C_STAT_NACK | I2C_STAT_TIMEO))
		i2c_error = 1;

	if (!i2c_error) {
		if (status & I2C_STAT_XRDY) {
			switch (alen) {
			case 2:
				/* Send address MSByte */
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || \
			defined(CONFIG_AM33XX)
				writew(((addr >> 8) & 0xFF), &i2c_base->data);

				/* Clearing XRDY event */
				writew((status & I2C_STAT_XRDY),
						&i2c_base->stat);
				/* wait for Transmit ready condition */
				status = wait_for_status_mask(I2C_STAT_XRDY |
						I2C_STAT_NACK);

				if (status & (I2C_STAT_NACK |
						I2C_STAT_TIMEO)) {
					i2c_error = 1;
					break;
				}
#endif
			case 1:
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || \
			defined(CONFIG_AM33XX)
				/* Send address LSByte */
				writew((addr & 0xFF), &i2c_base->data);
#else
				/* Send address Short word */
				writew((addr & 0xFFFF), &i2c_base->data);
#endif
				/* Clearing XRDY event */
				writew((status & I2C_STAT_XRDY),
					&i2c_base->stat);
				/*wait for Transmit ready condition */
				status = wait_for_status_mask(I2C_STAT_ARDY |
						I2C_STAT_NACK);

				if (status & (I2C_STAT_NACK |
					I2C_STAT_TIMEO)) {
					i2c_error = 1;
					break;
				}
			}
		} else
			i2c_error = 1;
	}

	/* Wait for ARDY to set */
	status = wait_for_status_mask(I2C_STAT_ARDY | I2C_STAT_NACK
			| I2C_STAT_AL);

	if (!i2c_error) {
		/* set slave address */
		writew(chip, &i2c_base->sa);
		writew((len & 0xFF), &i2c_base->cnt);
		/* Clear the Tx & Rx FIFOs */
		writew((readw(&i2c_base->buf) | I2C_RXFIFO_CLEAR |
			I2C_TXFIFO_CLEAR), &i2c_base->buf);
		/* need stop bit here */
		writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP,
			&i2c_base->con);

		for (i = 0; i < len; i++) {
			/* wait for Receive condition */
			status = wait_for_status_mask(I2C_STAT_RRDY |
				I2C_STAT_NACK);
			if (status & (I2C_STAT_NACK | I2C_STAT_TIMEO)) {
				i2c_error = 1;
				break;
			}

			if (status & I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || \
			defined(CONFIG_AM33XX)
				buffer[i] = readb(&i2c_base->data);
#else
				*((u16 *)&buffer[i]) =
					readw(&i2c_base->data) & 0xFFFF;
				i++;
#endif
				writew((status & I2C_STAT_RRDY),
					&i2c_base->stat);
				udelay(1000);
			} else {
				i2c_error = 1;
			}
		}
	}

	/* Wait for ARDY to set */
	status = wait_for_status_mask(I2C_STAT_ARDY | I2C_STAT_NACK
			| I2C_STAT_AL);

	if (i2c_error) {
		writew(0, &i2c_base->con);
		return 1;
	}

	writew(I2C_CON_EN, &i2c_base->con);

	while (readw(&i2c_base->stat)
		|| (readw(&i2c_base->con) & I2C_CON_MST)) {
		udelay(10000);
		writew(0xFFFF, &i2c_base->stat);
	}

	writew(I2C_CON_EN, &i2c_base->con);
	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	writew(0, &i2c_base->cnt);

	return 0;
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{

	int i, i2c_error = 0;
	u32 status;
	u16 writelen;

	if (alen > 2)
		return 1;

	if (alen < 2) {
		if (addr + len > 256)
			return 1;
	} else if (addr + len > 0xFFFF) {
		return 1;
	}

	/* wait until bus not busy */
	status = wait_for_bb();

	/* exiting on BUS busy */
	if (status & I2C_STAT_TIMEO)
		return 1;

	writelen = (len & 0xFFFF) + alen;

	/* two bytes */
	writew((writelen & 0xFFFF), &i2c_base->cnt);
	/* Clear the Tx & Rx FIFOs */
	writew((readw(&i2c_base->buf) | I2C_RXFIFO_CLEAR |
			I2C_TXFIFO_CLEAR), &i2c_base->buf);
	/* set slave address */
	writew(chip, &i2c_base->sa);
	/* stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
		I2C_CON_STP, &i2c_base->con);

	/* wait for Transmit ready condition */
	status = wait_for_status_mask(I2C_STAT_XRDY | I2C_STAT_NACK);

	if (status & (I2C_STAT_NACK | I2C_STAT_TIMEO))
		i2c_error = 1;

	if (!i2c_error) {
		if (status & I2C_STAT_XRDY) {
			switch (alen) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || \
			defined(CONFIG_AM33XX)
			case 2:
				/* send out MSB byte */
				writeb(((addr >> 8) & 0xFF), &i2c_base->data);
#else
				writeb((addr  & 0xFFFF), &i2c_base->data);
				break;
#endif
				/* Clearing XRDY event */
				writew((status & I2C_STAT_XRDY),
					&i2c_base->stat);
				/*waiting for Transmit ready * condition */
				status = wait_for_status_mask(I2C_STAT_XRDY |
						I2C_STAT_NACK);

				if (status & (I2C_STAT_NACK | I2C_STAT_TIMEO)) {
					i2c_error = 1;
					break;
				}
			case 1:
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || \
			defined(CONFIG_AM33XX)
				/* send out MSB byte */
				writeb((addr  & 0xFF), &i2c_base->data);
#else
				writew(((buffer[0] << 8) | (addr & 0xFF)),
					&i2c_base->data);
#endif
			}

			/* Clearing XRDY event */
			writew((status & I2C_STAT_XRDY), &i2c_base->stat);
		}

		/* waiting for Transmit ready condition */
		status = wait_for_status_mask(I2C_STAT_XRDY | I2C_STAT_NACK);

		if (status & (I2C_STAT_NACK | I2C_STAT_TIMEO))
			i2c_error = 1;

		if (!i2c_error) {
			for (i = ((alen > 1) ? 0 : 1); i < len; i++) {
				if (status & I2C_STAT_XRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || \
				defined(CONFIG_AM33XX)
					writeb((buffer[i] & 0xFF),
						&i2c_base->data);
#else
					writew((((buffer[i] << 8) |
					buffer[i + 1]) & 0xFFFF),
						&i2c_base->data);
					i++;
#endif
				} else
					i2c_error = 1;
					/* Clearing XRDY event */
					writew((status & I2C_STAT_XRDY),
						&i2c_base->stat);
					/* waiting for XRDY condition */
					status = wait_for_status_mask(
						I2C_STAT_XRDY |
						I2C_STAT_ARDY |
						I2C_STAT_NACK);
					if (status & (I2C_STAT_NACK |
						I2C_STAT_TIMEO)) {
						i2c_error = 1;
						break;
					}
					if (status & I2C_STAT_ARDY)
						break;
			}
		}
	}

	status = wait_for_status_mask(I2C_STAT_ARDY | I2C_STAT_NACK |
				I2C_STAT_AL);

	if (status & (I2C_STAT_NACK | I2C_STAT_TIMEO))
		i2c_error = 1;

	if (i2c_error) {
		writew(0, &i2c_base->con);
		return 1;
	}

	if (!i2c_error) {
		int eout = 200;

		writew(I2C_CON_EN, &i2c_base->con);
		while ((status = readw(&i2c_base->stat)) ||
				(readw(&i2c_base->con) & I2C_CON_MST)) {
			udelay(1000);
			/* have to read to clear intrrupt */
			writew(0xFFFF, &i2c_base->stat);
			if (--eout == 0)
				/* better leave with error than hang */
				break;
		}
	}

	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	writew(0, &i2c_base->cnt);
	return 0;
}

static u32 wait_for_bb(void)
{
	int timeout = I2C_TIMEOUT;
	u32 stat;

	while ((stat = readw(&i2c_base->stat) & I2C_STAT_BB) && timeout--) {
		writew(stat, &i2c_base->stat);
		udelay(1000);
	}

	if (timeout <= 0) {
		printf("timed out in wait_for_bb: I2C_STAT=%x\n",
			readw(&i2c_base->stat));
		stat |= I2C_STAT_TIMEO;
	}
	writew(0xFFFF, &i2c_base->stat);	 /* clear delayed stuff*/
	return stat;
}

static u32 wait_for_status_mask(u16 mask)
{
	u32 status;
	int timeout = I2C_TIMEOUT;

	do {
		udelay(1000);
		status = readw(&i2c_base->stat);
	} while (!(status & mask) && timeout--);

	if (timeout <= 0) {
		printf("timed out in wait_for_status_mask: I2C_STAT=%x\n",
			readw(&i2c_base->stat));
		writew(0xFFFF, &i2c_base->stat);
		status |= I2C_STAT_TIMEO;
	}
	return status;
}

int i2c_set_bus_num(unsigned int bus)
{
	if ((bus < 0) || (bus >= I2C_BUS_MAX)) {
		printf("Bad bus: %d\n", bus);
		return -1;
	}

#if I2C_BUS_MAX == 3
	if (bus == 2)
		i2c_base = (struct i2c *)I2C_BASE3;
	else
#endif
	if (bus == 1)
		i2c_base = (struct i2c *)I2C_BASE2;
	else
		i2c_base = (struct i2c *)I2C_BASE1;

	current_bus = bus;

	if (!bus_initialized[current_bus])
		i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	return 0;
}

int i2c_get_bus_num(void)
{
	return (int) current_bus;
}
