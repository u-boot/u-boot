/*
 * Freescale i.MX28 Register Accessors
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
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

#ifndef __MX28_REGS_COMMON_H__
#define __MX28_REGS_COMMON_H__

/*
 * The i.MX28 has interesting feature when it comes to register access. There
 * are four kinds of access to one particular register. Those are:
 *
 * 1) Common read/write access. To use this mode, just write to the address of
 *    the register.
 * 2) Set bits only access. To set bits, write which bits you want to set to the
 *    address of the register + 0x4.
 * 3) Clear bits only access. To clear bits, write which bits you want to clear
 *    to the address of the register + 0x8.
 * 4) Toggle bits only access. To toggle bits, write which bits you want to
 *    toggle to the address of the register + 0xc.
 *
 * IMPORTANT NOTE: Not all registers support accesses 2-4! Also, not all bits
 * can be set/cleared by pure write as in access type 1, some need to be
 * explicitly set/cleared by using access type 2-3.
 *
 * The following macros and structures allow the user to either access the
 * register in all aforementioned modes (by accessing reg_name, reg_name_set,
 * reg_name_clr, reg_name_tog) or pass the register structure further into
 * various functions with correct type information (by accessing reg_name_reg).
 *
 */

#define	__mx28_reg_8(name)		\
	uint8_t	name[4];		\
	uint8_t	name##_set[4];		\
	uint8_t	name##_clr[4];		\
	uint8_t	name##_tog[4];		\

#define	__mx28_reg_32(name)		\
	uint32_t name;			\
	uint32_t name##_set;		\
	uint32_t name##_clr;		\
	uint32_t name##_tog;

struct mx28_register_8 {
	__mx28_reg_8(reg)
};

struct mx28_register_32 {
	__mx28_reg_32(reg)
};

#define	mx28_reg_8(name)				\
	union {						\
		struct { __mx28_reg_8(name) };		\
		struct mx28_register_8 name##_reg;	\
	};

#define	mx28_reg_32(name)				\
	union {						\
		struct { __mx28_reg_32(name) };		\
		struct mx28_register_32 name##_reg;	\
	};

#endif	/* __MX28_REGS_COMMON_H__ */
