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

#ifndef _PPC440EPX_GRX_H_
#define _PPC440EPX_GRX_H_

#define CONFIG_SDRAM_PPC4xx_DENALI_DDR2	/* Denali DDR(2) controller */

#define CONFIG_NAND_NDFC

/*
 * Some SoC specific registers (not common for all 440 SoC's)
 */

/* Memory mapped registers */
#define CONFIG_SYS_PERIPHERAL_BASE	0xef600000 /* Internal Peripherals */

#define SPI0_MODE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0090)

#define CONFIG_SYS_NS16550_COM1	(CONFIG_SYS_PERIPHERAL_BASE + 0x0300)
#define CONFIG_SYS_NS16550_COM2	(CONFIG_SYS_PERIPHERAL_BASE + 0x0400)
#define CONFIG_SYS_NS16550_COM3	(CONFIG_SYS_PERIPHERAL_BASE + 0x0500)
#define CONFIG_SYS_NS16550_COM4	(CONFIG_SYS_PERIPHERAL_BASE + 0x0600)

#define GPIO0_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0b00)
#define GPIO1_BASE		(CONFIG_SYS_PERIPHERAL_BASE + 0x0c00)

/* DCR */
#define CPM0_ER			0x00b0
#define CPM1_ER			0x00f0
#define PLB3A0_ACR		0x0077
#define PLB4A0_ACR		0x0081
#define PLB4A1_ACR		0x0089
#define OPB2PLB40_BCTRL		0x0350
#define P4P3BO0_CFG		0x0026

/* SDR */
#define SDR0_DDRCFG		0x00e0
#define SDR0_PCI0		0x0300
#define SDR0_SDSTP2		0x4001
#define SDR0_SDSTP3		0x4003
#define SDR0_EMAC0RXST 		0x4301
#define SDR0_EMAC0TXST		0x4302
#define SDR0_CRYP0		0x4500

#define SDR0_SDSTP1_PAE_MASK		(0x80000000 >> 21)
#define SDR0_SDSTP1_PAME_MASK		(0x80000000 >> 27)

/* Pin Function Control Register 1 */
#define SDR0_PFC1_U1ME_MASK		0x02000000 /* UART1 Mode Enable */
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

#define SDR0_PFC2_SELECT_MASK		0xe0000000 /* Ethernet Pin select EMAC1 */
#define SDR0_PFC2_SELECT_CONFIG_1_1	0x60000000 /* 1xMII   using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_1_2	0x00000000 /* 1xMII   using  ZMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_2	0x60000000 /* 1xGMII  using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_3	0x80000000 /* 1xTBI   using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_4	0xa0000000 /* 2xRGMII using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_5	0xc0000000 /* 2xRTBI  using RGMII bridge */
#define SDR0_PFC2_SELECT_CONFIG_6	0x40000000 /* 2xSMII  using  ZMII bridge */

#define SDR0_USB2D0CR	0x0320
#define SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK 0x00000004 /* USB 2.0 Device/EBC
							 Master Selection */
#define SDR0_USB2D0CR_USB2DEV_SELECTION	0x00000004 /* USB 2.0 Device Selection*/
#define SDR0_USB2D0CR_EBC_SELECTION	0x00000000 /* EBC Selection */

#define SDR0_USB2D0CR_USB_DEV_INT_SEL_MASK 0x00000002 /* USB Device Interface
							 Selection */
#define SDR0_USB2D0CR_USB20D_DEVSEL	0x00000000 /* USB2.0 Device Selected */
#define SDR0_USB2D0CR_USB11D_DEVSEL	0x00000002 /* USB1.1 Device Selected */

#define SDR0_USB2D0CR_LEEN_MASK		0x00000001 /* Little Endian selection */
#define SDR0_USB2D0CR_LEEN_DISABLE	0x00000000 /* Little Endian Disable */
#define SDR0_USB2D0CR_LEEN_ENABLE	0x00000001 /* Little Endian Enable */

/* USB2 Host Control Register */
#define SDR0_USB2H0CR		0x0340
#define SDR0_USB2H0CR_WDINT_MASK	0x00000001 /* Host UTMI Word Interface*/
#define SDR0_USB2H0CR_WDINT_8BIT_60MHZ	0x00000000 /* 8-bit/60MHz */
#define SDR0_USB2H0CR_WDINT_16BIT_30MHZ	0x00000001 /* 16-bit/30MHz */
#define SDR0_USB2H0CR_EFLADJ_MASK	0x0000007e /* EHCI Frame Length
						      Adjustment */
/* USB2PHY0 Control Register */
#define SDR0_USB2PHY0CR		0x4103
#define SDR0_USB2PHY0CR_UTMICN_MASK	0x00100000

	/*  PHY UTMI interface connection */
#define SDR0_USB2PHY0CR_UTMICN_DEV	0x00000000 /* Device support */
#define SDR0_USB2PHY0CR_UTMICN_HOST	0x00100000 /* Host support */

#define SDR0_USB2PHY0CR_DWNSTR_MASK	0x00400000 /* Select downstream port mode */
#define SDR0_USB2PHY0CR_DWNSTR_DEV	0x00000000 /* Device */
#define SDR0_USB2PHY0CR_DWNSTR_HOST	0x00400000 /* Host   */

/* VBus detect (Device mode only)  */
#define SDR0_USB2PHY0CR_DVBUS_MASK	0x00800000
/* Pull-up resistance on D+ is disabled */
#define SDR0_USB2PHY0CR_DVBUS_PURDIS	0x00000000
/* Pull-up resistance on D+ is enabled */
#define SDR0_USB2PHY0CR_DVBUS_PUREN	0x00800000

/* PHY UTMI data width and clock select  */
#define SDR0_USB2PHY0CR_WDINT_MASK	0x01000000
#define SDR0_USB2PHY0CR_WDINT_8BIT_60MHZ 0x00000000 /* 8-bit data/60MHz */
#define SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ 0x01000000 /* 16-bit data/30MHz */

#define SDR0_USB2PHY0CR_LOOPEN_MASK	0x02000000 /* Loop back test enable  */
#define SDR0_USB2PHY0CR_LOOP_ENABLE	0x00000000 /* Loop back disabled */
/* Loop back enabled (only test purposes) */
#define SDR0_USB2PHY0CR_LOOP_DISABLE	0x02000000

/* Force XO block on during a suspend  */
#define SDR0_USB2PHY0CR_XOON_MASK	0x04000000
#define SDR0_USB2PHY0CR_XO_ON		0x00000000 /* PHY XO block is powered-on */
/* PHY XO block is powered-off when all ports are suspended */
#define SDR0_USB2PHY0CR_XO_OFF		0x04000000

#define SDR0_USB2PHY0CR_PWRSAV_MASK	0x08000000 /* Select PHY power-save mode  */
#define SDR0_USB2PHY0CR_PWRSAV_OFF	0x00000000 /* Non-power-save mode */
#define SDR0_USB2PHY0CR_PWRSAV_ON	0x08000000 /* Power-save mode. Valid only
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

/* USB2.0 Device */
/*
 * todo: check if this can be completely removed, only used in
 * cpu/ppc4xx/usbdev.c. And offsets are completely wrong. This could
 * never have actually worked. Best probably is to remove this
 * usbdev.c file completely (and these defines).
 */
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

/* Miscealleneaous Function Reg. */
#define SDR0_MFR_ETH0_CLK_SEL_MASK	0x08000000 /* Ethernet0 Clock Select */
#define SDR0_MFR_ETH0_CLK_SEL_EXT	0x00000000
#define SDR0_MFR_ETH1_CLK_SEL_MASK	0x04000000 /* Ethernet1 Clock Select */
#define SDR0_MFR_ETH1_CLK_SEL_EXT	0x00000000
#define SDR0_MFR_ZMII_MODE_MASK		0x03000000 /* ZMII Mode Mask */
#define SDR0_MFR_ZMII_MODE_MII		0x00000000 /* ZMII Mode MII */
#define SDR0_MFR_ZMII_MODE_SMII		0x01000000 /* ZMII Mode SMII */
#define SDR0_MFR_ZMII_MODE_BIT0		0x02000000 /* ZMII Mode Bit0 */
#define SDR0_MFR_ZMII_MODE_BIT1		0x01000000 /* ZMII Mode Bit1 */
#define SDR0_MFR_ZM_ENCODE(n)		((((u32)(n)) & 0x3) << 24)
#define SDR0_MFR_ZM_DECODE(n)		((((u32)(n)) << 24) & 0x3)
#define SDR0_MFR_PKT_REJ_MASK		0x00300000 /* Pkt Rej. Enable Mask */
#define SDR0_MFR_PKT_REJ_EN		0x00300000 /* Pkt Rej. Ena. on both EMAC3 0-1 */
#define SDR0_MFR_PKT_REJ_EN0		0x00200000 /* Pkt Rej. Enable on EMAC3(0) */
#define SDR0_MFR_PKT_REJ_EN1		0x00100000 /* Pkt Rej. Enable on EMAC3(1) */
#define SDR0_MFR_PKT_REJ_POL		0x00080000 /* Packet Reject Polarity */

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

#define SDR0_SRST0_BGO		0x80000000 /* PLB to OPB bridge */
#define SDR0_SRST0_PLB4		0x40000000 /* PLB4 arbiter */
#define SDR0_SRST0_EBC		0x20000000 /* External bus controller */
#define SDR0_SRST0_OPB		0x10000000 /* OPB arbiter */
#define SDR0_SRST0_UART0	0x08000000 /* Universal asynchronous receiver/
					      transmitter 0 */
#define SDR0_SRST0_UART1	0x04000000 /* Universal asynchronous receiver/
					      transmitter 1 */
#define SDR0_SRST0_IIC0		0x02000000 /* Inter integrated circuit 0 */
#define SDR0_SRST0_USB2H	0x01000000 /* USB2.0 Host */
#define SDR0_SRST0_GPIO		0x00800000 /* General purpose I/O */
#define SDR0_SRST0_GPT		0x00400000 /* General purpose timer */
#define SDR0_SRST0_DMC		0x00200000 /* DDR SDRAM memory controller */
#define SDR0_SRST0_PCI		0x00100000 /* PCI */
#define SDR0_SRST0_EMAC0	0x00080000 /* Ethernet media access controller 0 */
#define SDR0_SRST0_EMAC1	0x00040000 /* Ethernet media access controller 1 */
#define SDR0_SRST0_CPM0		0x00020000 /* Clock and power management */
#define SDR0_SRST0_ZMII		0x00010000 /* ZMII bridge */
#define SDR0_SRST0_UIC0		0x00008000 /* Universal interrupt controller 0 */
#define SDR0_SRST0_UIC1		0x00004000 /* Universal interrupt controller 1 */
#define SDR0_SRST0_IIC1		0x00002000 /* Inter integrated circuit 1 */
#define SDR0_SRST0_SCP		0x00001000 /* Serial communications port */
#define SDR0_SRST0_BGI		0x00000800 /* OPB to PLB bridge */
#define SDR0_SRST0_DMA		0x00000400 /* Direct memory access controller */
#define SDR0_SRST0_DMAC		0x00000200 /* DMA channel */
#define SDR0_SRST0_MAL		0x00000100 /* Media access layer */
#define SDR0_SRST0_USB2D	0x00000080 /* USB2.0 device */
#define SDR0_SRST0_GPTR		0x00000040 /* General purpose timer */
#define SDR0_SRST0_P4P3		0x00000010 /* PLB4 to PLB3 bridge */
#define SDR0_SRST0_P3P4		0x00000008 /* PLB3 to PLB4 bridge */
#define SDR0_SRST0_PLB3		0x00000004 /* PLB3 arbiter */
#define SDR0_SRST0_UART2	0x00000002 /* Universal asynchronous receiver/
					      transmitter 2 */
#define SDR0_SRST0_UART3	0x00000001 /* Universal asynchronous receiver/
					      transmitter 3 */

#define SDR0_SRST1_NDFC		0x80000000 /* Nand flash controller */
#define SDR0_SRST1_OPBA1	0x40000000 /* OPB Arbiter attached to PLB4 */
#define SDR0_SRST1_P4OPB0	0x20000000 /* PLB4 to OPB Bridge0 */
#define SDR0_SRST1_PLB42OPB0	SDR0_SRST1_P4OPB0
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

/* 440EPx boot strap options */
#define BOOT_STRAP_OPTION_A	0x00000000
#define BOOT_STRAP_OPTION_B	0x00000001
#define BOOT_STRAP_OPTION_D	0x00000003
#define BOOT_STRAP_OPTION_E	0x00000004

#endif /* _PPC440EPX_GRX_H_ */
