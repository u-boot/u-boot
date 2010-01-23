/*
 * (C) Copyright 2009
 * Ryan CHEN, ST Micoelectronics, ryan.chen@st.com
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

#ifndef __SPEAR_EMI_H__
#define __SPEAR_EMI_H__

#ifdef CONFIG_SPEAR_EMI

struct emi_bank_regs {
	u32 tap;
	u32 tsdp;
	u32 tdpw;
	u32 tdpr;
	u32 tdcs;
	u32 control;
};

struct emi_regs {
	struct emi_bank_regs bank_regs[CONFIG_SYS_MAX_FLASH_BANKS];
	u32 tout;
	u32 ack;
	u32 irq;
};

#define EMI_ACKMSK		0x40

/* control register definitions */
#define EMI_CNTL_ENBBYTEW	(1 << 2)
#define EMI_CNTL_ENBBYTER	(1 << 3)
#define EMI_CNTL_ENBBYTERW	(EMI_CNTL_ENBBYTER | EMI_CNTL_ENBBYTEW)

#endif

#endif
