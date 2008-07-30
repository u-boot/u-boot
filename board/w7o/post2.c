/*
 * (C) Copyright 2001
 * Bill Hunter, Wave 7 Optics, williamhunter@mediaone.net
 *   and
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
    volatile uchar *tchar = (uchar*)(CFG_NVRAM_BASE_ADDR + CFG_NVRAM_SIZE - 9);
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
    if ((hyst != (CFG_DTT_MAX_TEMP - CFG_DTT_HYSTERESIS)) ||
	(trip != CFG_DTT_MAX_TEMP) ||
	(temp < CFG_DTT_LOW_TEMP) || (temp > CFG_DTT_MAX_TEMP))
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
