/*
 * (C) Copyright 2003
 * Martin Krause, TQ-Systems GmbH, <martin.krause@tqs.de>
 *
 * Based on cpu/arm920t/serial.c, by Gary Jennejohn
 * (C) Copyright 2002 Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef _RS485_H_
#define _RS485_H_

#include <s3c2400.h>

int rs485_init (void);
int rs485_getc (void);
void rs485_putc (const char c);
int rs485_tstc (void);
void rs485_puts (const char *s);
void trab_rs485_enable_tx(void);
void trab_rs485_enable_rx(void);

#endif	/* _RS485_H_ */
