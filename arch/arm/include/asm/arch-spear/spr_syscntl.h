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

struct syscntl_regs {
	u32 scctrl;
	u32 scsysstat;
	u32 scimctrl;
	u32 scimsysstat;
	u32 scxtalctrl;
	u32 scpllctrl;
	u32 scpllfctrl;
	u32 scperctrl0;
	u32 scperctrl1;
	u32 scperen;
	u32 scperdis;
	const u32 scperclken;
	const u32 scperstat;
};
