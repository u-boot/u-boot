/*
 * board/eva/phantom.c
 *
 * Phantom RTC device driver for EVA
 *
 * Author: Sangmoon Kim
 *         dogoil@etinsys.com
 *
 * Copyright 2002 Etinsys Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <common.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_CMD_DATE)

#define RTC_BASE (CONFIG_SYS_NVRAM_BASE_ADDR + 0x7fff8)

#define RTC_YEAR                ( RTC_BASE + 7 )
#define RTC_MONTH               ( RTC_BASE + 6 )
#define RTC_DAY_OF_MONTH        ( RTC_BASE + 5 )
#define RTC_DAY_OF_WEEK         ( RTC_BASE + 4 )
#define RTC_HOURS               ( RTC_BASE + 3 )
#define RTC_MINUTES             ( RTC_BASE + 2 )
#define RTC_SECONDS             ( RTC_BASE + 1 )
#define RTC_CENTURY             ( RTC_BASE + 0 )

#define RTC_CONTROLA            RTC_CENTURY
#define RTC_CONTROLB            RTC_SECONDS
#define RTC_CONTROLC            RTC_DAY_OF_WEEK

#define RTC_CA_WRITE            0x80
#define RTC_CA_READ             0x40

#define RTC_CB_OSC_DISABLE      0x80

#define RTC_CC_BATTERY_FLAG     0x80
#define RTC_CC_FREQ_TEST        0x40


static int phantom_flag = -1;
static int century_flag = -1;

static uchar rtc_read(unsigned int addr)
{
	return *(volatile unsigned char *)(addr);
}

static void rtc_write(unsigned int addr, uchar val)
{
	*(volatile unsigned char *)(addr) = val;
}

static unsigned char phantom_rtc_sequence[] = {
	0xc5, 0x3a, 0xa3, 0x5c, 0xc5, 0x3a, 0xa3, 0x5c
};

static unsigned char* phantom_rtc_read(int addr, unsigned char rtc[8])
{
	int i, j;
	unsigned char v;
	unsigned char save = rtc_read(addr);

	for (j = 0; j < 8; j++) {
		v = phantom_rtc_sequence[j];
		for (i = 0; i < 8; i++) {
			rtc_write(addr, v & 1);
			v >>= 1;
		}
	}
	for (j = 0; j < 8; j++) {
		v = 0;
		for (i = 0; i < 8; i++) {
			if(rtc_read(addr) & 1)
				v |= 1 << i;
		}
		rtc[j] = v;
	}
	rtc_write(addr, save);
	return rtc;
}

static void phantom_rtc_write(int addr, unsigned char rtc[8])
{
	int i, j;
	unsigned char v;
	unsigned char save = rtc_read(addr);
	for (j = 0; j < 8; j++) {
		v = phantom_rtc_sequence[j];
		for (i = 0; i < 8; i++) {
			rtc_write(addr, v & 1);
			v >>= 1;
		}
	}
	for (j = 0; j < 8; j++) {
		v = rtc[j];
		for (i = 0; i < 8; i++) {
			rtc_write(addr, v & 1);
			v >>= 1;
		}
	}
	rtc_write(addr, save);
}

static int get_phantom_flag(void)
{
	int i;
	unsigned char rtc[8];

	phantom_rtc_read(RTC_BASE, rtc);

	for(i = 1; i < 8; i++) {
		if (rtc[i] != rtc[0])
			return 1;
	}
	return 0;
}

void rtc_reset(void)
{
	if (phantom_flag < 0)
		phantom_flag = get_phantom_flag();

	if (phantom_flag) {
		unsigned char rtc[8];
		phantom_rtc_read(RTC_BASE, rtc);
		if(rtc[4] & 0x30) {
			printf( "real-time-clock was stopped. Now starting...\n" );
			rtc[4] &= 0x07;
			phantom_rtc_write(RTC_BASE, rtc);
		}
	} else {
		uchar reg_a, reg_b, reg_c;
		reg_a = rtc_read( RTC_CONTROLA );
		reg_b = rtc_read( RTC_CONTROLB );

		if ( reg_b & RTC_CB_OSC_DISABLE )
		{
			printf( "real-time-clock was stopped. Now starting...\n" );
			reg_a |= RTC_CA_WRITE;
			reg_b &= ~RTC_CB_OSC_DISABLE;
			rtc_write( RTC_CONTROLA, reg_a );
			rtc_write( RTC_CONTROLB, reg_b );
		}

		/* make sure read/write clock register bits are cleared */
		reg_a &= ~( RTC_CA_WRITE | RTC_CA_READ );
		rtc_write( RTC_CONTROLA, reg_a );

		reg_c = rtc_read( RTC_CONTROLC );
		if (( reg_c & RTC_CC_BATTERY_FLAG ) == 0 )
			printf( "RTC battery low. Clock setting may not be reliable.\n");
	}
}

inline unsigned bcd2bin (uchar n)
{
	return ((((n >> 4) & 0x0F) * 10) + (n & 0x0F));
}

inline unsigned char bin2bcd (unsigned int n)
{
	return (((n / 10) << 4) | (n % 10));
}

static int get_century_flag(void)
{
	int flag = 0;
	int bcd, century;
	bcd = rtc_read( RTC_CENTURY );
	century = bcd2bin( bcd & 0x3F );
	rtc_write( RTC_CENTURY, bin2bcd(century+1));
	if (bcd == rtc_read( RTC_CENTURY ))
		flag = 1;
	rtc_write( RTC_CENTURY, bcd);
	return flag;
}

int rtc_get( struct rtc_time *tmp)
{
	if (phantom_flag < 0)
		phantom_flag = get_phantom_flag();

	if (phantom_flag)
	{
		unsigned char rtc[8];

		phantom_rtc_read(RTC_BASE, rtc);

		tmp->tm_sec	= bcd2bin(rtc[1] & 0x7f);
		tmp->tm_min	= bcd2bin(rtc[2] & 0x7f);
		tmp->tm_hour	= bcd2bin(rtc[3] & 0x1f);
		tmp->tm_wday	= bcd2bin(rtc[4] & 0x7);
		tmp->tm_mday	= bcd2bin(rtc[5] & 0x3f);
		tmp->tm_mon	= bcd2bin(rtc[6] & 0x1f);
		tmp->tm_year	= bcd2bin(rtc[7]) + 1900;
		tmp->tm_yday = 0;
		tmp->tm_isdst = 0;

		if( (rtc[3] & 0x80)  && (rtc[3] & 0x40) ) tmp->tm_hour += 12;
		if (tmp->tm_year < 1970) tmp->tm_year += 100;
	} else {
		uchar sec, min, hour;
		uchar mday, wday, mon, year;

		int century;

		uchar reg_a;

		if (century_flag < 0)
			century_flag = get_century_flag();

		reg_a = rtc_read( RTC_CONTROLA );
		/* lock clock registers for read */
		rtc_write( RTC_CONTROLA, ( reg_a | RTC_CA_READ ));

		sec     = rtc_read( RTC_SECONDS );
		min     = rtc_read( RTC_MINUTES );
		hour    = rtc_read( RTC_HOURS );
		mday    = rtc_read( RTC_DAY_OF_MONTH );
		wday    = rtc_read( RTC_DAY_OF_WEEK );
		mon     = rtc_read( RTC_MONTH );
		year    = rtc_read( RTC_YEAR );
		century = rtc_read( RTC_CENTURY );

		/* unlock clock registers after read */
		rtc_write( RTC_CONTROLA, ( reg_a & ~RTC_CA_READ ));

		tmp->tm_sec  = bcd2bin( sec  & 0x7F );
		tmp->tm_min  = bcd2bin( min  & 0x7F );
		tmp->tm_hour = bcd2bin( hour & 0x3F );
		tmp->tm_mday = bcd2bin( mday & 0x3F );
		tmp->tm_mon  = bcd2bin( mon & 0x1F );
		tmp->tm_wday = bcd2bin( wday & 0x07 );

		if (century_flag) {
			tmp->tm_year = bcd2bin( year ) +
				( bcd2bin( century & 0x3F ) * 100 );
		} else {
			tmp->tm_year = bcd2bin( year ) + 1900;
			if (tmp->tm_year < 1970) tmp->tm_year += 100;
		}

		tmp->tm_yday = 0;
		tmp->tm_isdst= 0;
	}

	return 0;
}

int rtc_set( struct rtc_time *tmp )
{
	if (phantom_flag < 0)
		phantom_flag = get_phantom_flag();

	if (phantom_flag) {
		uint year;
		unsigned char rtc[8];

		year = tmp->tm_year;
		year -= (year < 2000) ? 1900 : 2000;

		rtc[0] = bin2bcd(0);
		rtc[1] = bin2bcd(tmp->tm_sec);
		rtc[2] = bin2bcd(tmp->tm_min);
		rtc[3] = bin2bcd(tmp->tm_hour);
		rtc[4] = bin2bcd(tmp->tm_wday);
		rtc[5] = bin2bcd(tmp->tm_mday);
		rtc[6] = bin2bcd(tmp->tm_mon);
		rtc[7] = bin2bcd(year);

		phantom_rtc_write(RTC_BASE, rtc);
	} else {
		uchar reg_a;
		if (century_flag < 0)
			century_flag = get_century_flag();

		/* lock clock registers for write */
		reg_a = rtc_read( RTC_CONTROLA );
		rtc_write( RTC_CONTROLA, ( reg_a | RTC_CA_WRITE ));

		rtc_write( RTC_MONTH, bin2bcd( tmp->tm_mon ));

		rtc_write( RTC_DAY_OF_WEEK, bin2bcd( tmp->tm_wday ));
		rtc_write( RTC_DAY_OF_MONTH, bin2bcd( tmp->tm_mday ));
		rtc_write( RTC_HOURS, bin2bcd( tmp->tm_hour ));
		rtc_write( RTC_MINUTES, bin2bcd( tmp->tm_min ));
		rtc_write( RTC_SECONDS, bin2bcd( tmp->tm_sec ));

		/* break year up into century and year in century */
		if (century_flag) {
			rtc_write( RTC_YEAR, bin2bcd( tmp->tm_year % 100 ));
			rtc_write( RTC_CENTURY, bin2bcd( tmp->tm_year / 100 ));
			reg_a &= 0xc0;
			reg_a |= bin2bcd( tmp->tm_year / 100 );
		} else {
			rtc_write(RTC_YEAR, bin2bcd(tmp->tm_year -
				((tmp->tm_year < 2000) ? 1900 : 2000)));
		}

		/* unlock clock registers after read */
		rtc_write( RTC_CONTROLA, ( reg_a  & ~RTC_CA_WRITE ));
	}

	return 0;
}

#endif
