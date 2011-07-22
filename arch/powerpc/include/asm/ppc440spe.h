/*
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

#ifndef _PPC440SPE_H_
#define _PPC440SPE_H_

#define CONFIG_SDRAM_PPC4xx_IBM_DDR2	/* IBM DDR(2) controller */

/*
 * Some SoC specific registers (not common for all 440 SoC's)
 */

/* Memory mapped register */
#define CONFIG_SYS_PERIPHERAL_BASE	0xa0000000 /* Internal Peripherals */

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x0200)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_PERIPHERAL_BASE + 0x0300)

#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0700)

/* SDR's */
#define SDR0_PCI0	0x0300
#define SDR0_SDSTP2	0x0022
#define SDR0_SDSTP3	0x0023

#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 13)
#define SDR0_SDSTP1_PISE_MASK		(0x80000000 >> 15)
#define SDR0_SDSTP1_ERPN_MASK		(0x80000000 >> 12)
#define SDR0_SDSTP1_ERPN_EBC		0
#define SDR0_SDSTP1_ERPN_PCI		(0x80000000 >> 12)
#define SDR0_SDSTP1_EBCW_MASK		(0x80000000 >> 24)
#define SDR0_SDSTP1_EBCW_8_BITS		0
#define SDR0_SDSTP1_EBCW_16_BITS	(0x80000000 >> 24)

#define SDR0_PFC1_EM_1000		(0x80000000 >> 10)

#define SDR0_MFR_FIXD			(0x80000000 >> 3)	/* Workaround for PCI/DMA */

#define SDR0_PINSTP_BOOTSTRAP_MASK	0xC0000000  /* Strap Bits */
#define SDR0_PINSTP_BOOTSTRAP_SETTINGS0	0x00000000  /* Default strap settings 0
						       (EBC boot) */
#define SDR0_PINSTP_BOOTSTRAP_SETTINGS1	0x40000000  /* Default strap settings 1
						       (PCI boot) */
#define SDR0_PINSTP_BOOTSTRAP_IIC_54_EN	0x80000000  /* Serial Device Enabled -
						       Addr = 0x54 */
#define SDR0_PINSTP_BOOTSTRAP_IIC_50_EN	0xC0000000  /* Serial Device Enabled -
						       Addr = 0x50 */

#define SDR0_SRST0_DMC			0x00200000

#define PLLSYS0_ENG_MASK	0x80000000	/* 0 = SysClk, 1 = PLL VCO */
#define PLLSYS0_SRC_MASK	0x40000000	/* 0 = PLL A, 1 = PLL B */
#define PLLSYS0_SEL_MASK	0x38000000	/* 0 = PLL, 1 = CPU, 5 = PerClk */
#define PLLSYS0_TUNE_MASK	0x07fe0000	/* PLL Tune bits */
#define PLLSYS0_FB_DIV_MASK	0x0001f000	/* Feedback divisor */
#define PLLSYS0_FWD_DIV_A_MASK	0x00000f00	/* Fwd Div A */
#define PLLSYS0_FWD_DIV_B_MASK	0x000000e0	/* Fwd Div B */
#define PLLSYS0_PRI_DIV_B_MASK	0x0000001c	/* PLL Primary Divisor B */
#define PLLSYS0_OPB_DIV_MASK	0x00000003	/* OPB Divisor */

#define PLLC_ENG_MASK		0x20000000  /* PLL primary forward divisor source */
#define PLLC_SRC_MASK		0x20000000  /* PLL feedback source   */
#define PLLD_FBDV_MASK		0x1f000000  /* PLL Feedback Divisor  */
#define PLLD_FWDVA_MASK		0x000f0000  /* PLL Forward Divisor A */
#define PLLD_FWDVB_MASK		0x00000700  /* PLL Forward Divisor B */
#define PLLD_LFBDV_MASK		0x0000003f  /* PLL Local Feedback Divisor */

#define OPBDDV_MASK		0x03000000  /* OPB Clock Divisor Register */
#define PERDV_MASK		0x07000000  /* Peripheral Clock Divisor */
#define PRADV_MASK		0x07000000  /* Primary Divisor A */
#define PRBDV_MASK		0x07000000  /* Primary Divisor B */
#define SPCID_MASK		0x03000000  /* Sync PCI Divisor  */

/* Strap 1 Register */
#define PLLSYS1_LF_DIV_MASK	0xfc000000	/* PLL Local Feedback Divisor */
#define PLLSYS1_PERCLK_DIV_MASK 0x03000000	/* Peripheral Clk Divisor */
#define PLLSYS1_MAL_DIV_MASK	0x00c00000	/* MAL Clk Divisor */
#define PLLSYS1_RW_MASK		0x00300000	/* ROM width */
#define PLLSYS1_EAR_MASK	0x00080000	/* ERAP Address reset vector */
#define PLLSYS1_PAE_MASK	0x00040000	/* PCI arbitor enable */
#define PLLSYS1_PCHE_MASK	0x00020000	/* PCI host config enable */
#define PLLSYS1_PISE_MASK	0x00010000	/* PCI init seq. enable */
#define PLLSYS1_PCWE_MASK	0x00008000	/* PCI local cpu wait enable */
#define PLLSYS1_PPIM_MASK	0x00007800	/* PCI inbound map */
#define PLLSYS1_PR64E_MASK	0x00000400	/* PCI init Req64 enable */
#define PLLSYS1_PXFS_MASK	0x00000300	/* PCI-X Freq Sel */
#define PLLSYS1_RSVD_MASK	0x00000080	/* RSVD */
#define PLLSYS1_PDM_MASK	0x00000040	/* PCI-X Driver Mode */
#define PLLSYS1_EPS_MASK	0x00000038	/* Ethernet Pin Select */
#define PLLSYS1_RMII_MASK	0x00000004	/* RMII Mode */
#define PLLSYS1_TRE_MASK	0x00000002	/* GPIO Trace Enable */
#define PLLSYS1_NTO1_MASK	0x00000001	/* CPU:PLB N-to-1 ratio */

#define PCIL0_BRDGOPT1		(PCIL0_CFGBASE + 0x0040)
#define PCIL0_BRDGOPT2		(PCIL0_CFGBASE + 0x0044)

#endif /* _PPC440SPE_H_ */
