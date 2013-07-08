/*
 * (C) Copyright 2008
 * Gururaja Hebbar gururajakr@sanyo.co.in
 *
 * reference linux-2.6.20.6/drivers/rtc/rtc-pl031.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_CMD_DATE)

#ifndef CONFIG_SYS_RTC_PL031_BASE
#error CONFIG_SYS_RTC_PL031_BASE is not defined!
#endif

/*
 * Register definitions
 */
#define	RTC_DR		0x00	/* Data read register */
#define	RTC_MR		0x04	/* Match register */
#define	RTC_LR		0x08	/* Data load register */
#define	RTC_CR		0x0c	/* Control register */
#define	RTC_IMSC	0x10	/* Interrupt mask and set register */
#define	RTC_RIS		0x14	/* Raw interrupt status register */
#define	RTC_MIS		0x18	/* Masked interrupt status register */
#define	RTC_ICR		0x1c	/* Interrupt clear register */

#define RTC_CR_START	(1 << 0)

#define	RTC_WRITE_REG(addr, val) \
			(*(volatile unsigned int *)(CONFIG_SYS_RTC_PL031_BASE + (addr)) = (val))
#define	RTC_READ_REG(addr)	\
			(*(volatile unsigned int *)(CONFIG_SYS_RTC_PL031_BASE + (addr)))

static int pl031_initted = 0;

/* Enable RTC Start in Control register*/
void rtc_init(void)
{
	RTC_WRITE_REG(RTC_CR, RTC_CR_START);

	pl031_initted = 1;
}

/*
 * Reset the RTC. We set the date back to 1970-01-01.
 */
void rtc_reset(void)
{
	RTC_WRITE_REG(RTC_LR, 0x00);
	if(!pl031_initted)
		rtc_init();
}

/*
 * Set the RTC
*/
int rtc_set(struct rtc_time *tmp)
{
	unsigned long tim;

	if(!pl031_initted)
		rtc_init();

	if (tmp == NULL) {
		puts("Error setting the date/time\n");
		return -1;
	}

	/* Calculate number of seconds this incoming time represents */
	tim = mktime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
	                tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	RTC_WRITE_REG(RTC_LR, tim);

	return -1;
}

/*
 * Get the current time from the RTC
 */
int rtc_get(struct rtc_time *tmp)
{
	ulong tim;

	if(!pl031_initted)
		rtc_init();

	if (tmp == NULL) {
		puts("Error getting the date/time\n");
		return -1;
	}

	tim = RTC_READ_REG(RTC_DR);

	to_tm (tim, tmp);

	debug ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

#endif
