#ifndef PS7_INIT_HW_H           /* prevent circular inclusions */
#define PS7_INIT_HW_H           /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#define XPS_SYS_CTRL_BASEADDR          0xF8000000    

#define SLCR_BASE_ADDRESS XPS_SYS_CTRL_BASEADDR

/* MIO registers */
#define SLCR_LOCK               (SLCR_BASE_ADDRESS + 0x4)
#define SLCR_UNLOCK             (SLCR_BASE_ADDRESS + 0x8)
#define SLCR_MIO0              	(SLCR_BASE_ADDRESS + 0x700)
#define SLCR_MIO1              	(SLCR_BASE_ADDRESS + 0x704)
#define SLCR_MIO2              	(SLCR_BASE_ADDRESS + 0x708)
#define SLCR_MIO3              	(SLCR_BASE_ADDRESS + 0x70C)
#define SLCR_MIO4              	(SLCR_BASE_ADDRESS + 0x710)
#define SLCR_MIO5              	(SLCR_BASE_ADDRESS + 0x714)
#define SLCR_MIO6              	(SLCR_BASE_ADDRESS + 0x718)
#define SLCR_MIO7              	(SLCR_BASE_ADDRESS + 0x71C)
#define SLCR_MIO8              	(SLCR_BASE_ADDRESS + 0x720)
#define SLCR_MIO9              	(SLCR_BASE_ADDRESS + 0x724)
#define SLCR_MIO10             	(SLCR_BASE_ADDRESS + 0x728)
#define SLCR_MIO11             	(SLCR_BASE_ADDRESS + 0x72C)
#define SLCR_MIO12             	(SLCR_BASE_ADDRESS + 0x730)
#define SLCR_MIO13             	(SLCR_BASE_ADDRESS + 0x734)
#define SLCR_MIO14             	(SLCR_BASE_ADDRESS + 0x738)
#define SLCR_MIO15             	(SLCR_BASE_ADDRESS + 0x73C)
#define SLCR_MIO16             	(SLCR_BASE_ADDRESS + 0x740)
#define SLCR_MIO17             	(SLCR_BASE_ADDRESS + 0x744)
#define SLCR_MIO18             	(SLCR_BASE_ADDRESS + 0x748)
#define SLCR_MIO19             	(SLCR_BASE_ADDRESS + 0x74C)
#define SLCR_MIO20             	(SLCR_BASE_ADDRESS + 0x750)
#define SLCR_MIO21             	(SLCR_BASE_ADDRESS + 0x754)
#define SLCR_MIO22             	(SLCR_BASE_ADDRESS + 0x758)
#define SLCR_MIO23             	(SLCR_BASE_ADDRESS + 0x75C)
#define SLCR_MIO24             	(SLCR_BASE_ADDRESS + 0x760)
#define SLCR_MIO25             	(SLCR_BASE_ADDRESS + 0x764)
#define SLCR_MIO26             	(SLCR_BASE_ADDRESS + 0x768)
#define SLCR_MIO27             	(SLCR_BASE_ADDRESS + 0x76C)
#define SLCR_MIO28             	(SLCR_BASE_ADDRESS + 0x770)
#define SLCR_MIO29             	(SLCR_BASE_ADDRESS + 0x774)
#define SLCR_MIO30             	(SLCR_BASE_ADDRESS + 0x778)
#define SLCR_MIO31             	(SLCR_BASE_ADDRESS + 0x77C)
#define SLCR_MIO32             	(SLCR_BASE_ADDRESS + 0x780)
#define SLCR_MIO33             	(SLCR_BASE_ADDRESS + 0x784)
#define SLCR_MIO34             	(SLCR_BASE_ADDRESS + 0x788)
#define SLCR_MIO35             	(SLCR_BASE_ADDRESS + 0x78C)
#define SLCR_MIO36             	(SLCR_BASE_ADDRESS + 0x790)
#define SLCR_MIO37             	(SLCR_BASE_ADDRESS + 0x794)
#define SLCR_MIO38             	(SLCR_BASE_ADDRESS + 0x798)
#define SLCR_MIO39             	(SLCR_BASE_ADDRESS + 0x79C)
#define SLCR_MIO40             	(SLCR_BASE_ADDRESS + 0x7A0)
#define SLCR_MIO41             	(SLCR_BASE_ADDRESS + 0x7A4)
#define SLCR_MIO42             	(SLCR_BASE_ADDRESS + 0x7A8)
#define SLCR_MIO43             	(SLCR_BASE_ADDRESS + 0x7AC)
#define SLCR_MIO44             	(SLCR_BASE_ADDRESS + 0x7B0)
#define SLCR_MIO45             	(SLCR_BASE_ADDRESS + 0x7B4)
#define SLCR_MIO46             	(SLCR_BASE_ADDRESS + 0x7B8)
#define SLCR_MIO47             	(SLCR_BASE_ADDRESS + 0x7BC)
#define SLCR_MIO48             	(SLCR_BASE_ADDRESS + 0x7C0)
#define SLCR_MIO49             	(SLCR_BASE_ADDRESS + 0x7C4)
#define SLCR_MIO50             	(SLCR_BASE_ADDRESS + 0x7C8)
#define SLCR_MIO51             	(SLCR_BASE_ADDRESS + 0x7CC)
#define SLCR_MIO52             	(SLCR_BASE_ADDRESS + 0x7D0)
#define SLCR_MIO53             	(SLCR_BASE_ADDRESS + 0x7D4)
#define SLCR_SDIO0_WP_CD       	(SLCR_BASE_ADDRESS + 0x830)

/* PLL registers */
#define SLCR_ARM_PLL_CTRL 		(SLCR_BASE_ADDRESS + 0x100) /* ARM PLL Control */
#define SLCR_DDR_PLL_CTRL 		(SLCR_BASE_ADDRESS + 0x104) /* DDR PLL Control */
#define SLCR_IO_PLL_CTRL 		(SLCR_BASE_ADDRESS + 0x108) /* IO PLL Control */
#define SLCR_PLL_STATUS			(SLCR_BASE_ADDRESS + 0x10C) /* PLL Status */
#define SLCR_ARM_PLL_CFG 		(SLCR_BASE_ADDRESS + 0x110) /* ARM PLL Configuration */
#define SLCR_DDR_PLL_CFG 		(SLCR_BASE_ADDRESS + 0x114) /* DDR PLL Configuration */
#define SLCR_IO_PLL_CFG 		(SLCR_BASE_ADDRESS + 0x118) /* IO PLL Configuration */
#define SLCR_PLL_BG_CTRL		(SLCR_BASE_ADDRESS + 0x11C) /* PLL Bandgap control */
#define SLCR_ARM_CLK_CTRL 		(SLCR_BASE_ADDRESS + 0x120) /* CORTEX A9 Clock Control */
#define SLCR_DDR_CLK_CTRL 		(SLCR_BASE_ADDRESS + 0x124) /* DDR Clock Control */
#define SLCR_DCI_CLK_CTRL 		(SLCR_BASE_ADDRESS + 0x128) /* DCI clock control */
#define SLCR_APER_CLK_CTRL 		(SLCR_BASE_ADDRESS + 0x12C) /* AMBA Peripheral Clock Control */
#define SLCR_USB0_CLK_CTRL 		(SLCR_BASE_ADDRESS + 0x130) /* USB 0 ULPI Clock Control */
#define SLCR_USB1_CLK_CTRL 		(SLCR_BASE_ADDRESS + 0x134) /* USB 1 ULPI Clock Control */
#define SLCR_GEM0_RCLK_CTRL		(SLCR_BASE_ADDRESS + 0x138) /* Gigabit Ethernet MAC 0 RX Clock Control */
#define SLCR_GEM1_RCLK_CTRL		(SLCR_BASE_ADDRESS + 0x13C) /* Gigabit Ethernet MAC 0 RX Clock Control */
#define SLCR_GEM0_CLK_CTRL		(SLCR_BASE_ADDRESS + 0x140) /* Gigabit Ethernet MAC 0 Ref Clock Control */
#define SLCR_GEM1_CLK_CTRL		(SLCR_BASE_ADDRESS + 0x144) /* Gigabit Ethernet MAC 1 Ref Clock Control */
#define SLCR_SMC_CLK_CTRL		(SLCR_BASE_ADDRESS + 0x148) /* SMC Reference Clock Control */
#define SLCR_LQSPI_CLK_CTRL		(SLCR_BASE_ADDRESS + 0x14C) /* Linear Quad-SPI Reference Clock Control */
#define SLCR_SDIO_CLK_CTRL		(SLCR_BASE_ADDRESS + 0x150) /* SDIO Reference Clock Control */
#define SLCR_UART_CLK_CTRL		(SLCR_BASE_ADDRESS + 0x154) /* UART Reference Clock Control */
#define SLCR_SPI_CLK_CTRL		(SLCR_BASE_ADDRESS + 0x158) /* SPI Reference Clock Control */
#define SLCR_CAN_CLK_CTRL		(SLCR_BASE_ADDRESS + 0x15C) /* CAN Reference Clock Control */
#define SLCR_CAN_MIO_CLK_CTRL	(SLCR_BASE_ADDRESS + 0x160) /* CAN MIO Clock Control */
#define SLCR_DBG_CLK_CTRL	(SLCR_BASE_ADDRESS + 0x164) /* DBG Clock Control */
#define SLCR_PCAP_CLK_CTRL	(SLCR_BASE_ADDRESS + 0x168) /* PCAP Clock Control */
#define SLCR_TOPSW_CLK_CTRL	(SLCR_BASE_ADDRESS + 0x16C) /* TOPSW Clock Control */
#define SLCR_FPGA0_CLK_CTRL	(SLCR_BASE_ADDRESS + 0x170) /* FPGA0 Clock Control */
#define SLCR_FPGA1_CLK_CTRL	(SLCR_BASE_ADDRESS + 0x180) /* FPGA1 Clock Control */
#define SLCR_FPGA2_CLK_CTRL	(SLCR_BASE_ADDRESS + 0x190) /* FPGA2 Clock Control */
#define SLCR_FPGA3_CLK_CTRL	(SLCR_BASE_ADDRESS + 0x1A0) /* FPGA3 Clock Control */
#define SLCR_PLL_PREDIVISOR		(SLCR_BASE_ADDRESS + 0x1C0) /* PLL pre devisor */
#define SLCR_CLK_621_TRUE   	(SLCR_BASE_ADDRESS + 0x1C4) /* CPU enable the 6:2:1 mode */

/* MIO */
#define MIO_LQSPI			0x02
#define	MIO_USB				0x04
#define	MIO_GEM				0x02
#define	MIO_UART			0xE0
#define	MIO_SPI				0xA0
#define	MIO_CAN				0x20
#define	MIO_I2C				0x40
#define MIO_SDIO			0x80
#define MIO_GPIO			0x00
#define MIO_MDIO0			0x80
#define MIO_MDIO1			0xA0
#define MIO_NAND			0x10
#define MIO_SRAM_NOR		0x08
#define MIO_TTC				0xC0
#define MIO_WDT				0x60
#define MIO_MASK            0xFFF /* IOTYPE SPEED SEL TRI ENABLE Mask */
#define SDIO0_CD_SEL_SHIFT	16

#define TRI_ENABLE_IN				(1 << 0)
#define TRI_ENABLE_OUT				(0 << 0)
#define TRI_ENABLE_IN_OUT			(0 << 0)
#define SLOW_CMOS					(0 << 8)
#define FAST_CMOS					(1 << 8)
#define LVTTL						(0 << 9)
#define LVCMOS18					(1 << 9)
#define LVCMOS25					(2 << 9)
#define LVCMOS33					(3 << 9)
#define HSTL						(4 << 9)
#define PULLUP_ENABLE				(1 << 12)
#define PULLUP_DISABLE				(0 << 12)
#define DISABLE_RCVR				(1 << 13)

/* PLL */
#define PLL_RESET			  				(1 << 0)
#define PLL_PWRDWN			  				(1 << 1)
#define PLL_BYPASS_QUAL		  				(1 << 3)
#define PLL_BYPASS_FORCE	  				(1 << 4)
#define PLL_FDIV_SHIFT						12
#define UPDATE_CLR							(0 << 24)
#define UPDATE_SERVICED						(1 << 25)

#define PLL_RES_SHIFT                       4
#define PLL_CP_SHIFT                        8
#define PLL_LOCK_CNT_SHIFT                  12

/* CPU */
#define CPU_SRCSEL_SHIFT					4
#define CPU_DIVISOR_SHIFT					8
#define CPU_6OR4XCLKACT_ENABLE				(1 << 24)
#define CPU_3OR2XCLKACT_ENABLE				(1 << 25)
#define CPU_2XCLKACT_ENABLE					(1 << 26)
#define	CPU_1XCLKACT_ENABLE					(1 << 27)
#define	CPU_PERI_CLKACT_ENABLE				(1 << 28)

/* DDR */
#define DDR_3XCLKACT_ENABLE					(1 << 0)
#define DDR_2XCLKACT_ENABLE					(1 << 1)
#define DDR_3XCLK_DIVISOR_SHIFT				20
#define DDR_2XCLK_DIVISOR_SHIFT				26

/* DCI */
#define DCI_CLKACT_ENABLE					(1 << 0)
#define DCI_DIVISOR0_SHIFT					8
#define DCI_DIVISOR1_SHIFT					20

/* APER */
#define SLCR_DMA_CPU_2XCLKACT_ENABLE		(1 << 0)
#define SLCR_USB0_CPU_1XCLKACT_ENABLE		(1 << 2)
#define SLCR_USB1_CPU_1XCLKACT_ENABLE		(1 << 3)
#define SLCR_GEM0_CPU_1XCLKACT_ENABLE		(1 << 6)
#define SLCR_GEM1_CPU_1XCLKACT_ENABLE		(1 << 7)
#define SLCR_SDI0_CPU_1XCLKACT_ENABLE		(1 << 10)
#define SLCR_SDI1_CPU_1XCLKACT_ENABLE		(1 << 11)
#define SLCR_SPI0_CPU_1XCLKACT_ENABLE		(1 << 14)
#define SLCR_SPI1_CPU_1XCLKACT_ENABLE		(1 << 15)
#define SLCR_CAN0_CPU_1XCLKACT_ENABLE		(1 << 16)
#define SLCR_CAN1_CPU_1XCLKACT_ENABLE		(1 << 17)
#define SLCR_I2C0_CPU_1XCLKACT_ENABLE		(1 << 18)
#define SLCR_I2C1_CPU_1XCLKACT_ENABLE		(1 << 19)
#define SLCR_UART0_CPU_1XCLKACT_ENABLE		(1 << 20)
#define SLCR_UART1_CPU_1XCLKACT_ENABLE		(1 << 21)
#define SLCR_GPIO_CPU_1XCLKACT_ENABLE		(1 << 22)
#define SLCR_LQSPI_CPU_1XCLKACT_ENABLE		(1 << 23)
#define SLCR_SMC_CPU_1XCLKACT_ENABLE		(1 << 24)

/* PLL source */
#define IO_PLL			0x0
#define ARM_PLL			0x2
#define DDR_PLL			0x3

/* CPU PLL source */
#define CPU_ARM_PLL			0x0
#define CPU_DDR_PLL			0x2
#define CPU_IO_PLL			0x3

/* USB0 */
#define USB0_CLKACT_ENABLE					(1 << 0)
#define USB0_SRCSEL_SHIFT					4
#define USB0_DIVISOR0_SHIFT					8
#define USB0_DIVISOR1_SHIFT					20

/* USB1 */
#define USB1_CLKACT_ENABLE					(1 << 0)
#define USB1_SRCSEL_SHIFT					4
#define USB1_DIVISOR0_SHIFT					8
#define USB1_DIVISOR1_SHIFT					20

/* GEM0 RX */
#define GEM0_RX_CLKACT_ENABLE				(1 << 0)
#define GEM0_RX_SRCSEL_SHIFT				4
#define GEM0_MIO_RX_CLK                     0
#define GEM0_FMIO_RX_CLK                    1
/* GEM1 RX */
#define GEM1_RX_CLKACT_ENABLE				(1 << 0)
#define GEM1_RX_SRCSEL_SHIFT				4
#define GEM1_MIO_RX_CLK                     0
#define GEM1_FMIO_RX_CLK                    1
/* GEM0 */
#define GEM0_CLKACT_ENABLE					(1 << 0)
#define GEM0_SRCSEL_SHIFT					4
#define GEM0_DIVISOR0_SHIFT					8
#define GEM0_DIVISOR1_SHIFT					20

/* GEM1 */
#define GEM1_CLKACT_ENABLE					(1 << 0)
#define GEM1_SRCSEL_SHIFT					4
#define GEM1_DIVISOR0_SHIFT					8
#define GEM1_DIVISOR1_SHIFT					20

/* SMC */
#define SMC_CLKACT_ENABLE					(1 << 0)
#define SMC_SRCSEL_SHIFT					4
#define SMC_DIVISOR_SHIFT					8

/* LQSPI */
#define LQSPI_CLKACT_ENABLE					(1 << 0)
#define LQSPI_SRCSEL_SHIFT					4
#define LQSPI_DIVISOR_SHIFT					8

/* SDIO */
#define SDIO0_CLKACT_ENABLE					(1 << 0)
#define SDIO1_CLKACT_ENABLE					(1 << 1)
#define SDIO_SRCSEL_SHIFT					4
#define SDIO_DIVISOR_SHIFT					8

/* UART */
#define UART0_CLKACT_ENABLE					(1 << 0)
#define UART1_CLKACT_ENABLE					(1 << 1)
#define UART_SRCSEL_SHIFT					4
#define UART_DIVISOR_SHIFT					8

/* SPI */
#define SPI0_CLKACT_ENABLE					(1 << 0)
#define SPI1_CLKACT_ENABLE					(1 << 1)
#define SPI_SRCSEL_SHIFT					4
#define SPI_DIVISOR_SHIFT					8

/* CAN */
#define CAN0_CLKACT_ENABLE					(1 << 0)
#define CAN1_CLKACT_ENABLE					(1 << 1)
#define CAN_SRCSEL_SHIFT					4
#define CAN_DIVISOR0_SHIFT					8
#define CAN_DIVISOR1_SHIFT					20

/* CAN MIO */
#define CAN0_MUX
#define CAN0_REF_SEL
#define CAN1_MUX
#define CAN1_REF_SEL

/* FPGA */
#define FPGA0_SRCSEL_SHIFT					4
#define FPGA0_DIVISOR0_SHIFT					8
#define FPGA0_DIVISOR1_SHIFT					20

/* FPGA */
#define FPGA1_SRCSEL_SHIFT					4
#define FPGA1_DIVISOR0_SHIFT					8
#define FPGA1_DIVISOR1_SHIFT					20

/* FPGA */
#define FPGA2_SRCSEL_SHIFT					4
#define FPGA2_DIVISOR0_SHIFT					8
#define FPGA2_DIVISOR1_SHIFT					20

/* FPGA */
#define FPGA3_SRCSEL_SHIFT					4
#define FPGA3_DIVISOR0_SHIFT					8
#define FPGA3_DIVISOR1_SHIFT					20

/* PCAP */
#define PCAP_CLKACT_ENABLE					(1 << 0)
#define PCAP_SRCSEL_SHIFT					4
#define PCAP_DIVISOR_SHIFT					8

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
