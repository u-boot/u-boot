/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __MV88F6281GTW_GE_H
#define __MV88F6281GTW_GE_H

#define MV88F6281GTW_GE_OE_LOW		(~((1 << 7) | (1 << 12) \
					  |(1 << 20) | (1 << 21)))	/*enable GLED,RLED */
#define MV88F6281GTW_GE_OE_HIGH		(~((1 << 4)|(1 << 6)|(1 << 7)|(1 << 12) \
					  |(1 << 13)|(1 << 16)|(1 << 17)))
#define MV88F6281GTW_GE_OE_VAL_LOW	(1 << 20)	/*make GLED on */
#define MV88F6281GTW_GE_OE_VAL_HIGH	((1 << 6)|(1 << 13)|(1 << 16)|(1 << 17))


#endif /* __MV88F6281GTW_GE_H */
