/*
 * (C) Copyright 2008, 2009
 * Andreas Pfefferle, DENX Software Engineering, ap@denx.de.
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

#include <asm/io.h>
#include <common.h>
#include <command.h>
#include <config.h>
#include <bcd.h>
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

	tm->tm_sec  = BCD2BIN( buffer[0] & 0x7F);
	tm->tm_min  = BCD2BIN( buffer[1] & 0x7F);
	tm->tm_hour = BCD2BIN( buffer[2] & 0x3F);
	tm->tm_wday = BCD2BIN( buffer[3] & 0x07);
	tm->tm_mday = BCD2BIN((buffer[3] & 0xF0) >> 4 | (buffer[4] & 0x0F) << 4);
	tm->tm_mon  = BCD2BIN((buffer[4] & 0x30) >> 4 | (buffer[5] & 0x0F) << 4);
	tm->tm_year = BCD2BIN((buffer[5] & 0xF0) >> 4 | (buffer[6] & 0x0F) << 4) + 2000;
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
	buffer[0] = BIN2BCD(tm->tm_sec);
	buffer[1] = BIN2BCD(tm->tm_min);
	buffer[2] = BIN2BCD(tm->tm_hour);
	buffer[3] = BIN2BCD(tm->tm_wday);
	tmp = BIN2BCD(tm->tm_mday);
	buffer[3] |= (tmp & 0x0F) << 4;
	buffer[4] =  (tmp & 0xF0) >> 4;
	tmp = BIN2BCD(tm->tm_mon);
	buffer[4] |= (tmp & 0x0F) << 4;
	buffer[5] =  (tmp & 0xF0) >> 4;
	tmp = BIN2BCD(tm->tm_year  % 100);
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
