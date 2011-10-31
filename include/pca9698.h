/*
 * (C) Copyright 2011
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
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

#ifndef __PCA9698_H_
#define __PCA9698_H_

int pca9698_request(unsigned gpio, const char *label);
void pca9698_free(unsigned gpio);
int pca9698_direction_input(u8 addr, unsigned gpio);
int pca9698_direction_output(u8 addr, unsigned gpio, int value);
int pca9698_get_value(u8 addr, unsigned gpio);
int pca9698_set_value(u8 addr, unsigned gpio, int value);

#endif /* __PCA9698_H_ */
