/*
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <command.h>
#include <i2c.h>
#include <rtc.h>

#define RTC_RV3029_CTRL1	0x00
#define RTC_RV3029_CTRL1_EERE	(1 << 3)

#define RTC_RV3029_CTRL_STATUS	0x03
#define RTC_RV3029_CTRLS_EEBUSY	(1 << 7)

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

#define RTC_RV3029_EEPROM_CTRL	0x30
#define RTC_RV3029_TRICKLE_1K	(1 << 4)
#define RTC_RV3029_TRICKLE_5K	(1 << 5)
#define RTC_RV3029_TRICKLE_20K	(1 << 6)
#define RTC_RV3029_TRICKLE_80K	(1 << 7)

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

	debug( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec );

	return 0;
}

int rtc_set( struct rtc_time *tmp )
{
	int	ret;
	unsigned char buf[RTC_RV3029_PAGE_LEN];

	debug( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

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
	return ret;
}

/* sets EERE-Bit  (automatic EEPROM refresh) */
static void set_eere_bit(int state)
{
	unsigned char reg_ctrl1;

	(void)i2c_read(CONFIG_SYS_I2C_RTC_ADDR, RTC_RV3029_CTRL1, 1,
			&reg_ctrl1, 1);

	if (state)
		reg_ctrl1 |= RTC_RV3029_CTRL1_EERE;
	else
		reg_ctrl1 &= (~RTC_RV3029_CTRL1_EERE);

	(void)i2c_write(CONFIG_SYS_I2C_RTC_ADDR, RTC_RV3029_CTRL1, 1,
		&reg_ctrl1, 1);
}

/* waits until EEPROM page is no longer busy (times out after 10ms*loops) */
static int wait_eebusy(int loops)
{
	int i;
	unsigned char ctrl_status;

	for (i = 0; i < loops; i++) {
		(void)i2c_read(CONFIG_SYS_I2C_RTC_ADDR, RTC_RV3029_CTRL_STATUS,
			1, &ctrl_status, 1);

		if ((ctrl_status & RTC_RV3029_CTRLS_EEBUSY) == 0)
			break;
		udelay(10000);
	}
	return i;
}

void rtc_reset (void)
{
	unsigned char buf[RTC_RV3029_PAGE_LEN];

	buf[0] = RTC_RV3029_CTRL_SYS_R;
	(void)i2c_write(CONFIG_SYS_I2C_RTC_ADDR, RTC_RV3029_CTRL_RESET, 1,
			buf, 1);

#if defined(CONFIG_SYS_RV3029_TCR)
	/*
	 * because EEPROM_CTRL register is in EEPROM page it is necessary to
	 * disable automatic EEPROM refresh and check if EEPROM is busy
	 * before EEPORM_CTRL register may be accessed
	 */
	set_eere_bit(0);
	wait_eebusy(100);
	/* read current trickle charger setting */
	(void)i2c_read(CONFIG_SYS_I2C_RTC_ADDR, RTC_RV3029_EEPROM_CTRL,
			1, buf, 1);
	/* enable automatic EEPROM refresh again */
	set_eere_bit(1);

	/*
	 * to minimize EEPROM access write trickle charger setting only if it
	 * differs from current value
	 */
	if ((buf[0] & 0xF0) != CONFIG_SYS_RV3029_TCR) {
		buf[0] = (buf[0] & 0x0F) | CONFIG_SYS_RV3029_TCR;
		/*
		 * write trickle charger setting (disable autom. EEPROM
		 * refresh and wait until EEPROM is idle)
		 */
		set_eere_bit(0);
		wait_eebusy(100);
		(void)i2c_write(CONFIG_SYS_I2C_RTC_ADDR,
				RTC_RV3029_EEPROM_CTRL, 1, buf, 1);
		/*
		 * it is necessary to wait 10ms before EEBUSY-Bit may be read
		 * (this is not documented in the data sheet yet, but the
		 * manufacturer recommends it)
		 */
		udelay(10000);
		/* wait until EEPROM write access is finished */
		wait_eebusy(100);
		set_eere_bit(1);
	}
#endif
}
