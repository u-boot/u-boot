/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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


/* i8254.h Intel 8254 PIT registers */


#ifndef _ASMI386_I8254_H_
#define _ASMI386_I8954_H_       1


#define PIT_T0		0x00		/* PIT channel 0 count/status */
#define PIT_T1		0x01		/* PIT channel 1 count/status */
#define PIT_T2		0x02		/* PIT channel 2 count/status */
#define PIT_COMMAND	0x03		/* PIT mode control, latch and read back */

/* PIT Command Register Bit Definitions */

#define PIT_CMD_CTR0	0x00		/* Select PIT counter 0 */
#define PIT_CMD_CTR1	0x40		/* Select PIT counter 1 */
#define PIT_CMD_CTR2	0x80		/* Select PIT counter 2 */

#define PIT_CMD_LATCH	0x00		/* Counter Latch Command */
#define PIT_CMD_LOW	0x10		/* Access counter bits 7-0 */
#define PIT_CMD_HIGH	0x20		/* Access counter bits 15-8 */
#define PIT_CMD_BOTH	0x30		/* Access counter bits 15-0 in two accesses */

#define PIT_CMD_MODE0	0x00		/* Select mode 0 */
#define PIT_CMD_MODE1	0x02		/* Select mode 1 */
#define PIT_CMD_MODE2	0x04		/* Select mode 2 */
#define PIT_CMD_MODE3	0x06		/* Select mode 3 */
#define PIT_CMD_MODE4	0x08		/* Select mode 4 */
#define PIT_CMD_MODE5	0x0A		/* Select mode 5 */

#endif
