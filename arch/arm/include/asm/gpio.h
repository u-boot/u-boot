/*
 * Copyright (c) 2011, NVIDIA Corp. All rights reserved.
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

#ifndef _GPIO_H_
#define _GPIO_H_

#include <asm/arch/gpio.h>
/*
 * Generic GPIO API
 */

int gpio_request(int gp, const char *label);
void gpio_free(int gp);
void gpio_toggle_value(int gp);
int gpio_direction_input(int gp);
int gpio_direction_output(int gp, int value);
int gpio_get_value(int gp);
void gpio_set_value(int gp, int value);

#endif	/* _GPIO_H_ */
