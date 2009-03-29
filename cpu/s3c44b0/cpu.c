/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
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

/*
 * S3C44B0 CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/hardware.h>

static void s3c44b0_flush_cache(void)
{
	volatile int i;
	/* flush cycle */
	for(i=0x10002000;i<0x10004800;i+=16)
	{
		*((int *)i)=0x0;
	}
}


int cpu_init (void)
{
	icache_enable();

	return 0;
}

int cleanup_before_linux (void)
{
	/*
		cache memory should be enabled before calling
		Linux to make the kernel uncompression faster
	*/
	icache_enable();

	disable_interrupts ();

	return 0;
}

void reset_cpu (ulong addr)
{
	/*
		reset the cpu using watchdog
	*/

	/* Disable the watchdog.*/
	WTCON&=~(1<<5);

	/* set the timeout value to a short time... */
	WTCNT = 0x1;

	/* Enable the watchdog. */
	WTCON|=1;
	WTCON|=(1<<5);

	while(1) {
		/*NOP*/
	}
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	disable_interrupts ();
	reset_cpu (0);

	/*NOTREACHED*/
	return (0);
}

void icache_enable (void)
{
	ulong reg;

	s3c44b0_flush_cache();

	/*
		Init cache
		Non-cacheable area (everything outside RAM)
		0x0000:0000 - 0x0C00:0000
	 */
	NCACHBE0 = 0xC0000000;
	NCACHBE1 = 0x00000000;

	/*
		Enable chache
	*/
	reg = SYSCFG;
	reg |= 0x00000006; /* 8kB */
	SYSCFG = reg;
}

void icache_disable (void)
{
	ulong reg;

	reg = SYSCFG;
	reg &= ~0x00000006; /* 8kB */
	SYSCFG = reg;
}

int icache_status (void)
{
	return 0;
}

void dcache_enable (void)
{
	icache_enable();
}

void dcache_disable (void)
{
	icache_disable();
}

int dcache_status (void)
{
	return dcache_status();
}

/*
	RTC stuff
*/
#include <rtc.h>
#ifndef BCD2HEX
	#define BCD2HEX(n)  ((n>>4)*10+(n&0x0f))
#endif
#ifndef HEX2BCD
	#define HEX2BCD(x) ((((x) / 10) << 4) + (x) % 10)
#endif

int rtc_get (struct rtc_time* tm)
{
	RTCCON |= 1;
	tm->tm_year  = BCD2HEX(BCDYEAR);
	tm->tm_mon   = BCD2HEX(BCDMON);
	tm->tm_wday   = BCD2HEX(BCDDATE);
	tm->tm_mday   = BCD2HEX(BCDDAY);
	tm->tm_hour  = BCD2HEX(BCDHOUR);
	tm->tm_min  = BCD2HEX(BCDMIN);
	tm->tm_sec  = BCD2HEX(BCDSEC);

	if (tm->tm_sec==0) {
		/* we have to re-read the rtc data because of the "one second deviation" problem */
		/* see RTC datasheet for more info about it */
		tm->tm_year  = BCD2HEX(BCDYEAR);
		tm->tm_mon   = BCD2HEX(BCDMON);
		tm->tm_mday   = BCD2HEX(BCDDAY);
		tm->tm_wday   = BCD2HEX(BCDDATE);
		tm->tm_hour  = BCD2HEX(BCDHOUR);
		tm->tm_min  = BCD2HEX(BCDMIN);
		tm->tm_sec  = BCD2HEX(BCDSEC);
	}

	RTCCON &= ~1;

	if(tm->tm_year >= 70)
		tm->tm_year += 1900;
	else
		tm->tm_year += 2000;

	return 0;
}

int rtc_set (struct rtc_time* tm)
{
	if(tm->tm_year < 2000)
		tm->tm_year -= 1900;
	else
		tm->tm_year -= 2000;

	RTCCON |= 1;
	BCDYEAR = HEX2BCD(tm->tm_year);
	BCDMON = HEX2BCD(tm->tm_mon);
	BCDDAY = HEX2BCD(tm->tm_mday);
	BCDDATE = HEX2BCD(tm->tm_wday);
	BCDHOUR = HEX2BCD(tm->tm_hour);
	BCDMIN = HEX2BCD(tm->tm_min);
	BCDSEC = HEX2BCD(tm->tm_sec);
	RTCCON &= 1;

	return 0;
}

void rtc_reset (void)
{
	RTCCON |= 1;
	BCDYEAR = 0;
	BCDMON = 0;
	BCDDAY = 0;
	BCDDATE = 0;
	BCDHOUR = 0;
	BCDMIN = 0;
	BCDSEC = 0;
	RTCCON &= 1;
}
