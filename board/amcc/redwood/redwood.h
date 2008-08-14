/*
 * (C) Copyright 2008
 * Feng Kan, Applied Micro Circuit Corp., fkan@amcc.com.
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

#ifndef __REDWOOD_H_
#define __REDWOOD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------+
| Defines
+----------------------------------------------------------------------------*/
/* Pin Straps Reg */
#define SDR0_PSTRP0			0x0040
#define SDR0_PSTRP0_BOOTSTRAP_MASK	0xE0000000	/* Strap Bits */

#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS0	0x00000000	/* Default strap settings 0 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS1	0x20000000	/* Default strap settings 1 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS2	0x40000000	/* Default strap settings 2 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS3	0x60000000	/* Default strap settings 3 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS4	0x80000000	/* Default strap settings 4 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS5	0xA0000000	/* Default strap settings 5 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS6	0xC0000000	/* Default strap settings 6 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS7	0xE0000000	/* Default strap settings 7 */

#ifdef __cplusplus
}
#endif
#endif				/* __REDWOOD_H_ */
