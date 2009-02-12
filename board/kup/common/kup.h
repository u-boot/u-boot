/*
 * (C) Copyright 2004
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de.
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

#ifndef __KUP_H
#define __KUP_H

#define PA_8       0x0080
#define PA_11      0x0010
#define PA_12      0x0008

#define PB_14      0x00020000
#define PB_17      0x00004000

#define PC_9       0x0040

#define PA_RS485                 PA_11   /* SCC1: 0=RS232 1=RS485 */
#define PA_LED_YELLOW            PA_8
#define BP_USB_VCC               PB_14   /* VCC for USB devices 0=vcc on, 1=vcc off*/
#define PB_LCD_PWM	         PB_17	 /* PB 17 */
#define PC_SWITCH1               PC_9    /* Reboot switch */

extern void poweron_key (void);

extern void load_sernum_ethaddr(void);

#endif	/* __KUP_H */
