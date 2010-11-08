/*----------------------------------------------------------------------------+
|   This source code is dual-licensed.  You may use it under the terms of the
|   GNU General Public License version 2, or under the license below.
|
|	This source code has been made available to you by IBM on an AS-IS
|	basis.	Anyone receiving this source is licensed under IBM
|	copyrights to use it in any way he or she deems fit, including
|	copying it, modifying it, compiling it, and redistributing it either
|	with or without modifications.	No license under IBM patents or
|	patent applications is to be implied by the copyright license.
|
|	Any user of this software should understand that IBM cannot provide
|	technical support for this software and will not be responsible for
|	any consequences resulting from the use of this software.
|
|	Any person who transfers this source code or any derivative work
|	must include the IBM copyright notice, this paragraph, and the
|	preceding two paragraphs in the transferred software.
|
|	COPYRIGHT   I B M   CORPORATION 1999
|	LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+----------------------------------------------------------------------------*/

/*
 * (C) Copyright 2006
 * Sylvie Gohl,             AMCC/IBM, gohl.sylvie@fr.ibm.com
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Thierry Roman,           AMCC/IBM, thierry_roman@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 * Robert Snyder,           AMCC/IBM, rob.snyder@fr.ibm.com
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

#ifndef __PPC440_H__
#define __PPC440_H__

#define CONFIG_SYS_DCACHE_SIZE		(32 << 10)	/* For AMCC 440 CPUs */

/******************************************************************************
 * DCRs & Related
 ******************************************************************************/

/*-----------------------------------------------------------------------------
 | Clocking Controller
 +----------------------------------------------------------------------------*/
/* values for clkcfga register - indirect addressing of these regs */
#define CPR0_PLLC	0x0040
#define CPR0_PLLD	0x0060
#define CPR0_PRIMAD0	0x0080
#define CPR0_PRIMBD0	0x00a0
#define CPR0_OPBD0	0x00c0
#define CPR0_PERD	0x00e0
#define CPR0_MALD	0x0100
#define CPR0_SPCID	0x0120
#define CPR0_ICFG	0x0140

/* 440EPX boot strap options */
#define BOOT_STRAP_OPTION_A	0x00000000
#define BOOT_STRAP_OPTION_B	0x00000001
#define BOOT_STRAP_OPTION_D	0x00000003
#define BOOT_STRAP_OPTION_E	0x00000004

/* 440gx sdr register definations */
#define SDR0_SDSTP0	0x0020	    /* */
#define SDR0_SDSTP1	0x0021	    /* */
#define SDR0_PINSTP	0x0040
#define SDR0_SDCS0	0x0060
#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_DDRCFG	0x00e0
#endif /* defined(CONFIG_440EPX) || defined(CONFIG_440GRX) */
#define SDR0_EBC	0x0100
#define SDR0_UART0	0x0120	/* UART0 Config */
#define SDR0_UART1	0x0121	/* UART1 Config */
#define SDR0_UART2	0x0122	/* UART2 Config */
#define SDR0_UART3	0x0123	/* UART3 Config */
#define SDR0_CP440	0x0180
#define SDR0_XCR	0x01c0
#define SDR0_XPLLC	0x01c1
#define SDR0_XPLLD	0x01c2
#define SDR0_SRST	0x0200
#define SD0_AMP0	0x0240 /* Override PLB4 prioritiy for up to 8 masters */
#define SD0_AMP1	0x0241 /* Override PLB3 prioritiy for up to 8 masters */
#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define SDR0_PCI0	0x01c0
#else
#define SDR0_PCI0	0x0300
#endif
#define SDR0_USB0	0x0320
#define SDR0_CUST0	0x4000
#define SDR0_CUST1	0x4002
#define SDR0_PFC0	0x4100	/* Pin Function 0 */
#define SDR0_PFC1	0x4101	/* Pin Function 1 */
#define SDR0_MFR	0x4300	/* SDR0_MFR reg */

#if defined(CONFIG_440GX)
#define SD0_AMP		0x0240
#define SDR0_XPLLC	0x01c1
#define SDR0_XPLLD	0x01c2
#define SDR0_XCR	0x01c0
#define SDR0_SDSTP2	0x4001
#define SDR0_SDSTP3	0x4003
#endif	/* CONFIG_440GX */

/*----------------------------------------------------------------------------+
| Core Configuration/MMU configuration for 440 (CCR1 for 440x5 only).
+----------------------------------------------------------------------------*/
#define CCR0_PRE		0x40000000
#define CCR0_CRPE		0x08000000
#define CCR0_DSTG		0x00200000
#define CCR0_DAPUIB		0x00100000
#define CCR0_DTB		0x00008000
#define CCR0_GICBT		0x00004000
#define CCR0_GDCBT		0x00002000
#define CCR0_FLSTA		0x00000100
#define CCR0_ICSLC_MASK		0x0000000C
#define CCR0_ICSLT_MASK		0x00000003
#define CCR1_TCS_MASK		0x00000080
#define CCR1_TCS_INTCLK		0x00000000
#define CCR1_TCS_EXTCLK		0x00000080
#define MMUCR_SWOA		0x01000000
#define MMUCR_U1TE		0x00400000
#define MMUCR_U2SWOAE		0x00200000
#define MMUCR_DULXE		0x00800000
#define MMUCR_IULXE		0x00400000
#define MMUCR_STS		0x00100000
#define MMUCR_STID_MASK		0x000000FF

#ifdef CONFIG_440SPE
#undef SDR0_SDSTP2
#define SDR0_SDSTP2	0x0022
#undef SDR0_SDSTP3
#define SDR0_SDSTP3	0x0023
#define SDR0_DDR0	0x00E1
#define SDR0_UART2	0x0122
#define SDR0_XCR0	0x01c0
#define SDR0_XCR1	0x01c3
#define SDR0_XCR2	0x01c6
#define SDR0_XPLLC0	0x01c1
#define SDR0_XPLLD0	0x01c2
#define SDR0_XPLLC1	0x01c4	/* notRCW  - SG */
#define SDR0_XPLLD1	0x01c5	/* notRCW  - SG */
#define SDR0_XPLLC2	0x01c7	/* notRCW  - SG */
#define SDR0_XPLLD2	0x01c8	/* dnotRCW  - SG */
#define SD0_AMP0	0x0240
#define SD0_AMP1	0x0241
#define SDR0_CUST2	0x4004
#define SDR0_CUST3	0x4006
#define SDR0_SDSTP4	0x4001
#define SDR0_SDSTP5	0x4003
#define SDR0_SDSTP6	0x4005
#define SDR0_SDSTP7	0x4007

#endif /* CONFIG_440SPE */

/*-----------------------------------------------------------------------------
 | External Bus Controller
 +----------------------------------------------------------------------------*/
/* values for EBC0_CFGADDR register - indirect addressing of these regs */
#define PB0CR		0x00	/* periph bank 0 config reg		*/
#define PB1CR		0x01	/* periph bank 1 config reg		*/
#define PB2CR		0x02	/* periph bank 2 config reg		*/
#define PB3CR		0x03	/* periph bank 3 config reg		*/
#define PB4CR		0x04	/* periph bank 4 config reg		*/
#define PB5CR		0x05	/* periph bank 5 config reg		*/
#define PB6CR		0x06	/* periph bank 6 config reg		*/
#define PB7CR		0x07	/* periph bank 7 config reg		*/
#define PB0AP		0x10	/* periph bank 0 access parameters	*/
#define PB1AP		0x11	/* periph bank 1 access parameters	*/
#define PB2AP		0x12	/* periph bank 2 access parameters	*/
#define PB3AP		0x13	/* periph bank 3 access parameters	*/
#define PB4AP		0x14	/* periph bank 4 access parameters	*/
#define PB5AP		0x15	/* periph bank 5 access parameters	*/
#define PB6AP		0x16	/* periph bank 6 access parameters	*/
#define PB7AP		0x17	/* periph bank 7 access parameters	*/
#define PBEAR		0x20	/* periph bus error addr reg		*/
#define PBESR		0x21	/* periph bus error status reg		*/
#define EBC0_CFG	0x23	/* external bus configuration reg	*/

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

	/* PLB3 Arbiter */
#define PLB3_DCR_BASE		0x070
#define PLB3_ACR		(PLB3_DCR_BASE + 0x7)

	/* PLB4 Arbiter - PowerPC440EP Pass1 */
#define PLB4_DCR_BASE		0x080
#define PLB4_ACR		(PLB4_DCR_BASE + 0x1)

#define PLB4_ACR_WRP		(0x80000000 >> 7)

	/* Pin Function Control Register 1 */
#define SDR0_PFC1                    0x4101
#define SDR0_PFC1_U1ME_MASK         0x02000000 /* UART1 Mode Enable */
#define SDR0_PFC1_U1ME_DSR_DTR      0x00000000 /* UART1 in DSR/DTR Mode */
#define SDR0_PFC1_U1ME_CTS_RTS      0x02000000 /* UART1 in CTS/RTS Mode */
#define SDR0_PFC1_U0ME_MASK         0x00080000 /* UART0 Mode Enable */
#define SDR0_PFC1_U0ME_DSR_DTR      0x00000000 /* UART0 in DSR/DTR Mode */
#define SDR0_PFC1_U0ME_CTS_RTS      0x00080000 /* UART0 in CTS/RTS Mode */
#define SDR0_PFC1_U0IM_MASK         0x00040000 /* UART0 Interface Mode */
#define SDR0_PFC1_U0IM_8PINS        0x00000000 /* UART0 Interface Mode 8 pins */
#define SDR0_PFC1_U0IM_4PINS        0x00040000 /* UART0 Interface Mode 4 pins */
#define SDR0_PFC1_SIS_MASK          0x00020000 /* SCP or IIC1 Selection */
#define SDR0_PFC1_SIS_SCP_SEL       0x00000000 /* SCP Selected */
#define SDR0_PFC1_SIS_IIC1_SEL      0x00020000 /* IIC1 Selected */
#define SDR0_PFC1_UES_MASK          0x00010000 /* USB2D_RX_Active / EBC_Hold
						  Req Selection */
#define SDR0_PFC1_UES_USB2D_SEL     0x00000000 /* USB2D_RX_Active Selected */
#define SDR0_PFC1_UES_EBCHR_SEL     0x00010000 /* EBC_Hold Req Selected */
#define SDR0_PFC1_DIS_MASK          0x00008000 /* DMA_Req(1) / UIC_IRQ(5)
						  Selection */
#define SDR0_PFC1_DIS_DMAR_SEL      0x00000000 /* DMA_Req(1) Selected */
#define SDR0_PFC1_DIS_UICIRQ5_SEL   0x00008000 /* UIC_IRQ(5) Selected */
#define SDR0_PFC1_ERE_MASK          0x00004000 /* EBC Mast.Ext.Req.En./GPIO0(27)
						  Selection */
#define SDR0_PFC1_ERE_EXTR_SEL      0x00000000 /* EBC Mast.Ext.Req.En.
						  Selected */
#define SDR0_PFC1_ERE_GPIO0_27_SEL  0x00004000 /* GPIO0(27) Selected */
#define SDR0_PFC1_UPR_MASK          0x00002000 /* USB2 Device Packet Reject
						  Selection */
#define SDR0_PFC1_UPR_DISABLE       0x00000000 /* USB2 Device Packet Reject
						  Disable */
#define SDR0_PFC1_UPR_ENABLE        0x00002000 /* USB2 Device Packet Reject
						  Enable */

#define SDR0_PFC1_PLB_PME_MASK      0x00001000 /* PLB3/PLB4 Perf. Monitor Enable
						  Selection */
#define SDR0_PFC1_PLB_PME_PLB3_SEL  0x00000000 /* PLB3 Performance Monitor
						  Enable */
#define SDR0_PFC1_PLB_PME_PLB4_SEL  0x00001000 /* PLB3 Performance Monitor
						  Enable */
#define SDR0_PFC1_GFGGI_MASK        0x0000000F /* GPT Frequency Generation
						  Gated In */

	/* USB Control Register */
#define SDR0_USB0                    0x0320
#define SDR0_USB0_USB_DEVSEL_MASK   0x00000002 /* USB Device Selection */
#define SDR0_USB0_USB20D_DEVSEL     0x00000000 /* USB2.0 Device Selected */
#define SDR0_USB0_USB11D_DEVSEL     0x00000002 /* USB1.1 Device Selected */
#define SDR0_USB0_LEEN_MASK         0x00000001 /* Little Endian selection */
#define SDR0_USB0_LEEN_DISABLE      0x00000000 /* Little Endian Disable */
#define SDR0_USB0_LEEN_ENABLE       0x00000001 /* Little Endian Enable */

	/* Miscealleneaous Function Reg. */
#define SDR0_MFR                     0x4300
#define SDR0_MFR_ETH0_CLK_SEL_MASK   0x08000000 /* Ethernet0 Clock Select */
#define SDR0_MFR_ETH0_CLK_SEL_EXT    0x00000000
#define SDR0_MFR_ETH1_CLK_SEL_MASK   0x04000000 /* Ethernet1 Clock Select */
#define SDR0_MFR_ETH1_CLK_SEL_EXT    0x00000000
#define SDR0_MFR_ZMII_MODE_MASK      0x03000000 /* ZMII Mode Mask */
#define SDR0_MFR_ZMII_MODE_MII       0x00000000 /* ZMII Mode MII */
#define SDR0_MFR_ZMII_MODE_SMII      0x01000000 /* ZMII Mode SMII */
#define SDR0_MFR_ZMII_MODE_RMII_10M  0x02000000 /* ZMII Mode RMII - 10 Mbs */
#define SDR0_MFR_ZMII_MODE_RMII_100M 0x03000000 /* ZMII Mode RMII - 100 Mbs */
#define SDR0_MFR_ZMII_MODE_BIT0      0x02000000 /* ZMII Mode Bit0 */
#define SDR0_MFR_ZMII_MODE_BIT1      0x01000000 /* ZMII Mode Bit1 */
#define SDR0_MFR_ZM_ENCODE(n)        ((((unsigned long)(n))&0x3)<<24)
#define SDR0_MFR_ZM_DECODE(n)        ((((unsigned long)(n))<<24)&0x3)

#define SDR0_MFR_ERRATA3_EN0	0x00800000
#define SDR0_MFR_ERRATA3_EN1	0x00400000
#define SDR0_MFR_PKT_REJ_MASK	0x00180000 /* Pkt Rej. Enable Mask */
#define SDR0_MFR_PKT_REJ_EN	0x00180000 /* Pkt Rej. Ena. on both EMAC3 0-1 */
#define SDR0_MFR_PKT_REJ_EN0	0x00100000 /* Pkt Rej. Enable on EMAC3(0) */
#define SDR0_MFR_PKT_REJ_EN1	0x00080000 /* Pkt Rej. Enable on EMAC3(1) */
#define SDR0_MFR_PKT_REJ_POL	0x00200000 /* Packet Reject Polarity */

#define GPT0_COMP6			0x00000098
#define GPT0_COMP5			0x00000094
#define GPT0_COMP4			0x00000090
#define GPT0_COMP3			0x0000008C
#define GPT0_COMP2			0x00000088
#define GPT0_COMP1			0x00000084

#define GPT0_MASK6			0x000000D8
#define GPT0_MASK5			0x000000D4
#define GPT0_MASK4			0x000000D0
#define GPT0_MASK3			0x000000CC
#define GPT0_MASK2			0x000000C8
#define GPT0_MASK1			0x000000C4

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_USB2D0CR                 0x0320
#define SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK   0x00000004 /* USB 2.0 Device/EBC
							   Master Selection */
#define SDR0_USB2D0CR_USB2DEV_SELECTION	0x00000004 /* USB 2.0 Device Selection*/
#define SDR0_USB2D0CR_EBC_SELECTION	0x00000000 /* EBC Selection */

#define SDR0_USB2D0CR_USB_DEV_INT_SEL_MASK   0x00000002 /* USB Device Interface
							   Selection */
#define SDR0_USB2D0CR_USB20D_DEVSEL	0x00000000 /* USB2.0 Device Selected */
#define SDR0_USB2D0CR_USB11D_DEVSEL	0x00000002 /* USB1.1 Device Selected */

#define SDR0_USB2D0CR_LEEN_MASK		0x00000001 /* Little Endian selection */
#define SDR0_USB2D0CR_LEEN_DISABLE	0x00000000 /* Little Endian Disable */
#define SDR0_USB2D0CR_LEEN_ENABLE	0x00000001 /* Little Endian Enable */

	/* USB2 Host Control Register */
#define SDR0_USB2H0CR			0x0340
#define SDR0_USB2H0CR_WDINT_MASK	0x00000001 /* Host UTMI Word Interface*/
#define SDR0_USB2H0CR_WDINT_8BIT_60MHZ	0x00000000 /* 8-bit/60MHz */
#define SDR0_USB2H0CR_WDINT_16BIT_30MHZ	0x00000001 /* 16-bit/30MHz */
#define SDR0_USB2H0CR_EFLADJ_MASK	0x0000007e /* EHCI Frame Length
						      Adjustment */

	/* Pin Function Control Register 1 */
#define SDR0_PFC1   	0x4101
#define SDR0_PFC1_U1ME_MASK 		0x02000000 /* UART1 Mode Enable */
#define SDR0_PFC1_U1ME_DSR_DTR		0x00000000 /* UART1 in DSR/DTR Mode */
#define SDR0_PFC1_U1ME_CTS_RTS		0x02000000 /* UART1 in CTS/RTS Mode */

#define SDR0_PFC1_SELECT_MASK		0x01C00000 /* Ethernet Pin Select
						      EMAC 0 */
#define SDR0_PFC1_SELECT_CONFIG_1_1	0x00C00000 /* 1xMII   using RGMII
						      bridge */
#define SDR0_PFC1_SELECT_CONFIG_1_2	0x00000000 /* 1xMII   using  ZMII
						      bridge */
#define SDR0_PFC1_SELECT_CONFIG_2	0x00C00000 /* 1xGMII  using RGMII
						      bridge */
#define SDR0_PFC1_SELECT_CONFIG_3	0x01000000 /* 1xTBI   using RGMII
						      bridge */
#define SDR0_PFC1_SELECT_CONFIG_4	0x01400000 /* 2xRGMII using RGMII
						      bridge */
#define SDR0_PFC1_SELECT_CONFIG_5	0x01800000 /* 2xRTBI  using RGMII
						      bridge */
#define SDR0_PFC1_SELECT_CONFIG_6	0x00800000 /* 2xSMII  using  ZMII
						      bridge */

#define SDR0_PFC1_U0ME_MASK 	0x00080000 /* UART0 Mode Enable */
#define SDR0_PFC1_U0ME_DSR_DTR	0x00000000 /* UART0 in DSR/DTR Mode */
#define SDR0_PFC1_U0ME_CTS_RTS	0x00080000 /* UART0 in CTS/RTS Mode */
#define SDR0_PFC1_U0IM_MASK 	0x00040000 /* UART0 Interface Mode */
#define SDR0_PFC1_U0IM_8PINS	0x00000000 /* UART0 Interface Mode 8 pins */
#define SDR0_PFC1_U0IM_4PINS	0x00040000 /* UART0 Interface Mode 4 pins */
#define SDR0_PFC1_SIS_MASK  	0x00020000 /* SCP or IIC1 Selection */
#define SDR0_PFC1_SIS_SCP_SEL	0x00000000 /* SCP Selected */
#define SDR0_PFC1_SIS_IIC1_SEL	0x00020000 /* IIC1 Selected */
#define SDR0_PFC1_UES_MASK  	0x00010000 /* USB2D_RX_Active / EBC_Hold Req
					      Selection */
#define SDR0_PFC1_UES_USB2D_SEL	0x00000000 /* USB2D_RX_Active Selected */
#define SDR0_PFC1_UES_EBCHR_SEL	0x00010000 /* EBC_Hold Req Selected */
#define SDR0_PFC1_DIS_MASK  	0x00008000 /* DMA_Req(1) / UIC_IRQ(5)
					      Selection */
#define SDR0_PFC1_DIS_DMAR_SEL	0x00000000 /* DMA_Req(1) Selected */
#define SDR0_PFC1_DIS_UICIRQ5_SEL	0x00008000 /* UIC_IRQ(5) Selected */
#define SDR0_PFC1_ERE_MASK  	0x00004000 /* EBC Mast.Ext.Req.En./GPIO0(27)
					      Selection */
#define SDR0_PFC1_ERE_EXTR_SEL	0x00000000 /* EBC Mast.Ext.Req.En. Selected */
#define SDR0_PFC1_ERE_GPIO0_27_SEL	0x00004000 /* GPIO0(27) Selected */
#define SDR0_PFC1_UPR_MASK  	0x00002000 /* USB2 Device Packet Reject
					      Selection */
#define SDR0_PFC1_UPR_DISABLE	0x00000000 /* USB2 Device Packet Reject
					      Disable */
#define SDR0_PFC1_UPR_ENABLE	0x00002000 /* USB2 Device Packet Reject
					      Enable */

#define SDR0_PFC1_PLB_PME_MASK	0x00001000
	/* PLB3/PLB4 Perf. Monitor En. Selection */
#define SDR0_PFC1_PLB_PME_PLB3_SEL	0x00000000
	/* PLB3 Performance Monitor Enable */
#define SDR0_PFC1_PLB_PME_PLB4_SEL	0x00001000
	/* PLB3 Performance Monitor Enable */
#define SDR0_PFC1_GFGGI_MASK	0x0000000F /* GPT Frequency Generation
					      Gated In */

	/* Ethernet PLL Configuration Register */
#define SDR0_PFC2   	0x4102
#define SDR0_PFC2_TUNE_MASK 	0x01FF8000 /* Loop stability tuning bits */
#define SDR0_PFC2_MULTI_MASK	0x00007C00 /* Frequency multiplication
					      selector */
#define SDR0_PFC2_RANGEB_MASK	0x00000380 /* PLLOUTB/C frequency selector */
#define SDR0_PFC2_RANGEA_MASK	0x00000071 /* PLLOUTA frequency selector */

#define SDR0_PFC2_SELECT_MASK	    0xE0000000 /* Ethernet Pin select EMAC1 */
#define SDR0_PFC2_SELECT_CONFIG_1_1 0x60000000 /* 1xMII   using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_1_2 0x00000000 /* 1xMII   using  ZMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_2   0x60000000 /* 1xGMII  using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_3   0x80000000 /* 1xTBI   using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_4   0xA0000000 /* 2xRGMII using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_5   0xC0000000 /* 2xRTBI  using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_6   0x40000000 /* 2xSMII  using  ZMII bridge */

#define SDR0_PFC4		0x4104

	/* USB2PHY0 Control Register */
#define SDR0_USB2PHY0CR	0x4103
#define SDR0_USB2PHY0CR_UTMICN_MASK	0x00100000

	/*  PHY UTMI interface connection */
#define SDR0_USB2PHY0CR_UTMICN_DEV	0x00000000 /* Device support */
#define SDR0_USB2PHY0CR_UTMICN_HOST	0x00100000 /* Host support */

#define SDR0_USB2PHY0CR_DWNSTR_MASK 0x00400000 /* Select downstream port mode */
#define SDR0_USB2PHY0CR_DWNSTR_DEV  0x00000000 /* Device */
#define SDR0_USB2PHY0CR_DWNSTR_HOST 0x00400000 /* Host   */

#define SDR0_USB2PHY0CR_DVBUS_MASK	0x00800000
	/* VBus detect (Device mode only)  */
#define SDR0_USB2PHY0CR_DVBUS_PURDIS	0x00000000
	/* Pull-up resistance on D+ is disabled */
#define SDR0_USB2PHY0CR_DVBUS_PUREN	0x00800000
	/* Pull-up resistance on D+ is enabled */

#define SDR0_USB2PHY0CR_WDINT_MASK	0x01000000
	/* PHY UTMI data width and clock select  */
#define SDR0_USB2PHY0CR_WDINT_8BIT_60MHZ 0x00000000 /* 8-bit data/60MHz */
#define SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ 0x01000000 /* 16-bit data/30MHz */

#define SDR0_USB2PHY0CR_LOOPEN_MASK	0x02000000 /* Loop back test enable  */
#define SDR0_USB2PHY0CR_LOOP_ENABLE	0x00000000 /* Loop back disabled */
#define SDR0_USB2PHY0CR_LOOP_DISABLE	0x02000000
	/* Loop back enabled (only test purposes) */

#define SDR0_USB2PHY0CR_XOON_MASK	0x04000000
	/* Force XO block on during a suspend  */
#define SDR0_USB2PHY0CR_XO_ON	0x00000000 /* PHY XO block is powered-on */
#define SDR0_USB2PHY0CR_XO_OFF	0x04000000
  /* PHY XO block is powered-off when all ports are suspended */

#define SDR0_USB2PHY0CR_PWRSAV_MASK 0x08000000 /* Select PHY power-save mode  */
#define SDR0_USB2PHY0CR_PWRSAV_OFF  0x00000000 /* Non-power-save mode */
#define SDR0_USB2PHY0CR_PWRSAV_ON   0x08000000 /* Power-save mode. Valid only
						  for full-speed operation */

#define SDR0_USB2PHY0CR_XOREF_MASK	0x10000000 /* Select reference clock
						      source  */
#define SDR0_USB2PHY0CR_XOREF_INTERNAL	0x00000000 /* PHY PLL uses chip internal
						  48M clock as a reference */
#define SDR0_USB2PHY0CR_XOREF_XO	0x10000000 /* PHY PLL uses internal XO
						  block output as a reference */

#define SDR0_USB2PHY0CR_XOCLK_MASK	0x20000000 /* Select clock for XO
						      block*/
#define SDR0_USB2PHY0CR_XOCLK_EXTERNAL	0x00000000 /* PHY macro used an external
						      clock */
#define SDR0_USB2PHY0CR_XOCLK_CRYSTAL	0x20000000 /* PHY macro uses the clock
						      from a crystal */

#define SDR0_USB2PHY0CR_CLKSEL_MASK	0xc0000000 /* Select ref clk freq */
#define SDR0_USB2PHY0CR_CLKSEL_12MHZ	0x00000000 /* Select ref clk freq
						      = 12 MHz */
#define SDR0_USB2PHY0CR_CLKSEL_48MHZ	0x40000000 /* Select ref clk freq
						      = 48 MHz */
#define SDR0_USB2PHY0CR_CLKSEL_24MHZ	0x80000000 /* Select ref clk freq
						      = 24 MHz */

	/* Miscealleneaous Function Reg. */
#define SDR0_MFR    	0x4300
#define SDR0_MFR_ETH0_CLK_SEL_MASK	0x08000000 /* Ethernet0 Clock Select */
#define SDR0_MFR_ETH0_CLK_SEL_EXT	0x00000000
#define SDR0_MFR_ETH1_CLK_SEL_MASK	0x04000000 /* Ethernet1 Clock Select */
#define SDR0_MFR_ETH1_CLK_SEL_EXT	0x00000000
#define SDR0_MFR_ZMII_MODE_MASK	0x03000000 /* ZMII Mode Mask */
#define SDR0_MFR_ZMII_MODE_MII	0x00000000 /* ZMII Mode MII */
#define SDR0_MFR_ZMII_MODE_SMII	0x01000000 /* ZMII Mode SMII */
#define SDR0_MFR_ZMII_MODE_BIT0	0x02000000 /* ZMII Mode Bit0 */
#define SDR0_MFR_ZMII_MODE_BIT1	0x01000000 /* ZMII Mode Bit1 */
#define SDR0_MFR_ZM_ENCODE(n)        ((((unsigned long)(n))&0x3)<<24)
#define SDR0_MFR_ZM_DECODE(n)        ((((unsigned long)(n))<<24)&0x3)

#define SDR0_MFR_ERRATA3_EN0	0x00800000
#define SDR0_MFR_ERRATA3_EN1	0x00400000
#define SDR0_MFR_PKT_REJ_MASK	0x00180000 /* Pkt Rej. Enable Mask */
#define SDR0_MFR_PKT_REJ_EN	0x00180000 /* Pkt Rej. Ena. on both EMAC3 0-1 */
#define SDR0_MFR_PKT_REJ_EN0	0x00100000 /* Pkt Rej. Enable on EMAC3(0) */
#define SDR0_MFR_PKT_REJ_EN1	0x00080000 /* Pkt Rej. Enable on EMAC3(1) */
#define SDR0_MFR_PKT_REJ_POL	0x00200000 /* Packet Reject Polarity */

#endif /* defined(CONFIG_440EPX) || defined(CONFIG_440GRX) */

	/* CUST1 Customer Configuration Register1 */
#define SDR0_CUST1	0x4002
#define SDR0_CUST1_NDRSC_MASK	0xFFFF0000 /* NDRSC Device Read Count */
#define SDR0_CUST1_NDRSC_ENCODE(n) ((((unsigned long)(n))&0xFFFF)<<16)
#define SDR0_CUST1_NDRSC_DECODE(n) ((((unsigned long)(n))>>16)&0xFFFF)

	/* Pin Function Control Register 0 */
#define SDR0_PFC0   	0x4100
#define SDR0_PFC0_CPU_TR_EN_MASK	0x00000100 /* CPU Trace Enable Mask */
#define SDR0_PFC0_CPU_TRACE_EN	0x00000100 /* CPU Trace Enable */
#define SDR0_PFC0_CPU_TRACE_DIS	0x00000100 /* CPU Trace Disable */
#define SDR0_PFC0_CTE_ENCODE(n)    ((((unsigned long)(n))&0x01)<<8)
#define SDR0_PFC0_CTE_DECODE(n)    ((((unsigned long)(n))>>8)&0x01)

	/* Pin Function Control Register 1 */
#define SDR0_PFC1   	0x4101
#define SDR0_PFC1_U1ME_MASK	0x02000000 /* UART1 Mode Enable */
#define SDR0_PFC1_U1ME_DSR_DTR	0x00000000 /* UART1 in DSR/DTR Mode */
#define SDR0_PFC1_U1ME_CTS_RTS	0x02000000 /* UART1 in CTS/RTS Mode */
#define SDR0_PFC1_U0ME_MASK	0x00080000 /* UART0 Mode Enable */
#define SDR0_PFC1_U0ME_DSR_DTR	0x00000000 /* UART0 in DSR/DTR Mode */
#define SDR0_PFC1_U0ME_CTS_RTS	0x00080000 /* UART0 in CTS/RTS Mode */
#define SDR0_PFC1_U0IM_MASK	0x00040000 /* UART0 Interface Mode */
#define SDR0_PFC1_U0IM_8PINS	0x00000000 /* UART0 Interface Mode 8 pins */
#define SDR0_PFC1_U0IM_4PINS	0x00040000 /* UART0 Interface Mode 4 pins */
#define SDR0_PFC1_SIS_MASK	0x00020000 /* SCP or IIC1 Selection */
#define SDR0_PFC1_SIS_SCP_SEL	0x00000000 /* SCP Selected */
#define SDR0_PFC1_SIS_IIC1_SEL	0x00020000 /* IIC1 Selected */
#define SDR0_PFC1_UES_MASK	0x00010000 /* USB2D_RX_Active / EBC_Hold Req
					      Selection */
#define SDR0_PFC1_UES_USB2D_SEL	0x00000000 /* USB2D_RX_Active Selected */
#define SDR0_PFC1_UES_EBCHR_SEL	0x00010000 /* EBC_Hold Req Selected */
#define SDR0_PFC1_DIS_MASK	0x00008000 /* DMA_Req(1) / UIC_IRQ(5)
					      Selection */
#define SDR0_PFC1_DIS_DMAR_SEL	0x00000000 /* DMA_Req(1) Selected */
#define SDR0_PFC1_DIS_UICIRQ5_SEL	0x00008000 /* UIC_IRQ(5) Selected */
#define SDR0_PFC1_ERE_MASK	0x00004000 /* EBC Mast.Ext.Req.En./GPIO0(27)
					      Selection */
#define SDR0_PFC1_ERE_EXTR_SEL	0x00000000 /* EBC Mast.Ext.Req.En. Selected */
#define SDR0_PFC1_ERE_GPIO0_27_SEL	0x00004000 /* GPIO0(27) Selected */
#define SDR0_PFC1_UPR_MASK	0x00002000 /* USB2 Device Packet Reject
					      Selection */
#define SDR0_PFC1_UPR_DISABLE	0x00000000 /* USB2 Device Packet Reject
					      Disable */
#define SDR0_PFC1_UPR_ENABLE	0x00002000 /* USB2 Device Packet Reject
					      Enable */

#define SDR0_PFC1_PLB_PME_MASK	0x00001000 /* PLB3/PLB4 Perf. Monitor En.
					      Selection */
#define SDR0_PFC1_PLB_PME_PLB3_SEL	0x00000000 /* PLB3 Performance Monitor
					      Enable */
#define SDR0_PFC1_PLB_PME_PLB4_SEL	0x00001000 /* PLB3 Performance Monitor
					       Enable */
#define SDR0_PFC1_GFGGI_MASK	0x0000000F /* GPT Frequency Generation
					       Gated In */

#endif /* 440EP || 440GR || 440EPX || 440GRX */

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
	/* CUST0 Customer Configuration Register0 */
#define SDR0_CUST0  	0x4000
#define SDR0_CUST0_MUX_E_N_G_MASK	0xC0000000 /* Mux_Emac_NDFC_GPIO */
#define SDR0_CUST0_MUX_EMAC_SEL	0x40000000 /* Emac Selection */
#define SDR0_CUST0_MUX_NDFC_SEL	0x80000000 /* NDFC Selection */
#define SDR0_CUST0_MUX_GPIO_SEL	0xC0000000 /* GPIO Selection */

#define SDR0_CUST0_NDFC_EN_MASK	0x20000000 /* NDFC Enable Mask */
#define SDR0_CUST0_NDFC_ENABLE	0x20000000 /* NDFC Enable */
#define SDR0_CUST0_NDFC_DISABLE	0x00000000 /* NDFC Disable */

#define SDR0_CUST0_NDFC_BW_MASK	  0x10000000 /* NDFC Boot Width */
#define SDR0_CUST0_NDFC_BW_16_BIT 0x10000000 /* NDFC Boot Width = 16 Bit */
#define SDR0_CUST0_NDFC_BW_8_BIT  0x00000000 /* NDFC Boot Width =  8 Bit */

#define SDR0_CUST0_NDFC_BP_MASK	0x0F000000 /* NDFC Boot Page */
#define SDR0_CUST0_NDFC_BP_ENCODE(n) ((((unsigned long)(n))&0xF)<<24)
#define SDR0_CUST0_NDFC_BP_DECODE(n) ((((unsigned long)(n))>>24)&0x0F)

#define SDR0_CUST0_NDFC_BAC_MASK	0x00C00000 /* NDFC Boot Address Cycle */
#define SDR0_CUST0_NDFC_BAC_ENCODE(n) ((((unsigned long)(n))&0x3)<<22)
#define SDR0_CUST0_NDFC_BAC_DECODE(n) ((((unsigned long)(n))>>22)&0x03)

#define SDR0_CUST0_NDFC_ARE_MASK	0x00200000 /* NDFC Auto Read Enable */
#define SDR0_CUST0_NDFC_ARE_ENABLE	0x00200000 /* NDFC Auto Read Enable */
#define SDR0_CUST0_NDFC_ARE_DISABLE	0x00000000 /* NDFC Auto Read Disable */

#define SDR0_CUST0_NRB_MASK	0x00100000 /* NDFC Ready / Busy */
#define SDR0_CUST0_NRB_BUSY	0x00100000 /* Busy */
#define SDR0_CUST0_NRB_READY	0x00000000 /* Ready */

#define SDR0_CUST0_NDRSC_MASK	0x0000FFF0 /* NDFC Device Reset Count Mask */
#define SDR0_CUST0_NDRSC_ENCODE(n) ((((unsigned long)(n))&0xFFF)<<4)
#define SDR0_CUST0_NDRSC_DECODE(n) ((((unsigned long)(n))>>4)&0xFFF)

#define SDR0_CUST0_CHIPSELGAT_MASK  0x0000000F /* Chip Select Gating Mask */
#define SDR0_CUST0_CHIPSELGAT_DIS   0x00000000 /* Chip Select Gating Disable */
#define SDR0_CUST0_CHIPSELGAT_ENALL 0x0000000F /*All Chip Select Gating Enable*/
#define SDR0_CUST0_CHIPSELGAT_EN0   0x00000008 /* Chip Select0 Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN1   0x00000004 /* Chip Select1 Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN2   0x00000002 /* Chip Select2 Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN3   0x00000001 /* Chip Select3 Gating Enable */
#endif

/*-----------------------------------------------------------------------------
 | On-Chip Buses
 +----------------------------------------------------------------------------*/
/* TODO: as needed */

/*-----------------------------------------------------------------------------
 | Clocking, Power Management and Chip Control
 +----------------------------------------------------------------------------*/
#if defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_460SX)
#define CNTRL_DCR_BASE 0x160
#else
#define CNTRL_DCR_BASE 0x0b0
#endif

#define CPC0_SYS0	(CNTRL_DCR_BASE+0x30)	/* System configuration reg 0 */
#define CPC0_SYS1	(CNTRL_DCR_BASE+0x31)	/* System configuration reg 1 */

#define CPC0_STRP0	(CNTRL_DCR_BASE+0x34)	/* Power-on config reg 0 (RO) */
#define CPC0_STRP1	(CNTRL_DCR_BASE+0x35)	/* Power-on config reg 1 (RO) */

#define CPC0_GPIO	(CNTRL_DCR_BASE+0x38)	/* GPIO config reg (440GP) */

#define CPC0_CR0		(CNTRL_DCR_BASE+0x3b)	/* Control 0 register */
#define CPC0_CR1		(CNTRL_DCR_BASE+0x3a)	/* Control 1 register */

/*-----------------------------------------------------------------------------
 | DMA
 +----------------------------------------------------------------------------*/
#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define DMA_DCR_BASE 0x200
#else
#define DMA_DCR_BASE 0x100
#endif
#define DMACR0	(DMA_DCR_BASE+0x00)  /* DMA channel control register 0	     */
#define DMACT0	(DMA_DCR_BASE+0x01)  /* DMA count register 0		     */
#define DMACR1	(DMA_DCR_BASE+0x08)  /* DMA channel control register 1	     */
#define DMACT1	(DMA_DCR_BASE+0x09)  /* DMA count register 1		     */
#define DMACR2	(DMA_DCR_BASE+0x10)  /* DMA channel control register 2	     */
#define DMACT2	(DMA_DCR_BASE+0x11)  /* DMA count register 2		     */
#define DMACR3	(DMA_DCR_BASE+0x18)  /* DMA channel control register 2	     */
#define DMASR	(DMA_DCR_BASE+0x20)  /* DMA status register		     */
#define DMASGC	(DMA_DCR_BASE+0x23)  /* DMA scatter/gather command register  */

/*-----------------------------------------------------------------------------
 | Memory Access Layer
 +----------------------------------------------------------------------------*/
#define MAL_DCR_BASE 0x180
#define MAL0_CFG	(MAL_DCR_BASE + 0x00)	/* MAL Config reg	*/
#define MAL0_ESR	(MAL_DCR_BASE + 0x01)	/* Error Status (Read/Clear) */
#define MAL0_IER	(MAL_DCR_BASE + 0x02)	/* Interrupt enable */
#define MAL0_TXCASR	(MAL_DCR_BASE + 0x04)	/* TX Channel active (set) */
#define MAL0_TXCARR	(MAL_DCR_BASE + 0x05)	/* TX Channel active (reset) */
#define MAL0_TXEOBISR	(MAL_DCR_BASE + 0x06)	/* TX End of buffer int status*/
#define MAL0_TXDEIR	(MAL_DCR_BASE + 0x07)	/* TX Descr. Error Int */
#define MAL0_TXBADDR	(MAL_DCR_BASE + 0x09)	/* TX descriptor base addr*/
#define MAL0_RXCASR	(MAL_DCR_BASE + 0x10)	/* RX Channel active (set) */
#define MAL0_RXCARR	(MAL_DCR_BASE + 0x11)	/* RX Channel active (reset) */
#define MAL0_RXEOBISR	(MAL_DCR_BASE + 0x12)	/* RX End of buffer int status*/
#define MAL0_RXDEIR	(MAL_DCR_BASE + 0x13)	/* RX Descr. Error Int */
#define MAL0_RXBADDR	(MAL_DCR_BASE + 0x15)	/* RX descriptor base addr */
#define MAL0_TXCTP0R	(MAL_DCR_BASE + 0x20)	/* TX 0 Channel table pointer */
#define MAL0_TXCTP1R	(MAL_DCR_BASE + 0x21)	/* TX 1 Channel table pointer */
#define MAL0_TXCTP2R	(MAL_DCR_BASE + 0x22)	/* TX 2 Channel table pointer */
#define MAL0_TXCTP3R	(MAL_DCR_BASE + 0x23)	/* TX 3 Channel table pointer */
#define MAL0_RXCTP0R	(MAL_DCR_BASE + 0x40)	/* RX 0 Channel table pointer */
#define MAL0_RXCTP1R	(MAL_DCR_BASE + 0x41)	/* RX 1 Channel table pointer */
#define MAL0_RCBS0	(MAL_DCR_BASE + 0x60)	/* RX 0 Channel buffer size */
#define MAL0_RCBS1	(MAL_DCR_BASE + 0x61)	/* RX 1 Channel buffer size */
#if defined(CONFIG_440GX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define MAL0_RXCTP2R	(MAL_DCR_BASE + 0x42)	/* RX 2 Channel table pointer */
#define MAL0_RXCTP3R	(MAL_DCR_BASE + 0x43)	/* RX 3 Channel table pointer */
#define MAL0_RXCTP8R	(MAL_DCR_BASE + 0x48)	/* RX 8 Channel table pointer */
#define MAL0_RXCTP16R	(MAL_DCR_BASE + 0x50)	/* RX 16 Channel table pointer*/
#define MAL0_RXCTP24R	(MAL_DCR_BASE + 0x58)	/* RX 24 Channel table pointer*/
#define MAL0_RCBS2	(MAL_DCR_BASE + 0x62)	/* RX 2 Channel buffer size */
#define MAL0_RCBS3	(MAL_DCR_BASE + 0x63)	/* RX 3 Channel buffer size */
#define MAL0_RCBS8	(MAL_DCR_BASE + 0x68)	/* RX 8 Channel buffer size */
#define MAL0_RCBS16	(MAL_DCR_BASE + 0x70)	/* RX 16 Channel buffer size */
#define MAL0_RCBS24	(MAL_DCR_BASE + 0x78)	/* RX 24 Channel buffer size */
#endif /* CONFIG_440GX */

/*-----------------------------------------------------------------------------+
|  SDR0 Bit Settings
+-----------------------------------------------------------------------------*/
#if defined(CONFIG_440SP)
#define SDR0_DDR0			0x00E1
#define SDR0_DDR0_DPLLRST		0x80000000
#define SDR0_DDR0_DDRM_MASK		0x60000000
#define SDR0_DDR0_DDRM_DDR1		0x20000000
#define SDR0_DDR0_DDRM_DDR2		0x40000000
#define SDR0_DDR0_DDRM_ENCODE(n)	((((unsigned long)(n))&0x03)<<29)
#define SDR0_DDR0_DDRM_DECODE(n)	((((unsigned long)(n))>>29)&0x03)
#define SDR0_DDR0_TUNE_ENCODE(n)	((((unsigned long)(n))&0x2FF)<<0)
#define SDR0_DDR0_TUNE_DECODE(n)	((((unsigned long)(n))>>0)&0x2FF)
#endif

#if defined(CONFIG_440SPE) || defined(CONFIG_460SX)
#define SDR0_CP440			0x0180
#define SDR0_CP440_ERPN_MASK		0x30000000
#define SDR0_CP440_ERPN_MASK_HI		0x3000
#define SDR0_CP440_ERPN_MASK_LO		0x0000
#define SDR0_CP440_ERPN_EBC		0x10000000
#define SDR0_CP440_ERPN_EBC_HI		0x1000
#define SDR0_CP440_ERPN_EBC_LO		0x0000
#define SDR0_CP440_ERPN_PCI		0x20000000
#define SDR0_CP440_ERPN_PCI_HI		0x2000
#define SDR0_CP440_ERPN_PCI_LO		0x0000
#define SDR0_CP440_ERPN_ENCODE(n)	((((unsigned long)(n))&0x03)<<28)
#define SDR0_CP440_ERPN_DECODE(n)	((((unsigned long)(n))>>28)&0x03)
#define SDR0_CP440_NTO1_MASK		0x00000002
#define SDR0_CP440_NTO1_NTOP		0x00000000
#define SDR0_CP440_NTO1_NTO1		0x00000002
#define SDR0_CP440_NTO1_ENCODE(n)	((((unsigned long)(n))&0x01)<<1)
#define SDR0_CP440_NTO1_DECODE(n)	((((unsigned long)(n))>>1)&0x01)

#define SDR0_SDSTP0			0x0020
#define SDR0_SDSTP0_ENG_MASK		0x80000000
#define SDR0_SDSTP0_ENG_PLLDIS		0x00000000
#define SDR0_SDSTP0_ENG_PLLENAB		0x80000000
#define SDR0_SDSTP0_ENG_ENCODE(n)	((((unsigned long)(n))&0x01)<<31)
#define SDR0_SDSTP0_ENG_DECODE(n)	((((unsigned long)(n))>>31)&0x01)
#define SDR0_SDSTP0_SRC_MASK		0x40000000
#define SDR0_SDSTP0_SRC_PLLOUTA		0x00000000
#define SDR0_SDSTP0_SRC_PLLOUTB		0x40000000
#define SDR0_SDSTP0_SRC_ENCODE(n)	((((unsigned long)(n))&0x01)<<30)
#define SDR0_SDSTP0_SRC_DECODE(n)	((((unsigned long)(n))>>30)&0x01)
#define SDR0_SDSTP0_SEL_MASK		0x38000000
#define SDR0_SDSTP0_SEL_PLLOUT		0x00000000
#define SDR0_SDSTP0_SEL_CPU		0x08000000
#define SDR0_SDSTP0_SEL_EBC		0x28000000
#define SDR0_SDSTP0_SEL_ENCODE(n)	((((unsigned long)(n))&0x07)<<27)
#define SDR0_SDSTP0_SEL_DECODE(n)	((((unsigned long)(n))>>27)&0x07)
#define SDR0_SDSTP0_TUNE_MASK		0x07FE0000
#define SDR0_SDSTP0_TUNE_ENCODE(n)	((((unsigned long)(n))&0x3FF)<<17)
#define SDR0_SDSTP0_TUNE_DECODE(n)	((((unsigned long)(n))>>17)&0x3FF)
#define SDR0_SDSTP0_FBDV_MASK		0x0001F000
#define SDR0_SDSTP0_FBDV_ENCODE(n)	((((unsigned long)(n))&0x1F)<<12)
#define SDR0_SDSTP0_FBDV_DECODE(n) ((((((unsigned long)(n))>>12)-1)&0x1F)+1)
#define SDR0_SDSTP0_FWDVA_MASK		0x00000F00
#define SDR0_SDSTP0_FWDVA_ENCODE(n)	((((unsigned long)(n))&0x0F)<<8)
#define SDR0_SDSTP0_FWDVA_DECODE(n)	((((((unsigned long)(n))>>8)-1)&0x0F)+1)
#define SDR0_SDSTP0_FWDVB_MASK		0x000000E0
#define SDR0_SDSTP0_FWDVB_ENCODE(n)	((((unsigned long)(n))&0x07)<<5)
#define SDR0_SDSTP0_FWDVB_DECODE(n)	((((((unsigned long)(n))>>5)-1)&0x07)+1)
#define SDR0_SDSTP0_PRBDV0_MASK		0x0000001C
#define SDR0_SDSTP0_PRBDV0_ENCODE(n)	((((unsigned long)(n))&0x07)<<2)
#define SDR0_SDSTP0_PRBDV0_DECODE(n)	((((((unsigned long)(n))>>2)-1)&0x07)+1)
#define SDR0_SDSTP0_OPBDV0_MASK		0x00000003
#define SDR0_SDSTP0_OPBDV0_ENCODE(n)	((((unsigned long)(n))&0x03)<<0)
#define SDR0_SDSTP0_OPBDV0_DECODE(n)	((((((unsigned long)(n))>>0)-1)&0x03)+1)


#define SDR0_SDSTP1			0x0021
#define SDR0_SDSTP1_LFBDV_MASK		0xFC000000
#define SDR0_SDSTP1_LFBDV_ENCODE(n)	((((unsigned long)(n))&0x3F)<<26)
#define SDR0_SDSTP1_LFBDV_DECODE(n)	((((unsigned long)(n))>>26)&0x3F)
#define SDR0_SDSTP1_PERDV0_MASK		0x03000000
#define SDR0_SDSTP1_PERDV0_ENCODE(n)	((((unsigned long)(n))&0x03)<<24)
#define SDR0_SDSTP1_PERDV0_DECODE(n)	((((unsigned long)(n))>>24)&0x03)
#define SDR0_SDSTP1_MALDV0_MASK		0x00C00000
#define SDR0_SDSTP1_MALDV0_ENCODE(n)	((((unsigned long)(n))&0x03)<<22)
#define SDR0_SDSTP1_MALDV0_DECODE(n)	((((unsigned long)(n))>>22)&0x03)
#define SDR0_SDSTP1_DDR_MODE_MASK	0x00300000
#define SDR0_SDSTP1_DDR1_MODE		0x00100000
#define SDR0_SDSTP1_DDR2_MODE		0x00200000
#define SDR0_SDSTP1_DDR_ENCODE(n)	((((unsigned long)(n))&0x03)<<20)
#define SDR0_SDSTP1_DDR_DECODE(n)	((((unsigned long)(n))>>20)&0x03)
#define SDR0_SDSTP1_ERPN_MASK		0x00080000
#define SDR0_SDSTP1_ERPN_EBC		0x00000000
#define SDR0_SDSTP1_ERPN_PCI		0x00080000
#define SDR0_SDSTP1_PAE_MASK		0x00040000
#define SDR0_SDSTP1_PAE_DISABLE		0x00000000
#define SDR0_SDSTP1_PAE_ENABLE		0x00040000
#define SDR0_SDSTP1_PAE_ENCODE(n)	((((unsigned long)(n))&0x01)<<18)
#define SDR0_SDSTP1_PAE_DECODE(n)	((((unsigned long)(n))>>18)&0x01)
#define SDR0_SDSTP1_PHCE_MASK		0x00020000
#define SDR0_SDSTP1_PHCE_DISABLE	0x00000000
#define SDR0_SDSTP1_PHCE_ENABLE		0x00020000
#define SDR0_SDSTP1_PHCE_ENCODE(n)	((((unsigned long)(n))&0x01)<<17)
#define SDR0_SDSTP1_PHCE_DECODE(n)	((((unsigned long)(n))>>17)&0x01)
#define SDR0_SDSTP1_PISE_MASK		0x00010000
#define SDR0_SDSTP1_PISE_DISABLE	0x00000000
#define SDR0_SDSTP1_PISE_ENABLE		0x00001000
#define SDR0_SDSTP1_PISE_ENCODE(n)	((((unsigned long)(n))&0x01)<<16)
#define SDR0_SDSTP1_PISE_DECODE(n)	((((unsigned long)(n))>>16)&0x01)
#define SDR0_SDSTP1_PCWE_MASK		0x00008000
#define SDR0_SDSTP1_PCWE_DISABLE	0x00000000
#define SDR0_SDSTP1_PCWE_ENABLE		0x00008000
#define SDR0_SDSTP1_PCWE_ENCODE(n)	((((unsigned long)(n))&0x01)<<15)
#define SDR0_SDSTP1_PCWE_DECODE(n)	((((unsigned long)(n))>>15)&0x01)
#define SDR0_SDSTP1_PPIM_MASK		0x00007800
#define SDR0_SDSTP1_PPIM_ENCODE(n)	((((unsigned long)(n))&0x0F)<<11)
#define SDR0_SDSTP1_PPIM_DECODE(n)	((((unsigned long)(n))>>11)&0x0F)
#define SDR0_SDSTP1_PR64E_MASK		0x00000400
#define SDR0_SDSTP1_PR64E_DISABLE	0x00000000
#define SDR0_SDSTP1_PR64E_ENABLE	0x00000400
#define SDR0_SDSTP1_PR64E_ENCODE(n)	((((unsigned long)(n))&0x01)<<10)
#define SDR0_SDSTP1_PR64E_DECODE(n)	((((unsigned long)(n))>>10)&0x01)
#define SDR0_SDSTP1_PXFS_MASK		0x00000300
#define SDR0_SDSTP1_PXFS_100_133	0x00000000
#define SDR0_SDSTP1_PXFS_66_100		0x00000100
#define SDR0_SDSTP1_PXFS_50_66		0x00000200
#define SDR0_SDSTP1_PXFS_0_50		0x00000300
#define SDR0_SDSTP1_PXFS_ENCODE(n)	((((unsigned long)(n))&0x03)<<8)
#define SDR0_SDSTP1_PXFS_DECODE(n)	((((unsigned long)(n))>>8)&0x03)
#define SDR0_SDSTP1_EBCW_MASK		0x00000080 /* SOP */
#define SDR0_SDSTP1_EBCW_8_BITS		0x00000000 /* SOP */
#define SDR0_SDSTP1_EBCW_16_BITS	0x00000080 /* SOP */
#define SDR0_SDSTP1_DBGEN_MASK		0x00000030 /* $218C */
#define SDR0_SDSTP1_DBGEN_FUNC		0x00000000
#define SDR0_SDSTP1_DBGEN_TRACE		0x00000010
#define SDR0_SDSTP1_DBGEN_ENCODE(n) ((((unsigned long)(n))&0x03)<<4) /* $218C */
#define SDR0_SDSTP1_DBGEN_DECODE(n) ((((unsigned long)(n))>>4)&0x03) /* $218C */
#define SDR0_SDSTP1_ETH_MASK		0x00000004
#define SDR0_SDSTP1_ETH_10_100		0x00000000
#define SDR0_SDSTP1_ETH_GIGA		0x00000004
#define SDR0_SDSTP1_ETH_ENCODE(n)	((((unsigned long)(n))&0x01)<<2)
#define SDR0_SDSTP1_ETH_DECODE(n)	((((unsigned long)(n))>>2)&0x01)
#define SDR0_SDSTP1_NTO1_MASK		0x00000001
#define SDR0_SDSTP1_NTO1_DISABLE	0x00000000
#define SDR0_SDSTP1_NTO1_ENABLE		0x00000001
#define SDR0_SDSTP1_NTO1_ENCODE(n)	((((unsigned long)(n))&0x01)<<0)
#define SDR0_SDSTP1_NTO1_DECODE(n)	((((unsigned long)(n))>>0)&0x01)

#define SDR0_SDSTP2			0x0022
#define SDR0_SDSTP2_P1AE_MASK		0x80000000
#define SDR0_SDSTP2_P1AE_DISABLE	0x00000000
#define SDR0_SDSTP2_P1AE_ENABLE		0x80000000
#define SDR0_SDSTP2_P1AE_ENCODE(n)	((((unsigned long)(n))&0x01)<<31)
#define SDR0_SDSTP2_P1AE_DECODE(n)	((((unsigned long)(n))>>31)&0x01)
#define SDR0_SDSTP2_P1HCE_MASK		0x40000000
#define SDR0_SDSTP2_P1HCE_DISABLE	0x00000000
#define SDR0_SDSTP2_P1HCE_ENABLE	0x40000000
#define SDR0_SDSTP2_P1HCE_ENCODE(n)	((((unsigned long)(n))&0x01)<<30)
#define SDR0_SDSTP2_P1HCE_DECODE(n)	((((unsigned long)(n))>>30)&0x01)
#define SDR0_SDSTP2_P1ISE_MASK		0x20000000
#define SDR0_SDSTP2_P1ISE_DISABLE	0x00000000
#define SDR0_SDSTP2_P1ISE_ENABLE	0x20000000
#define SDR0_SDSTP2_P1ISE_ENCODE(n)	((((unsigned long)(n))&0x01)<<29)
#define SDR0_SDSTP2_P1ISE_DECODE(n)	((((unsigned long)(n))>>29)&0x01)
#define SDR0_SDSTP2_P1CWE_MASK		0x10000000
#define SDR0_SDSTP2_P1CWE_DISABLE	0x00000000
#define SDR0_SDSTP2_P1CWE_ENABLE	0x10000000
#define SDR0_SDSTP2_P1CWE_ENCODE(n)	((((unsigned long)(n))&0x01)<<28)
#define SDR0_SDSTP2_P1CWE_DECODE(n)	((((unsigned long)(n))>>28)&0x01)
#define SDR0_SDSTP2_P1PIM_MASK		0x0F000000
#define SDR0_SDSTP2_P1PIM_ENCODE(n)	((((unsigned long)(n))&0x0F)<<24)
#define SDR0_SDSTP2_P1PIM_DECODE(n)	((((unsigned long)(n))>>24)&0x0F)
#define SDR0_SDSTP2_P1R64E_MASK		0x00800000
#define SDR0_SDSTP2_P1R64E_DISABLE	0x00000000
#define SDR0_SDSTP2_P1R64E_ENABLE	0x00800000
#define SDR0_SDSTP2_P1R64E_ENCODE(n)	((((unsigned long)(n))&0x01)<<23)
#define SDR0_SDSTP2_P1R64E_DECODE(n)	((((unsigned long)(n))>>23)&0x01)
#define SDR0_SDSTP2_P1XFS_MASK		0x00600000
#define SDR0_SDSTP2_P1XFS_100_133	0x00000000
#define SDR0_SDSTP2_P1XFS_66_100	0x00200000
#define SDR0_SDSTP2_P1XFS_50_66		0x00400000
#define SDR0_SDSTP2_P1XFS_0_50		0x00600000
#define SDR0_SDSTP2_P1XFS_ENCODE(n)	((((unsigned long)(n))&0x03)<<21)
#define SDR0_SDSTP2_P1XFS_DECODE(n)	((((unsigned long)(n))>>21)&0x03)
#define SDR0_SDSTP2_P2AE_MASK		0x00040000
#define SDR0_SDSTP2_P2AE_DISABLE	0x00000000
#define SDR0_SDSTP2_P2AE_ENABLE		0x00040000
#define SDR0_SDSTP2_P2AE_ENCODE(n)	((((unsigned long)(n))&0x01)<<18)
#define SDR0_SDSTP2_P2AE_DECODE(n)	((((unsigned long)(n))>>18)&0x01)
#define SDR0_SDSTP2_P2HCE_MASK		0x00020000
#define SDR0_SDSTP2_P2HCE_DISABLE	0x00000000
#define SDR0_SDSTP2_P2HCE_ENABLE	0x00020000
#define SDR0_SDSTP2_P2HCE_ENCODE(n)	((((unsigned long)(n))&0x01)<<17)
#define SDR0_SDSTP2_P2HCE_DECODE(n)	((((unsigned long)(n))>>17)&0x01)
#define SDR0_SDSTP2_P2ISE_MASK		0x00010000
#define SDR0_SDSTP2_P2ISE_DISABLE	0x00000000
#define SDR0_SDSTP2_P2ISE_ENABLE	0x00010000
#define SDR0_SDSTP2_P2ISE_ENCODE(n)	((((unsigned long)(n))&0x01)<<16)
#define SDR0_SDSTP2_P2ISE_DECODE(n)	((((unsigned long)(n))>>16)&0x01)
#define SDR0_SDSTP2_P2CWE_MASK		0x00008000
#define SDR0_SDSTP2_P2CWE_DISABLE	0x00000000
#define SDR0_SDSTP2_P2CWE_ENABLE	0x00008000
#define SDR0_SDSTP2_P2CWE_ENCODE(n)	((((unsigned long)(n))&0x01)<<15)
#define SDR0_SDSTP2_P2CWE_DECODE(n)	((((unsigned long)(n))>>15)&0x01)
#define SDR0_SDSTP2_P2PIM_MASK		0x00007800
#define SDR0_SDSTP2_P2PIM_ENCODE(n)	((((unsigned long)(n))&0x0F)<<11)
#define SDR0_SDSTP2_P2PIM_DECODE(n)	((((unsigned long)(n))>>11)&0x0F)
#define SDR0_SDSTP2_P2XFS_MASK		0x00000300
#define SDR0_SDSTP2_P2XFS_100_133	0x00000000
#define SDR0_SDSTP2_P2XFS_66_100	0x00000100
#define SDR0_SDSTP2_P2XFS_50_66		0x00000200
#define SDR0_SDSTP2_P2XFS_0_50		0x00000100
#define SDR0_SDSTP2_P2XFS_ENCODE(n)	((((unsigned long)(n))&0x03)<<8)
#define SDR0_SDSTP2_P2XFS_DECODE(n)	((((unsigned long)(n))>>8)&0x03)

#define SDR0_SDSTP3			0x0023

#define SDR0_PINSTP			0x0040
#define SDR0_PINSTP_BOOTSTRAP_MASK	0xC0000000  /* Strap Bits */
#define SDR0_PINSTP_BOOTSTRAP_SETTINGS0	0x00000000  /* Default strap settings 0
							(EBC boot) */
#define SDR0_PINSTP_BOOTSTRAP_SETTINGS1	0x40000000  /* Default strap settings 1
							(PCI boot) */
#define SDR0_PINSTP_BOOTSTRAP_IIC_54_EN	0x80000000  /* Serial Device Enabled -
							Addr = 0x54 */
#define SDR0_PINSTP_BOOTSTRAP_IIC_50_EN	0xC0000000  /* Serial Device Enabled -
							Addr = 0x50 */
#define SDR0_SDCS			0x0060
#define SDR0_ECID0			0x0080
#define SDR0_ECID1			0x0081
#define SDR0_ECID2			0x0082
#define SDR0_JTAG			0x00C0

#define SDR0_DDR0			0x00E1
#define SDR0_DDR0_DPLLRST		0x80000000
#define SDR0_DDR0_DDRM_MASK		0x60000000
#define SDR0_DDR0_DDRM_DDR1		0x20000000
#define SDR0_DDR0_DDRM_DDR2		0x40000000
#define SDR0_DDR0_DDRM_ENCODE(n)	((((unsigned long)(n))&0x03)<<29)
#define SDR0_DDR0_DDRM_DECODE(n)	((((unsigned long)(n))>>29)&0x03)
#define SDR0_DDR0_TUNE_ENCODE(n)	((((unsigned long)(n))&0x2FF)<<0)
#define SDR0_DDR0_TUNE_DECODE(n)	((((unsigned long)(n))>>0)&0x2FF)

#define SDR0_UART0			0x0120
#define SDR0_UART1			0x0121
#define SDR0_UART2			0x0122
#define SDR0_SLPIPE			0x0220

#define SDR0_AMP0			0x0240
#define SDR0_AMP0_PRIORITY		0xFFFF0000
#define SDR0_AMP0_ALTERNATE_PRIORITY	0x0000FF00
#define SDR0_AMP0_RESERVED_BITS_MASK	0x000000FF

#define SDR0_AMP1			0x0241
#define SDR0_AMP1_PRIORITY		0xFC000000
#define SDR0_AMP1_ALTERNATE_PRIORITY	0x0000E000
#define SDR0_AMP1_RESERVED_BITS_MASK	0x03FF1FFF

#define SDR0_MIRQ0			0x0260
#define SDR0_MIRQ1			0x0261
#define SDR0_MALTBL			0x0280
#define SDR0_MALRBL			0x02A0
#define SDR0_MALTBS			0x02C0
#define SDR0_MALRBS			0x02E0

/* Reserved for Customer Use */
#define SDR0_CUST0			0x4000
#define SDR0_CUST0_AUTONEG_MASK		0x8000000
#define SDR0_CUST0_NO_AUTONEG		0x0000000
#define SDR0_CUST0_AUTONEG		0x8000000
#define SDR0_CUST0_ETH_FORCE_MASK	0x6000000
#define SDR0_CUST0_ETH_FORCE_10MHZ	0x0000000
#define SDR0_CUST0_ETH_FORCE_100MHZ	0x2000000
#define SDR0_CUST0_ETH_FORCE_1000MHZ	0x4000000
#define SDR0_CUST0_ETH_DUPLEX_MASK	0x1000000
#define SDR0_CUST0_ETH_HALF_DUPLEX	0x0000000
#define SDR0_CUST0_ETH_FULL_DUPLEX	0x1000000

#define SDR0_SDSTP4			0x4001
#define SDR0_CUST1			0x4002
#define SDR0_SDSTP5			0x4003
#define SDR0_CUST2			0x4004
#define SDR0_SDSTP6			0x4005
#define SDR0_CUST3			0x4006
#define SDR0_SDSTP7			0x4007

#define SDR0_PFC0			0x4100
#define SDR0_PFC0_GPIO_0		0x80000000
#define SDR0_PFC0_PCIX0REQ2_N		0x00000000
#define SDR0_PFC0_GPIO_1		0x40000000
#define SDR0_PFC0_PCIX0REQ3_N		0x00000000
#define SDR0_PFC0_GPIO_2		0x20000000
#define SDR0_PFC0_PCIX0GNT2_N		0x00000000
#define SDR0_PFC0_GPIO_3		0x10000000
#define SDR0_PFC0_PCIX0GNT3_N		0x00000000
#define SDR0_PFC0_GPIO_4		0x08000000
#define SDR0_PFC0_PCIX1REQ2_N		0x00000000
#define SDR0_PFC0_GPIO_5		0x04000000
#define SDR0_PFC0_PCIX1REQ3_N		0x00000000
#define SDR0_PFC0_GPIO_6		0x02000000
#define SDR0_PFC0_PCIX1GNT2_N		0x00000000
#define SDR0_PFC0_GPIO_7		0x01000000
#define SDR0_PFC0_PCIX1GNT3_N		0x00000000
#define SDR0_PFC0_GPIO_8		0x00800000
#define SDR0_PFC0_PERREADY		0x00000000
#define SDR0_PFC0_GPIO_9		0x00400000
#define SDR0_PFC0_PERCS1_N		0x00000000
#define SDR0_PFC0_GPIO_10		0x00200000
#define SDR0_PFC0_PERCS2_N		0x00000000
#define SDR0_PFC0_GPIO_11		0x00100000
#define SDR0_PFC0_IRQ0			0x00000000
#define SDR0_PFC0_GPIO_12		0x00080000
#define SDR0_PFC0_IRQ1			0x00000000
#define SDR0_PFC0_GPIO_13		0x00040000
#define SDR0_PFC0_IRQ2			0x00000000
#define SDR0_PFC0_GPIO_14		0x00020000
#define SDR0_PFC0_IRQ3			0x00000000
#define SDR0_PFC0_GPIO_15		0x00010000
#define SDR0_PFC0_IRQ4			0x00000000
#define SDR0_PFC0_GPIO_16		0x00008000
#define SDR0_PFC0_IRQ5			0x00000000
#define SDR0_PFC0_GPIO_17		0x00004000
#define SDR0_PFC0_PERBE0_N		0x00000000
#define SDR0_PFC0_GPIO_18		0x00002000
#define SDR0_PFC0_PCI0GNT0_N		0x00000000
#define SDR0_PFC0_GPIO_19		0x00001000
#define SDR0_PFC0_PCI0GNT1_N		0x00000000
#define SDR0_PFC0_GPIO_20		0x00000800
#define SDR0_PFC0_PCI0REQ0_N		0x00000000
#define SDR0_PFC0_GPIO_21		0x00000400
#define SDR0_PFC0_PCI0REQ1_N		0x00000000
#define SDR0_PFC0_GPIO_22		0x00000200
#define SDR0_PFC0_PCI1GNT0_N		0x00000000
#define SDR0_PFC0_GPIO_23		0x00000100
#define SDR0_PFC0_PCI1GNT1_N		0x00000000
#define SDR0_PFC0_GPIO_24		0x00000080
#define SDR0_PFC0_PCI1REQ0_N		0x00000000
#define SDR0_PFC0_GPIO_25		0x00000040
#define SDR0_PFC0_PCI1REQ1_N		0x00000000
#define SDR0_PFC0_GPIO_26		0x00000020
#define SDR0_PFC0_PCI2GNT0_N		0x00000000
#define SDR0_PFC0_GPIO_27		0x00000010
#define SDR0_PFC0_PCI2GNT1_N		0x00000000
#define SDR0_PFC0_GPIO_28		0x00000008
#define SDR0_PFC0_PCI2REQ0_N		0x00000000
#define SDR0_PFC0_GPIO_29		0x00000004
#define SDR0_PFC0_PCI2REQ1_N		0x00000000
#define SDR0_PFC0_GPIO_30		0x00000002
#define SDR0_PFC0_UART1RX		0x00000000
#define SDR0_PFC0_GPIO_31		0x00000001
#define SDR0_PFC0_UART1TX		0x00000000

#define SDR0_PFC1			0x4101
#define SDR0_PFC1_UART1_CTS_RTS_MASK	0x02000000
#define SDR0_PFC1_UART1_DSR_DTR		0x00000000
#define SDR0_PFC1_UART1_CTS_RTS		0x02000000
#define SDR0_PFC1_UART2_IN_SERVICE_MASK	0x01000000
#define SDR0_PFC1_UART2_NOT_IN_SERVICE	0x00000000
#define SDR0_PFC1_UART2_IN_SERVICE	0x01000000
#define SDR0_PFC1_ETH_GIGA_MASK		0x00200000
#define SDR0_PFC1_ETH_10_100		0x00000000
#define SDR0_PFC1_ETH_GIGA		0x00200000
#define SDR0_PFC1_ETH_GIGA_ENCODE(n)	((((unsigned long)(n))&0x1)<<21)
#define SDR0_PFC1_ETH_GIGA_DECODE(n)	((((unsigned long)(n))>>21)&0x01)
#define SDR0_PFC1_CPU_TRACE_MASK	0x00180000   /* $218C */
#define SDR0_PFC1_CPU_NO_TRACE		0x00000000
#define SDR0_PFC1_CPU_TRACE		0x00080000
#define SDR0_PFC1_CPU_TRACE_ENCODE(n)	((((unsigned long)(n))&0x3)<<19)
							/* $218C */
#define SDR0_PFC1_CPU_TRACE_DECODE(n)	((((unsigned long)(n))>>19)&0x03)
							/* $218C */

#define SDR0_MFR			0x4300
#endif	/* CONFIG_440SPE	*/

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
/* Pin Function Control Register 0 (SDR0_PFC0) */
#define SDR0_PFC0		0x4100
#define SDR0_PFC0_DBG		0x00008000	/* debug enable */
#define SDR0_PFC0_G49E		0x00004000	/* GPIO 49 enable */
#define SDR0_PFC0_G50E		0x00002000	/* GPIO 50 enable */
#define SDR0_PFC0_G51E		0x00001000	/* GPIO 51 enable */
#define SDR0_PFC0_G52E		0x00000800	/* GPIO 52 enable */
#define SDR0_PFC0_G53E		0x00000400	/* GPIO 53 enable */
#define SDR0_PFC0_G54E		0x00000200	/* GPIO 54 enable */
#define SDR0_PFC0_G55E		0x00000100	/* GPIO 55 enable */
#define SDR0_PFC0_G56E		0x00000080	/* GPIO 56 enable */
#define SDR0_PFC0_G57E		0x00000040	/* GPIO 57 enable */
#define SDR0_PFC0_G58E		0x00000020	/* GPIO 58 enable */
#define SDR0_PFC0_G59E		0x00000010	/* GPIO 59 enable */
#define SDR0_PFC0_G60E		0x00000008	/* GPIO 60 enable */
#define SDR0_PFC0_G61E		0x00000004	/* GPIO 61 enable */
#define SDR0_PFC0_G62E		0x00000002	/* GPIO 62 enable */
#define SDR0_PFC0_G63E		0x00000001	/* GPIO 63 enable */

/* Pin Function Control Register 1 (SDR0_PFC1) */
#define SDR0_PFC1		0x4101
#define SDR0_PFC1_U1ME_MASK	0x02000000	/* UART1 Mode Enable */
#define SDR0_PFC1_U1ME_DSR_DTR	0x00000000	/* UART1 in DSR/DTR Mode */
#define SDR0_PFC1_U1ME_CTS_RTS	0x02000000	/* UART1 in CTS/RTS Mode */
#define SDR0_PFC1_U0ME_MASK	0x00080000	/* UART0 Mode Enable */
#define SDR0_PFC1_U0ME_DSR_DTR	0x00000000	/* UART0 in DSR/DTR Mode */
#define SDR0_PFC1_U0ME_CTS_RTS	0x00080000	/* UART0 in CTS/RTS Mode */
#define SDR0_PFC1_U0IM_MASK	0x00040000	/* UART0 Interface Mode */
#define SDR0_PFC1_U0IM_8PINS	0x00000000	/* UART0 Interface Mode 8 pins*/
#define SDR0_PFC1_U0IM_4PINS	0x00040000	/* UART0 Interface Mode 4 pins*/
#define SDR0_PFC1_SIS_MASK	0x00020000	/* SCP or IIC1 Selection */
#define SDR0_PFC1_SIS_SCP_SEL	0x00000000	/* SCP Selected */
#define SDR0_PFC1_SIS_IIC1_SEL	0x00020000	/* IIC1 Selected */

#define SDR0_ECID0		0x0080
#define SDR0_ECID1		0x0081
#define SDR0_ECID2		0x0082
#define SDR0_ECID3		0x0083

/* Ethernet PLL Configuration Register (SDR0_ETH_PLL) */
#define SDR0_ETH_PLL		0x4102
#define SDR0_ETH_PLL_PLLLOCK	 0x80000000	/*Ethernet PLL lock indication*/
#define SDR0_ETH_PLL_REF_CLK_SEL 0x10000000	/* Ethernet reference clock */
#define SDR0_ETH_PLL_BYPASS	 0x08000000	/* bypass mode enable */
#define SDR0_ETH_PLL_STOPCLK	 0x04000000	/* output clock disable */
#define SDR0_ETH_PLL_TUNE_MASK	 0x03FF0000	/* loop stability tuning bits */
#define SDR0_ETH_PLL_TUNE_ENCODE(n)	((((unsigned long)(n))&0x3ff)<<16)
#define SDR0_ETH_PLL_MULTI_MASK	 0x0000FF00	/* frequency multiplication */
#define SDR0_ETH_PLL_MULTI_ENCODE(n)	((((unsigned long)(n))&0xff)<<8)
#define SDR0_ETH_PLL_RANGEB_MASK 0x000000F0	/* PLLOUTB/C frequency */
#define SDR0_ETH_PLL_RANGEB_ENCODE(n)	((((unsigned long)(n))&0x0f)<<4)
#define SDR0_ETH_PLL_RANGEA_MASK 0x0000000F	/* PLLOUTA frequency */
#define SDR0_ETH_PLL_RANGEA_ENCODE(n)	(((unsigned long)(n))&0x0f)

/* Ethernet Configuration Register (SDR0_ETH_CFG) */
#define SDR0_ETH_CFG		0x4103
#define SDR0_ETH_CFG_SGMII3_LPBK	0x00800000 /*SGMII3 port loopback
						    enable */
#define SDR0_ETH_CFG_SGMII2_LPBK	0x00400000 /*SGMII2 port loopback
						    enable */
#define SDR0_ETH_CFG_SGMII1_LPBK	0x00200000 /*SGMII1 port loopback
						    enable */
#define SDR0_ETH_CFG_SGMII0_LPBK	0x00100000 /*SGMII0 port loopback
						    enable */
#define SDR0_ETH_CFG_SGMII_MASK		0x00070000 /*SGMII Mask */
#define SDR0_ETH_CFG_SGMII2_ENABLE	0x00040000 /*SGMII2 port enable */
#define SDR0_ETH_CFG_SGMII1_ENABLE	0x00020000 /*SGMII1 port enable */
#define SDR0_ETH_CFG_SGMII0_ENABLE	0x00010000 /*SGMII0 port enable */
#define SDR0_ETH_CFG_TAHOE1_BYPASS	0x00002000 /*TAHOE1 Bypass selector */
#define SDR0_ETH_CFG_TAHOE0_BYPASS	0x00001000 /*TAHOE0 Bypass selector */
#define SDR0_ETH_CFG_EMAC3_PHY_CLK_SEL	0x00000800 /*EMAC 3 PHY clock selector*/
#define SDR0_ETH_CFG_EMAC2_PHY_CLK_SEL	0x00000400 /*EMAC 2 PHY clock selector*/
#define SDR0_ETH_CFG_EMAC1_PHY_CLK_SEL	0x00000200 /*EMAC 1 PHY clock selector*/
#define SDR0_ETH_CFG_EMAC0_PHY_CLK_SEL	0x00000100 /*EMAC 0 PHY clock selector*/
#define SDR0_ETH_CFG_EMAC_2_1_SWAP	0x00000080 /*Swap EMAC2 with EMAC1 */
#define SDR0_ETH_CFG_EMAC_0_3_SWAP	0x00000040 /*Swap EMAC0 with EMAC3 */
#define SDR0_ETH_CFG_MDIO_SEL_MASK	0x00000030 /*MDIO source selector mask*/
#define SDR0_ETH_CFG_MDIO_SEL_EMAC0	0x00000000 /*MDIO source - EMAC0 */
#define SDR0_ETH_CFG_MDIO_SEL_EMAC1	0x00000010 /*MDIO source - EMAC1 */
#define SDR0_ETH_CFG_MDIO_SEL_EMAC2	0x00000020 /*MDIO source - EMAC2 */
#define SDR0_ETH_CFG_MDIO_SEL_EMAC3	0x00000030 /*MDIO source - EMAC3 */
#define SDR0_ETH_CFG_ZMII_MODE_MASK	0x0000000C /*ZMII bridge mode selector
						    mask */
#define SDR0_ETH_CFG_ZMII_SEL_MII	0x00000000 /*ZMII bridge mode - MII */
#define SDR0_ETH_CFG_ZMII_SEL_SMII	0x00000004 /*ZMII bridge mode - SMII */
#define SDR0_ETH_CFG_ZMII_SEL_RMII_10	0x00000008 /*ZMII bridge mode - RMII
						    (10 Mbps) */
#define SDR0_ETH_CFG_ZMII_SEL_RMII_100	0x0000000C /*ZMII bridge mode - RMII
						    (100 Mbps) */
#define SDR0_ETH_CFG_GMC1_BRIDGE_SEL	0x00000002 /*GMC Port 1 bridge
						     selector */
#define SDR0_ETH_CFG_GMC0_BRIDGE_SEL	0x00000001 /*GMC Port 0 bridge
						    selector */

#define SDR0_ETH_CFG_ZMII_MODE_SHIFT		4
#define SDR0_ETH_CFG_ZMII_MII_MODE		0x00
#define SDR0_ETH_CFG_ZMII_SMII_MODE		0x01
#define SDR0_ETH_CFG_ZMII_RMII_MODE_10M		0x10
#define SDR0_ETH_CFG_ZMII_RMII_MODE_100M	0x11

/* Ethernet Status Register */
#define SDR0_ETH_STS		0x4104

/* Miscealleneaous Function Reg. (SDR0_MFR) */
#define SDR0_MFR		0x4300
#define SDR0_MFR_T0TxFL		0x00800000	/* force parity error TAHOE0 Tx
						    FIFO bits 0:63 */
#define SDR0_MFR_T0TxFH		0x00400000	/* force parity error TAHOE0 Tx
						    FIFO bits 64:127 */
#define SDR0_MFR_T1TxFL		0x00200000	/* force parity error TAHOE1 Tx
						    FIFO bits 0:63 */
#define SDR0_MFR_T1TxFH		0x00100000	/* force parity error TAHOE1 Tx
						    FIFO bits 64:127 */
#define SDR0_MFR_E0TxFL		0x00008000	/* force parity error EMAC0 Tx
						    FIFO bits 0:63 */
#define SDR0_MFR_E0TxFH		0x00004000	/* force parity error EMAC0 Tx
						    FIFO bits 64:127 */
#define SDR0_MFR_E0RxFL		0x00002000	/* force parity error EMAC0 Rx
						    FIFO bits 0:63 */
#define SDR0_MFR_E0RxFH		0x00001000	/* force parity error EMAC0 Rx
						    FIFO bits 64:127 */
#define SDR0_MFR_E1TxFL		0x00000800	/* force parity error EMAC1 Tx
						    FIFO bits 0:63 */
#define SDR0_MFR_E1TxFH		0x00000400	/* force parity error EMAC1 Tx
						    FIFO bits 64:127 */
#define SDR0_MFR_E1RxFL		0x00000200	/* force parity error EMAC1 Rx
						    FIFO bits 0:63 */
#define SDR0_MFR_E1RxFH		0x00000100	/* force parity error EMAC1 Rx
						    FIFO bits 64:127 */
#define SDR0_MFR_E2TxFL		0x00000080	/* force parity error EMAC2 Tx
						    FIFO bits 0:63 */
#define SDR0_MFR_E2TxFH		0x00000040	/* force parity error EMAC2 Tx
						    FIFO bits 64:127 */
#define SDR0_MFR_E2RxFL		0x00000020	/* force parity error EMAC2 Rx
						    FIFO bits 0:63 */
#define SDR0_MFR_E2RxFH		0x00000010	/* force parity error EMAC2 Rx
						    FIFO bits 64:127 */
#define SDR0_MFR_E3TxFL		0x00000008	/* force parity error EMAC3 Tx
						    FIFO bits 0:63 */
#define SDR0_MFR_E3TxFH		0x00000004	/* force parity error EMAC3 Tx
						    FIFO bits 64:127 */
#define SDR0_MFR_E3RxFL		0x00000002	/* force parity error EMAC3 Rx
						    FIFO bits 0:63 */
#define SDR0_MFR_E3RxFH		0x00000001	/* force parity error EMAC3 Rx
						    FIFO bits 64:127 */

/* EMACx TX Status Register (SDR0_EMACxTXST)*/
#define SDR0_EMAC0TXST		0x4400
#define SDR0_EMAC1TXST		0x4401
#define SDR0_EMAC2TXST		0x4402
#define SDR0_EMAC3TXST		0x4403

#define SDR0_EMACxTXST_FUR	0x02000000 /*TX FIFO underrun */
#define SDR0_EMACxTXST_BC	0x01000000 /*broadcase address */
#define SDR0_EMACxTXST_MC	0x00800000 /*multicast address */
#define SDR0_EMACxTXST_UC	0x00400000 /*unicast address */
#define SDR0_EMACxTXST_FP	0x00200000 /*frame paused by control packet */
#define SDR0_EMACxTXST_BFCS	0x00100000 /*bad FCS in the transmitted frame */
#define SDR0_EMACxTXST_CPF	0x00080000 /*TX control pause frame */
#define SDR0_EMACxTXST_CF	0x00040000 /*TX control frame */
#define SDR0_EMACxTXST_MSIZ	0x00020000 /* 1024-maxsize bytes transmitted */
#define SDR0_EMACxTXST_1023	0x00010000 /*512-1023 bytes transmitted */
#define SDR0_EMACxTXST_511	0x00008000 /*256-511 bytes transmitted */
#define SDR0_EMACxTXST_255	0x00004000 /*128-255 bytes transmitted */
#define SDR0_EMACxTXST_127	0x00002000 /*65-127 bytes transmitted */
#define SDR0_EMACxTXST_64	0x00001000 /*64 bytes transmitted */
#define SDR0_EMACxTXST_SQE	0x00000800 /*SQE indication */
#define SDR0_EMACxTXST_LOC	0x00000400 /*loss of carrier sense */
#define SDR0_EMACxTXST_IERR	0x00000080 /*EMAC internal error */
#define SDR0_EMACxTXST_EDF	0x00000040 /*excessive deferral */
#define SDR0_EMACxTXST_ECOL	0x00000020 /*excessive collisions */
#define SDR0_EMACxTXST_LCOL	0x00000010 /*late collision */
#define SDR0_EMACxTXST_DFFR	0x00000008 /*deferred frame */
#define SDR0_EMACxTXST_MCOL	0x00000004 /*multiple collision frame */
#define SDR0_EMACxTXST_SCOL	0x00000002 /*single collision frame */
#define SDR0_EMACxTXST_TXOK	0x00000001 /*transmit OK */

/* EMACx RX Status Register (SDR0_EMACxRXST)*/
#define SDR0_EMAC0RXST		0x4404
#define SDR0_EMAC1RXST		0x4405
#define SDR0_EMAC2RXST		0x4406
#define SDR0_EMAC3RXST		0x4407

#define SDR0_EMACxRXST_FOR	0x20000000	/* RX FIFO overrun */
#define SDR0_EMACxRXST_BC	0x10000000	/* broadcast address */
#define SDR0_EMACxRXST_MC	0x08000000	/* multicast address */
#define SDR0_EMACxRXST_UC	0x04000000	/* unicast address */
#define SDR0_EMACxRXST_UPR_MASK	0x03800000	/* user priority field */
#define SDR0_EMACxRXST_UPR_ENCODE(n)	((((unsigned long)(n))&0x07)<<23)
#define SDR0_EMACxRXST_VLAN	0x00400000	/* RX VLAN tagged frame */
#define SDR0_EMACxRXST_LOOP	0x00200000	/* received in loop-back mode */
#define SDR0_EMACxRXST_UOP	0x00100000	/* RX unsupported opcode */
#define SDR0_EMACxRXST_CPF	0x00080000	/* RX control pause frame */
#define SDR0_EMACxRXST_CF	0x00040000	/* RX control frame*/
#define SDR0_EMACxRXST_MSIZ	0x00020000	/* 1024-MaxSize bytes recieved*/
#define SDR0_EMACxRXST_1023	0x00010000	/* 512-1023 bytes received */
#define SDR0_EMACxRXST_511	0x00008000	/* 128-511 bytes received */
#define SDR0_EMACxRXST_255	0x00004000	/* 128-255 bytes received */
#define SDR0_EMACxRXST_127	0x00002000	/* 65-127 bytes received */
#define SDR0_EMACxRXST_64	0x00001000	/* 64 bytes received */
#define SDR0_EMACxRXST_RUNT	0x00000800	/* runt frame */
#define SDR0_EMACxRXST_SEVT	0x00000400	/* short event */
#define SDR0_EMACxRXST_AERR	0x00000200	/* alignment error */
#define SDR0_EMACxRXST_SERR	0x00000100	/* received with symbol error */
#define SDR0_EMACxRXST_BURST	0x00000040	/* received burst */
#define SDR0_EMACxRXST_F2L	0x00000020	/* frame is to long */
#define SDR0_EMACxRXST_OERR	0x00000010	/* out of range length error */
#define SDR0_EMACxRXST_IERR	0x00000008	/* in range length error */
#define SDR0_EMACxRXST_LOST	0x00000004	/* frame lost due to internal
						   EMAC receive error */
#define SDR0_EMACxRXST_BFCS	0x00000002 /* bad FCS in the recieved frame */
#define SDR0_EMACxRXST_RXOK	0x00000001	/* Recieve OK */

/* EMACx TX Status Register (SDR0_EMACxREJCNT)*/
#define SDR0_EMAC0REJCNT	0x4408
#define SDR0_EMAC1REJCNT	0x4409
#define SDR0_EMAC2REJCNT	0x440A
#define SDR0_EMAC3REJCNT	0x440B

#define SDR0_DDR0			0x00E1
#define SDR0_DDR0_DPLLRST		0x80000000
#define SDR0_DDR0_DDRM_MASK		0x60000000
#define SDR0_DDR0_DDRM_DDR1		0x20000000
#define SDR0_DDR0_DDRM_DDR2		0x40000000
#define SDR0_DDR0_DDRM_ENCODE(n)	((((unsigned long)(n))&0x03)<<29)
#define SDR0_DDR0_DDRM_DECODE(n)	((((unsigned long)(n))>>29)&0x03)
#define SDR0_DDR0_TUNE_ENCODE(n)	((((unsigned long)(n))&0x2FF)<<0)
#define SDR0_DDR0_TUNE_DECODE(n)	((((unsigned long)(n))>>0)&0x2FF)

#define AHB_TOP			0xA4
#define AHB_BOT			0xA5
#define SDR0_AHB_CFG		0x370
#define SDR0_USB2HOST_CFG	0x371
#endif /* CONFIG_460EX || CONFIG_460GT */

#define SDR0_SDCS_SDD			(0x80000000 >> 31)

#if defined(CONFIG_440GP)
#define CPC0_STRP1_PAE_MASK		(0x80000000 >> 11)
#define CPC0_STRP1_PISE_MASK		(0x80000000 >> 13)
#endif /* defined(CONFIG_440GP) */
#if defined(CONFIG_440GX) || defined(CONFIG_440SP) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 13)
#define SDR0_SDSTP1_PISE_MASK		(0x80000000 >> 15)
#endif /* defined(CONFIG_440GX) || defined(CONFIG_440SP) */
#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 21)
#define SDR0_SDSTP1_PAME_MASK		(0x80000000 >> 27)
#endif /* defined(CONFIG_440EP) || defined(CONFIG_440GR) */

#define SDR0_UARTX_UXICS_MASK		0xF0000000
#define SDR0_UARTX_UXICS_PLB		0x20000000
#define SDR0_UARTX_UXEC_MASK		0x00800000
#define SDR0_UARTX_UXEC_INT		0x00000000
#define SDR0_UARTX_UXEC_EXT		0x00800000
#define SDR0_UARTX_UXDTE_MASK		0x00400000
#define SDR0_UARTX_UXDTE_DISABLE	0x00000000
#define SDR0_UARTX_UXDTE_ENABLE		0x00400000
#define SDR0_UARTX_UXDRE_MASK		0x00200000
#define SDR0_UARTX_UXDRE_DISABLE	0x00000000
#define SDR0_UARTX_UXDRE_ENABLE		0x00200000
#define SDR0_UARTX_UXDC_MASK		0x00100000
#define SDR0_UARTX_UXDC_NOTCLEARED	0x00000000
#define SDR0_UARTX_UXDC_CLEARED		0x00100000
#define SDR0_UARTX_UXDIV_MASK		0x000000FF
#define SDR0_UARTX_UXDIV_ENCODE(n)	((((unsigned long)(n))&0xFF)<<0)
#define SDR0_UARTX_UXDIV_DECODE(n)	((((((unsigned long)(n))>>0)-1)&0xFF)+1)

#define SDR0_CPU440_EARV_MASK		0x30000000
#define SDR0_CPU440_EARV_EBC		0x10000000
#define SDR0_CPU440_EARV_PCI		0x20000000
#define SDR0_CPU440_EARV_ENCODE(n)	((((unsigned long)(n))&0x03)<<28)
#define SDR0_CPU440_EARV_DECODE(n)	((((unsigned long)(n))>>28)&0x03)
#define SDR0_CPU440_NTO1_MASK		0x00000002
#define SDR0_CPU440_NTO1_NTOP		0x00000000
#define SDR0_CPU440_NTO1_NTO1		0x00000002
#define SDR0_CPU440_NTO1_ENCODE(n)	((((unsigned long)(n))&0x01)<<1)
#define SDR0_CPU440_NTO1_DECODE(n)	((((unsigned long)(n))>>1)&0x01)

#define SDR0_XCR_PAE_MASK		0x80000000
#define SDR0_XCR_PAE_DISABLE		0x00000000
#define SDR0_XCR_PAE_ENABLE		0x80000000
#define SDR0_XCR_PAE_ENCODE(n)		((((unsigned long)(n))&0x01)<<31)
#define SDR0_XCR_PAE_DECODE(n)		((((unsigned long)(n))>>31)&0x01)
#define SDR0_XCR_PHCE_MASK		0x40000000
#define SDR0_XCR_PHCE_DISABLE		0x00000000
#define SDR0_XCR_PHCE_ENABLE		0x40000000
#define SDR0_XCR_PHCE_ENCODE(n)		((((unsigned long)(n))&0x01)<<30)
#define SDR0_XCR_PHCE_DECODE(n)		((((unsigned long)(n))>>30)&0x01)
#define SDR0_XCR_PISE_MASK		0x20000000
#define SDR0_XCR_PISE_DISABLE		0x00000000
#define SDR0_XCR_PISE_ENABLE		0x20000000
#define SDR0_XCR_PISE_ENCODE(n)		((((unsigned long)(n))&0x01)<<29)
#define SDR0_XCR_PISE_DECODE(n)		((((unsigned long)(n))>>29)&0x01)
#define SDR0_XCR_PCWE_MASK		0x10000000
#define SDR0_XCR_PCWE_DISABLE		0x00000000
#define SDR0_XCR_PCWE_ENABLE		0x10000000
#define SDR0_XCR_PCWE_ENCODE(n)		((((unsigned long)(n))&0x01)<<28)
#define SDR0_XCR_PCWE_DECODE(n)		((((unsigned long)(n))>>28)&0x01)
#define SDR0_XCR_PPIM_MASK		0x0F000000
#define SDR0_XCR_PPIM_ENCODE(n)		((((unsigned long)(n))&0x0F)<<24)
#define SDR0_XCR_PPIM_DECODE(n)		((((unsigned long)(n))>>24)&0x0F)
#define SDR0_XCR_PR64E_MASK		0x00800000
#define SDR0_XCR_PR64E_DISABLE		0x00000000
#define SDR0_XCR_PR64E_ENABLE		0x00800000
#define SDR0_XCR_PR64E_ENCODE(n)	((((unsigned long)(n))&0x01)<<23)
#define SDR0_XCR_PR64E_DECODE(n)	((((unsigned long)(n))>>23)&0x01)
#define SDR0_XCR_PXFS_MASK		0x00600000
#define SDR0_XCR_PXFS_HIGH		0x00000000
#define SDR0_XCR_PXFS_MED		0x00200000
#define SDR0_XCR_PXFS_LOW		0x00400000
#define SDR0_XCR_PXFS_ENCODE(n)		((((unsigned long)(n))&0x03)<<21)
#define SDR0_XCR_PXFS_DECODE(n)		((((unsigned long)(n))>>21)&0x03)
#define SDR0_XCR_PDM_MASK		0x00000040
#define SDR0_XCR_PDM_MULTIPOINT		0x00000000
#define SDR0_XCR_PDM_P2P		0x00000040
#define SDR0_XCR_PDM_ENCODE(n)		((((unsigned long)(n))&0x01)<<19)
#define SDR0_XCR_PDM_DECODE(n)		((((unsigned long)(n))>>19)&0x01)

#define SDR0_PFC0_UART1_DSR_CTS_EN_MASK 0x00030000
#define SDR0_PFC0_GEIE_MASK		0x00003E00
#define SDR0_PFC0_GEIE_TRE		0x00003E00
#define SDR0_PFC0_GEIE_NOTRE		0x00000000
#define SDR0_PFC0_TRE_MASK		0x00000100
#define SDR0_PFC0_TRE_DISABLE		0x00000000
#define SDR0_PFC0_TRE_ENABLE		0x00000100
#define SDR0_PFC0_TRE_ENCODE(n)		((((unsigned long)(n))&0x01)<<8)
#define SDR0_PFC0_TRE_DECODE(n)		((((unsigned long)(n))>>8)&0x01)

#define SDR0_PFC1_UART1_DSR_CTS_MASK	0x02000000
#define SDR0_PFC1_EPS_MASK		0x01C00000
#define SDR0_PFC1_EPS_GROUP0		0x00000000
#define SDR0_PFC1_EPS_GROUP1		0x00400000
#define SDR0_PFC1_EPS_GROUP2		0x00800000
#define SDR0_PFC1_EPS_GROUP3		0x00C00000
#define SDR0_PFC1_EPS_GROUP4		0x01000000
#define SDR0_PFC1_EPS_GROUP5		0x01400000
#define SDR0_PFC1_EPS_GROUP6		0x01800000
#define SDR0_PFC1_EPS_GROUP7		0x01C00000
#define SDR0_PFC1_EPS_ENCODE(n)		((((unsigned long)(n))&0x07)<<22)
#define SDR0_PFC1_EPS_DECODE(n)		((((unsigned long)(n))>>22)&0x07)
#define SDR0_PFC1_RMII_MASK		0x00200000
#define SDR0_PFC1_RMII_100MBIT		0x00000000
#define SDR0_PFC1_RMII_10MBIT		0x00200000
#define SDR0_PFC1_RMII_ENCODE(n)	((((unsigned long)(n))&0x01)<<21)
#define SDR0_PFC1_RMII_DECODE(n)	((((unsigned long)(n))>>21)&0x01)
#define SDR0_PFC1_CTEMS_MASK		0x00100000
#define SDR0_PFC1_CTEMS_EMS		0x00000000
#define SDR0_PFC1_CTEMS_CPUTRACE	0x00100000

#define SDR0_MFR_TAH0_MASK		0x80000000
#define SDR0_MFR_TAH0_ENABLE		0x00000000
#define SDR0_MFR_TAH0_DISABLE		0x80000000
#define SDR0_MFR_TAH1_MASK		0x40000000
#define SDR0_MFR_TAH1_ENABLE		0x00000000
#define SDR0_MFR_TAH1_DISABLE		0x40000000
#define SDR0_MFR_PCM_MASK		0x20000000
#define SDR0_MFR_PCM_PPC440GX		0x00000000
#define SDR0_MFR_PCM_PPC440GP		0x20000000
#define SDR0_MFR_ECS_MASK		0x10000000
#define SDR0_MFR_ECS_INTERNAL		0x10000000

#define SDR0_MFR_ETH0_CLK_SEL		0x08000000 /* Ethernet0 Clock Select */
#define SDR0_MFR_ETH1_CLK_SEL		0x04000000 /* Ethernet1 Clock Select */
#define SDR0_MFR_ZMII_MODE_MASK		0x03000000 /* ZMII Mode Mask   */
#define SDR0_MFR_ZMII_MODE_MII		0x00000000 /* ZMII Mode MII  */
#define SDR0_MFR_ZMII_MODE_SMII		0x01000000 /* ZMII Mode SMII */
#define SDR0_MFR_ZMII_MODE_RMII_10M	0x02000000 /* ZMII Mode RMII - 10 Mbs */
#define SDR0_MFR_ZMII_MODE_RMII_100M	0x03000000 /* ZMII Mode RMII - 100 Mbs*/
#define SDR0_MFR_ZMII_MODE_BIT0		0x02000000 /* ZMII Mode Bit0 */
#define SDR0_MFR_ZMII_MODE_BIT1		0x01000000 /* ZMII Mode Bit1 */
#define SDR0_MFR_ERRATA3_EN0		0x00800000
#define SDR0_MFR_ERRATA3_EN1		0x00400000
#if defined(CONFIG_440GX) /* test-only: only 440GX or 440SPE??? */
#define SDR0_MFR_PKT_REJ_MASK	0x00300000 /* Pkt Rej. Enable Mask */
#define SDR0_MFR_PKT_REJ_EN	0x00300000 /* Pkt Rej. Enable on both EMAC3
					      0-1 */
#define SDR0_MFR_PKT_REJ_EN0	0x00200000 /* Pkt Rej. Enable on EMAC3(0) */
#define SDR0_MFR_PKT_REJ_EN1	0x00100000 /* Pkt Rej. Enable on EMAC3(1) */
#define SDR0_MFR_PKT_REJ_POL	0x00080000 /* Packet Reject Polarity      */
#endif


#if defined(CONFIG_440EPX)
#define CPM0_ER			0x000000B0
#define CPM1_ER			0x000000F0
#define PLB4A0_ACR		0x00000081
#define PLB4A1_ACR		0x00000089
#define PLB3A0_ACR		0x00000077
#define OPB2PLB40_BCTRL		0x00000350
#define P4P3BO0_CFG		0x00000026
#define SPI0_MODE               0xEF600090 /* SPI Mode Regsgiter */

#endif

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_PFC1_EPS_ENCODE(n)		((((unsigned long)(n))&0x07)<<22)
#define SDR0_PFC1_EPS_DECODE(n)		((((unsigned long)(n))>>22)&0x07)
#define SDR0_PFC2_EPS_ENCODE(n)		((((unsigned long)(n))&0x07)<<29)
#define SDR0_PFC2_EPS_DECODE(n)		((((unsigned long)(n))>>29)&0x07)
#endif

#define SDR0_MFR_ECS_MASK		0x10000000
#define SDR0_MFR_ECS_INTERNAL		0x10000000

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define SDR0_SRST0	 0x200
#define SDR0_SRST0_BGO 	 0x80000000 /* PLB to OPB bridge */
#define SDR0_SRST0_PLB4	 0x40000000 /* PLB4 arbiter */
#define SDR0_SRST0_EBC 	 0x20000000 /* External bus controller */
#define SDR0_SRST0_OPB 	 0x10000000 /* OPB arbiter */
#define SDR0_SRST0_UART0 0x08000000 /* Universal asynchronous receiver/
				       transmitter 0 */
#define SDR0_SRST0_UART1 0x04000000 /* Universal asynchronous receiver/
				       transmitter 1 */
#define SDR0_SRST0_IIC0	 0x02000000 /* Inter integrated circuit 0 */
#define SDR0_SRST0_USB2H 0x01000000 /* USB2.0 Host */
#define SDR0_SRST0_GPIO	 0x00800000 /* General purpose I/O */
#define SDR0_SRST0_GPT 	 0x00400000 /* General purpose timer */
#define SDR0_SRST0_DMC 	 0x00200000 /* DDR SDRAM memory controller */
#define SDR0_SRST0_PCI 	 0x00100000 /* PCI */
#define SDR0_SRST0_EMAC0 0x00080000 /* Ethernet media access controller 0 */
#define SDR0_SRST0_EMAC1 0x00040000 /* Ethernet media access controller 1 */
#define SDR0_SRST0_CPM0	 0x00020000 /* Clock and power management */
#define SDR0_SRST0_ZMII	 0x00010000 /* ZMII bridge */
#define SDR0_SRST0_UIC0	 0x00008000 /* Universal interrupt controller 0 */
#define SDR0_SRST0_UIC1	 0x00004000 /* Universal interrupt controller 1 */
#define SDR0_SRST0_IIC1	 0x00002000 /* Inter integrated circuit 1 */
#define SDR0_SRST0_SCP 	 0x00001000 /* Serial communications port */
#define SDR0_SRST0_BGI 	 0x00000800 /* OPB to PLB bridge */
#define SDR0_SRST0_DMA 	 0x00000400 /* Direct memory access controller */
#define SDR0_SRST0_DMAC	 0x00000200 /* DMA channel */
#define SDR0_SRST0_MAL 	 0x00000100 /* Media access layer */
#define SDR0_SRST0_USB2D 0x00000080 /* USB2.0 device */
#define SDR0_SRST0_GPTR	 0x00000040 /* General purpose timer */
#define SDR0_SRST0_P4P3	 0x00000010 /* PLB4 to PLB3 bridge */
#define SDR0_SRST0_P3P4	 0x00000008 /* PLB3 to PLB4 bridge */
#define SDR0_SRST0_PLB3	 0x00000004 /* PLB3 arbiter */
#define SDR0_SRST0_UART2 0x00000002 /* Universal asynchronous receiver/
				       transmitter 2 */
#define SDR0_SRST0_UART3 0x00000001 /* Universal asynchronous receiver/
				       transmitter 3 */

#define SDR0_SRST1		0x201
#define SDR0_SRST1_NDFC		0x80000000 /* Nand flash controller */
#define SDR0_SRST1_OPBA1	0x40000000 /* OPB Arbiter attached to PLB4 */
#define SDR0_SRST1_P4OPB0	0x20000000 /* PLB4 to OPB Bridge0 */
#define SDR0_SRST1_PLB42OPB0    SDR0_SRST1_P4OPB0
#define SDR0_SRST1_DMA4		0x10000000 /* DMA to PLB4 */
#define SDR0_SRST1_DMA4CH	0x08000000 /* DMA Channel to PLB4 */
#define SDR0_SRST1_OPBA2	0x04000000 /* OPB Arbiter attached to PLB4
					      USB 2.0 Host */
#define SDR0_SRST1_OPB2PLB40	0x02000000 /* OPB to PLB4 Bridge attached to
					      USB 2.0 Host */
#define SDR0_SRST1_PLB42OPB1	0x01000000 /* PLB4 to OPB Bridge attached to
					      USB 2.0 Host */
#define SDR0_SRST1_CPM1		0x00800000 /* Clock and Power management 1 */
#define SDR0_SRST1_UIC2		0x00400000 /* Universal Interrupt Controller 2*/
#define SDR0_SRST1_CRYP0	0x00200000 /* Security Engine */
#define SDR0_SRST1_USB20PHY	0x00100000 /* USB 2.0 Phy */
#define SDR0_SRST1_USB2HUTMI	0x00080000 /* USB 2.0 Host UTMI Interface */
#define SDR0_SRST1_USB2HPHY	0x00040000 /* USB 2.0 Host Phy Interface */
#define SDR0_SRST1_SRAM0	0x00020000 /* Internal SRAM Controller */
#define SDR0_SRST1_RGMII0	0x00010000 /* RGMII Bridge */
#define SDR0_SRST1_ETHPLL	0x00008000 /* Ethernet PLL */
#define SDR0_SRST1_FPU 		0x00004000 /* Floating Point Unit */
#define SDR0_SRST1_KASU0	0x00002000 /* Kasumi Engine */

#define SDR0_EMAC0RXST 		0x00004301 /* */
#define SDR0_EMAC0TXST		0x00004302 /* */
#define SDR0_CRYP0		0x00004500
#define SDR0_EBC0		0x00000100
#define SDR0_SDSTP2		0x00004001
#define SDR0_SDSTP3		0x00004001
#elif defined(CONFIG_460EX) || defined(CONFIG_460GT)

#define SDR0_SRST0		SDR0_SRST  /* for compatability reasons */
#define SDR0_SRST0_BGO		0x80000000 /* PLB to OPB bridge */
#define SDR0_SRST0_PLB4		0x40000000 /* PLB4 arbiter */
#define SDR0_SRST0_EBC		0x20000000 /* External bus controller */
#define SDR0_SRST0_OPB		0x10000000 /* OPB arbiter */
#define SDR0_SRST0_UART0	0x08000000 /* Universal asynchronous receiver/
					      transmitter 0 */
#define SDR0_SRST0_UART1	0x04000000 /* Universal asynchronous receiver/
					      transmitter 1 */
#define SDR0_SRST0_IIC0		0x02000000 /* Inter integrated circuit 0 */
#define SDR0_SRST0_IIC1		0x01000000 /* Inter integrated circuit 1 */
#define SDR0_SRST0_GPIO0	0x00800000 /* General purpose I/O 0 */
#define SDR0_SRST0_GPT		0x00400000 /* General purpose timer */
#define SDR0_SRST0_DMC		0x00200000 /* DDR SDRAM memory controller */
#define SDR0_SRST0_PCI		0x00100000 /* PCI */
#define SDR0_SRST0_CPM0		0x00020000 /* Clock and power management */
#define SDR0_SRST0_IMU		0x00010000 /* I2O DMA */
#define SDR0_SRST0_UIC0		0x00008000 /* Universal interrupt controller 0*/
#define SDR0_SRST0_UIC1		0x00004000 /* Universal interrupt controller 1*/
#define SDR0_SRST0_SRAM		0x00002000 /* Universal interrupt controller 0*/
#define SDR0_SRST0_UIC2		0x00001000 /* Universal interrupt controller 2*/
#define SDR0_SRST0_UIC3		0x00000800 /* Universal interrupt controller 3*/
#define SDR0_SRST0_OCM		0x00000400 /* Universal interrupt controller 0*/
#define SDR0_SRST0_UART2	0x00000200 /* Universal asynchronous receiver/
					      transmitter 2 */
#define SDR0_SRST0_MAL		0x00000100 /* Media access layer */
#define SDR0_SRST0_GPTR         0x00000040 /* General purpose timer */
#define SDR0_SRST0_L2CACHE	0x00000004 /* L2 Cache */
#define SDR0_SRST0_UART3	0x00000002 /* Universal asynchronous receiver/
					      transmitter 3 */
#define SDR0_SRST0_GPIO1	0x00000001 /* General purpose I/O 1 */

#define SDR0_SRST1		0x201
#define SDR0_SRST1_RLL		0x80000000 /* SRIO RLL */
#define SDR0_SRST1_SCP		0x40000000 /* Serial communications port */
#define SDR0_SRST1_PLBARB	0x20000000 /* PLB Arbiter */
#define SDR0_SRST1_EIPPKP	0x10000000 /* EIPPPKP */
#define SDR0_SRST1_EIP94	0x08000000 /* EIP 94 */
#define SDR0_SRST1_EMAC0	0x04000000 /* Ethernet media access
					      controller 0 */
#define SDR0_SRST1_EMAC1	0x02000000 /* Ethernet media access
					      controller 1 */
#define SDR0_SRST1_EMAC2	0x01000000 /* Ethernet media access
					      controller 2 */
#define SDR0_SRST1_EMAC3	0x00800000 /* Ethernet media access
					      controller 3 */
#define SDR0_SRST1_ZMII		0x00400000 /* Ethernet ZMII/RMII/SMII */
#define SDR0_SRST1_RGMII0	0x00200000 /* Ethernet RGMII/RTBI 0 */
#define SDR0_SRST1_RGMII1	0x00100000 /* Ethernet RGMII/RTBI 1 */
#define SDR0_SRST1_DMA4		0x00080000 /* DMA to PLB4 */
#define SDR0_SRST1_DMA4CH	0x00040000 /* DMA Channel to PLB4 */
#define SDR0_SRST1_SATAPHY	0x00020000 /* Serial ATA PHY */
#define SDR0_SRST1_SRIODEV	0x00010000 /* Serial Rapid IO core, PCS, and
					      serdes */
#define SDR0_SRST1_SRIOPCS	0x00008000 /* Serial Rapid IO core and PCS */
#define SDR0_SRST1_NDFC		0x00004000 /* Nand flash controller */
#define SDR0_SRST1_SRIOPLB	0x00002000 /* Serial Rapid IO PLB */
#define SDR0_SRST1_ETHPLL	0x00001000 /* Ethernet PLL */
#define SDR0_SRST1_TAHOE1	0x00000800 /* Ethernet Tahoe 1 */
#define SDR0_SRST1_TAHOE0	0x00000400 /* Ethernet Tahoe 0 */
#define SDR0_SRST1_SGMII0	0x00000200 /* Ethernet SGMII 0 */
#define SDR0_SRST1_SGMII1	0x00000100 /* Ethernet SGMII 1 */
#define SDR0_SRST1_SGMII2	0x00000080 /* Ethernet SGMII 2 */
#define SDR0_SRST1_AHB		0x00000040 /* PLB4XAHB bridge */
#define SDR0_SRST1_USBOTGPHY	0x00000020 /* USB 2.0 OTG PHY */
#define SDR0_SRST1_USBOTG	0x00000010 /* USB 2.0 OTG controller */
#define SDR0_SRST1_USBHOST	0x00000008 /* USB 2.0 Host controller */
#define SDR0_SRST1_AHBDMAC	0x00000004 /* AHB DMA controller */
#define SDR0_SRST1_AHBICM	0x00000002 /* AHB inter-connect matrix */
#define SDR0_SRST1_SATA		0x00000001 /* Serial ATA controller */

#else

#define SDR0_SRST_BGO			0x80000000
#define SDR0_SRST_PLB			0x40000000
#define SDR0_SRST_EBC			0x20000000
#define SDR0_SRST_OPB			0x10000000
#define SDR0_SRST_UART0			0x08000000
#define SDR0_SRST_UART1			0x04000000
#define SDR0_SRST_IIC0			0x02000000
#define SDR0_SRST_IIC1			0x01000000
#define SDR0_SRST_GPIO			0x00800000
#define SDR0_SRST_GPT			0x00400000
#define SDR0_SRST_DMC			0x00200000
#define SDR0_SRST_PCI			0x00100000
#define SDR0_SRST_EMAC0			0x00080000
#define SDR0_SRST_EMAC1			0x00040000
#define SDR0_SRST_CPM			0x00020000
#define SDR0_SRST_IMU			0x00010000
#define SDR0_SRST_UIC01			0x00008000
#define SDR0_SRST_UICB2			0x00004000
#define SDR0_SRST_SRAM			0x00002000
#define SDR0_SRST_EBM			0x00001000
#define SDR0_SRST_BGI			0x00000800
#define SDR0_SRST_DMA			0x00000400
#define SDR0_SRST_DMAC			0x00000200
#define SDR0_SRST_MAL			0x00000100
#define SDR0_SRST_ZMII			0x00000080
#define SDR0_SRST_GPTR			0x00000040
#define SDR0_SRST_PPM			0x00000020
#define SDR0_SRST_EMAC2			0x00000010
#define SDR0_SRST_EMAC3			0x00000008
#define SDR0_SRST_RGMII			0x00000001

#endif

/*-----------------------------------------------------------------------------+
|  Clocking
+-----------------------------------------------------------------------------*/
#if defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_460SX)
#define PLLSYS0_FWD_DIV_A_MASK	0x000000f0	/* Fwd Div A */
#define PLLSYS0_FWD_DIV_B_MASK	0x0000000f	/* Fwd Div B */
#define PLLSYS0_FB_DIV_MASK	0x0000ff00	/* Feedback divisor */
#define PLLSYS0_OPB_DIV_MASK	0x0c000000	/* OPB Divisor */
#define PLLSYS0_PLBEDV0_DIV_MASK 0xe0000000	/* PLB Early Clock Divisor */
#define PLLSYS0_PERCLK_DIV_MASK 0x03000000	/* Peripheral Clk Divisor */
#define PLLSYS0_SEL_MASK	0x18000000	/* 0 = PLL, 1 = PerClk */
#elif !defined (CONFIG_440GX) && \
    !defined(CONFIG_440EP) && !defined(CONFIG_440GR) && \
    !defined(CONFIG_440EPX) && !defined(CONFIG_440GRX) && \
    !defined(CONFIG_440SP) && !defined(CONFIG_440SPE)
#define PLLSYS0_TUNE_MASK	0xffc00000	/* PLL TUNE bits	    */
#define PLLSYS0_FB_DIV_MASK	0x003c0000	/* Feedback divisor	    */
#define PLLSYS0_FWD_DIV_A_MASK	0x00038000	/* Forward divisor A	    */
#define PLLSYS0_FWD_DIV_B_MASK	0x00007000	/* Forward divisor B	    */
#define PLLSYS0_OPB_DIV_MASK	0x00000c00	/* OPB divisor		    */
#define PLLSYS0_EPB_DIV_MASK	0x00000300	/* EPB divisor		    */
#define PLLSYS0_EXTSL_MASK	0x00000080	/* PerClk feedback path	    */
#define PLLSYS0_RW_MASK		0x00000060	/* ROM width		    */
#define PLLSYS0_RL_MASK		0x00000010	/* ROM location		    */
#define PLLSYS0_ZMII_SEL_MASK	0x0000000c	/* ZMII selection	    */
#define PLLSYS0_BYPASS_MASK	0x00000002	/* Bypass PLL		    */
#define PLLSYS0_NTO1_MASK	0x00000001	/* CPU:PLB N-to-1 ratio	    */

#define PLL_VCO_FREQ_MIN	500		/* Min VCO freq (MHz)	    */
#define PLL_VCO_FREQ_MAX	1000		/* Max VCO freq (MHz)	    */
#define PLL_CPU_FREQ_MAX	400		/* Max CPU freq (MHz)	    */
#define PLL_PLB_FREQ_MAX	133		/* Max PLB freq (MHz)	    */
#else /* !CONFIG_440GX or CONFIG_440EP or CONFIG_440GR */
#define PLLSYS0_ENG_MASK	0x80000000	/* 0 = SysClk, 1 = PLL VCO */
#define PLLSYS0_SRC_MASK	0x40000000	/* 0 = PLL A, 1 = PLL B */
#define PLLSYS0_SEL_MASK	0x38000000 /* 0 = PLL, 1 = CPU, 5 = PerClk */
#define PLLSYS0_TUNE_MASK	0x07fe0000	/* PLL Tune bits */
#define PLLSYS0_FB_DIV_MASK	0x0001f000	/* Feedback divisor */
#define PLLSYS0_FWD_DIV_A_MASK	0x00000f00	/* Fwd Div A */
#define PLLSYS0_FWD_DIV_B_MASK	0x000000e0	/* Fwd Div B */
#define PLLSYS0_PRI_DIV_B_MASK	0x0000001c	/* PLL Primary Divisor B */
#define PLLSYS0_OPB_DIV_MASK	0x00000003	/* OPB Divisor */

#define PLLC_ENG_MASK       0x20000000  /* PLL primary forward divisor source */
#define PLLC_SRC_MASK       0x20000000  /* PLL feedback source   */
#define PLLD_FBDV_MASK      0x1f000000  /* PLL Feedback Divisor  */
#define PLLD_FWDVA_MASK     0x000f0000  /* PLL Forward Divisor A */
#define PLLD_FWDVB_MASK     0x00000700  /* PLL Forward Divisor B */
#define PLLD_LFBDV_MASK     0x0000003f  /* PLL Local Feedback Divisor */

#define OPBDDV_MASK         0x03000000  /* OPB Clock Divisor Register */
#define PERDV_MASK          0x07000000  /* Periferal Clock Divisor */
#define PRADV_MASK          0x07000000  /* Primary Divisor A */
#define PRBDV_MASK          0x07000000  /* Primary Divisor B */
#define SPCID_MASK          0x03000000  /* Sync PCI Divisor  */

#define PLL_VCO_FREQ_MIN	500		/* Min VCO freq (MHz)	    */
#define PLL_VCO_FREQ_MAX	1000		/* Max VCO freq (MHz)	    */
#define PLL_CPU_FREQ_MAX	400		/* Max CPU freq (MHz)	    */
#define PLL_PLB_FREQ_MAX	133		/* Max PLB freq (MHz)	    */

/* Strap 1 Register */
#define PLLSYS1_LF_DIV_MASK	0xfc000000	/* PLL Local Feedback Divisor */
#define PLLSYS1_PERCLK_DIV_MASK 0x03000000	/* Peripheral Clk Divisor */
#define PLLSYS1_MAL_DIV_MASK	0x00c00000	/* MAL Clk Divisor */
#define PLLSYS1_RW_MASK		0x00300000	/* ROM width */
#define PLLSYS1_EAR_MASK	0x00080000	/* ERAP Addres reset vector */
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
#endif /* CONFIG_440GX */

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define CPR0_ICFG_RLI_MASK	0x80000000
#define CPR0_ICFG_ICS_MASK	0x00000007
#define CPR0_SPCID_SPCIDV0_MASK	0x03000000
#define CPR0_SPCID_SPCIDV0_DIV1	0x01000000
#define CPR0_SPCID_SPCIDV0_DIV2	0x02000000
#define CPR0_SPCID_SPCIDV0_DIV3	0x03000000
#define CPR0_SPCID_SPCIDV0_DIV4	0x00000000
#define CPR0_PERD_PERDV0_MASK	0x07000000
#endif

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define CPR0_ICFG_RLI_MASK	0x80000000

#define CPR0_PLLC_RST		0x80000000
#define CPR0_PLLC_ENG		0x40000000
#endif

/*-----------------------------------------------------------------------------
| PCI Internal Registers et. al. (accessed via plb)
+----------------------------------------------------------------------------*/
#define PCIL0_CFGADR		(CONFIG_SYS_PCI_BASE + 0x0ec00000)
#define PCIL0_CFGDATA		(CONFIG_SYS_PCI_BASE + 0x0ec00004)
#define PCIL0_CFGBASE		(CONFIG_SYS_PCI_BASE + 0x0ec80000)
#define PCIL0_IOBASE		(CONFIG_SYS_PCI_BASE + 0x08000000)

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

/* PCI Local Configuration Registers
   --------------------------------- */
#define PCI_MMIO_LCR_BASE (CONFIG_SYS_PCI_BASE + 0x0f400000) /* Real =>
					      0x0EF400000 */

/* PCI Master Local Configuration Registers */
#define PCIL0_PMM0LA	(PCI_MMIO_LCR_BASE + 0x00) /* PMM0 Local Address */
#define PCIL0_PMM0MA	(PCI_MMIO_LCR_BASE + 0x04) /* PMM0 Mask/Attribute */
#define PCIL0_PMM0PCILA	(PCI_MMIO_LCR_BASE + 0x08) /* PMM0 PCI Low Address */
#define PCIL0_PMM0PCIHA	(PCI_MMIO_LCR_BASE + 0x0C) /* PMM0 PCI High Address */
#define PCIL0_PMM1LA	(PCI_MMIO_LCR_BASE + 0x10) /* PMM1 Local Address */
#define PCIL0_PMM1MA	(PCI_MMIO_LCR_BASE + 0x14) /* PMM1 Mask/Attribute */
#define PCIL0_PMM1PCILA	(PCI_MMIO_LCR_BASE + 0x18) /* PMM1 PCI Low Address */
#define PCIL0_PMM1PCIHA	(PCI_MMIO_LCR_BASE + 0x1C) /* PMM1 PCI High Address */
#define PCIL0_PMM2LA	(PCI_MMIO_LCR_BASE + 0x20) /* PMM2 Local Address */
#define PCIL0_PMM2MA	(PCI_MMIO_LCR_BASE + 0x24) /* PMM2 Mask/Attribute */
#define PCIL0_PMM2PCILA	(PCI_MMIO_LCR_BASE + 0x28) /* PMM2 PCI Low Address */
#define PCIL0_PMM2PCIHA	(PCI_MMIO_LCR_BASE + 0x2C) /* PMM2 PCI High Address */

/* PCI Target Local Configuration Registers */
#define PCIL0_PTM1MS	(PCI_MMIO_LCR_BASE + 0x30) /* PTM1 Memory Size/
						      Attribute */
#define PCIL0_PTM1LA	(PCI_MMIO_LCR_BASE + 0x34) /* PTM1 Local Addr. Reg */
#define PCIL0_PTM2MS	(PCI_MMIO_LCR_BASE + 0x38) /* PTM2 Memory Size/
						      Attribute */
#define PCIL0_PTM2LA	(PCI_MMIO_LCR_BASE + 0x3C) /* PTM2 Local Addr. Reg */

#else

#define PCIL0_VENDID		(PCIL0_CFGBASE + PCI_VENDOR_ID )
#define PCIL0_DEVID		(PCIL0_CFGBASE + PCI_DEVICE_ID )
#define PCIL0_CMD		(PCIL0_CFGBASE + PCI_COMMAND )
#define PCIL0_STATUS		(PCIL0_CFGBASE + PCI_STATUS )
#define PCIL0_REVID		(PCIL0_CFGBASE + PCI_REVISION_ID )
#define PCIL0_CLS		(PCIL0_CFGBASE + PCI_CLASS_CODE)
#define PCIL0_CACHELS		(PCIL0_CFGBASE + PCI_CACHE_LINE_SIZE )
#define PCIL0_LATTIM		(PCIL0_CFGBASE + PCI_LATENCY_TIMER )
#define PCIL0_HDTYPE		(PCIL0_CFGBASE + PCI_HEADER_TYPE )
#define PCIL0_BIST		(PCIL0_CFGBASE + PCI_BIST )
#define PCIL0_BAR0		(PCIL0_CFGBASE + PCI_BASE_ADDRESS_0 )
#define PCIL0_BAR1		(PCIL0_CFGBASE + PCI_BASE_ADDRESS_1 )
#define PCIL0_BAR2		(PCIL0_CFGBASE + PCI_BASE_ADDRESS_2 )
#define PCIL0_BAR3		(PCIL0_CFGBASE + PCI_BASE_ADDRESS_3 )
#define PCIL0_BAR4		(PCIL0_CFGBASE + PCI_BASE_ADDRESS_4 )
#define PCIL0_BAR5		(PCIL0_CFGBASE + PCI_BASE_ADDRESS_5 )
#define PCIL0_CISPTR		(PCIL0_CFGBASE + PCI_CARDBUS_CIS )
#define PCIL0_SBSYSVID		(PCIL0_CFGBASE + PCI_SUBSYSTEM_VENDOR_ID )
#define PCIL0_SBSYSID		(PCIL0_CFGBASE + PCI_SUBSYSTEM_ID )
#define PCIL0_EROMBA		(PCIL0_CFGBASE + PCI_ROM_ADDRESS )
#define PCIL0_CAP		(PCIL0_CFGBASE + PCI_CAPABILITY_LIST )
#define PCIL0_RES0		(PCIL0_CFGBASE + 0x0035 )
#define PCIL0_RES1		(PCIL0_CFGBASE + 0x0036 )
#define PCIL0_RES2		(PCIL0_CFGBASE + 0x0038 )
#define PCIL0_INTLN		(PCIL0_CFGBASE + PCI_INTERRUPT_LINE )
#define PCIL0_INTPN		(PCIL0_CFGBASE + PCI_INTERRUPT_PIN )
#define SDR0_EMACxTXST_FUR	0x02000000	/* TX FIFO underrun */
#define SDR0_EMACxTXST_BC	0x01000000	/* broadcase address */
#define SDR0_EMACxTXST_MC	0x00800000	/* multicast address */
#define SDR0_EMACxTXST_UC	0x00400000	/* unicast address */
#define SDR0_EMACxTXST_FP	0x00200000 /* frame paused by control packet */
#define SDR0_EMACxTXST_BFCS	0x00100000 /* bad FCS in the transmitted frame*/
#define SDR0_EMACxTXST_CPF	0x00080000	/* TX control pause frame */
#define SDR0_EMACxTXST_CF	0x00040000	/* TX control frame */
#define SDR0_EMACxTXST_MSIZ	0x00020000 /* 1024-maxsize bytes transmitted */
#define SDR0_EMACxTXST_1023	0x00010000	/* 512-1023 bytes transmitted */
#define SDR0_EMACxTXST_511	0x00008000	/* 256-511 bytes transmitted */
#define SDR0_EMACxTXST_255	0x00004000	/* 128-255 bytes transmitted */
#define SDR0_EMACxTXST_127	0x00002000	/* 65-127 bytes transmitted */
#define SDR0_EMACxTXST_64	0x00001000	/* 64 bytes transmitted */
#define SDR0_EMACxTXST_SQE	0x00000800	/* SQE indication */
#define SDR0_EMACxTXST_LOC	0x00000400	/* loss of carrier sense */
#define SDR0_EMACxTXST_IERR	0x00000080	/* EMAC internal error */
#define SDR0_EMACxTXST_EDF	0x00000040	/* excessive deferral */
#define SDR0_EMACxTXST_ECOL	0x00000020	/* excessive collisions */
#define SDR0_EMACxTXST_LCOL	0x00000010	/* late collision */
#define SDR0_EMACxTXST_DFFR	0x00000008	/* deferred frame */
#define SDR0_EMACxTXST_MCOL	0x00000004	/* multiple collision frame */
#define SDR0_EMACxTXST_SCOL	0x00000002	/* single collision frame */
#define SDR0_EMACxTXST_TXOK	0x00000001	/* transmit OK */

#define PCIL0_MINGNT		(PCIL0_CFGBASE + PCI_MIN_GNT )
#define PCIL0_MAXLTNCY		(PCIL0_CFGBASE + PCI_MAX_LAT )

#define PCIL0_BRDGOPT1		(PCIL0_CFGBASE + 0x0040)
#define PCIL0_BRDGOPT2		(PCIL0_CFGBASE + 0x0044)

#define PCIL0_POM0LAL		(PCIL0_CFGBASE + 0x0068)
#define PCIL0_POM0LAH		(PCIL0_CFGBASE + 0x006c)
#define PCIL0_POM0SA		(PCIL0_CFGBASE + 0x0070)
#define PCIL0_POM0PCIAL		(PCIL0_CFGBASE + 0x0074)
#define PCIL0_POM0PCIAH		(PCIL0_CFGBASE + 0x0078)
#define PCIL0_POM1LAL		(PCIL0_CFGBASE + 0x007c)
#define PCIL0_POM1LAH		(PCIL0_CFGBASE + 0x0080)
#define PCIL0_POM1SA		(PCIL0_CFGBASE + 0x0084)
#define PCIL0_POM1PCIAL		(PCIL0_CFGBASE + 0x0088)
#define PCIL0_POM1PCIAH		(PCIL0_CFGBASE + 0x008c)
#define PCIL0_POM2SA		(PCIL0_CFGBASE + 0x0090)

#define PCIL0_PIM0SA		(PCIL0_CFGBASE + 0x0098)
#define PCIL0_PIM0LAL		(PCIL0_CFGBASE + 0x009c)
#define PCIL0_PIM0LAH		(PCIL0_CFGBASE + 0x00a0)
#define PCIL0_PIM1SA		(PCIL0_CFGBASE + 0x00a4)
#define PCIL0_PIM1LAL		(PCIL0_CFGBASE + 0x00a8)
#define PCIL0_PIM1LAH		(PCIL0_CFGBASE + 0x00ac)
#define PCIL0_PIM2SA		(PCIL0_CFGBASE + 0x00b0)
#define PCIL0_PIM2LAL		(PCIL0_CFGBASE + 0x00b4)
#define PCIL0_PIM2LAH		(PCIL0_CFGBASE + 0x00b8)

#define PCIL0_STS		(PCIL0_CFGBASE + 0x00e0)

#endif /* !defined(CONFIG_440EP) !defined(CONFIG_440GR) */

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

/* USB2.0 Device */
#define USB2D0_BASE         CONFIG_SYS_USB2D0_BASE

#define USB2D0_INTRIN       (USB2D0_BASE + 0x00000000)

#define USB2D0_INTRIN       (USB2D0_BASE + 0x00000000) /* Interrupt register for
				Endpoint 0 plus IN Endpoints 1 to 3 */
#define USB2D0_POWER        (USB2D0_BASE + 0x00000000) /* Power management
				register */
#define USB2D0_FADDR        (USB2D0_BASE + 0x00000000) /* Function address
				register */
#define USB2D0_INTRINE      (USB2D0_BASE + 0x00000000) /* Interrupt enable
				register for USB2D0_INTRIN */
#define USB2D0_INTROUT      (USB2D0_BASE + 0x00000000) /* Interrupt register for
				OUT Endpoints 1 to 3 */
#define USB2D0_INTRUSBE     (USB2D0_BASE + 0x00000000) /* Interrupt enable
				register for USB2D0_INTRUSB */
#define USB2D0_INTRUSB      (USB2D0_BASE + 0x00000000) /* Interrupt register for
				common USB interrupts */
#define USB2D0_INTROUTE     (USB2D0_BASE + 0x00000000) /* Interrupt enable
				register for IntrOut */
#define USB2D0_TSTMODE      (USB2D0_BASE + 0x00000000) /* Enables the USB 2.0
				test modes */
#define USB2D0_INDEX        (USB2D0_BASE + 0x00000000) /* Index register for
			     selecting the Endpoint status/control registers */
#define USB2D0_FRAME        (USB2D0_BASE + 0x00000000) /* Frame number */
#define USB2D0_INCSR0       (USB2D0_BASE + 0x00000000) /* Control Status
	  register for Endpoint 0. (Index register set to select Endpoint 0) */
#define USB2D0_INCSR        (USB2D0_BASE + 0x00000000) /* Control Status
       register for IN Endpoint. (Index register set to select Endpoints 13) */
#define USB2D0_INMAXP       (USB2D0_BASE + 0x00000000) /* Maximum packet
	   size for IN Endpoint. (Index register set to select Endpoints 13) */
#define USB2D0_OUTCSR       (USB2D0_BASE + 0x00000000) /* Control Status
      register for OUT Endpoint. (Index register set to select Endpoints 13) */
#define USB2D0_OUTMAXP      (USB2D0_BASE + 0x00000000) /* Maximum packet
	  size for OUT Endpoint. (Index register set to select Endpoints 13) */
#define USB2D0_OUTCOUNT0    (USB2D0_BASE + 0x00000000) /* Number of received
	 bytes in Endpoint 0 FIFO. (Index register set to select Endpoint 0) */
#define USB2D0_OUTCOUNT     (USB2D0_BASE + 0x00000000) /* Number of bytes in
	      OUT Endpoint FIFO. (Index register set to select Endpoints 13) */
#endif

/******************************************************************************
 * GPIO macro register defines
 ******************************************************************************/
#if defined(CONFIG_440GP) || defined(CONFIG_440GX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460SX)
#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE+0x00000700)

#define GPIO0_OR		(GPIO0_BASE+0x0)
#define GPIO0_TCR		(GPIO0_BASE+0x4)
#define GPIO0_ODR		(GPIO0_BASE+0x18)
#define GPIO0_IR		(GPIO0_BASE+0x1C)
#endif /* CONFIG_440GP */

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE+0x00000B00)
#define GPIO1_BASE		(CONFIG_SYS_PERIPHERAL_BASE+0x00000C00)

#define GPIO0_OR		(GPIO0_BASE+0x0)
#define GPIO0_TCR		(GPIO0_BASE+0x4)
#define GPIO0_OSRL		(GPIO0_BASE+0x8)
#define GPIO0_OSRH		(GPIO0_BASE+0xC)
#define GPIO0_TSRL		(GPIO0_BASE+0x10)
#define GPIO0_TSRH		(GPIO0_BASE+0x14)
#define GPIO0_ODR		(GPIO0_BASE+0x18)
#define GPIO0_IR		(GPIO0_BASE+0x1C)
#define GPIO0_RR1		(GPIO0_BASE+0x20)
#define GPIO0_RR2		(GPIO0_BASE+0x24)
#define GPIO0_RR3		(GPIO0_BASE+0x28)
#define GPIO0_ISR1L		(GPIO0_BASE+0x30)
#define GPIO0_ISR1H		(GPIO0_BASE+0x34)
#define GPIO0_ISR2L		(GPIO0_BASE+0x38)
#define GPIO0_ISR2H		(GPIO0_BASE+0x3C)
#define GPIO0_ISR3L		(GPIO0_BASE+0x40)
#define GPIO0_ISR3H		(GPIO0_BASE+0x44)

#define GPIO1_OR		(GPIO1_BASE+0x0)
#define GPIO1_TCR		(GPIO1_BASE+0x4)
#define GPIO1_OSRL		(GPIO1_BASE+0x8)
#define GPIO1_OSRH		(GPIO1_BASE+0xC)
#define GPIO1_TSRL		(GPIO1_BASE+0x10)
#define GPIO1_TSRH		(GPIO1_BASE+0x14)
#define GPIO1_ODR		(GPIO1_BASE+0x18)
#define GPIO1_IR		(GPIO1_BASE+0x1C)
#define GPIO1_RR1		(GPIO1_BASE+0x20)
#define GPIO1_RR2		(GPIO1_BASE+0x24)
#define GPIO1_RR3		(GPIO1_BASE+0x28)
#define GPIO1_ISR1L		(GPIO1_BASE+0x30)
#define GPIO1_ISR1H		(GPIO1_BASE+0x34)
#define GPIO1_ISR2L		(GPIO1_BASE+0x38)
#define GPIO1_ISR2H		(GPIO1_BASE+0x3C)
#define GPIO1_ISR3L		(GPIO1_BASE+0x40)
#define GPIO1_ISR3H		(GPIO1_BASE+0x44)
#endif

#ifndef __ASSEMBLY__

#endif	/* _ASMLANGUAGE */

#endif	/* __PPC440_H__ */
