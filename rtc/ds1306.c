/*
 * (C) Copyright 2002 SIXNET, dge@sixnetio.com.
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

/*
 * Date & Time support for DS1306 RTC using software SPI
 */

#include <common.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_RTC_DS1306) && (CONFIG_COMMANDS & CFG_CMD_DATE)

static unsigned int bin2bcd(unsigned int n);
static unsigned char bcd2bin(unsigned char c);
static void soft_spi_send(unsigned char n);
static unsigned char soft_spi_read(void);
static void init_spi(void);

/*-----------------------------------------------------------------------
 * Definitions
 */

#define	PB_SPISCK	0x00000002	/* PB 30 */
#define PB_SPIMOSI	0x00000004	/* PB 29 */
#define PB_SPIMISO	0x00000008	/* PB 28 */
#define PB_SPI_CE	0x00010000	/* PB 15 */

/* ------------------------------------------------------------------------- */

/* read clock time from DS1306 and return it in *tmp */
void rtc_get(struct rtc_time *tmp)
{
    volatile immap_t *immap = (immap_t *)CFG_IMMR;
    unsigned char spi_byte;	/* Data Byte */

    init_spi();		/* set port B for software SPI */

    /* Now we can enable the DS1306 RTC */
    immap->im_cpm.cp_pbdat |= PB_SPI_CE;
    udelay(10);

    /* Shift out the address (0) of the time in the Clock Chip */
    soft_spi_send(0);

    /* Put the clock readings into the rtc_time structure */
    tmp->tm_sec = bcd2bin(soft_spi_read());	/* Read seconds */
    tmp->tm_min = bcd2bin(soft_spi_read());	/* Read minutes */

    /* Hours are trickier */
    spi_byte = soft_spi_read();	/* Read Hours into temporary value */
    if (spi_byte & 0x40) {
	/* 12 hour mode bit is set (time is in 1-12 format) */
	if (spi_byte & 0x20) {
	    /* since PM we add 11 to get 0-23 for hours */
	    tmp->tm_hour = (bcd2bin(spi_byte & 0x1F)) + 11;
	}
	else {
	    /* since AM we subtract 1 to get 0-23 for hours */
	    tmp->tm_hour = (bcd2bin(spi_byte & 0x1F)) - 1;
	}
    }
    else {
	/* Otherwise, 0-23 hour format */
	tmp->tm_hour = (bcd2bin(spi_byte & 0x3F));
    }

    soft_spi_read();		/* Read and discard Day of week */
    tmp->tm_mday = bcd2bin(soft_spi_read());	/* Read Day of the Month */
    tmp->tm_mon = bcd2bin(soft_spi_read());	/* Read Month */

    /* Read Year and convert to this century */
    tmp->tm_year = bcd2bin(soft_spi_read()) + 2000;

    /* Now we can disable the DS1306 RTC */
    immap->im_cpm.cp_pbdat &= ~PB_SPI_CE;	/* Disable DS1306 Chip */
    udelay(10);

    GregorianDay(tmp);		/* Determine the day of week */

    debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	  tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	  tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}

/* ------------------------------------------------------------------------- */

/* set clock time in DS1306 RTC and in MPC8xx RTC */
void rtc_set(struct rtc_time *tmp)
{
    volatile immap_t *immap = (immap_t *)CFG_IMMR;

    init_spi();		/* set port B for software SPI */

    /* Now we can enable the DS1306 RTC */
    immap->im_cpm.cp_pbdat |= PB_SPI_CE;	/* Enable DS1306 Chip */
    udelay(10);

    /* First disable write protect in the clock chip control register */
    soft_spi_send(0x8F);	/* send address of the control register */
    soft_spi_send(0x00);	/* send control register contents */

    /* Now disable the DS1306 to terminate the write */
    immap->im_cpm.cp_pbdat &= ~PB_SPI_CE;
    udelay(10);

    /* Now enable the DS1306 to initiate a new write */
    immap->im_cpm.cp_pbdat |= PB_SPI_CE;
    udelay(10);

    /* Next, send the address of the clock time write registers */
    soft_spi_send(0x80);	/* send address of the first time register */

    /* Use Burst Mode to send all of the time data to the clock */
    bin2bcd(tmp->tm_sec);
    soft_spi_send(bin2bcd(tmp->tm_sec));	/* Send Seconds */
    soft_spi_send(bin2bcd(tmp->tm_min));	/* Send Minutes */
    soft_spi_send(bin2bcd(tmp->tm_hour));	/* Send Hour */
    soft_spi_send(bin2bcd(tmp->tm_wday));	/* Send Day of the Week */
    soft_spi_send(bin2bcd(tmp->tm_mday));	/* Send Day of Month */
    soft_spi_send(bin2bcd(tmp->tm_mon));	/* Send Month */
    soft_spi_send(bin2bcd(tmp->tm_year - 2000));	/* Send Year */

    /* Now we can disable the Clock chip to terminate the burst write */
    immap->im_cpm.cp_pbdat &= ~PB_SPI_CE;	/* Disable DS1306 Chip */
    udelay(10);

    /* Now we can enable the Clock chip to initiate a new write */
    immap->im_cpm.cp_pbdat |= PB_SPI_CE;	/* Enable DS1306 Chip */
    udelay(10);

    /* First we Enable write protect in the clock chip control register */
    soft_spi_send(0x8F);	/* send address of the control register */
    soft_spi_send(0x40);	/* send out Control Register contents */

    /* Now disable the DS1306 */
    immap->im_cpm.cp_pbdat &= ~PB_SPI_CE;	/*  Disable DS1306 Chip */
    udelay(10);

    /* Set standard MPC8xx clock to the same time so Linux will
     * see the time even if it doesn't have a DS1306 clock driver.
     * This helps with experimenting with standard kernels.
     */
    {
	ulong tim;

	tim = mktime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
		     tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	immap->im_sitk.sitk_rtck = KAPWR_KEY;
	immap->im_sit.sit_rtc = tim;
    }

    debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	  tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	  tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}

/* ------------------------------------------------------------------------- */

void rtc_reset(void)
{
    return;			/* nothing to do */
}

/* ------------------------------------------------------------------------- */

static unsigned char bcd2bin(unsigned char n)
{
    return ((((n >> 4) & 0x0F) * 10) + (n & 0x0F));
}

/* ------------------------------------------------------------------------- */

static unsigned int bin2bcd(unsigned int n)
{
    return (((n / 10) << 4) | (n % 10));
}

/* ------------------------------------------------------------------------- */

/* Initialize Port B for software SPI */
static void init_spi(void) {
    volatile immap_t *immap = (immap_t *)CFG_IMMR;

    /* Force output pins to begin at logic 0 */
    immap->im_cpm.cp_pbdat &= ~(PB_SPI_CE | PB_SPIMOSI | PB_SPISCK);

    /* Set these 3 signals as outputs */
    immap->im_cpm.cp_pbdir |= (PB_SPIMOSI | PB_SPI_CE | PB_SPISCK);

    immap->im_cpm.cp_pbdir &= ~PB_SPIMISO;	/* Make MISO pin an input */
    udelay(10);
}

/* ------------------------------------------------------------------------- */

/* NOTE: soft_spi_send() assumes that the I/O lines are configured already */
static void soft_spi_send(unsigned char n)
{
    volatile immap_t *immap = (immap_t *)CFG_IMMR;
    unsigned char bitpos;	/* bit position to receive */
    unsigned char i;		/* Loop Control */

    /* bit position to send, start with most significant bit */
    bitpos = 0x80;

    /* Send 8 bits to software SPI */
    for (i = 0; i < 8; i++) {	/* Loop for 8 bits */
	immap->im_cpm.cp_pbdat |= PB_SPISCK;	/* Raise SCK */

	if (n & bitpos)
	    immap->im_cpm.cp_pbdat |= PB_SPIMOSI;	/* Set MOSI to 1 */
	else
	    immap->im_cpm.cp_pbdat &= ~PB_SPIMOSI;	/* Set MOSI to 0 */
	udelay(10);

	immap->im_cpm.cp_pbdat &= ~PB_SPISCK;	/* Lower SCK */
	udelay(10);

	bitpos >>= 1;		/* Shift for next bit position */
    }
}

/* ------------------------------------------------------------------------- */

/* NOTE: soft_spi_read() assumes that the I/O lines are configured already */
static unsigned char soft_spi_read(void)
{
    volatile immap_t *immap = (immap_t *)CFG_IMMR;

    unsigned char spi_byte = 0;	/* Return value, assume success */
    unsigned char bitpos;	/* bit position to receive */
    unsigned char i;		/* Loop Control */

    /* bit position to receive, start with most significant bit */
    bitpos = 0x80;

    /* Read 8 bits here */
    for (i = 0; i < 8; i++) {	/* Do 8 bits in loop */
	immap->im_cpm.cp_pbdat |= PB_SPISCK;	/* Raise SCK */
	udelay(10);
	if (immap->im_cpm.cp_pbdat & PB_SPIMISO)	/* Get a bit of data */
	    spi_byte |= bitpos;	/* Set data accordingly */
	immap->im_cpm.cp_pbdat &= ~PB_SPISCK;	/* Lower SCK */
	udelay(10);
	bitpos >>= 1;		/* Shift for next bit position */
    }

    return spi_byte;		/* Return the byte read */
}

/* ------------------------------------------------------------------------- */

#endif
