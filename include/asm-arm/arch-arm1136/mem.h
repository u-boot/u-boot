/*
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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

#ifndef _OMAP24XX_MEM_H_
#define _OMAP24XX_MEM_H_

#define SDRC_CS0_OSET    0x0
#define SDRC_CS1_OSET    0x30  /* mirror CS1 regs appear offset 0x30 from CS0 */

#ifndef __ASSEMBLY__
/* struct's for holding data tables for current boards, they are getting used
   early in init when NO global access are there */
struct sdrc_data_s {
	u32    sdrc_sharing;
	u32    sdrc_mdcfg_0;
	u32    sdrc_actim_ctrla_0;
	u32    sdrc_actim_ctrlb_0;
	u32    sdrc_rfr_ctrl;
	u32    sdrc_mr_0;
	u32    sdrc_dlla_ctrl;
	u32    sdrc_dllb_ctrl;
} /*__attribute__ ((packed))*/;
typedef struct sdrc_data_s sdrc_data_t;
#endif

/* Slower full frequency range default timings for x32 operation*/
#define H4_2420_SDRC_SHARING               0x00000100
#define H4_2420_SDRC_MDCFG_0               0x01702011
#define H4_2420_SDRC_ACTIM_CTRLA_0         0x9bead909
#define H4_2420_SDRC_ACTIM_CTRLB_0         0x00000014
#define H4_2420_SDRC_RFR_CTRL_ES1          0x00002401
#define H4_2420_SDRC_RFR_CTRL              0x0002da01
#define H4_2420_SDRC_MR_0                  0x00000032
#define H4_2420_SDRC_DLLA_CTRL             0x00007307
#define H4_2420_SDRC_DLLB_CTRL             0x00007307

#define H4_2422_SDRC_SHARING               0x00004b00
#define H4_2422_SDRC_MDCFG_0               0x00801011
#define H4_2422_SDRC_ACTIM_CTRLA_0         0x9BEAD909
#define H4_2422_SDRC_ACTIM_CTRLB_0         0x00000020
#define H4_2422_SDRC_RFR_CTRL_ES1          0x00002401
#define H4_2422_SDRC_RFR_CTRL              0x0002da03
#define H4_2422_SDRC_MR_0                  0x00000032
#define H4_2422_SDRC_DLLA_CTRL             0x00007307
#define H4_2422_SDRC_DLLB_CTRL             0x00007307

#define H4_2420_COMBO_MDCFG_0              0x00801011

/* optimized timings */
#define H4_2420_SDRC_ACTIM_CTRLA_0_100MHz  0x5A59B485
#define H4_2420_SDRC_ACTIM_CTRLB_0_100MHz  0x0000000e


#ifdef PRCM_CONFIG_II        /* L3 at 100MHz */
#define H4_24XX_GPMC_CONFIG1_0   0x3
#define H4_24XX_GPMC_CONFIG2_0   0x001f1f01
#define H4_24XX_GPMC_CONFIG3_0   0x00030301
#define H4_24XX_GPMC_CONFIG4_0   0x0C030C03
#define H4_24XX_GPMC_CONFIG5_0   0x01131F1F
#define H4_24XX_GPMC_CONFIG7_0   (0x00000C40|(H4_CS0_BASE >> 24))

#define H4_24XX_GPMC_CONFIG1_1   0x00011000
#define H4_24XX_GPMC_CONFIG2_1   0x001F1F00
#define H4_24XX_GPMC_CONFIG3_1   0x00080802
#define H4_24XX_GPMC_CONFIG4_1   0x1C091C09
#define H4_24XX_GPMC_CONFIG5_1   0x031A1F1F
#define H4_24XX_GPMC_CONFIG6_1   0x000003C2
#define H4_24XX_GPMC_CONFIG7_1   (0x00000F40|(H4_CS1_BASE >> 24))
#endif

#ifdef PRCM_CONFIG_III  /* L3 at 133MHz */
#define H4_24XX_GPMC_CONFIG1_0   0x3
#define H4_24XX_GPMC_CONFIG2_0   0x001f1f01
#define H4_24XX_GPMC_CONFIG3_0   0x001F1F00
#define H4_24XX_GPMC_CONFIG4_0   0x16061606
#define H4_24XX_GPMC_CONFIG5_0   0x01131F1F
#define H4_24XX_GPMC_CONFIG7_0   (0x00000C40|(H4_CS0_BASE >> 24))

#define H4_24XX_GPMC_CONFIG1_1   0x00011000
#define H4_24XX_GPMC_CONFIG2_1   0x001f1f01
#define H4_24XX_GPMC_CONFIG3_1   0x001F1F00
#define H4_24XX_GPMC_CONFIG4_1   0x1A061A06
#define H4_24XX_GPMC_CONFIG5_1   0x041F1F1F
#define H4_24XX_GPMC_CONFIG6_1   0x000004C4
#define H4_24XX_GPMC_CONFIG7_1   (0x00000F40|(H4_CS1_BASE >> 24))
#endif

#ifdef CONFIG_APTIX /* SDRC-SDR for Aptix x16 */
#define VAL_H4_SDRC_SHARING_16   0x00002400  /* No-Tristate, 16bit on D31-D16, CS1=dont care */
#define VAL_H4_SDRC_SHARING      0x00000100
#define VAL_H4_SDRC_MCFG_0_16    0x00901000  /* SDR-SDRAM,External,x16 bit */
#define VAL_H4_SDRC_MCFG_0       0x01702011
#define VAL_H4_SDRC_MR_0         0x00000029  /* Burst=2, Serial Mode, CAS 3*/
#define VAL_H4_SDRC_RFR_CTRL_0   0x00001703  /* refresh time */
#define VAL_H4_SDRC_DCDL2_CTRL   0x5A59B485
#endif

#endif
