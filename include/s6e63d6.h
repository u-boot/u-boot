/*
 * Copyright (C) 2009
 * Guennadi Liakhovetski, DENX Software Engineering, <lg@denx.de>
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
#ifndef _S6E63D6_H_
#define _S6E63D6_H_

struct s6e63d6 {
	unsigned int bus;
	unsigned int cs;
	unsigned int id;
	struct spi_slave *slave;
};

extern int s6e63d6_init(struct s6e63d6 *data);
extern int s6e63d6_index(struct s6e63d6 *data, u8 idx);
extern int s6e63d6_param(struct s6e63d6 *data, u16 param);

#endif
