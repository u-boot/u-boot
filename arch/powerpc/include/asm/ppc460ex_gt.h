/*
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PPC460EX_GT_H_
#define _PPC460EX_GT_H_

#define CONFIG_SDRAM_PPC4xx_IBM_DDR2	/* IBM DDR(2) controller */

#define CONFIG_NAND_NDFC

/*
 * Some SoC specific registers
 */

/* Memory mapped registers */
#define CONFIG_SYS_PERIPHERAL_BASE	0xef600000 /* Internal Peripherals */

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x0300)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_PERIPHERAL_BASE + 0x0400)
#define CONFIG_SYS_NS16550_COM3	(CONFIG_SYS_PERIPHERAL_BASE + 0x0500)
#define CONFIG_SYS_NS16550_COM4	(CONFIG_SYS_PERIPHERAL_BASE + 0x0600)

#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0b00)
#define GPIO1_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0c00)

/* DCR */
#define AHB_TOP			0x00a4
#define AHB_BOT			0x00a5

/* SDR */
#define SDR0_PCI0		0x01c0
#define SDR0_AHB_CFG		0x0370
#define SDR0_USB2HOST_CFG	0x0371
#define SDR0_ETH_PLL		0x4102
#define SDR0_ETH_CFG		0x4103
#define SDR0_ETH_STS		0x4104

/*
 * Register bits and masks
 */
#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 13)
#define SDR0_SDSTP1_PISE_MASK		(0x80000000 >> 15)

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

/* Ethernet PLL Configuration Register (SDR0_ETH_PLL) */
#define SDR0_ETH_PLL_PLLLOCK	0x80000000	/* Ethernet PLL lock indication */

/* Ethernet Configuration Register (SDR0_ETH_CFG) */
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
#define SDR0_ETH_CFG_GMC1_BRIDGE_SEL	0x00000002 /*GMC Port 1 bridge
						     selector */
#define SDR0_ETH_CFG_GMC0_BRIDGE_SEL	0x00000001 /*GMC Port 0 bridge
						    selector */

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

#define PLLSYS0_FWD_DIV_A_MASK	0x000000f0	/* Fwd Div A */
#define PLLSYS0_FWD_DIV_B_MASK	0x0000000f	/* Fwd Div B */
#define PLLSYS0_FB_DIV_MASK	0x0000ff00	/* Feedback divisor */
#define PLLSYS0_OPB_DIV_MASK	0x0c000000	/* OPB Divisor */
#define PLLSYS0_PLBEDV0_DIV_MASK 0xe0000000	/* PLB Early Clock Divisor */
#define PLLSYS0_PERCLK_DIV_MASK 0x03000000	/* Peripheral Clk Divisor */
#define PLLSYS0_SEL_MASK	0x18000000	/* 0 = PLL, 1 = PerClk */

#define CPR0_ICFG_RLI_MASK	0x80000000

#define CPR0_PLLC_RST		0x80000000
#define CPR0_PLLC_ENG		0x40000000

#define PCIL0_BRDGOPT1		(PCIL0_CFGBASE + 0x0040)
#define PCIL0_BRDGOPT2		(PCIL0_CFGBASE + 0x0044)

#endif /* _PPC460EX_GT_H_ */
