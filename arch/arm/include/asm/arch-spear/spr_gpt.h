/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
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

#ifndef _SPR_GPT_H
#define _SPR_GPT_H

struct gpt_regs {
	u8 reserved[0x80];
	u32 control;
	u32 status;
	u32 compare;
	u32 count;
	u32 capture_re;
	u32 capture_fe;
};

/*
 * TIMER_CONTROL register settings
 */

#define GPT_PRESCALER_MASK		0x000F
#define GPT_PRESCALER_1			0x0000
#define GPT_PRESCALER_2 		0x0001
#define GPT_PRESCALER_4 		0x0002
#define GPT_PRESCALER_8 		0x0003
#define GPT_PRESCALER_16		0x0004
#define GPT_PRESCALER_32		0x0005
#define GPT_PRESCALER_64		0x0006
#define GPT_PRESCALER_128		0x0007
#define GPT_PRESCALER_256		0x0008

#define GPT_MODE_SINGLE_SHOT		0x0010
#define GPT_MODE_AUTO_RELOAD		0x0000

#define GPT_ENABLE			0x0020

#define GPT_CAPT_MODE_MASK		0x00C0
#define GPT_CAPT_MODE_NONE		0x0000
#define GPT_CAPT_MODE_RE		0x0040
#define GPT_CAPT_MODE_FE		0x0080
#define GPT_CAPT_MODE_BOTH		0x00C0

#define GPT_INT_MATCH			0x0100
#define GPT_INT_FE			0x0200
#define GPT_INT_RE			0x0400

/*
 * TIMER_STATUS register settings
 */

#define GPT_STS_MATCH			0x0001
#define GPT_STS_FE			0x0002
#define GPT_STS_RE			0x0004

/*
 * TIMER_COMPARE register settings
 */

#define GPT_FREE_RUNNING		0xFFFF

/* Timer, HZ specific defines */
#define CONFIG_SPEAR_HZ			1000
#define CONFIG_SPEAR_HZ_CLOCK		8300000

#endif
