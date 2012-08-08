/*
 * Freescale i.MX27 RTC Register Definitions
 *
 * Copyright (C) 2012 Philippe Reynes <tremyfr@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#ifndef __MX27_REGS_RTC_H__
#define __MX27_REGS_RTC_H__

#ifndef	__ASSEMBLY__
struct rtc_regs {
	u32 hourmin;
	u32 seconds;
	u32 alrm_hm;
	u32 alrm_sec;
	u32 rtcctl;
	u32 rtcisr;
	u32 rtcienr;
	u32 stpwch;
	u32 dayr;
	u32 dayalarm;
};
#endif /* __ASSEMBLY__*/

#endif	/* __MX28_REGS_RTC_H__ */
