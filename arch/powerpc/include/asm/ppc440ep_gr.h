/*
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PPC440EP_GR_H_
#define _PPC440EP_GR_H_

#define CONFIG_SDRAM_PPC4xx_IBM_DDR	/* IBM DDR controller */

#define CONFIG_NAND_NDFC

/*
 * Some SoC specific registers (not common for all 440 SoC's)
 */

/* Memory mapped registers */
#define CONFIG_SYS_PERIPHERAL_BASE	0xef600000 /* Internal Peripherals */

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x0300)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_PERIPHERAL_BASE + 0x0400)
#define CONFIG_SYS_NS16550_COM3	(CONFIG_SYS_PERIPHERAL_BASE + 0x0500)
#define CONFIG_SYS_NS16550_COM4	(CONFIG_SYS_PERIPHERAL_BASE + 0x0600)

#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0b00)
#define GPIO1_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0c00)

/* SDR's */
#define SDR0_PCI0	0x0300
#define SDR0_SDSTP2	0x4001
#define SDR0_SDSTP3	0x4003

#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 21)
#define SDR0_SDSTP1_PAME_MASK		(0x80000000 >> 27)

/* Pin Function Control Register 1 */
#define SDR0_PFC1_U1ME_MASK		0x02000000 /* UART1 Mode Enable */
#define SDR0_PFC1_U1ME_DSR_DTR		0x00000000 /* UART1 in DSR/DTR Mode */
#define SDR0_PFC1_U1ME_CTS_RTS		0x02000000 /* UART1 in CTS/RTS Mode */
#define SDR0_PFC1_U0ME_MASK		0x00080000 /* UART0 Mode Enable */
#define SDR0_PFC1_U0ME_DSR_DTR		0x00000000 /* UART0 in DSR/DTR Mode */
#define SDR0_PFC1_U0ME_CTS_RTS		0x00080000 /* UART0 in CTS/RTS Mode */
#define SDR0_PFC1_U0IM_MASK		0x00040000 /* UART0 Interface Mode */
#define SDR0_PFC1_U0IM_8PINS		0x00000000 /* UART0 Interface Mode 8 pins */
#define SDR0_PFC1_U0IM_4PINS		0x00040000 /* UART0 Interface Mode 4 pins */
#define SDR0_PFC1_SIS_MASK		0x00020000 /* SCP or IIC1 Selection */
#define SDR0_PFC1_SIS_SCP_SEL		0x00000000 /* SCP Selected */
#define SDR0_PFC1_SIS_IIC1_SEL		0x00020000 /* IIC1 Selected */
#define SDR0_PFC1_UES_MASK		0x00010000 /* USB2D_RX_Active / EBC_Hold
						      Req Selection */
#define SDR0_PFC1_UES_USB2D_SEL		0x00000000 /* USB2D_RX_Active Selected */
#define SDR0_PFC1_UES_EBCHR_SEL		0x00010000 /* EBC_Hold Req Selected */
#define SDR0_PFC1_DIS_MASK		0x00008000 /* DMA_Req(1) / UIC_IRQ(5)
						      Selection */
#define SDR0_PFC1_DIS_DMAR_SEL		0x00000000 /* DMA_Req(1) Selected */
#define SDR0_PFC1_DIS_UICIRQ5_SEL	0x00008000 /* UIC_IRQ(5) Selected */
#define SDR0_PFC1_ERE_MASK		0x00004000 /* EBC Mast.Ext.Req.En./GPIO0(27)
						      Selection */
#define SDR0_PFC1_ERE_EXTR_SEL		0x00000000 /* EBC Mast.Ext.Req.En.
						      Selected */
#define SDR0_PFC1_ERE_GPIO0_27_SEL	0x00004000 /* GPIO0(27) Selected */
#define SDR0_PFC1_UPR_MASK		0x00002000 /* USB2 Device Packet Reject
						      Selection */
#define SDR0_PFC1_UPR_DISABLE		0x00000000 /* USB2 Device Packet Reject
						      Disable */
#define SDR0_PFC1_UPR_ENABLE		0x00002000 /* USB2 Device Packet Reject
						      Enable */
#define SDR0_PFC1_PLB_PME_MASK		0x00001000 /* PLB3/PLB4 Perf. Monitor Enable
						      Selection */
#define SDR0_PFC1_PLB_PME_PLB3_SEL	0x00000000 /* PLB3 Performance Monitor
						      Enable */
#define SDR0_PFC1_PLB_PME_PLB4_SEL	0x00001000 /* PLB3 Performance Monitor
						      Enable */
#define SDR0_PFC1_GFGGI_MASK		0x0000000F /* GPT Frequency Generation
						      Gated In */

/* USB Control Register */
#define SDR0_USB0_USB_DEVSEL_MASK	0x00000002 /* USB Device Selection */
#define SDR0_USB0_USB20D_DEVSEL		0x00000000 /* USB2.0 Device Selected */
#define SDR0_USB0_USB11D_DEVSEL		0x00000002 /* USB1.1 Device Selected */
#define SDR0_USB0_LEEN_MASK		0x00000001 /* Little Endian selection */
#define SDR0_USB0_LEEN_DISABLE		0x00000000 /* Little Endian Disable */
#define SDR0_USB0_LEEN_ENABLE		0x00000001 /* Little Endian Enable */

/* Miscealleneaous Function Reg. */
#define SDR0_MFR_ETH0_CLK_SEL_MASK	0x08000000 /* Ethernet0 Clock Select */
#define SDR0_MFR_ETH0_CLK_SEL_EXT	0x00000000
#define SDR0_MFR_ETH1_CLK_SEL_MASK	0x04000000 /* Ethernet1 Clock Select */
#define SDR0_MFR_ETH1_CLK_SEL_EXT	0x00000000
#define SDR0_MFR_ZMII_MODE_MASK		0x03000000 /* ZMII Mode Mask */
#define SDR0_MFR_ZMII_MODE_MII		0x00000000 /* ZMII Mode MII */
#define SDR0_MFR_ZMII_MODE_SMII		0x01000000 /* ZMII Mode SMII */
#define SDR0_MFR_ZMII_MODE_RMII_10M	0x02000000 /* ZMII Mode RMII - 10 Mbs */
#define SDR0_MFR_ZMII_MODE_RMII_100M	0x03000000 /* ZMII Mode RMII - 100 Mbs */
#define SDR0_MFR_ZMII_MODE_BIT0		0x02000000 /* ZMII Mode Bit0 */
#define SDR0_MFR_ZMII_MODE_BIT1		0x01000000 /* ZMII Mode Bit1 */
#define SDR0_MFR_ZM_ENCODE(n)		((((u32)(n)) & 0x3) << 24)
#define SDR0_MFR_ZM_DECODE(n)		((((u32)(n)) << 24) & 0x3)

#define SDR0_MFR_ERRATA3_EN0		0x00800000
#define SDR0_MFR_ERRATA3_EN1		0x00400000
#define SDR0_MFR_PKT_REJ_MASK		0x00180000 /* Pkt Rej. Enable Mask */
#define SDR0_MFR_PKT_REJ_EN		0x00180000 /* Pkt Rej. Ena. on both EMAC3 0-1 */
#define SDR0_MFR_PKT_REJ_EN0		0x00100000 /* Pkt Rej. Enable on EMAC3(0) */
#define SDR0_MFR_PKT_REJ_EN1		0x00080000 /* Pkt Rej. Enable on EMAC3(1) */
#define SDR0_MFR_PKT_REJ_POL		0x00200000 /* Packet Reject Polarity */

/* CUST0 Customer Configuration Register0 */
#define SDR0_CUST0_MUX_E_N_G_MASK	0xC0000000 /* Mux_Emac_NDFC_GPIO */
#define SDR0_CUST0_MUX_EMAC_SEL		0x40000000 /* Emac Selection */
#define SDR0_CUST0_MUX_NDFC_SEL		0x80000000 /* NDFC Selection */
#define SDR0_CUST0_MUX_GPIO_SEL		0xC0000000 /* GPIO Selection */

#define SDR0_CUST0_NDFC_EN_MASK		0x20000000 /* NDFC Enable Mask */
#define SDR0_CUST0_NDFC_ENABLE		0x20000000 /* NDFC Enable */
#define SDR0_CUST0_NDFC_DISABLE		0x00000000 /* NDFC Disable */

#define SDR0_CUST0_NDFC_BW_MASK		0x10000000 /* NDFC Boot Width */
#define SDR0_CUST0_NDFC_BW_16_BIT	0x10000000 /* NDFC Boot Width = 16 Bit */
#define SDR0_CUST0_NDFC_BW_8_BIT	0x00000000 /* NDFC Boot Width =  8 Bit */

#define SDR0_CUST0_NDFC_BP_MASK		0x0F000000 /* NDFC Boot Page */
#define SDR0_CUST0_NDFC_BP_ENCODE(n)	((((u32)(n)) & 0xF) << 24)
#define SDR0_CUST0_NDFC_BP_DECODE(n)	((((u32)(n)) >> 24) & 0xF)

#define SDR0_CUST0_NDFC_BAC_MASK	0x00C00000 /* NDFC Boot Address Cycle */
#define SDR0_CUST0_NDFC_BAC_ENCODE(n)	((((u32)(n)) & 0x3) << 22)
#define SDR0_CUST0_NDFC_BAC_DECODE(n)	((((u32)(n)) >> 22) & 0x3)

#define SDR0_CUST0_NDFC_ARE_MASK	0x00200000 /* NDFC Auto Read Enable */
#define SDR0_CUST0_NDFC_ARE_ENABLE	0x00200000 /* NDFC Auto Read Enable */
#define SDR0_CUST0_NDFC_ARE_DISABLE	0x00000000 /* NDFC Auto Read Disable */

#define SDR0_CUST0_NRB_MASK		0x00100000 /* NDFC Ready / Busy */
#define SDR0_CUST0_NRB_BUSY		0x00100000 /* Busy */
#define SDR0_CUST0_NRB_READY		0x00000000 /* Ready */

#define SDR0_CUST0_NDRSC_MASK		0x0000FFF0 /* NDFC Device Reset Count Mask */
#define SDR0_CUST0_NDRSC_ENCODE(n)	((((u32)(n)) & 0xFFF) << 4)
#define SDR0_CUST0_NDRSC_DECODE(n)	((((u32)(n)) >> 4) & 0xFFF)

#define SDR0_CUST0_CHIPSELGAT_MASK	0x0000000F /* Chip Select Gating Mask */
#define SDR0_CUST0_CHIPSELGAT_DIS	0x00000000 /* Chip Select Gating Disable */
#define SDR0_CUST0_CHIPSELGAT_ENALL	0x0000000F /*All Chip Select Gating Enable*/
#define SDR0_CUST0_CHIPSELGAT_EN0	0x00000008 /* Chip Select0 Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN1	0x00000004 /* Chip Select1 Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN2	0x00000002 /* Chip Select2 Gating Enable */
#define SDR0_CUST0_CHIPSELGAT_EN3	0x00000001 /* Chip Select3 Gating Enable */

#define SDR0_SRST_DMC			0x00200000

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

#define CPR0_ICFG_RLI_MASK	0x80000000
#define CPR0_ICFG_ICS_MASK	0x00000007
#define CPR0_SPCID_SPCIDV0_MASK	0x03000000
#define CPR0_SPCID_SPCIDV0_DIV1	0x01000000
#define CPR0_SPCID_SPCIDV0_DIV2	0x02000000
#define CPR0_SPCID_SPCIDV0_DIV3	0x03000000
#define CPR0_SPCID_SPCIDV0_DIV4	0x00000000
#define CPR0_PERD_PERDV0_MASK	0x07000000

#define PCI_MMIO_LCR_BASE	(CONFIG_SYS_PCI_BASE + 0x0f400000) /* Real =>
								      0x0EF400000 */

/* PCI Master Local Configuration Registers */
#define PCIL0_PMM0LA		(PCI_MMIO_LCR_BASE + 0x00) /* PMM0 Local Address */
#define PCIL0_PMM0MA		(PCI_MMIO_LCR_BASE + 0x04) /* PMM0 Mask/Attribute */
#define PCIL0_PMM0PCILA		(PCI_MMIO_LCR_BASE + 0x08) /* PMM0 PCI Low Address */
#define PCIL0_PMM0PCIHA		(PCI_MMIO_LCR_BASE + 0x0C) /* PMM0 PCI High Address */
#define PCIL0_PMM1LA		(PCI_MMIO_LCR_BASE + 0x10) /* PMM1 Local Address */
#define PCIL0_PMM1MA		(PCI_MMIO_LCR_BASE + 0x14) /* PMM1 Mask/Attribute */
#define PCIL0_PMM1PCILA		(PCI_MMIO_LCR_BASE + 0x18) /* PMM1 PCI Low Address */
#define PCIL0_PMM1PCIHA		(PCI_MMIO_LCR_BASE + 0x1C) /* PMM1 PCI High Address */
#define PCIL0_PMM2LA		(PCI_MMIO_LCR_BASE + 0x20) /* PMM2 Local Address */
#define PCIL0_PMM2MA		(PCI_MMIO_LCR_BASE + 0x24) /* PMM2 Mask/Attribute */
#define PCIL0_PMM2PCILA		(PCI_MMIO_LCR_BASE + 0x28) /* PMM2 PCI Low Address */
#define PCIL0_PMM2PCIHA		(PCI_MMIO_LCR_BASE + 0x2C) /* PMM2 PCI High Address */

/* PCI Target Local Configuration Registers */
#define PCIL0_PTM1MS		(PCI_MMIO_LCR_BASE + 0x30) /* PTM1 Memory Size/
							      Attribute */
#define PCIL0_PTM1LA		(PCI_MMIO_LCR_BASE + 0x34) /* PTM1 Local Addr. Reg */
#define PCIL0_PTM2MS		(PCI_MMIO_LCR_BASE + 0x38) /* PTM2 Memory Size/
							      Attribute */
#define PCIL0_PTM2LA		(PCI_MMIO_LCR_BASE + 0x3C) /* PTM2 Local Addr. Reg */

#endif /* _PPC440EP_GR_H_ */
