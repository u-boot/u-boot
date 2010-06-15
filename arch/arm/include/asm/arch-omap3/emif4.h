/*
 * Auther:
 *       Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Copyright (C) 2010
 * Texas Instruments Incorporated - http://www.ti.com/
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

#ifndef _EMIF_H_
#define _EMIF_H_

/*
 * Configuration values
 */
#define EMIF4_TIM1_T_RP		(0x3 << 25)
#define EMIF4_TIM1_T_RCD	(0x3 << 21)
#define EMIF4_TIM1_T_WR		(0x3 << 17)
#define EMIF4_TIM1_T_RAS	(0x8 << 12)
#define EMIF4_TIM1_T_RC		(0xA << 6)
#define EMIF4_TIM1_T_RRD	(0x2 << 3)
#define EMIF4_TIM1_T_WTR	(0x2)

#define EMIF4_TIM2_T_XP		(0x2 << 28)
#define EMIF4_TIM2_T_ODT	(0x0 << 25)
#define EMIF4_TIM2_T_XSNR	(0x1C << 16)
#define EMIF4_TIM2_T_XSRD	(0xC8 << 6)
#define EMIF4_TIM2_T_RTP	(0x1 << 3)
#define EMIF4_TIM2_T_CKE	(0x2)

#define EMIF4_TIM3_T_RFC	(0x25 << 4)
#define EMIF4_TIM3_T_RAS_MAX	(0x7)

#define EMIF4_PWR_IDLE_MODE	(0x2 << 30)
#define EMIF4_PWR_DPD_DIS	(0x0 << 10)
#define EMIF4_PWR_DPD_EN	(0x1 << 10)
#define EMIF4_PWR_LP_MODE	(0x0 << 8)
#define EMIF4_PWR_PM_TIM	(0x0)

#define EMIF4_INITREF_DIS	(0x0 << 31)
#define EMIF4_REFRESH_RATE	(0x50F)

#define EMIF4_CFG_SDRAM_TYP	(0x2 << 29)
#define EMIF4_CFG_IBANK_POS	(0x0 << 27)
#define EMIF4_CFG_DDR_TERM	(0x0 << 24)
#define EMIF4_CFG_DDR2_DDQS	(0x1 << 23)
#define EMIF4_CFG_DDR_DIS_DLL	(0x0 << 20)
#define EMIF4_CFG_SDR_DRV	(0x0 << 18)
#define EMIF4_CFG_NARROW_MD	(0x0 << 14)
#define EMIF4_CFG_CL		(0x5 << 10)
#define EMIF4_CFG_ROWSIZE	(0x0 << 7)
#define EMIF4_CFG_IBANK		(0x3 << 4)
#define EMIF4_CFG_EBANK		(0x0 << 3)
#define EMIF4_CFG_PGSIZE	(0x2)

/*
 * EMIF4 PHY Control 1 register configuration
 */
#define EMIF4_DDR1_EXT_STRB_EN	(0x1 << 7)
#define EMIF4_DDR1_EXT_STRB_DIS	(0x0 << 7)
#define EMIF4_DDR1_PWRDN_DIS	(0x0 << 6)
#define EMIF4_DDR1_PWRDN_EN	(0x1 << 6)
#define EMIF4_DDR1_READ_LAT	(0x6 << 0)

#endif /* endif _EMIF_H_ */
