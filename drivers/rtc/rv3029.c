/*
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de
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
#include <command.h>
#include <i2c.h>
#include <rtc.h>

#define RTC_RV3029_CTRL_RESET	0x04
#define RTC_RV3029_CTRL_SYS_R	(1 << 4)

#define RTC_RV3029_CLOCK_PAGE	0x08
#define RTC_RV3029_PAGE_LEN	7

#define RV3029C2_W_SECONDS	0x00
#define RV3029C2_W_MINUTES	0x01
#define RV3029C2_W_HOURS	0x02
#define RV3029C2_W_DATE		0x03
#define RV3029C2_W_DAYS		0x04
#define RV3029C2_W_MONTHS	0x05
#define RV3029C2_W_YEARS	0x06

#define RV3029C2_REG_HR_12_24          (1 << 6)  /* 24h/12h mode */
#define RV3029C2_REG_HR_PM             (1 << 5)  /* PM/AM bit in 12h mode */

int rtc_get( struct rtc_time *tmp )
{
	int	ret;
	unsigned char buf[RTC_RV3029_PAGE_LEN];

	ret = i2c_read(CONFIG_SYS_I2C_RTC_ADDR, RTC_RV3029_CLOCK_PAGE, 1, buf, \
			RTC_RV3029_PAGE_LEN);
	if (ret) {
		printf("%s: error reading RTC: %x\n", __func__, ret);
		return -1;
	}
	tmp->tm_sec  = bcd2bin( buf[RV3029C2_W_SECONDS] & 0x7f);
	tmp->tm_min  = bcd2bin( buf[RV3029C2_W_MINUTES] & 0x7f);
	if (buf[RV3029C2_W_HOURS] & RV3029C2_REG_HR_12_24) {
		/* 12h format */
		tmp->tm_hour = bcd2bin(buf[RV3029C2_W_HOURS] & 0x1f);
		if (buf[RV3029C2_W_HOURS] & RV3029C2_REG_HR_PM)
			/* PM flag set */
			tmp->tm_hour += 12;
	} else
		tmp->tm_hour = bcd2bin(buf[RV3029C2_W_HOURS] & 0x3f);

	tmp->tm_mday = bcd2bin( buf[RV3029C2_W_DATE] & 0x3F );
	tmp->tm_mon  = bcd2bin( buf[RV3029C2_W_MONTHS] & 0x1F );
	tmp->tm_wday = bcd2bin( buf[RV3029C2_W_DAYS] & 0x07 );
	/* RTC supports only years > 1999 */
	tmp->tm_year = bcd2bin( buf[RV3029C2_W_YEARS]) + 2000;
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

#ifdef RTC_DEBUG
	printf( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec );

#endif
	return 0;
}

int rtc_set( struct rtc_time *tmp )
{
	int	ret;
	unsigned char buf[RTC_RV3029_PAGE_LEN];
#ifdef RTC_DEBUG
	printf( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	if (tmp->tm_year < 2000) {
		printf("RTC: year %d < 2000 not possible\n", tmp->tm_year);
		return -1;
	}
	buf[RV3029C2_W_SECONDS] = bin2bcd(tmp->tm_sec);
	buf[RV3029C2_W_MINUTES] = bin2bcd(tmp->tm_min);
	buf[RV3029C2_W_HOURS] = bin2bcd(tmp->tm_hour);
	/* set 24h format */
	buf[RV3029C2_W_HOURS] &= ~RV3029C2_REG_HR_12_24;
	buf[RV3029C2_W_DATE] = bin2bcd(tmp->tm_mday);
	buf[RV3029C2_W_DAYS] = bin2bcd(tmp->tm_wday);
	buf[RV3029C2_W_MONTHS] = bin2bcd(tmp->tm_mon);
	tmp->tm_year -= 2000;
	buf[RV3029C2_W_YEARS] = bin2bcd(tmp->tm_year);
	ret = i2c_write(CONFIG_SYS_I2C_RTC_ADDR, RTC_RV3029_CLOCK_PAGE, 1,
			buf, RTC_RV3029_PAGE_LEN);

	/* give the RTC some time to update */
	udelay(1000);
	return 0;
}

void rtc_reset (void)
{
	int	ret;
	unsigned char buf[RTC_RV3029_PAGE_LEN];

	buf[0] = RTC_RV3029_CTRL_SYS_R;
	ret = i2c_write(CONFIG_SYS_I2C_RTC_ADDR, RTC_RV3029_CTRL_RESET, 1,
			buf, 1);
}
