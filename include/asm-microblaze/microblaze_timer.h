/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.cz>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define TIMER_ENABLE_ALL    0x400 /* ENALL */
#define TIMER_PWM           0x200 /* PWMA0 */
#define TIMER_INTERRUPT     0x100 /* T0INT */
#define TIMER_ENABLE        0x080 /* ENT0 */
#define TIMER_ENABLE_INTR   0x040 /* ENIT0 */
#define TIMER_RESET         0x020 /* LOAD0 */
#define TIMER_RELOAD        0x010 /* ARHT0 */
#define TIMER_EXT_CAPTURE   0x008 /* CAPT0 */
#define TIMER_EXT_COMPARE   0x004 /* GENT0 */
#define TIMER_DOWN_COUNT    0x002 /* UDT0 */
#define TIMER_CAPTURE_MODE  0x001 /* MDT0 */

typedef volatile struct microblaze_timer_t {
	int control; /* control/statuc register TCSR */
	int loadreg; /* load register TLR */
	int counter; /* timer/counter register */
} microblaze_timer_t;
