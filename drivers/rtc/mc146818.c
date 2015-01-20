/*
 * (C) Copyright 2001
 * Denis Peter MPL AG Switzerland. d.peter@mpl.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Date & Time support for the MC146818 (PIXX4) RTC
 */

/*#define	DEBUG*/

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <version.h>

#if defined(__I386__) || defined(CONFIG_MALTA)
#include <asm/io.h>
#define in8(p) inb(p)
#define out8(p, v) outb(v, p)
#endif

#if defined(CONFIG_CMD_DATE)

/* Set this to 1 to clear the CMOS RAM */
#define CLEAR_CMOS 0

#define RTC_PORT_MC146818	CONFIG_SYS_ISA_IO_BASE_ADDRESS +  0x70
#define RTC_SECONDS		0x00
#define RTC_SECONDS_ALARM	0x01
#define RTC_MINUTES		0x02
#define RTC_MINUTES_ALARM	0x03
#define RTC_HOURS		0x04
#define RTC_HOURS_ALARM		0x05
#define RTC_DAY_OF_WEEK		0x06
#define RTC_DATE_OF_MONTH	0x07
#define RTC_MONTH		0x08
#define RTC_YEAR		0x09
#define RTC_CONFIG_A		0x0A
#define RTC_CONFIG_B		0x0B
#define RTC_CONFIG_C		0x0C
#define RTC_CONFIG_D		0x0D
#define RTC_REG_SIZE		0x80

#define RTC_CONFIG_A_REF_CLCK_32KHZ	(1 << 5)
#define RTC_CONFIG_A_RATE_1024HZ	6

#define RTC_CONFIG_B_24H		(1 << 1)

#define RTC_CONFIG_D_VALID_RAM_AND_TIME	0x80

/* ------------------------------------------------------------------------- */

int rtc_get (struct rtc_time *tmp)
{
	uchar sec, min, hour, mday, wday, mon, year;
  /* here check if rtc can be accessed */
	while ((rtc_read8(RTC_CONFIG_A) & 0x80) == 0x80);
	sec	= rtc_read8(RTC_SECONDS);
	min	= rtc_read8(RTC_MINUTES);
	hour	= rtc_read8(RTC_HOURS);
	mday	= rtc_read8(RTC_DATE_OF_MONTH);
	wday	= rtc_read8(RTC_DAY_OF_WEEK);
	mon	= rtc_read8(RTC_MONTH);
	year	= rtc_read8(RTC_YEAR);
#ifdef RTC_DEBUG
	printf ( "Get RTC year: %02x mon/cent: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x\n",
		year, mon, mday, wday,
		hour, min, sec );
	printf ( "Alarms: month: %02x hour: %02x min: %02x sec: %02x\n",
		rtc_read8(RTC_CONFIG_D) & 0x3F,
		rtc_read8(RTC_HOURS_ALARM),
		rtc_read8(RTC_MINUTES_ALARM),
		rtc_read8(RTC_SECONDS_ALARM));
#endif
	tmp->tm_sec  = bcd2bin (sec  & 0x7F);
	tmp->tm_min  = bcd2bin (min  & 0x7F);
	tmp->tm_hour = bcd2bin (hour & 0x3F);
	tmp->tm_mday = bcd2bin (mday & 0x3F);
	tmp->tm_mon  = bcd2bin (mon & 0x1F);
	tmp->tm_year = bcd2bin (year);
	tmp->tm_wday = bcd2bin (wday & 0x07);
	if(tmp->tm_year<70)
		tmp->tm_year+=2000;
	else
		tmp->tm_year+=1900;
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;
#ifdef RTC_DEBUG
	printf ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	return 0;
}

int rtc_set (struct rtc_time *tmp)
{
#ifdef RTC_DEBUG
	printf ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif
	rtc_write8(RTC_CONFIG_B, 0x82); /* disable the RTC to update the regs */

	rtc_write8(RTC_YEAR, bin2bcd(tmp->tm_year % 100));
	rtc_write8(RTC_MONTH, bin2bcd(tmp->tm_mon));
	rtc_write8(RTC_DAY_OF_WEEK, bin2bcd(tmp->tm_wday));
	rtc_write8(RTC_DATE_OF_MONTH, bin2bcd(tmp->tm_mday));
	rtc_write8(RTC_HOURS, bin2bcd(tmp->tm_hour));
	rtc_write8(RTC_MINUTES, bin2bcd(tmp->tm_min));
	rtc_write8(RTC_SECONDS, bin2bcd(tmp->tm_sec));
	rtc_write8(RTC_CONFIG_B, 0x02); /* enable the RTC to update the regs */

	return 0;
}

void rtc_reset (void)
{
	rtc_write8(RTC_CONFIG_B, 0x82); /* disable the RTC to update the regs */
	rtc_write8(RTC_CONFIG_A, 0x20); /* Normal OP */
	rtc_write8(RTC_CONFIG_B, 0x00);
	rtc_write8(RTC_CONFIG_B, 0x00);
	rtc_write8(RTC_CONFIG_B, 0x02); /* enable the RTC to update the regs */
}

/* ------------------------------------------------------------------------- */

/*
 * use direct memory access
 */
int rtc_read8(int reg)
{
#ifdef CONFIG_SYS_RTC_REG_BASE_ADDR
	return in8(CONFIG_SYS_RTC_REG_BASE_ADDR + reg);
#else
	int ofs = 0;

	if (reg >= 128) {
		ofs = 2;
		reg -= 128;
	}
	out8(RTC_PORT_MC146818 + ofs, reg);

	return in8(RTC_PORT_MC146818 + ofs + 1);
#endif
}

void rtc_write8(int reg, uchar val)
{
#ifdef CONFIG_SYS_RTC_REG_BASE_ADDR
	out8(CONFIG_SYS_RTC_REG_BASE_ADDR + reg, val);
#else
	int ofs = 0;

	if (reg >= 128) {
		ofs = 2;
		reg -= 128;
	}
	out8(RTC_PORT_MC146818 + ofs, reg);
	out8(RTC_PORT_MC146818 + ofs + 1, val);
#endif
}

u32 rtc_read32(int reg)
{
	u32 value = 0;
	int i;

	for (i = 0; i < sizeof(value); i++)
		value |= rtc_read8(reg + i) << (i << 3);

	return value;
}

void rtc_write32(int reg, u32 value)
{
	int i;

	for (i = 0; i < sizeof(value); i++)
		rtc_write8(reg + i, (value >> (i << 3)) & 0xff);
}

void rtc_init(void)
{
#if CLEAR_CMOS
	int i;

	rtc_write8(RTC_SECONDS_ALARM, 0);
	rtc_write8(RTC_MINUTES_ALARM, 0);
	rtc_write8(RTC_HOURS_ALARM, 0);
	for (i = RTC_CONFIG_A; i < RTC_REG_SIZE; i++)
		rtc_write8(i, 0);
	printf("RTC: zeroing CMOS RAM\n");
#endif

	/* Setup the real time clock */
	rtc_write8(RTC_CONFIG_B, RTC_CONFIG_B_24H);
	/* Setup the frequency it operates at */
	rtc_write8(RTC_CONFIG_A, RTC_CONFIG_A_REF_CLCK_32KHZ |
		  RTC_CONFIG_A_RATE_1024HZ);
	/* Ensure all reserved bits are 0 in register D */
	rtc_write8(RTC_CONFIG_D, RTC_CONFIG_D_VALID_RAM_AND_TIME);

	/* Clear any pending interrupts */
	rtc_read8(RTC_CONFIG_C);
}
#endif
