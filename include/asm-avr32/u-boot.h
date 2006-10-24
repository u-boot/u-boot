/*
 * Copyright (C) 2004-2006 Atmel Corporation
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
#ifndef __ASM_U_BOOT_H__
#define __ASM_U_BOOT_H__ 1

typedef struct bd_info {
	unsigned long		bi_baudrate;
	unsigned long		bi_ip_addr;
	unsigned char		bi_enetaddr[6];
	unsigned char		bi_phy_id[4];
	struct environment_s	*bi_env;
	unsigned long		bi_board_number;
	void			*bi_boot_params;
	struct {
		unsigned long	start;
		unsigned long	size;
	}			bi_dram[CONFIG_NR_DRAM_BANKS];
	unsigned long		bi_flashstart;
	unsigned long		bi_flashsize;
	unsigned long		bi_flashoffset;
} bd_t;

#define bi_memstart bi_dram[0].start
#define bi_memsize bi_dram[0].size

/**
 *  container_of - cast a member of a structure out to the containing structure
 *
 *    @ptr:        the pointer to the member.
 *    @type:       the type of the container struct this is embedded in.
 *    @member:     the name of the member within the struct.
 */
#define container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})

#endif /* __ASM_U_BOOT_H__ */
