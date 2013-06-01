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
 */

#include <common.h>

#include <asm/arch/i2c.h>
#include <asm/io.h>

#include "omap24xx_i2c.h"

DECLARE_GLOBAL_DATA_PTR;

#define I2C_TIMEOUT	1000

/* Absolutely safe for status update at 100 kHz I2C: */
#define I2C_WAIT	200

static int wait_for_bb(void);
static u16 wait_for_event(void);
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
	int timeout = I2C_TIMEOUT;

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

	writew(0, &i2c_base->con);
	writew(psc, &i2c_base->psc);
	writew(scll, &i2c_base->scll);
	writew(sclh, &i2c_base->sclh);

	/* own address */
	writew(slaveadd, &i2c_base->oa);
	writew(I2C_CON_EN, &i2c_base->con);
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX)
	/*
	 * Have to enable interrupts for OMAP2/3, these IPs don't have
	 * an 'irqstatus_raw' register and we shall have to poll 'stat'
	 */
	writew(I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
	       I2C_IE_NACK_IE | I2C_IE_AL_IE, &i2c_base->ie);
#endif
	udelay(1000);
	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	writew(0, &i2c_base->cnt);

	if (gd->flags & GD_FLG_RELOC)
		bus_initialized[current_bus] = 1;
}

static void flush_fifo(void)
{	u16 stat;

	/* note: if you try and read data when its not there or ready
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

/*
 * i2c_probe: Use write access. Allows to identify addresses that are
 *            write-only (like the config register of dual-port EEPROMs)
 */
int i2c_probe(uchar chip)
{
	u16 status;
	int res = 1; /* default = fail */

	if (chip == readw(&i2c_base->oa))
		return res;

	/* Wait until bus is free */
	if (wait_for_bb())
		return res;

	/* No data transfer, slave addr only */
	writew(0, &i2c_base->cnt);
	/* Set slave address */
	writew(chip, &i2c_base->sa);
	/* Stop bit needed here */
	writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX |
	       I2C_CON_STP, &i2c_base->con);

	status = wait_for_event();

	if ((status & ~I2C_STAT_XRDY) == 0 || (status & I2C_STAT_AL)) {
		/*
		 * With current high-level command implementation, notifying
		 * the user shall flood the console with 127 messages. If
		 * silent exit is desired upon unconfigured bus, remove the
		 * following 'if' section:
		 */
		if (status == I2C_STAT_XRDY)
			printf("i2c_probe: pads on bus %d probably not configured (status=0x%x)\n",
			       current_bus, status);

		goto pr_exit;
	}

	/* Check for ACK (!NAK) */
	if (!(status & I2C_STAT_NACK)) {
		res = 0;			/* Device found */
		udelay(I2C_WAIT);		/* Required by AM335X in SPL */
		/* Abort transfer (force idle state) */
		writew(I2C_CON_MST | I2C_CON_TRX, &i2c_base->con); /* Reset */
		udelay(1000);
		writew(I2C_CON_EN | I2C_CON_MST | I2C_CON_TRX |
		       I2C_CON_STP, &i2c_base->con);		/* STP */
	}
pr_exit:
	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	writew(0, &i2c_base->cnt);
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
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
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

	/* Wait until bus not busy */
	if (wait_for_bb())
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
			status = wait_for_event();
			/* Try to identify bus that is not padconf'd for I2C */
			if (status == I2C_STAT_XRDY) {
				i2c_error = 2;
				printf("i2c_read (addr phase): pads on bus %d probably not configured (status=0x%x)\n",
				       current_bus, status);
				goto rd_exit;
			}
			if (status == 0 || status & I2C_STAT_NACK) {
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
		status = wait_for_event();
		/*
		 * Try to identify bus that is not padconf'd for I2C. This
		 * state could be left over from previous transactions if
		 * the address phase is skipped due to alen=0.
		 */
		if (status == I2C_STAT_XRDY) {
			i2c_error = 2;
			printf("i2c_read (data phase): pads on bus %d probably not configured (status=0x%x)\n",
			       current_bus, status);
			goto rd_exit;
		}
		if (status == 0 || status & I2C_STAT_NACK) {
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
	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	writew(0, &i2c_base->cnt);
	return i2c_error;
}

/* i2c_write: Address (reg offset) may be 0, 1 or 2 bytes long. */
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int i;
	u16 status;
	int i2c_error = 0;

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

	/* Wait until bus not busy */
	if (wait_for_bb())
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
		status = wait_for_event();
		/* Try to identify bus that is not padconf'd for I2C */
		if (status == I2C_STAT_XRDY) {
			i2c_error = 2;
			printf("i2c_write: pads on bus %d probably not configured (status=0x%x)\n",
			       current_bus, status);
			goto wr_exit;
		}
		if (status == 0 || status & I2C_STAT_NACK) {
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
		status = wait_for_event();
		if (status == 0 || status & I2C_STAT_NACK) {
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

wr_exit:
	flush_fifo();
	writew(0xFFFF, &i2c_base->stat);
	writew(0, &i2c_base->cnt);
	return i2c_error;
}

/*
 * Wait for the bus to be free by checking the Bus Busy (BB)
 * bit to become clear
 */
static int wait_for_bb(void)
{
	int timeout = I2C_TIMEOUT;
	u16 stat;

	writew(0xFFFF, &i2c_base->stat);	/* clear current interrupts...*/
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX)
	while ((stat = readw(&i2c_base->stat) & I2C_STAT_BB) && timeout--) {
#else
	/* Read RAW status */
	while ((stat = readw(&i2c_base->irqstatus_raw) &
		I2C_STAT_BB) && timeout--) {
#endif
		writew(stat, &i2c_base->stat);
		udelay(I2C_WAIT);
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
static u16 wait_for_event(void)
{
	u16 status;
	int timeout = I2C_TIMEOUT;

	do {
		udelay(I2C_WAIT);
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX)
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
		printf("Check if pads/pull-ups of bus %d are properly configured\n",
		       current_bus);
		writew(0xFFFF, &i2c_base->stat);
		status = 0;
	}

	return status;
}

int i2c_set_bus_num(unsigned int bus)
{
	if (bus >= I2C_BUS_MAX) {
		printf("Bad bus: %x\n", bus);
		return -1;
	}

	switch (bus) {
	default:
		bus = 0;	/* Fall through */
	case 0:
		i2c_base = (struct i2c *)I2C_BASE1;
		break;
	case 1:
		i2c_base = (struct i2c *)I2C_BASE2;
		break;
#if (I2C_BUS_MAX > 2)
	case 2:
		i2c_base = (struct i2c *)I2C_BASE3;
		break;
#if (I2C_BUS_MAX > 3)
	case 3:
		i2c_base = (struct i2c *)I2C_BASE4;
		break;
#if (I2C_BUS_MAX > 4)
	case 4:
		i2c_base = (struct i2c *)I2C_BASE5;
		break;
#endif
#endif
#endif
	}

	current_bus = bus;

	if (!bus_initialized[current_bus])
		i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	return 0;
}

int i2c_get_bus_num(void)
{
	return (int) current_bus;
}
