/*
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

#ifndef _DS1722_H_
#define _DS1722_H_

#define DS1722_RESOLUTION_8BIT	0x0
#define DS1722_RESOLUTION_9BIT	0x1
#define DS1722_RESOLUTION_10BIT	0x2
#define DS1722_RESOLUTION_11BIT	0x3
#define DS1722_RESOLUTION_12BIT	0x4

int ds1722_probe(int dev);

#endif /* _DS1722_H_ */
