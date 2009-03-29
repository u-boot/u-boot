/*
 * (C) Copyright 2009
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
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
 *
 */

#ifndef _TWS_H_
#define _TWS_H_

/*
 * Read/Write interface:
 *   buffer:  Where to read/write the data
 *   len:     How many bits to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int tws_read(uchar *buffer, int len);
int tws_write(uchar *buffer, int len);

#endif	/* _TWS_H_ */
