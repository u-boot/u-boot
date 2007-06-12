/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
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
 * Date & Time support for the MK48T59 RTC
 */


#if defined(CONFIG_RTC_MK48T59) && ((CONFIG_COMMANDS & CFG_CMD_DATE) || defined(CONFIG_CMD_DATE))

#define RTC_PORT_ADDR0		CFG_ISA_IO +  0x70
#define RTC_PORT_ADDR1		RTC_PORT_ADDR0 +  0x1
#define RTC_PORT_DATA		CFG_ISA_IO +  0x76

/* RTC Offsets */
#define RTC_SECONDS             0x1FF9
#define RTC_MINUTES             0x1FFA
#define RTC_HOURS               0x1FFB
#define RTC_DAY_OF_WEEK         0x1FFC
#define RTC_DAY_OF_MONTH        0x1FFD
#define RTC_MONTH               0x1FFE
#define RTC_YEAR                0x1FFF

#define RTC_CONTROLA            0x1FF8
#define RTC_CA_WRITE            0x80
#define RTC_CA_READ             0x40
#define RTC_CA_CALIB_SIGN       0x20
#define RTC_CA_CALIB_MASK       0x1f

#define RTC_CONTROLB            0x1FF9
#define RTC_CB_STOP             0x80

#define RTC_WATCHDOG			0x1FF7
#define RTC_WDS					0x80
#define RTC_WD_RB_16TH			0x0
#define RTC_WD_RB_4TH			0x1
#define RTC_WD_RB_1				0x2
#define RTC_WD_RB_4				0x3

void rtc_set_watchdog(short multi, short res);
void *nvram_read(void *dest, const short src, size_t count);
void nvram_write(short dest, const void *src, size_t count);

#endif
