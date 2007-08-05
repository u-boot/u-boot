/*
 * (C) Copyright 2001, 2002, 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Keith Outwater, keith_outwater@mvis.com`
 * Steven Scholz, steven.scholz@imc-berlin.de
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
 * Date & Time support (no alarms) for Dallas Semiconductor (now Maxim)
 * DS1374 Real Time Clock (RTC).
 *
 * based on ds1337.c
 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

#if (defined(CONFIG_RTC_DS1374)) && defined(CONFIG_CMD_DATE)

/*---------------------------------------------------------------------*/
#undef DEBUG_RTC
#define DEBUG_RTC

#ifdef DEBUG_RTC
#define DEBUGR(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGR(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

#ifndef CFG_I2C_RTC_ADDR
# define CFG_I2C_RTC_ADDR	0x68
#endif

#if defined(CONFIG_RTC_DS1374) && (CFG_I2C_SPEED > 400000)
# error The DS1374 is specified up to 400kHz in fast mode!
#endif

/*
 * RTC register addresses
 */
#define RTC_TOD_CNT_BYTE0_ADDR		0x00 /* TimeOfDay */
#define RTC_TOD_CNT_BYTE1_ADDR		0x01
#define RTC_TOD_CNT_BYTE2_ADDR		0x02
#define RTC_TOD_CNT_BYTE3_ADDR		0x03

#define RTC_WD_ALM_CNT_BYTE0_ADDR	0x04
#define RTC_WD_ALM_CNT_BYTE1_ADDR	0x05
#define RTC_WD_ALM_CNT_BYTE2_ADDR	0x06

#define RTC_CTL_ADDR			0x07 /* RTC-CoNTrol-register */
#define RTC_SR_ADDR			0x08 /* RTC-StatusRegister */
#define RTC_TCS_DS_ADDR			0x09 /* RTC-TrickleChargeSelect DiodeSelect-register */

#define RTC_CTL_BIT_AIE			(1<<0) /* Bit 0 - Alarm Interrupt enable */
#define RTC_CTL_BIT_RS1			(1<<1) /* Bit 1/2 - Rate Select square wave output */
#define RTC_CTL_BIT_RS2			(1<<2) /* Bit 2/2 - Rate Select square wave output */
#define RTC_CTL_BIT_WDSTR		(1<<3) /* Bit 3 - Watchdog Reset Steering */
#define RTC_CTL_BIT_BBSQW		(1<<4) /* Bit 4 - Battery-Backed Square-Wave */
#define RTC_CTL_BIT_WD_ALM		(1<<5) /* Bit 5 - Watchdoc/Alarm Counter Select */
#define RTC_CTL_BIT_WACE		(1<<6) /* Bit 6 - Watchdog/Alarm Counter Enable WACE*/
#define RTC_CTL_BIT_EN_OSC		(1<<7) /* Bit 7 - Enable Oscilator */

#define RTC_SR_BIT_AF			0x01 /* Bit 0 = Alarm Flag */
#define RTC_SR_BIT_OSF			0x80 /* Bit 7 - Osc Stop Flag */

typedef unsigned char boolean_t;

#ifndef TRUE
#define TRUE ((boolean_t)(0==0))
#endif
#ifndef FALSE
#define FALSE (!TRUE)
#endif

const char RtcTodAddr[] = {
	RTC_TOD_CNT_BYTE0_ADDR,
	RTC_TOD_CNT_BYTE1_ADDR,
	RTC_TOD_CNT_BYTE2_ADDR,
	RTC_TOD_CNT_BYTE3_ADDR
};

static uchar rtc_read (uchar reg);
static void rtc_write (uchar reg, uchar val, boolean_t set);
static void rtc_write_raw (uchar reg, uchar val);

/*
 * Get the current time from the RTC
 */
void rtc_get (struct rtc_time *tm){

	unsigned long time1, time2;
	unsigned int limit;
	unsigned char tmp;
	unsigned int i;

	/*
	 * Since the reads are being performed one byte at a time,
	 * there is a chance that a carry will occur during the read.
	 * To detect this, 2 reads are performed and compared.
	 */
	limit = 10;
	do {
		i = 4;
		time1 = 0;
		while (i--) {
			tmp = rtc_read(RtcTodAddr[i]);
			time1 = (time1 << 8) | (tmp & 0xff);
		}

		i = 4;
		time2 = 0;
		while (i--) {
			tmp = rtc_read(RtcTodAddr[i]);
			time2 = (time2 << 8) | (tmp & 0xff);
		}
	} while ((time1 != time2) && limit--);

	if (time1 != time2) {
		printf("can't get consistent time from rtc chip\n");
	}

	DEBUGR ("Get RTC s since 1.1.1970: %d\n", time1);

	to_tm(time1, tm); /* To Gregorian Date */

	if (rtc_read(RTC_SR_ADDR) & RTC_SR_BIT_OSF)
		printf ("### Warning: RTC oscillator has stopped\n");

	DEBUGR ("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);
}

/*
 * Set the RTC
 */
void rtc_set (struct rtc_time *tmp){

	unsigned long time;
	unsigned i;

	DEBUGR ("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	if (tmp->tm_year < 1970 || tmp->tm_year > 2069)
		printf("WARNING: year should be between 1970 and 2069!\n");

	time = mktime(tmp->tm_year, tmp->tm_mon,
			tmp->tm_mday, tmp->tm_hour,
			tmp->tm_min, tmp->tm_sec);

	DEBUGR ("Set RTC s since 1.1.1970: %d (0x%02x)\n", time, time);

	/* write to RTC_TOD_CNT_BYTEn_ADDR */
	for (i = 0; i <= 3; i++) {
		rtc_write_raw(RtcTodAddr[i], (unsigned char)(time & 0xff));
		time = time >> 8;
	}

	/* Start clock */
	rtc_write(RTC_CTL_ADDR, RTC_CTL_BIT_EN_OSC, FALSE);
}

/*
 * Reset the RTC. We setting the date back to 1970-01-01.
 * We also enable the oscillator output on the SQW/OUT pin and program
 * it for 32,768 Hz output. Note that according to the datasheet, turning
 * on the square wave output increases the current drain on the backup
 * battery to something between 480nA and 800nA.
 */
void rtc_reset (void){

	struct rtc_time tmp;

	/* clear status flags */
	rtc_write (RTC_SR_ADDR, (RTC_SR_BIT_AF|RTC_SR_BIT_OSF), FALSE); /* clearing OSF and AF */

	/* Initialise DS1374 oriented to MPC8349E-ADS */
	rtc_write (RTC_CTL_ADDR, (RTC_CTL_BIT_EN_OSC
				 |RTC_CTL_BIT_WACE
				 |RTC_CTL_BIT_AIE), FALSE);/* start osc, disable WACE, clear AIE
							      - set to 0 */
	rtc_write (RTC_CTL_ADDR, (RTC_CTL_BIT_WD_ALM
				|RTC_CTL_BIT_WDSTR
				|RTC_CTL_BIT_RS1
				|RTC_CTL_BIT_RS2
				|RTC_CTL_BIT_BBSQW), TRUE);/* disable WD/ALM, WDSTR set to INT-pin,
							      set BBSQW and SQW to 32k
							      - set to 1 */
	tmp.tm_year = 1970;
	tmp.tm_mon = 1;
	tmp.tm_mday= 1;
	tmp.tm_hour = 0;
	tmp.tm_min = 0;
	tmp.tm_sec = 0;

	rtc_set(&tmp);

	printf("RTC:   %4d-%02d-%02d %2d:%02d:%02d UTC\n",
		tmp.tm_year, tmp.tm_mon, tmp.tm_mday,
		tmp.tm_hour, tmp.tm_min, tmp.tm_sec);

	rtc_write(RTC_WD_ALM_CNT_BYTE2_ADDR,0xAC, TRUE);
	rtc_write(RTC_WD_ALM_CNT_BYTE1_ADDR,0xDE, TRUE);
	rtc_write(RTC_WD_ALM_CNT_BYTE2_ADDR,0xAD, TRUE);
}

/*
 * Helper functions
 */
static uchar rtc_read (uchar reg)
{
	return (i2c_reg_read (CFG_I2C_RTC_ADDR, reg));
}

static void rtc_write (uchar reg, uchar val, boolean_t set)
{
	if (set == TRUE) {
		val |= i2c_reg_read (CFG_I2C_RTC_ADDR, reg);
		i2c_reg_write (CFG_I2C_RTC_ADDR, reg, val);
	} else {
		val = i2c_reg_read (CFG_I2C_RTC_ADDR, reg) & ~val;
		i2c_reg_write (CFG_I2C_RTC_ADDR, reg, val);
	}
}

static void rtc_write_raw (uchar reg, uchar val)
{
		i2c_reg_write (CFG_I2C_RTC_ADDR, reg, val);
}
#endif
