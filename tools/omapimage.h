/*
 * (C) Copyright 2010
 * Linaro LTD, www.linaro.org
 * Author John Rigby <john.rigby@linaro.org>
 * Based on TI's signGP.c
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

#ifndef _OMAPIMAGE_H_
#define _OMAPIMAGE_H_

struct ch_toc {
	uint32_t section_offset;
	uint32_t section_size;
	uint8_t unused[12];
	uint8_t section_name[12];
};

struct ch_settings {
	uint32_t section_key;
	uint8_t valid;
	uint8_t version;
	uint16_t reserved;
	uint32_t flags;
};

struct gp_header {
	uint32_t size;
	uint32_t load_addr;
};

#define KEY_CHSETTINGS 0xC0C0C0C1
#endif /* _OMAPIMAGE_H_ */
