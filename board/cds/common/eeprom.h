/*
 * Copyright 2004 Freescale Semiconductor.
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

#ifndef __EEPROM_H_
#define __EEPROM_H_


/*
 * EEPROM Board System Register interface.
 */


/*
 * CPU Board Revision
 */
#define MPC85XX_CPU_BOARD_REV(maj, min)	((((maj)&0xff) << 8) | ((min) & 0xff))
#define MPC85XX_CPU_BOARD_MAJOR(rev)	(((rev) >> 8) & 0xff)
#define MPC85XX_CPU_BOARD_MINOR(rev)	((rev) & 0xff)

#define MPC85XX_CPU_BOARD_REV_UNKNOWN	MPC85XX_CPU_BOARD_REV(0,0)
#define MPC85XX_CPU_BOARD_REV_1_0	MPC85XX_CPU_BOARD_REV(1,0)
#define MPC85XX_CPU_BOARD_REV_1_1	MPC85XX_CPU_BOARD_REV(1,1)

/*
 * Returns CPU board revision register as a 16-bit value with
 * the Major in the high byte, and Minor in the low byte.
 */
extern unsigned int get_cpu_board_revision(void);


#endif	/* __CADMUS_H_ */
