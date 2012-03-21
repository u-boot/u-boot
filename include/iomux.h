/*
 * (C) Copyright 2008
 * Gary Jennejohn, DENX Software Engineering GmbH, garyj@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _IO_MUX_H
#define _IO_MUX_H

#include <stdio_dev.h>

/*
 * Stuff required to support console multiplexing.
 */

/*
 * Pointers to devices used for each file type.  Defined in console.c
 * but storage is allocated in iomux.c.
 */
extern struct stdio_dev **console_devices[MAX_FILES];
/*
 * The count of devices assigned to each FILE.  Defined in console.c
 * and populated in iomux.c.
 */
extern int cd_count[MAX_FILES];

int iomux_doenv(const int, const char *);
void iomux_printdevs(const int);
struct stdio_dev *search_device(int, const char *);

#endif /* _IO_MUX_H */
