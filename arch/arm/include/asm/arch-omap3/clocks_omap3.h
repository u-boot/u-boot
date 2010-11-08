/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _CLOCKS_OMAP3_H_
#define _CLOCKS_OMAP3_H_

#define PLL_STOP		1	/* PER & IVA */
#define PLL_LOW_POWER_BYPASS	5	/* MPU, IVA & CORE */
#define PLL_FAST_RELOCK_BYPASS	6	/* CORE */
#define PLL_LOCK		7	/* MPU, IVA, CORE & PER */

/*
 * The following configurations are OPP and SysClk value independant
 * and hence are defined here. All the other DPLL related values are
 * tabulated in lowlevel_init.S.
 */

/* CORE DPLL */
#define CORE_M3X2	2	/* 332MHz : CM_CLKSEL1_EMU */
#define CORE_SSI_DIV	3	/* 221MHz : CM_CLKSEL_CORE */
#define CORE_FUSB_DIV	2	/* 41.5MHz: */
#define CORE_L4_DIV	2	/* 83MHz  : L4 */
#define CORE_L3_DIV	2	/* 166MHz : L3 {DDR} */
#define GFX_DIV		2	/* 83MHz  : CM_CLKSEL_GFX */
#define WKUP_RSM	2	/* 41.5MHz: CM_CLKSEL_WKUP */

/* PER DPLL */
#define PER_M6X2	3	/* 288MHz: CM_CLKSEL1_EMU */
#define PER_M5X2	4	/* 216MHz: CM_CLKSEL_CAM */
#define PER_M4X2	2	/* 432MHz: CM_CLKSEL_DSS-dss1 */
#define PER_M3X2	16	/* 54MHz : CM_CLKSEL_DSS-tv */

#define CLSEL1_EMU_VAL ((CORE_M3X2 << 16) | (PER_M6X2 << 24) | (0x0A50))

/* MPU DPLL */

#define MPU_M_12_ES1		0x0FE
#define MPU_N_12_ES1		0x07
#define MPU_FSEL_12_ES1		0x05
#define MPU_M2_12_ES1		0x01

#define MPU_M_12_ES2		0x0FA
#define MPU_N_12_ES2		0x05
#define MPU_FSEL_12_ES2		0x07
#define MPU_M2_ES2		0x01

#define MPU_M_12		0x085
#define MPU_N_12		0x05
#define MPU_FSEL_12		0x07
#define MPU_M2_12		0x01

#define MPU_M_13_ES1		0x17D
#define MPU_N_13_ES1		0x0C
#define MPU_FSEL_13_ES1		0x03
#define MPU_M2_13_ES1		0x01

#define MPU_M_13_ES2		0x1F4
#define MPU_N_13_ES2		0x0C
#define MPU_FSEL_13_ES2		0x03
#define MPU_M2_13_ES2		0x01

#define MPU_M_13		0x10A
#define MPU_N_13		0x0C
#define MPU_FSEL_13		0x03
#define MPU_M2_13		0x01

#define MPU_M_19P2_ES1		0x179
#define MPU_N_19P2_ES1		0x12
#define MPU_FSEL_19P2_ES1	0x04
#define MPU_M2_19P2_ES1		0x01

#define MPU_M_19P2_ES2		0x271
#define MPU_N_19P2_ES2		0x17
#define MPU_FSEL_19P2_ES2	0x03
#define MPU_M2_19P2_ES2		0x01

#define MPU_M_19P2		0x14C
#define MPU_N_19P2		0x17
#define MPU_FSEL_19P2		0x03
#define MPU_M2_19P2		0x01

#define MPU_M_26_ES1		0x17D
#define MPU_N_26_ES1		0x19
#define MPU_FSEL_26_ES1		0x03
#define MPU_M2_26_ES1		0x01

#define MPU_M_26_ES2		0x0FA
#define MPU_N_26_ES2		0x0C
#define MPU_FSEL_26_ES2		0x07
#define MPU_M2_26_ES2		0x01

#define MPU_M_26		0x085
#define MPU_N_26		0x0C
#define MPU_FSEL_26		0x07
#define MPU_M2_26		0x01

#define MPU_M_38P4_ES1		0x1FA
#define MPU_N_38P4_ES1		0x32
#define MPU_FSEL_38P4_ES1	0x03
#define MPU_M2_38P4_ES1		0x01

#define MPU_M_38P4_ES2		0x271
#define MPU_N_38P4_ES2		0x2F
#define MPU_FSEL_38P4_ES2	0x03
#define MPU_M2_38P4_ES2		0x01

#define MPU_M_38P4		0x14C
#define MPU_N_38P4		0x2F
#define MPU_FSEL_38P4		0x03
#define MPU_M2_38P4		0x01

/* IVA DPLL */

#define IVA_M_12_ES1		0x07D
#define IVA_N_12_ES1		0x05
#define IVA_FSEL_12_ES1		0x07
#define IVA_M2_12_ES1		0x01

#define IVA_M_12_ES2		0x0B4
#define IVA_N_12_ES2		0x05
#define IVA_FSEL_12_ES2		0x07
#define IVA_M2_12_ES2		0x01

#define IVA_M_12		0x085
#define IVA_N_12		0x05
#define IVA_FSEL_12		0x07
#define IVA_M2_12		0x01

#define IVA_M_13_ES1		0x0FA
#define IVA_N_13_ES1		0x0C
#define IVA_FSEL_13_ES1		0x03
#define IVA_M2_13_ES1		0x01

#define IVA_M_13_ES2		0x168
#define IVA_N_13_ES2		0x0C
#define IVA_FSEL_13_ES2		0x03
#define IVA_M2_13_ES2		0x01

#define IVA_M_13		0x10A
#define IVA_N_13		0x0C
#define IVA_FSEL_13		0x03
#define IVA_M2_13		0x01

#define IVA_M_19P2_ES1		0x082
#define IVA_N_19P2_ES1		0x09
#define IVA_FSEL_19P2_ES1	0x07
#define IVA_M2_19P2_ES1		0x01

#define IVA_M_19P2_ES2		0x0E1
#define IVA_N_19P2_ES2		0x0B
#define IVA_FSEL_19P2_ES2	0x06
#define IVA_M2_19P2_ES2		0x01

#define IVA_M_19P2		0x14C
#define IVA_N_19P2		0x17
#define IVA_FSEL_19P2		0x03
#define IVA_M2_19P2		0x01

#define IVA_M_26_ES1		0x07D
#define IVA_N_26_ES1		0x0C
#define IVA_FSEL_26_ES1		0x07
#define IVA_M2_26_ES1		0x01

#define IVA_M_26_ES2		0x0B4
#define IVA_N_26_ES2		0x0C
#define IVA_FSEL_26_ES2		0x07
#define IVA_M2_26_ES2		0x01

#define IVA_M_26		0x085
#define IVA_N_26		0x0C
#define IVA_FSEL_26		0x07
#define IVA_M2_26		0x01

#define IVA_M_38P4_ES1		0x13F
#define IVA_N_38P4_ES1		0x30
#define IVA_FSEL_38P4_ES1	0x03
#define IVA_M2_38P4_ES1		0x01

#define IVA_M_38P4_ES2		0x0E1
#define IVA_N_38P4_ES2		0x17
#define IVA_FSEL_38P4_ES2	0x06
#define IVA_M2_38P4_ES2		0x01

#define IVA_M_38P4		0x14C
#define IVA_N_38P4		0x2F
#define IVA_FSEL_38P4		0x03
#define IVA_M2_38P4		0x01

/* CORE DPLL */

#define CORE_M_12		0xA6
#define CORE_N_12		0x05
#define CORE_FSEL_12		0x07
#define CORE_M2_12		0x01	/* M3 of 2 */

#define CORE_M_12_ES1		0x19F
#define CORE_N_12_ES1		0x0E
#define CORE_FSL_12_ES1		0x03
#define CORE_M2_12_ES1		0x1	/* M3 of 2 */

#define CORE_M_13		0x14C
#define CORE_N_13		0x0C
#define CORE_FSEL_13		0x03
#define CORE_M2_13		0x01	/* M3 of 2 */

#define CORE_M_13_ES1		0x1B2
#define CORE_N_13_ES1		0x10
#define CORE_FSL_13_ES1		0x03
#define CORE_M2_13_ES1		0x01	/* M3 of 2 */

#define CORE_M_19P2		0x19F
#define CORE_N_19P2		0x17
#define CORE_FSEL_19P2		0x03
#define CORE_M2_19P2		0x01	/* M3 of 2 */

#define CORE_M_19P2_ES1		0x19F
#define CORE_N_19P2_ES1		0x17
#define CORE_FSL_19P2_ES1	0x03
#define CORE_M2_19P2_ES1	0x01	/* M3 of 2 */

#define CORE_M_26		0xA6
#define CORE_N_26		0x0C
#define CORE_FSEL_26		0x07
#define CORE_M2_26		0x01	/* M3 of 2 */

#define CORE_M_26_ES1		0x1B2
#define CORE_N_26_ES1		0x21
#define CORE_FSL_26_ES1		0x03
#define CORE_M2_26_ES1		0x01	/* M3 of 2 */

#define CORE_M_38P4		0x19F
#define CORE_N_38P4		0x2F
#define CORE_FSEL_38P4		0x03
#define CORE_M2_38P4		0x01	/* M3 of 2 */

#define CORE_M_38P4_ES1		0x19F
#define CORE_N_38P4_ES1		0x2F
#define CORE_FSL_38P4_ES1	0x03
#define CORE_M2_38P4_ES1	0x01	/* M3 of 2 */

/* PER DPLL */

#define PER_M_12		0xD8
#define PER_N_12		0x05
#define PER_FSEL_12		0x07
#define PER_M2_12		0x09

#define PER_M_13		0x1B0
#define PER_N_13		0x0C
#define PER_FSEL_13		0x03
#define PER_M2_13		0x09

#define PER_M_19P2		0xE1
#define PER_N_19P2		0x09
#define PER_FSEL_19P2		0x07
#define PER_M2_19P2		0x09

#define PER_M_26		0xD8
#define PER_N_26		0x0C
#define PER_FSEL_26		0x07
#define PER_M2_26		0x09

#define PER_M_38P4		0xE1
#define PER_N_38P4		0x13
#define PER_FSEL_38P4		0x07
#define PER_M2_38P4		0x09

/* 36XX PER DPLL */

#define PER_36XX_M_12		0x1B0
#define PER_36XX_N_12		0x05
#define PER_36XX_FSEL_12	0x07
#define PER_36XX_M2_12		0x09

#define PER_36XX_M_13		0x360
#define PER_36XX_N_13		0x0C
#define PER_36XX_FSEL_13	0x03
#define PER_36XX_M2_13		0x09

#define PER_36XX_M_19P2		0x1C2
#define PER_36XX_N_19P2		0x09
#define PER_36XX_FSEL_19P2	0x07
#define PER_36XX_M2_19P2	0x09

#define PER_36XX_M_26		0x1B0
#define PER_36XX_N_26		0x0C
#define PER_36XX_FSEL_26	0x07
#define PER_36XX_M2_26		0x09

#define PER_36XX_M_38P4		0x1C2
#define PER_36XX_N_38P4		0x13
#define PER_36XX_FSEL_38P4	0x07
#define PER_36XX_M2_38P4	0x09

#endif	/* endif _CLOCKS_OMAP3_H_ */
