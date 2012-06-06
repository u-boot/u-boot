/*
 * Another absolute-dummy "driver".
 */

#include <rtc.h>

void rtc_reset(void)
{
	return;
}

int rtc_get(struct rtc_time *t)
{
	t->tm_sec = 0;
	t->tm_min = 0;
	t->tm_hour = 0;
	t->tm_mday = 0;
	t->tm_mon = 0;
	t->tm_year = 0;
	t->tm_wday = 0;
	t->tm_yday = 0;
	t->tm_isdst = 0;

	return 0;
}

int rtc_set(struct rtc_time *t)
{
	return 0;
}
