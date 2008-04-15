/*
 * Copyright (C) 2008, Guennadi Liakhovetski <lg@denx.de>
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

#include <common.h>
#include <rtc.h>
#include <spi.h>

int rtc_get(struct rtc_time *rtc)
{
	u32 day1, day2, time;
	u32 reg;
	int err, tim, i = 0;

	spi_select(1, 0, SPI_MODE_2 | SPI_CS_HIGH);

	do {
		reg = 0x2c000000;
		err = spi_xfer(0, 32, (uchar *)&reg, (uchar *)&day1);

		if (err)
			return err;

		reg = 0x28000000;
		err = spi_xfer(0, 32, (uchar *)&reg, (uchar *)&time);

		if (err)
			return err;

		reg = 0x2c000000;
		err = spi_xfer(0, 32, (uchar *)&reg, (uchar *)&day2);

		if (err)
			return err;
	} while (day1 != day2 && i++ < 3);

	tim = day1 * 86400 + time;
	to_tm(tim, rtc);

	rtc->tm_yday = 0;
	rtc->tm_isdst = 0;

	return 0;
}

void rtc_set(struct rtc_time *rtc)
{
	u32 time, day, reg;

	time = mktime(rtc->tm_year, rtc->tm_mon, rtc->tm_mday,
		      rtc->tm_hour, rtc->tm_min, rtc->tm_sec);
	day = time / 86400;
	time %= 86400;

	reg = 0x2c000000 | day | 0x80000000;
	spi_xfer(0, 32, (uchar *)&reg, (uchar *)&day);

	reg = 0x28000000 | time | 0x80000000;
	spi_xfer(0, 32, (uchar *)&reg, (uchar *)&time);
}

void rtc_reset(void)
{
}
