/*
 * (C) Copyright 2008, 2009
 * Andreas Pfefferle, DENX Software Engineering, ap@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <common.h>
#include <command.h>
#include <config.h>
#include <rtc.h>
#include <tws.h>

#if defined(CONFIG_CMD_DATE)

/*
 * Note: The acrobatics below is due to the hideously ingenius idea of
 * the chip designers.  As the chip does not allow register
 * addressing, all values need to be read and written in one go.  Sure
 * enough, the 'wday' field (0-6) is transferred using the economic
 * number of 4 bits right in the middle of the packet.....
 */

int rtc_get(struct rtc_time *tm)
{
	int rel = 0;
	uchar buffer[7];

	memset(buffer, 0, 7);

	/* Read 52 bits into our buffer */
	tws_read(buffer, 52);

	tm->tm_sec  = bcd2bin( buffer[0] & 0x7F);
	tm->tm_min  = bcd2bin( buffer[1] & 0x7F);
	tm->tm_hour = bcd2bin( buffer[2] & 0x3F);
	tm->tm_wday = bcd2bin( buffer[3] & 0x07);
	tm->tm_mday = bcd2bin((buffer[3] & 0xF0) >> 4 | (buffer[4] & 0x0F) << 4);
	tm->tm_mon  = bcd2bin((buffer[4] & 0x30) >> 4 | (buffer[5] & 0x0F) << 4);
	tm->tm_year = bcd2bin((buffer[5] & 0xF0) >> 4 | (buffer[6] & 0x0F) << 4) + 2000;
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	if (tm->tm_sec & 0x80) {
		puts("### Warning: RTC Low Voltage - date/time not reliable\n");
		rel = -1;
	}

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return rel;
}

int rtc_set(struct rtc_time *tm)
{
	uchar buffer[7];
	uchar tmp;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	memset(buffer, 0, 7);
	buffer[0] = bin2bcd(tm->tm_sec);
	buffer[1] = bin2bcd(tm->tm_min);
	buffer[2] = bin2bcd(tm->tm_hour);
	buffer[3] = bin2bcd(tm->tm_wday);
	tmp = bin2bcd(tm->tm_mday);
	buffer[3] |= (tmp & 0x0F) << 4;
	buffer[4] =  (tmp & 0xF0) >> 4;
	tmp = bin2bcd(tm->tm_mon);
	buffer[4] |= (tmp & 0x0F) << 4;
	buffer[5] =  (tmp & 0xF0) >> 4;
	tmp = bin2bcd(tm->tm_year  % 100);
	buffer[5] |= (tmp & 0x0F) << 4;
	buffer[6] =  (tmp & 0xF0) >> 4;

	/* Write the resulting 52 bits to device */
	tws_write(buffer, 52);

	return 0;
}

void rtc_reset(void)
{
	struct rtc_time tmp;

	tmp.tm_sec = 0;
	tmp.tm_min = 0;
	tmp.tm_hour = 0;
	tmp.tm_wday = 4;
	tmp.tm_mday = 1;
	tmp.tm_mon = 1;
	tmp.tm_year = 2000;
	rtc_set(&tmp);
}

#endif
