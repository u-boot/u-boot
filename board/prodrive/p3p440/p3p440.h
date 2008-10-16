/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#ifndef __P3P440_H__
#define __P3P440_H__

#define CONFIG_SYS_GPIO_RDY	(0x80000000 >> 11)
#define CONFIG_SYS_MONARCH_IO	(0x80000000 >> 18)
#define CONFIG_SYS_EREADY_IO	(0x80000000 >> 20)
#define CONFIG_SYS_LED_GREEN	(0x80000000 >> 21)
#define CONFIG_SYS_LED_RED	(0x80000000 >> 22)

#define LED_OFF		1
#define LED_GREEN	2
#define LED_RED		3
#define LED_ORANGE	4

long int fixed_sdram(void);

#endif /* __P3P440_H__ */
