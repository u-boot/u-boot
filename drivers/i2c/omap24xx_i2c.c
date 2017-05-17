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
 * Copyright (c) 2013 Lubomir Popov <lpopov@mm-sol.com>, MM Solutions
 * New i2c_read, i2c_write and i2c_probe functions, tested on OMAP4
 * (4430/60/70), OMAP5 (5430) and AM335X (3359); should work on older
 * OMAPs and derivatives as well. The only anticipated exception would
 * be the OMAP2420, which shall require driver modification.
 * - Rewritten i2c_read to operate correctly with all types of chips
 *   (old function could not read consistent data from some I2C slaves).
 * - Optimized i2c_write.
 * - New i2c_probe, performs write access vs read. The old probe could
 *   hang the system under certain conditions (e.g. unconfigured pads).
 * - The read/write/probe functions try to identify unconfigured bus.
 * - Status functions now read irqstatus_raw as per TRM guidelines
 *   (except for OMAP243X and OMAP34XX).
 * - Driver now supports up to I2C5 (OMAP5).
 *
 * Copyright (c) 2014 Hannes Schmelzer <oe5hpm@oevsv.at>, B&R
 * - Added support for set_speed
 *
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>

#include <asm/arch/i2c.h>
#include <asm/io.h>

#include "omap24xx_i2c.h"

DECLARE_GLOBAL_DATA_PTR;

#define I2C_TIMEOUT	1000

/* Absolutely safe for status update at 100 kHz I2C: */
#define I2C_WAIT	200

struct omap_i2c {
	struct udevice *clk;
	struct i2c *regs;
	unsigned int speed;
	int waitdelay;
	int clk_id;
};

static int omap24_i2c_findpsc(u32 *pscl, u32 *psch, uint speed)
{
	unsigned long internal_clk = 0, fclk;
	unsigned int prescaler;

	/*
	 * This method is only called for Standard and Fast Mode speeds
	 *
	 * For some TI SoCs it is explicitly written in TRM (e,g, SPRUHZ6G,
	 * page 5685, Table 24-7)
	 * that the internal I2C clock (after prescaler) should be between
	 * 7-12 MHz (at least for Fast Mode (FS)).
	 *
	 * Such approach is used in v4.9 Linux kernel in:
	 * ./drivers/i2c/busses/i2c-omap.c (omap_i2c_init function).
	 */

	speed /= 1000; /* convert speed to kHz */

	if (speed > 100)
		internal_clk = 9600;
	else
		internal_clk = 4000;

	fclk = I2C_IP_CLK / 1000;
	prescaler = fclk / internal_clk;
	prescaler = prescaler - 1;

	if (speed > 100) {
		unsigned long scl;

		/* Fast mode */
		scl = internal_clk / speed;
		*pscl = scl - (scl / 3) - I2C_FASTSPEED_SCLL_TRIM;
		*psch = (scl / 3) - I2C_FASTSPEED_SCLH_TRIM;
	} else {
		/* Standard mode */
		*pscl = internal_clk / (speed * 2) - I2C_FASTSPEED_SCLL_TRIM;
		*psch = internal_clk / (speed * 2) - I2C_FASTSPEED_SCLH_TRIM;
	}

	debug("%s: speed [kHz]: %d psc: 0x%x sscl: 0x%x ssch: 0x%x\n",
	      __func__, speed, prescaler, *pscl, *psch);

	if (*pscl <= 0 || *psch <= 0 || prescaler <= 0)
		return -EINVAL;

	return prescaler;
}

/*
 * Wait for the bus to be free by checking the Bus Busy (BB)
 * bit to become clear
 */
static int wait_for_bb(struct i2c *i2c_base, int waitdelay)
{
	int timeout = I2C_TIMEOUT;
	u16 stat;

	writew(0xFFFF, &i2c_base->stat);	/* clear current interrupts...*/
#if defined(CONFIG_OMAP34XX)
	while ((stat = readw(&i2c_base->stat) & I2C_STAT_BB) && timeout--) {
#else
	/* Read RAW status */
	while ((stat = readw(&i2c_base->irqstatus_raw) &
		I2C_STAT_BB) && timeout--) {
#endif
		writew(stat, &i2c_base->stat);
		udelay(waitdelay);
	}

	if (timeout <= 0) {
		printf("Timed out in wait_for_bb: status=%04x\n",
		       stat);
		return 1;
	}
	writew(0xFFFF, &i2c_base->stat);	 /* clear delayed stuff*/
	return 0;
}

/*
 * Wait for the I2C controller to complete current action
 * and update status
 */
static u16 wait_for_event(struct i2c *i2c_base, int waitdelay)
{
	u16 status;
	int timeout = I2C_TIMEOUT;

	do {
		udelay(waitdelay);
#if defined(CONFIG_OMAP34XX)
		status = readw(&i2c_base->stat);
#else
		/* Read RAW status */
		status = readw(&i2c_base->irqstatus_raw);
#endif
	} while (!(status &
		   (I2C_STAT_ROVR | I2C_STAT_XUDF | I2C_STAT_XRDY |
		    I2C_STAT_RRDY | I2C_STAT_ARDY | I2C_STAT_NACK |
		    I2C_STAT_AL)) && timeout--);

	if (timeout <= 0) {
		printf("Timed out in wait_for_event: status=%04x\n",
		       status);
		/*
		 * If status is still 0 here, probably the bus pads have
		 * not been configured for I2C, and/or pull-ups are missing.
		 */
		printf("Check if pads/pull-ups of bus are properly configured\n");
		writew(0xFFFF, &i2c_base->stat);
		status = 0;
	}

	return status;
}

static void flush_fifo(struct i2c *i2c_base)
{
	u16 stat;

	/*
	 * note: if you try and read data when its not there or ready
	 * you get a bus error
	 */
	while (1) {
		stat = readw(&i2c_base->stat);
		if (stat == I2C_STAT_RRDY) {
			readb(&i2c_base->data);
			writew(I2C_STAT_RRDY, &i2c_base->stat);
			udelay(1000);
		} else
			break;
	}
}

static int __omap24_i2c_setspeed(struct i2c *i2c_base, uint speed,
				 int *waitdelay)
{
	int psc, fsscll = 0, fssclh = 0;
	int hsscll = 0, hssclh = 0;
	u32 scll = 0, sclh = 0;

	if (speed >= OMAP_I2C_HIGH_SPEED) {
		/* High speed */
		psc = I2C_IP_CLK / I2C_INTERNAL_SAMPLING_CLK;
		psc -= 1;
		if (psc < I2C_PSC_MIN) {
			printf("Error : I2C unsupported prescaler %d\n", psc);
			return -1;
		}

		/* For first phase of HS mode */
		fsscll = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		fssclh = fsscll;

		fsscll -= I2C_HIGHSPEED_PHASE_ONE_SCLL_TRIM;
		fssclh -= I2C_HIGHSPEED_PHASE_ONE_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			puts("Error : I2C initializing first phase clock\n");
			return -1;
		}

		/* For second phase of HS mode */
		hsscll = hssclh = I2C_INTERNAL_SAMPLING_CLK / (2 * speed);

		hsscll -= I2C_HIGHSPEED_PHASE_TWO_SCLL_TRIM;
		hssclh -= I2C_HIGHSPEED_PHASE_TWO_SCLH_TRIM;
		if (((fsscll < 0) || (fssclh < 0)) ||
		    ((fsscll > 255) || (fssclh > 255))) {
			puts("Error : I2C initializing second phase clock\n");
			return -1;
		}

		scll = (unsigned int)hsscll << 8 | (unsigned int)fsscll;
		sclh = (unsigned int)hssclh << 8 | (unsigned int)fssclh;

	} else {
		/* Standard and fast speed */
		psc = omap24_i2c_findpsc(&scll, &sclh, speed);
		if (0 > psc) {
			puts("Error : I2C initializing clock\n");
			return -1;
		}
	}

	*waitdelay = (10000000 / speed) * 2; /* wait for 20 clkperiods */
	writew(0, &i2c_base->con);
	writew(psc, &i2c_base->psc);
	writew(scll, &i2c_base->scll);
	writew(sclh, &i2c_base->sclh);
	writew(I2C_CON_EN, &i2c_base->con);
	writew(0xFFFF, &i2c_base->stat);	/* clear all pending status */

	return 0;
}

static void omap24_i2c_deblock(struct i2c *i2c_base)
{
	int i;
	u16 systest;
	u16 orgsystest;

	/* set test mode ST_EN = 1 */
	orgsystest = readw(&i2c_base->systest);
	systest = orgsystest;
	/* enable testmode */
	systest |= I2C_SYSTEST_ST_EN;
	writew(systest, &i2c_base->systest);
	systest &= ~I2C_SYSTEST_TMODE_MASK;
	systest |= 3 << I2C_SYSTEST_TMODE_SHIFT;
	writew(systest, &i2c_base->systest);

	/* set SCL, SDA  = 1 */
	systest |= I2C_SYSTEST_SCL_O | I2C_SYSTEST_SDA_O;
	writew(systest, &i2c_base->systest);
	udelay(10);

	/* toggle scl 9 clocks */
	for (i = 0; i < 9; i++) {
		/* SCL = 0 */
		systest &= ~I2C_SYSTEST_SCL_O;
		writew(systest, &i2c_base->systest);
		udelay(10);
		/* SCL = 1 */
		systest |= I2C_SYSTEST_SCL_O;
		writew(systest, &i2c_base->systest);
		udelay(10);
	}

	/* send stop */
	systest &= ~I2C_SYSTEST_SDA_O;
	writew(systest, &i2c_base->systest);
	udelay(10);
	systest |= I2C_SYSTEST_SCL_O | I2C_SYSTEST_SDA_O;
	writew(systest, &i2c_base->systest);
	udelay(10);

	/* restore original mode */
	writew(orgsystest, &i2c_base->systest);
}

static void __omap24_i2c_init(struct i2c *i2c_base, int speed, int slaveadd,
			      int *waitdelay)
{
	int timeout = I2C_TIMEOUT;
	int deblock = 1;

retry:
	if (readw(&i2c_base->con) & I2C_CON_EN) {
		writew(0, &i2c_base->con);
		udelay(50000);
	}

	writew(0x2, &i2c_base->sysc); /* for ES2 after soft reset */
	udelay(1000);

	writew(I2C_CON_EN, &i2c_base->con);
	while (!(readw(&i2c_base->syss) & I2C_SYSS_RDONE) && timeout--) {
		if (timeout <= 0) {
			puts("ERROR: Timeout in soft-reset\n");
			return;
		}
		udelay(1000);
	}

	if (0 != __omap24_i2c_setspeed(i2c_base, speed, waitdelay)) {
		printf("ERROR: failed to setup I2C bus-speed!\n");
		return;
	}

	/* own address */
	writew(slaveadd, &i2c_base->oa);

#if defined(CONFIG_OMAP34XX)
	/*
	 * Have to enable interrupts for OMAP2/3, these IPs don't have
	 * an 'irqstatus_raw' register and we shall have to poll 'stat'
	 */
	writew(I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
	       I2C_IE_NACK_IE | I2C_IE_AL_IE, &i2c_base->ie);
#endif
	udelay(1000);
	flush_fifo(i2c_base);
	writew(0xFFFF, &i2c_base->stat);

	/* Handle possible failed I2C state */
	if (wait_for_bb(i2c_base, *waitdelay))
		if (deblock == 1) {
			omap24_i2c_deblock(i2c_base);
			deblock = 0;
			goto retry;
		}
}

/*
 * i2c_probe: Use write access. Allows to identify addresses that are
 *            write-only (like the config register of dual-port EEPROMs)
 */
static int __omap24_i2c_probe(struct i2c *i2c_base, int waitdelay, uchar chip)
{
	u16 status;
	int res = 1; /* default = fail */

	if (chip == readw(&i2c_base->oa))
		return res;

	/* Wait until bus is free */
	if (wait_for_bb(i2c_base, waitdelay))
		return res;

	/* No data transfer, slave addr only */
	writew(chip, &i2c_base->sa);
	/* Stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
	       I2C_CON_STP, &i2c_base->con);

	status = wait_for_event(i2c_base, waitdelay);

	if ((status & ~I2C_STAT_XRDY) == 0 || (status & I2C_STAT_AL)) {
		/*
		 * With current high-level command implementation, notifying
		 * the user shall flood the console with 127 messages. If
		 * silent exit is desired upon unconfigured bus, remove the
		 * following 'if' section:
		 */
		if (status == I2C_STAT_XRDY)
			printf("i2c_probe: pads on bus probably not configured (status=0x%x)\n",
			       status);

		goto pr_exit;
	}

	/* Check for ACK (!NAK) */
	if (!(status & I2C_STAT_NACK)) {
		res = 0;				/* Device found */
		udelay(waitdelay);/* Required by AM335X in SPL */
		/* Abort transfer (force idle state) */
		writew(I2C_CON_MST | I2C_CON_TRX, &i2c_base->con); /* Reset */
		udelay(1000);
		writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_TRX |
		       I2C_CON_STP, &i2c_base->con);		/* STP */
	}
pr_exit:
	flush_fifo(i2c_base);
	writew(0xFFFF, &i2c_base->stat);
	return res;
}

/*
 * i2c_read: Function now uses a single I2C read transaction with bulk transfer
 *           of the requested number of bytes (note that the 'i2c md' command
 *           limits this to 16 bytes anyway). If CONFIG_I2C_REPEATED_START is
 *           defined in the board config header, this transaction shall be with
 *           Repeated Start (Sr) between the address and data phases; otherwise
 *           Stop-Start (P-S) shall be used (some I2C chips do require a P-S).
 *           The address (reg offset) may be 0, 1 or 2 bytes long.
 *           Function now reads correctly from chips that return more than one
 *           byte of data per addressed register (like TI temperature sensors),
 *           or that do not need a register address at all (such as some clock
 *           distributors).
 */
static int __omap24_i2c_read(struct i2c *i2c_base, int waitdelay, uchar chip,
			     uint addr, int alen, uchar *buffer, int len)
{
	int i2c_error = 0;
	u16 status;

	if (alen < 0) {
		puts("I2C read: addr len < 0\n");
		return 1;
	}
	if (len < 0) {
		puts("I2C read: data len < 0\n");
		return 1;
	}
	if (buffer == NULL) {
		puts("I2C read: NULL pointer passed\n");
		return 1;
	}

	if (alen > 2) {
		printf("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > (1 << 16)) {
		puts("I2C read: address out of range\n");
		return 1;
	}

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones
	 * like Catalyst 24WC04/08/16 which has 9/10/11 bits of
	 * address and the extra bits end up in the "chip address"
	 * bit slots. This makes a 24WC08 (1Kbyte) chip look like
	 * four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to
	 * still be one byte because the extra address bits are
	 * hidden in the chip address.
	 */
	if (alen > 0)
		chip |= ((addr >> (alen * 8)) &
			 CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif

	/* Wait until bus not busy */
	if (wait_for_bb(i2c_base, waitdelay))
		return 1;

	/* Zero, one or two bytes reg address (offset) */
	writew(alen, &i2c_base->cnt);
	/* Set slave address */
	writew(chip, &i2c_base->sa);

	if (alen) {
		/* Must write reg offset first */
#ifdef CONFIG_I2C_REPEATED_START
		/* No stop bit, use Repeated Start (Sr) */
		writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT |
		       I2C_CON_TRX, &i2c_base->con);
#else
		/* Stop - Start (P-S) */
		writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP |
		       I2C_CON_TRX, &i2c_base->con);
#endif
		/* Send register offset */
		while (1) {
			status = wait_for_event(i2c_base, waitdelay);
			/* Try to identify bus that is not padconf'd for I2C */
			if (status == I2C_STAT_XRDY) {
				i2c_error = 2;
				printf("i2c_read (addr phase): pads on bus probably not configured (status=0x%x)\n",
				       status);
				goto rd_exit;
			}
			if (status == 0 || (status & I2C_STAT_NACK)) {
				i2c_error = 1;
				printf("i2c_read: error waiting for addr ACK (status=0x%x)\n",
				       status);
				goto rd_exit;
			}
			if (alen) {
				if (status & I2C_STAT_XRDY) {
					alen--;
					/* Do we have to use byte access? */
					writeb((addr >> (8 * alen)) & 0xff,
					       &i2c_base->data);
					writew(I2C_STAT_XRDY, &i2c_base->stat);
				}
			}
			if (status & I2C_STAT_ARDY) {
				writew(I2C_STAT_ARDY, &i2c_base->stat);
				break;
			}
		}
	}
	/* Set slave address */
	writew(chip, &i2c_base->sa);
	/* Read len bytes from slave */
	writew(len, &i2c_base->cnt);
	/* Need stop bit here */
	writew(I2C_CON_EN | I2C_CON_MST |
	       I2C_CON_STT | I2C_CON_STP,
	       &i2c_base->con);

	/* Receive data */
	while (1) {
		status = wait_for_event(i2c_base, waitdelay);
		/*
		 * Try to identify bus that is not padconf'd for I2C. This
		 * state could be left over from previous transactions if
		 * the address phase is skipped due to alen=0.
		 */
		if (status == I2C_STAT_XRDY) {
			i2c_error = 2;
			printf("i2c_read (data phase): pads on bus probably not configured (status=0x%x)\n",
			       status);
			goto rd_exit;
		}
		if (status == 0 || (status & I2C_STAT_NACK)) {
			i2c_error = 1;
			goto rd_exit;
		}
		if (status & I2C_STAT_RRDY) {
			*buffer++ = readb(&i2c_base->data);
			writew(I2C_STAT_RRDY, &i2c_base->stat);
		}
		if (status & I2C_STAT_ARDY) {
			writew(I2C_STAT_ARDY, &i2c_base->stat);
			break;
		}
	}

rd_exit:
	flush_fifo(i2c_base);
	writew(0xFFFF, &i2c_base->stat);
	return i2c_error;
}

/* i2c_write: Address (reg offset) may be 0, 1 or 2 bytes long. */
static int __omap24_i2c_write(struct i2c *i2c_base, int waitdelay, uchar chip,
			      uint addr, int alen, uchar *buffer, int len)
{
	int i;
	u16 status;
	int i2c_error = 0;
	int timeout = I2C_TIMEOUT;

	if (alen < 0) {
		puts("I2C write: addr len < 0\n");
		return 1;
	}

	if (len < 0) {
		puts("I2C write: data len < 0\n");
		return 1;
	}

	if (buffer == NULL) {
		puts("I2C write: NULL pointer passed\n");
		return 1;
	}

	if (alen > 2) {
		printf("I2C write: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > (1 << 16)) {
		printf("I2C write: address 0x%x + 0x%x out of range\n",
		       addr, len);
		return 1;
	}

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones
	 * like Catalyst 24WC04/08/16 which has 9/10/11 bits of
	 * address and the extra bits end up in the "chip address"
	 * bit slots. This makes a 24WC08 (1Kbyte) chip look like
	 * four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to
	 * still be one byte because the extra address bits are
	 * hidden in the chip address.
	 */
	if (alen > 0)
		chip |= ((addr >> (alen * 8)) &
			 CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif

	/* Wait until bus not busy */
	if (wait_for_bb(i2c_base, waitdelay))
		return 1;

	/* Start address phase - will write regoffset + len bytes data */
	writew(alen + len, &i2c_base->cnt);
	/* Set slave address */
	writew(chip, &i2c_base->sa);
	/* Stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
	       I2C_CON_STP, &i2c_base->con);

	while (alen) {
		/* Must write reg offset (one or two bytes) */
		status = wait_for_event(i2c_base, waitdelay);
		/* Try to identify bus that is not padconf'd for I2C */
		if (status == I2C_STAT_XRDY) {
			i2c_error = 2;
			printf("i2c_write: pads on bus probably not configured (status=0x%x)\n",
			       status);
			goto wr_exit;
		}
		if (status == 0 || (status & I2C_STAT_NACK)) {
			i2c_error = 1;
			printf("i2c_write: error waiting for addr ACK (status=0x%x)\n",
			       status);
			goto wr_exit;
		}
		if (status & I2C_STAT_XRDY) {
			alen--;
			writeb((addr >> (8 * alen)) & 0xff, &i2c_base->data);
			writew(I2C_STAT_XRDY, &i2c_base->stat);
		} else {
			i2c_error = 1;
			printf("i2c_write: bus not ready for addr Tx (status=0x%x)\n",
			       status);
			goto wr_exit;
		}
	}
	/* Address phase is over, now write data */
	for (i = 0; i < len; i++) {
		status = wait_for_event(i2c_base, waitdelay);
		if (status == 0 || (status & I2C_STAT_NACK)) {
			i2c_error = 1;
			printf("i2c_write: error waiting for data ACK (status=0x%x)\n",
			       status);
			goto wr_exit;
		}
		if (status & I2C_STAT_XRDY) {
			writeb(buffer[i], &i2c_base->data);
			writew(I2C_STAT_XRDY, &i2c_base->stat);
		} else {
			i2c_error = 1;
			printf("i2c_write: bus not ready for data Tx (i=%d)\n",
			       i);
			goto wr_exit;
		}
	}
	/*
	 * poll ARDY bit for making sure that last byte really has been
	 * transferred on the bus.
	 */
	do {
		status = wait_for_event(i2c_base, waitdelay);
	} while (!(status & I2C_STAT_ARDY) && timeout--);
	if (timeout <= 0)
		printf("i2c_write: timed out writig last byte!\n");

wr_exit:
	flush_fifo(i2c_base);
	writew(0xFFFF, &i2c_base->stat);
	return i2c_error;
}

#ifndef CONFIG_DM_I2C
/*
 * The legacy I2C functions. These need to get removed once
 * all users of this driver are converted to DM.
 */
static struct i2c *omap24_get_base(struct i2c_adapter *adap)
{
	switch (adap->hwadapnr) {
	case 0:
		return (struct i2c *)I2C_BASE1;
		break;
	case 1:
		return (struct i2c *)I2C_BASE2;
		break;
#if (I2C_BUS_MAX > 2)
	case 2:
		return (struct i2c *)I2C_BASE3;
		break;
#if (I2C_BUS_MAX > 3)
	case 3:
		return (struct i2c *)I2C_BASE4;
		break;
#if (I2C_BUS_MAX > 4)
	case 4:
		return (struct i2c *)I2C_BASE5;
		break;
#endif
#endif
#endif
	default:
		printf("wrong hwadapnr: %d\n", adap->hwadapnr);
		break;
	}
	return NULL;
}


static int omap24_i2c_read(struct i2c_adapter *adap, uchar chip, uint addr,
			   int alen, uchar *buffer, int len)
{
	struct i2c *i2c_base = omap24_get_base(adap);

	return __omap24_i2c_read(i2c_base, adap->waitdelay, chip, addr,
				 alen, buffer, len);
}


static int omap24_i2c_write(struct i2c_adapter *adap, uchar chip, uint addr,
			    int alen, uchar *buffer, int len)
{
	struct i2c *i2c_base = omap24_get_base(adap);

	return __omap24_i2c_write(i2c_base, adap->waitdelay, chip, addr,
				  alen, buffer, len);
}

static uint omap24_i2c_setspeed(struct i2c_adapter *adap, uint speed)
{
	struct i2c *i2c_base = omap24_get_base(adap);
	int ret;

	ret = __omap24_i2c_setspeed(i2c_base, speed, &adap->waitdelay);
	if (ret) {
		error("%s: set i2c speed failed\n", __func__);
		return ret;
	}

	adap->speed = speed;

	return 0;
}

static void omap24_i2c_init(struct i2c_adapter *adap, int speed, int slaveadd)
{
	struct i2c *i2c_base = omap24_get_base(adap);

	return __omap24_i2c_init(i2c_base, speed, slaveadd, &adap->waitdelay);
}

static int omap24_i2c_probe(struct i2c_adapter *adap, uchar chip)
{
	struct i2c *i2c_base = omap24_get_base(adap);

	return __omap24_i2c_probe(i2c_base, adap->waitdelay, chip);
}

#if !defined(CONFIG_SYS_OMAP24_I2C_SPEED1)
#define CONFIG_SYS_OMAP24_I2C_SPEED1 CONFIG_SYS_OMAP24_I2C_SPEED
#endif
#if !defined(CONFIG_SYS_OMAP24_I2C_SLAVE1)
#define CONFIG_SYS_OMAP24_I2C_SLAVE1 CONFIG_SYS_OMAP24_I2C_SLAVE
#endif

U_BOOT_I2C_ADAP_COMPLETE(omap24_0, omap24_i2c_init, omap24_i2c_probe,
			 omap24_i2c_read, omap24_i2c_write, omap24_i2c_setspeed,
			 CONFIG_SYS_OMAP24_I2C_SPEED,
			 CONFIG_SYS_OMAP24_I2C_SLAVE,
			 0)
U_BOOT_I2C_ADAP_COMPLETE(omap24_1, omap24_i2c_init, omap24_i2c_probe,
			 omap24_i2c_read, omap24_i2c_write, omap24_i2c_setspeed,
			 CONFIG_SYS_OMAP24_I2C_SPEED1,
			 CONFIG_SYS_OMAP24_I2C_SLAVE1,
			 1)
#if (I2C_BUS_MAX > 2)
#if !defined(CONFIG_SYS_OMAP24_I2C_SPEED2)
#define CONFIG_SYS_OMAP24_I2C_SPEED2 CONFIG_SYS_OMAP24_I2C_SPEED
#endif
#if !defined(CONFIG_SYS_OMAP24_I2C_SLAVE2)
#define CONFIG_SYS_OMAP24_I2C_SLAVE2 CONFIG_SYS_OMAP24_I2C_SLAVE
#endif

U_BOOT_I2C_ADAP_COMPLETE(omap24_2, omap24_i2c_init, omap24_i2c_probe,
			 omap24_i2c_read, omap24_i2c_write, NULL,
			 CONFIG_SYS_OMAP24_I2C_SPEED2,
			 CONFIG_SYS_OMAP24_I2C_SLAVE2,
			 2)
#if (I2C_BUS_MAX > 3)
#if !defined(CONFIG_SYS_OMAP24_I2C_SPEED3)
#define CONFIG_SYS_OMAP24_I2C_SPEED3 CONFIG_SYS_OMAP24_I2C_SPEED
#endif
#if !defined(CONFIG_SYS_OMAP24_I2C_SLAVE3)
#define CONFIG_SYS_OMAP24_I2C_SLAVE3 CONFIG_SYS_OMAP24_I2C_SLAVE
#endif

U_BOOT_I2C_ADAP_COMPLETE(omap24_3, omap24_i2c_init, omap24_i2c_probe,
			 omap24_i2c_read, omap24_i2c_write, NULL,
			 CONFIG_SYS_OMAP24_I2C_SPEED3,
			 CONFIG_SYS_OMAP24_I2C_SLAVE3,
			 3)
#if (I2C_BUS_MAX > 4)
#if !defined(CONFIG_SYS_OMAP24_I2C_SPEED4)
#define CONFIG_SYS_OMAP24_I2C_SPEED4 CONFIG_SYS_OMAP24_I2C_SPEED
#endif
#if !defined(CONFIG_SYS_OMAP24_I2C_SLAVE4)
#define CONFIG_SYS_OMAP24_I2C_SLAVE4 CONFIG_SYS_OMAP24_I2C_SLAVE
#endif

U_BOOT_I2C_ADAP_COMPLETE(omap24_4, omap24_i2c_init, omap24_i2c_probe,
			 omap24_i2c_read, omap24_i2c_write, NULL,
			 CONFIG_SYS_OMAP24_I2C_SPEED4,
			 CONFIG_SYS_OMAP24_I2C_SLAVE4,
			 4)
#endif
#endif
#endif

#else /* CONFIG_DM_I2C */

static int omap_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	struct omap_i2c *priv = dev_get_priv(bus);
	int ret;

	debug("i2c_xfer: %d messages\n", nmsgs);
	for (; nmsgs > 0; nmsgs--, msg++) {
		debug("i2c_xfer: chip=0x%x, len=0x%x\n", msg->addr, msg->len);
		if (msg->flags & I2C_M_RD) {
			ret = __omap24_i2c_read(priv->regs, priv->waitdelay,
						msg->addr, 0, 0, msg->buf,
						msg->len);
		} else {
			ret = __omap24_i2c_write(priv->regs, priv->waitdelay,
						 msg->addr, 0, 0, msg->buf,
						 msg->len);
		}
		if (ret) {
			debug("i2c_write: error sending\n");
			return -EREMOTEIO;
		}
	}

	return 0;
}

static int omap_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct omap_i2c *priv = dev_get_priv(bus);

	priv->speed = speed;

	return __omap24_i2c_setspeed(priv->regs, speed, &priv->waitdelay);
}

static int omap_i2c_probe_chip(struct udevice *bus, uint chip_addr,
				     uint chip_flags)
{
	struct omap_i2c *priv = dev_get_priv(bus);

	return __omap24_i2c_probe(priv->regs, priv->waitdelay, chip_addr);
}

static int omap_i2c_probe(struct udevice *bus)
{
	struct omap_i2c *priv = dev_get_priv(bus);

	__omap24_i2c_init(priv->regs, priv->speed, 0, &priv->waitdelay);

	return 0;
}

static int omap_i2c_ofdata_to_platdata(struct udevice *bus)
{
	struct omap_i2c *priv = dev_get_priv(bus);

	priv->regs = map_physmem(devfdt_get_addr(bus), sizeof(void *),
				 MAP_NOCACHE);
	priv->speed = CONFIG_SYS_OMAP24_I2C_SPEED;

	return 0;
}

static const struct dm_i2c_ops omap_i2c_ops = {
	.xfer		= omap_i2c_xfer,
	.probe_chip	= omap_i2c_probe_chip,
	.set_bus_speed	= omap_i2c_set_bus_speed,
};

static const struct udevice_id omap_i2c_ids[] = {
	{ .compatible = "ti,omap3-i2c" },
	{ .compatible = "ti,omap4-i2c" },
	{ }
};

U_BOOT_DRIVER(i2c_omap) = {
	.name	= "i2c_omap",
	.id	= UCLASS_I2C,
	.of_match = omap_i2c_ids,
	.ofdata_to_platdata = omap_i2c_ofdata_to_platdata,
	.probe	= omap_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct omap_i2c),
	.ops	= &omap_i2c_ops,
	.flags  = DM_FLAG_PRE_RELOC,
};

#endif /* CONFIG_DM_I2C */
