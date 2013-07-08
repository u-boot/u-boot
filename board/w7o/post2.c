/*
 * (C) Copyright 2001
 * Bill Hunter, Wave 7 Optics, williamhunter@mediaone.net
 *   and
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#include <common.h>
#include <config.h>
#include <rtc.h>
#include "errors.h"
#include "dtt.h"

/* for LM75 DTT POST test */
#define DTT_READ_TEMP		0x0
#define DTT_CONFIG		0x1
#define DTT_TEMP_HYST		0x2
#define DTT_TEMP_SET		0x3

#if defined(CONFIG_RTC_M48T35A)
void rtctest(void)
{
    volatile uchar *tchar = (uchar*)(CONFIG_SYS_NVRAM_BASE_ADDR + CONFIG_SYS_NVRAM_SIZE - 9);
    struct rtc_time tmp;

    /* set up led code for RTC tests */
    log_stat(ERR_RTCG);

    /*
     * Do RTC battery test. The first write after power up
     * fails if battery is low.
     */
    *tchar = 0xaa;
    if ((*tchar ^ 0xaa) != 0x0) log_warn(ERR_RTCBAT);
    *tchar = 0x55;				/* Reset test address */

    /*
     * Now lets check the validity of the values in the RTC.
     */
    rtc_get(&tmp);
    if ((tmp.tm_sec < 0)	| (tmp.tm_sec  > 59)   |
	(tmp.tm_min < 0)	| (tmp.tm_min  > 59)   |
	(tmp.tm_hour < 0)	| (tmp.tm_hour > 23)   |
	(tmp.tm_mday < 1 )	| (tmp.tm_mday > 31)   |
	(tmp.tm_mon < 1 )	| (tmp.tm_mon  > 12)   |
	(tmp.tm_year < 2000)	| (tmp.tm_year > 2500) |
	(tmp.tm_wday < 1 )	| (tmp.tm_wday > 7)) {
	log_warn(ERR_RTCTIM);
	rtc_reset();
    }

    /*
     * Now lets do a check to see if the NV RAM is there.
     */
    *tchar = 0xaa;
    if ((*tchar ^ 0xaa) != 0x0) log_err(ERR_RTCVAL);
    *tchar = 0x55;				/* Reset test address */

} /* rtctest() */
#endif	/* CONFIG_RTC_M48T35A */


#ifdef CONFIG_DTT_LM75
int dtt_test(int sensor)
{
    short temp, trip, hyst;

    /* get values */
    temp = dtt_read(sensor, DTT_READ_TEMP) / 256;
    trip = dtt_read(sensor, DTT_TEMP_SET) / 256;
    hyst = dtt_read(sensor, DTT_TEMP_HYST) / 256;

    /* check values */
    if ((hyst != (CONFIG_SYS_DTT_MAX_TEMP - CONFIG_SYS_DTT_HYSTERESIS)) ||
	(trip != CONFIG_SYS_DTT_MAX_TEMP) ||
	(temp < CONFIG_SYS_DTT_LOW_TEMP) || (temp > CONFIG_SYS_DTT_MAX_TEMP))
	return 1;

    return 0;
} /* dtt_test() */
#endif /* CONFIG_DTT_LM75 */

/*****************************************/

void post2(void)
{
#if defined(CONFIG_RTC_M48T35A)
    rtctest();
#endif	/* CONFIG_RTC_M48T35A */

#ifdef CONFIG_DTT_LM75
    log_stat(ERR_TempG);
    if(dtt_test(2) != 0) log_warn(ERR_Ttest0);
    if(dtt_test(4) != 0) log_warn(ERR_Ttest1);
#endif /* CONFIG_DTT_LM75 */
} /* post2() */
