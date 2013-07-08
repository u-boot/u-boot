/*
 * (C) Copyright 2008
 * Feng Kan, Applied Micro Circuit Corp., fkan@amcc.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
