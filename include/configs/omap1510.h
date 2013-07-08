/*
 *
 * BRIEF MODULE DESCRIPTION
 *   OMAP hardware map
 *
 * Copyright (C) 2001 RidgeRun, Inc. (http://www.ridgerun.com)
 * Author: RidgeRun, Inc.
 *	   Greg Lonnon (glonnon@ridgerun.com) or info@ridgerun.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/sizes.h>

/*
  There are 2 sets of general I/O -->
  1. GPIO (shared between ARM & DSP, configured by ARM)
  2. MPUIO which can be used only by the ARM.

  Base address FFFB:5000 is where the ARM accesses the MPUIO control registers
  (see 7.2.2 of the TRM for MPUIO reg definitions).

  Base address E101:5000 is reserved for ARM access of the same MPUIO control
  regs, but via the DSP I/O map.  This address is unavailable on 1510.

  Base address FFFC:E000 is where the ARM accesses the GPIO config registers
  directly via its own peripheral bus.

  Base address E101:E000 is where the ARM can access the same GPIO config
  registers, but the access takes place through the ARM port interface (called
  API or MPUI) via the DSP's peripheral bus (DSP I/O space).

  Therefore, the ARM should setup the GPIO regs thru the FFFC:E000 addresses
  instead of the E101:E000 addresses.  The DSP has only read access of the pin
  control register, so this may explain the inability to write to E101:E018.
  Try accessing pin control reg at FFFC:E018.
 */
#define OMAP1510_GPIO_BASE	 0xfffce000
#define OMAP1510_GPIO_START	 OMAP1510_GPIO_BASE
#define OMAP1510_GPIO_SIZE	 SZ_4K

#define OMAP1510_MCBSP1_BASE	 0xE1011000
#define OMAP1510_MCBSP1_SIZE	 SZ_4K
#define OMAP1510_MCBSP1_START	 0xE1011000

#define OMAP1510_MCBSP2_BASE	 0xFFFB1000

#define OMAP1510_MCBSP3_BASE	 0xE1017000
#define OMAP1510_MCBSP3_SIZE	 SZ_4K
#define OMAP1510_MCBSP3_START	 0xE1017000

/*
 * Where's the flush address (for flushing D and I cache?)
 */
#define FLUSH_BASE		0xdf000000
#define FLUSH_BASE_PHYS 0x00000000

#ifndef __ASSEMBLER__

#define PCIO_BASE		0

/*
 * RAM definitions
 */
#define MAPTOPHYS(a)		((unsigned long)(a) - PAGE_OFFSET)
#define KERNTOPHYS(a)		((unsigned long)(&a))
#define KERNEL_BASE		(0x10008000)
#endif

/* macro to get at IO space when running virtually */
#define IO_ADDRESS(x) ((x))

/* ----------------------------------------------------------------------------
 *  OMAP1510 system registers
 * ----------------------------------------------------------------------------
 */

#define OMAP1510_UART1_BASE	    0xfffb0000	 /* "BLUETOOTH-UART" */
#define OMAP1510_UART2_BASE	    0xfffb0800	 /* "MODEM-UART" */
#define OMAP1510_RTC_BASE	    0xfffb4800	 /* RTC */
#define OMAP1510_UART3_BASE	    0xfffb9800	 /* Shared MPU/DSP UART */
#define OMAP1510_COM_MCBSP2_BASE    0xffff1000	 /* Com McBSP2 */
#define OMAP1510_AUDIO_MCBSP_BASE   0xffff1800	 /* Audio McBSP2 */
#define OMAP1510_ARMIO_BASE	    0xfffb5000	 /* keyboard/gpio */

/*
 * OMAP1510 UART3 Registers
 */

#define OMAP_MPU_UART3_BASE  0xFFFB9800 /* UART3 through MPU bus */

/* UART3 Registers Maping through MPU bus */

#define UART3_RHR	 (OMAP_MPU_UART3_BASE + 0)
#define UART3_THR	 (OMAP_MPU_UART3_BASE + 0)
#define UART3_DLL	 (OMAP_MPU_UART3_BASE + 0)
#define UART3_IER	 (OMAP_MPU_UART3_BASE + 4)
#define UART3_DLH	 (OMAP_MPU_UART3_BASE + 4)
#define UART3_IIR	 (OMAP_MPU_UART3_BASE + 8)
#define UART3_FCR	 (OMAP_MPU_UART3_BASE + 8)
#define UART3_EFR	 (OMAP_MPU_UART3_BASE + 8)
#define UART3_LCR	 (OMAP_MPU_UART3_BASE + 0x0C)
#define UART3_MCR	 (OMAP_MPU_UART3_BASE + 0x10)
#define UART3_XON1_ADDR1 (OMAP_MPU_UART3_BASE + 0x10)
#define UART3_XON2_ADDR2 (OMAP_MPU_UART3_BASE + 0x14)
#define UART3_LSR	 (OMAP_MPU_UART3_BASE + 0x14)
#define UART3_TCR	 (OMAP_MPU_UART3_BASE + 0x18)
#define UART3_MSR	 (OMAP_MPU_UART3_BASE + 0x18)
#define UART3_XOFF1	 (OMAP_MPU_UART3_BASE + 0x18)
#define UART3_XOFF2	 (OMAP_MPU_UART3_BASE + 0x1C)
#define UART3_SPR	 (OMAP_MPU_UART3_BASE + 0x1C)
#define UART3_TLR	 (OMAP_MPU_UART3_BASE + 0x1C)
#define UART3_MDR1	 (OMAP_MPU_UART3_BASE + 0x20)
#define UART3_MDR2	 (OMAP_MPU_UART3_BASE + 0x24)
#define UART3_SFLSR	 (OMAP_MPU_UART3_BASE + 0x28)
#define UART3_TXFLL	 (OMAP_MPU_UART3_BASE + 0x28)
#define UART3_RESUME	 (OMAP_MPU_UART3_BASE + 0x2C)
#define UART3_TXFLH	 (OMAP_MPU_UART3_BASE + 0x2C)
#define UART3_SFREGL	 (OMAP_MPU_UART3_BASE + 0x30)
#define UART3_RXFLL	 (OMAP_MPU_UART3_BASE + 0x30)
#define UART3_SFREGH	 (OMAP_MPU_UART3_BASE + 0x34)
#define UART3_RXFLH	 (OMAP_MPU_UART3_BASE + 0x34)
#define UART3_BLR	 (OMAP_MPU_UART3_BASE + 0x38)
#define UART3_ACREG	 (OMAP_MPU_UART3_BASE + 0x3C)
#define UART3_DIV16	 (OMAP_MPU_UART3_BASE + 0x3C)
#define UART3_SCR	 (OMAP_MPU_UART3_BASE + 0x40)
#define UART3_SSR	 (OMAP_MPU_UART3_BASE + 0x44)
#define UART3_EBLR	 (OMAP_MPU_UART3_BASE + 0x48)
#define UART3_OSC_12M_SEL (OMAP_MPU_UART3_BASE + 0x4C)
#define UART3_MVR	 (OMAP_MPU_UART3_BASE + 0x50)

/*
 * Configuration Registers
 */
#define FUNC_MUX_CTRL_0		0xfffe1000
#define FUNC_MUX_CTRL_1		0xfffe1004
#define FUNC_MUX_CTRL_2		0xfffe1008
#define COMP_MODE_CTRL_0	0xfffe100c
#define FUNC_MUX_CTRL_3		0xfffe1010
#define FUNC_MUX_CTRL_4		0xfffe1014
#define FUNC_MUX_CTRL_5		0xfffe1018
#define FUNC_MUX_CTRL_6		0xfffe101C
#define FUNC_MUX_CTRL_7		0xfffe1020
#define FUNC_MUX_CTRL_8		0xfffe1024
#define FUNC_MUX_CTRL_9		0xfffe1028
#define FUNC_MUX_CTRL_A		0xfffe102C
#define FUNC_MUX_CTRL_B		0xfffe1030
#define FUNC_MUX_CTRL_C		0xfffe1034
#define FUNC_MUX_CTRL_D		0xfffe1038
#define PULL_DWN_CTRL_0		0xfffe1040
#define PULL_DWN_CTRL_1		0xfffe1044
#define PULL_DWN_CTRL_2		0xfffe1048
#define PULL_DWN_CTRL_3		0xfffe104c
#define GATE_INH_CTRL_0		0xfffe1050
#define VOLTAGE_CTRL_0		0xfffe1060
#define TEST_DBG_CTRL_0		0xfffe1070

#define MOD_CONF_CTRL_0		0xfffe1080

#ifdef CONFIG_OMAP1610 /* 1610 Configuration Register */

#define USB_OTG_CTRL			0xFFFB040C
#define USB_TRANSCEIVER_CTRL	0xFFFE1064
#define PULL_DWN_CTRL_4 0xFFFE10AC
#define PU_PD_SEL_0		0xFFFE10B4
#define PU_PD_SEL_1		0xFFFE10B8
#define PU_PD_SEL_2		0xFFFE10BC
#define PU_PD_SEL_3		0xFFFE10C0
#define PU_PD_SEL_4		0xFFFE10C4

#endif
/*
 * Traffic Controller Memory Interface Registers
 */
#define TCMIF_BASE		0xfffecc00
#define IMIF_PRIO		(TCMIF_BASE + 0x00)
#define EMIFS_PRIO_REG		(TCMIF_BASE + 0x04)
#define EMIFF_PRIO_REG		(TCMIF_BASE + 0x08)
#define EMIFS_CONFIG_REG	(TCMIF_BASE + 0x0c)
#define EMIFS_CS0_CONFIG	(TCMIF_BASE + 0x10)
#define EMIFS_CS1_CONFIG	(TCMIF_BASE + 0x14)
#define EMIFS_CS2_CONFIG	(TCMIF_BASE + 0x18)
#define EMIFS_CS3_CONFIG	(TCMIF_BASE + 0x1c)
#define EMIFF_SDRAM_CONFIG	(TCMIF_BASE + 0x20)
#define EMIFF_MRS		(TCMIF_BASE + 0x24)
#define TC_TIMEOUT1		(TCMIF_BASE + 0x28)
#define TC_TIMEOUT2		(TCMIF_BASE + 0x2c)
#define TC_TIMEOUT3		(TCMIF_BASE + 0x30)
#define TC_ENDIANISM		(TCMIF_BASE + 0x34)
#define EMIFF_SDRAM_CONFIG_2	(TCMIF_BASE + 0x3c)
#define EMIF_CFG_DYNAMIC_WS	(TCMIF_BASE + 0x40)

/*
 * LCD Panel
 */
#define TI925_LCD_BASE		0xFFFEC000
#define TI925_LCD_CONTROL	(TI925_LCD_BASE)
#define TI925_LCD_TIMING0	(TI925_LCD_BASE+0x4)
#define TI925_LCD_TIMING1	(TI925_LCD_BASE+0x8)
#define TI925_LCD_TIMING2	(TI925_LCD_BASE+0xc)
#define TI925_LCD_STATUS	(TI925_LCD_BASE+0x10)
#define TI925_LCD_SUBPANEL	(TI925_LCD_BASE+0x14)

#define OMAP_LCD_CONTROL	TI925_LCD_CONTROL

/* I2C Registers */

#define I2C_BASE		0xfffb3800

#define I2C_REV			(I2C_BASE + 0x00)
#define I2C_IE			(I2C_BASE + 0x04)
#define I2C_STAT		(I2C_BASE + 0x08)
#define I2C_IV			(I2C_BASE + 0x0c)
#define I2C_BUF			(I2C_BASE + 0x14)
#define I2C_CNT			(I2C_BASE + 0x18)
#define I2C_DATA		(I2C_BASE + 0x1c)
#define I2C_CON			(I2C_BASE + 0x24)
#define I2C_OA			(I2C_BASE + 0x28)
#define I2C_SA			(I2C_BASE + 0x2c)
#define I2C_PSC			(I2C_BASE + 0x30)
#define I2C_SCLL		(I2C_BASE + 0x34)
#define I2C_SCLH		(I2C_BASE + 0x38)
#define I2C_SYSTEST		(I2C_BASE + 0x3c)

/* I2C masks */

/* I2C Interrupt Enable Register (I2C_IE): */

#define I2C_IE_XRDY_IE	(1 << 4)	/* Transmit data ready interrupt enable */
#define I2C_IE_RRDY_IE	(1 << 3)	/* Receive data ready interrupt enable */
#define I2C_IE_ARDY_IE	(1 << 2)	/* Register access ready interrupt enable */
#define I2C_IE_NACK_IE	(1 << 1)	/* No acknowledgment interrupt enable */
#define I2C_IE_AL_IE	(1 << 0)	/* Arbitration lost interrupt enable */

/* I2C Status Register (I2C_STAT): */

#define I2C_STAT_SBD	(1 << 15)	/* Single byte data */
#define I2C_STAT_BB	(1 << 12)	/* Bus busy */
#define I2C_STAT_ROVR	(1 << 11)	/* Receive overrun */
#define I2C_STAT_XUDF	(1 << 10)	/* Transmit underflow */
#define I2C_STAT_AAS	(1 << 9)	/* Address as slave */
#define I2C_STAT_AD0	(1 << 8)	/* Address zero */
#define I2C_STAT_XRDY	(1 << 4)	/* Transmit data ready */
#define I2C_STAT_RRDY	(1 << 3)	/* Receive data ready */
#define I2C_STAT_ARDY	(1 << 2)	/* Register access ready */
#define I2C_STAT_NACK	(1 << 1)	/* No acknowledgment interrupt enable */
#define I2C_STAT_AL	(1 << 0)	/* Arbitration lost interrupt enable */

/* I2C Interrupt Vector Register (I2C_IV): */

/* I2C Interrupt Code Register (I2C_INTCODE): */

#define I2C_INTCODE_MASK	7
#define I2C_INTCODE_NONE	0
#define I2C_INTCODE_AL		1	/* Arbitration lost */
#define I2C_INTCODE_NAK		2	/* No acknowledgement/general call */
#define I2C_INTCODE_ARDY	3	/* Register access ready */
#define I2C_INTCODE_RRDY	4	/* Rcv data ready */
#define I2C_INTCODE_XRDY	5	/* Xmit data ready */

/* I2C Buffer Configuration Register (I2C_BUF): */

#define I2C_BUF_RDMA_EN		(1 << 15)	/* Receive DMA channel enable */
#define I2C_BUF_XDMA_EN		(1 << 7)	/* Transmit DMA channel enable */

/* I2C Configuration Register (I2C_CON): */

#define I2C_CON_EN	(1 << 15)	/* I2C module enable */
#define I2C_CON_BE	(1 << 14)	/* Big endian mode */
#define I2C_CON_STB	(1 << 11)	/* Start byte mode (master mode only) */
#define I2C_CON_MST	(1 << 10)	/* Master/slave mode */
#define I2C_CON_TRX	(1 << 9)	/* Transmitter/receiver mode (master mode only) */
#define I2C_CON_XA	(1 << 8)	/* Expand address */
#define I2C_CON_RM	(1 << 2)	/* Repeat mode (master mode only) */
#define I2C_CON_STP	(1 << 1)	/* Stop condition (master mode only) */
#define I2C_CON_STT	(1 << 0)	/* Start condition (master mode only) */

/* I2C System Test Register (I2C_SYSTEST): */

#define I2C_SYSTEST_ST_EN	(1 << 15)	/* System test enable */
#define I2C_SYSTEST_FREE	(1 << 14)	/* Free running mode (on breakpoint) */
#define I2C_SYSTEST_TMODE_MASK	(3 << 12)	/* Test mode select */
#define I2C_SYSTEST_TMODE_SHIFT (12)		/* Test mode select */
#define I2C_SYSTEST_SCL_I	(1 << 3)	/* SCL line sense input value */
#define I2C_SYSTEST_SCL_O	(1 << 2)	/* SCL line drive output value */
#define I2C_SYSTEST_SDA_I	(1 << 1)	/* SDA line sense input value */
#define I2C_SYSTEST_SDA_O	(1 << 0)	/* SDA line drive output value */

/*
 * MMC/SD Host Controller Registers
 */

#define OMAP_MMC_CMD	 0xFFFB7800 /* MMC Command */
#define OMAP_MMC_ARGL	 0xFFFB7804 /* MMC argument low */
#define OMAP_MMC_ARGH	 0xFFFB7808 /* MMC argument high */
#define OMAP_MMC_CON	 0xFFFB780C /* MMC system configuration */
#define OMAP_MMC_STAT	 0xFFFB7810 /* MMC status */
#define OMAP_MMC_IE	 0xFFFB7814 /* MMC system interrupt enable */
#define OMAP_MMC_CTO	 0xFFFB7818 /* MMC command time-out */
#define OMAP_MMC_DTO	 0xFFFB781C /* MMC data time-out */
#define OMAP_MMC_DATA	 0xFFFB7820 /* MMC TX/RX FIFO data */
#define OMAP_MMC_BLEN	 0xFFFB7824 /* MMC block length */
#define OMAP_MMC_NBLK	 0xFFFB7828 /* MMC number of blocks */
#define OMAP_MMC_BUF	 0xFFFB782C /* MMC buffer configuration */
#define OMAP_MMC_SPI	 0xFFFB7830 /* MMC serial port interface */
#define OMAP_MMC_SDIO	 0xFFFB7834 /* MMC SDIO mode configuration */
#define OMAP_MMC_SYST	 0xFFFB7838 /* MMC system test */
#define OMAP_MMC_REV	 0xFFFB783C /* MMC module version */
#define OMAP_MMC_RSP0	 0xFFFB7840 /* MMC command response 0 */
#define OMAP_MMC_RSP1	 0xFFFB7844 /* MMC command response 1 */
#define OMAP_MMC_RSP2	 0xFFFB7848 /* MMC command response 2 */
#define OMAP_MMC_RSP3	 0xFFFB784C /* MMC command response 3 */
#define OMAP_MMC_RSP4	 0xFFFB7850 /* MMC command response 4 */
#define OMAP_MMC_RSP5	 0xFFFB7854 /* MMC command response 5 */
#define OMAP_MMC_RSP6	 0xFFFB7858 /* MMC command response 6 */
#define OMAP_MMC_RSP7	 0xFFFB785C /* MMC command response 4 */

/* MMC masks */

#define OMAP_MMC_END_OF_CMD	(1 << 0)	/* End of command phase */
#define OMAP_MMC_CARD_BUSY	(1 << 2)	/* Card enter busy state */
#define OMAP_MMC_BLOCK_RS	(1 << 3)	/* Block received/sent */
#define OMAP_MMC_EOF_BUSY	(1 << 4)	/* Card exit busy state */
#define OMAP_MMC_DATA_TIMEOUT	(1 << 5)	/* Data response time-out */
#define OMAP_MMC_DATA_CRC	(1 << 6)	/* Date CRC error */
#define OMAP_MMC_CMD_TIMEOUT	(1 << 7)	/* Command response time-out */
#define OMAP_MMC_CMD_CRC	(1 << 8)	/* Command CRC error */
#define OMAP_MMC_A_FULL		(1 << 10)	/* Buffer almost full */
#define OMAP_MMC_A_EMPTY	(1 << 11)	/* Buffer almost empty */
#define OMAP_MMC_OCR_BUSY	(1 << 12)	/* OCR busy */
#define OMAP_MMC_CARD_IRQ	(1 << 13)	/* Card IRQ received */
#define OMAP_MMC_CARD_ERR	(1 << 14)	/* Card status error in response */

/* 2.9.2 MPUI Interface Registers FFFE:C900 */

#define MPUI_CTRL_REG		(volatile __u32 *)(0xfffec900)
#define MPUI_DEBUG_ADDR		(volatile __u32 *)(0xfffec904)
#define MPUI_DEBUG_DATA		(volatile __u32 *)(0xfffec908)
#define MPUI_DEBUG_FLAG		(volatile __u16 *)(0xfffec90c)
#define MPUI_STATUS_REG		(volatile __u16 *)(0xfffec910)
#define MPUI_DSP_STATUS_REG	(volatile __u16 *)(0xfffec914)
#define MPUI_DSP_BOOT_CONFIG	(volatile __u16 *)(0xfffec918)
#define MPUI_DSP_API_CONFIG	(volatile __u16 *)(0xfffec91c)

/* 2.9.6 Traffic Controller Memory Interface Registers: */
#define OMAP_IMIF_PRIO_REG		0xfffecc00
#define OMAP_EMIFS_PRIO_REG		0xfffecc04
#define OMAP_EMIFF_PRIO_REG		0xfffecc08
#define OMAP_EMIFS_CONFIG_REG		0xfffecc0c
#define OMAP_EMIFS_CS0_CONFIG		0xfffecc10
#define OMAP_EMIFS_CS1_CONFIG		0xfffecc14
#define OMAP_EMIFS_CS2_CONFIG		0xfffecc18
#define OMAP_EMIFS_CS3_CONFIG		0xfffecc1c
#define OMAP_EMIFF_SDRAM_CONFIG		0xfffecc20
#define OMAP_EMIFF_MRS			0xfffecc24
#define OMAP_TIMEOUT1			0xfffecc28
#define OMAP_TIMEOUT2			0xfffecc2c
#define OMAP_TIMEOUT3			0xfffecc30
#define OMAP_ENDIANISM			0xfffecc34

/* 2.9.10 EMIF Slow Interface Configuration Register (EMIFS_CONFIG_REG): */
#define OMAP_EMIFS_CONFIG_FR		(1 << 4)
#define OMAP_EMIFS_CONFIG_PDE		(1 << 3)
#define OMAP_EMIFS_CONFIG_PWD_EN	(1 << 2)
#define OMAP_EMIFS_CONFIG_BM		(1 << 1)
#define OMAP_EMIFS_CONFIG_WP		(1 << 0)

/*
 * Memory chunk set aside for the Framebuffer in SRAM
 */
#define SRAM_FRAMEBUFFER_MEMORY OMAP1510_SRAM_BASE


/*
 * DMA
 */

#define OMAP1510_DMA_BASE 0xFFFED800
#define OMAP_DMA_BASE	  OMAP1510_DMA_BASE

/* Global Register selection */
#define NO_GLOBAL_DMA_ACCESS 0

/* Channel select field
 * NOTE: all other channels are linear, chan0 is 0, chan1 is 1, etc...
 */
#define LCD_CHANNEL 0xc

/* Register Select Field (LCD) */
#define DMA_LCD_CTRL		0
#define DMA_LCD_TOP_F1_L	1
#define DMA_LCD_TOP_F1_U	2
#define DMA_LCD_BOT_F1_L	3
#define DMA_LCD_BOT_F1_U	4

#define LCD_FRAME_MODE		(1<<0)
#define LCD_FRAME_IT_IE		(1<<1)
#define LCD_BUS_ERROR_IT_IE	(1<<2)
#define LCD_FRAME_1_IT_COND	(1<<3)
#define LCD_FRAME_2_IT_COND	(1<<4)
#define LCD_BUS_ERROR_IT_COND	(1<<5)
#define LCD_SOURCE_IMIF		(1<<6)

/*
 * Real-Time Clock
 */

#define RTC_SECONDS		(volatile __u8 *)(OMAP1510_RTC_BASE + 0x00)
#define RTC_MINUTES		(volatile __u8 *)(OMAP1510_RTC_BASE + 0x04)
#define RTC_HOURS		(volatile __u8 *)(OMAP1510_RTC_BASE + 0x08)
#define RTC_DAYS		(volatile __u8 *)(OMAP1510_RTC_BASE + 0x0C)
#define RTC_MONTHS		(volatile __u8 *)(OMAP1510_RTC_BASE + 0x10)
#define RTC_YEARS		(volatile __u8 *)(OMAP1510_RTC_BASE + 0x14)
#define RTC_CTRL		(volatile __u8 *)(OMAP1510_RTC_BASE + 0x40)


/* ---------------------------------------------------------------------------
 *  OMAP1510 Interrupt Handlers
 * ---------------------------------------------------------------------------
 *
 */
#define OMAP_IH1_BASE		0xfffecb00
#define OMAP_IH2_BASE		0xfffe0000
#define OMAP1510_ITR		0x0
#define OMAP1510_MASK		0x4

#define INTERRUPT_HANDLER_BASE	 OMAP_IH1_BASE
#define INTERRUPT_INPUT_REGISTER OMAP1510_ITR
#define INTERRUPT_MASK_REGISTER	 OMAP1510_MASK


/* ---------------------------------------------------------------------------
 *  OMAP1510 TIMERS
 * ---------------------------------------------------------------------------
 *
 */

#define OMAP1510_32kHz_TIMER_BASE 0xfffb9000

/* 32k Timer Registers */
#define TIMER32k_CR	0x08
#define TIMER32k_TVR	0x00
#define TIMER32k_TCR	0x04

/* 32k Timer Control Register definition */
#define TIMER32k_TSS	(1<<0)
#define TIMER32k_TRB	(1<<1)
#define TIMER32k_INT	(1<<2)
#define TIMER32k_ARL	(1<<3)

/* MPU Timer base addresses  */
#define OMAP1510_MPUTIMER_BASE	0xfffec500
#define OMAP1510_MPUTIMER_OFF	0x00000100

#define OMAP1510_TIMER1_BASE	0xfffec500
#define OMAP1510_TIMER2_BASE	0xfffec600
#define OMAP1510_TIMER3_BASE	0xfffec700

/* MPU Timer Registers */
#define CNTL_TIMER	0
#define LOAD_TIM	4
#define READ_TIM	8

/*  CNTL_TIMER register bits */
#define MPUTIM_FREE		(1<<6)
#define MPUTIM_CLOCK_ENABLE	(1<<5)
#define MPUTIM_PTV_MASK		(0x7<<MPUTIM_PTV_BIT)
#define MPUTIM_PTV_BIT		2
#define MPUTIM_AR		(1<<1)
#define MPUTIM_ST		(1<<0)

/* ---------------------------------------------------------------------------
 *  OMAP1510 GPIO (SHARED)
 * ---------------------------------------------------------------------------
 *
 */
#define GPIO_DATA_INPUT_REG	(OMAP1510_GPIO_BASE + 0x0)
#define GPIO_DATA_OUTPUT_REG	(OMAP1510_GPIO_BASE + 0x4)
#define GPIO_DIR_CONTROL_REG	(OMAP1510_GPIO_BASE + 0x8)
#define GPIO_INT_CONTROL_REG	(OMAP1510_GPIO_BASE + 0xc)
#define GPIO_INT_MASK_REG	(OMAP1510_GPIO_BASE + 0x10)
#define GPIO_INT_STATUS_REG	(OMAP1510_GPIO_BASE + 0x14)
#define GPIO_PIN_CONTROL_REG	(OMAP1510_GPIO_BASE + 0x18)


/* ---------------------------
 * OMAP1510 MPUIO (ARM only)
 *----------------------------
 */
#define OMAP1510_MPUIO_BASE	0xFFFB5000
#define MPUIO_DATA_INPUT_REG	(OMAP1510_MPUIO_BASE + 0x0)
#define MPUIO_DATA_OUTPUT_REG	(OMAP1510_MPUIO_BASE + 0x4)
#define MPUIO_DIR_CONTROL_REG	(OMAP1510_MPUIO_BASE + 0x8)

/* ---------------------------------------------------------------------------
 *  OMAP1510 TIPB (only)
 * ---------------------------------------------------------------------------
 *
 */
#define TIPB_PUBLIC_CNTL_BASE		0xfffed300
#define MPU_PUBLIC_TIPB_CNTL_REG	(TIPB_PUBLIC_CNTL_BASE + 0x8)
#define TIPB_PRIVATE_CNTL_BASE		0xfffeca00
#define MPU_PRIVATE_TIPB_CNTL_REG	(TIPB_PRIVATE_CNTL_BASE + 0x8)

/*
 * ---------------------------------------------------------------------------
 *  OMAP1510 Camera Interface
 * ---------------------------------------------------------------------------
 */
#define CAMERA_BASE		(IO_BASE + 0x6800)
#define CAM_CTRLCLOCK_REG	(CAMERA_BASE + 0x00)
#define CAM_IT_STATUS_REG	(CAMERA_BASE + 0x04)
#define CAM_MODE_REG		(CAMERA_BASE + 0x08)
#define CAM_STATUS_REG		(CAMERA_BASE + 0x0C)
#define CAM_CAMDATA_REG		(CAMERA_BASE + 0x10)
#define CAM_GPIO_REG		(CAMERA_BASE + 0x14)
#define CAM_PEAK_CTR_REG	(CAMERA_BASE + 0x18)

#if 0
#ifndef __ASSEMBLY__
typedef struct {
	__u32 ctrlclock;
	__u32 it_status;
	__u32 mode;
	__u32 status;
	__u32 camdata;
	__u32 gpio;
	__u32 peak_counter;
} camera_regs_t;
#endif
#endif

/* CTRLCLOCK bit shifts */
#define FOSCMOD_BIT	0
#define FOSCMOD_MASK	(0x7 << FOSCMOD_BIT)
#define	 FOSCMOD_12MHz	0x0
#define	 FOSCMOD_6MHz	0x2
#define	 FOSCMOD_9_6MHz 0x4
#define	 FOSCMOD_24MHz	0x5
#define	 FOSCMOD_8MHz	0x6
#define POLCLK		(1<<3)
#define CAMEXCLK_EN	(1<<4)
#define MCLK_EN		(1<<5)
#define DPLL_EN		(1<<6)
#define LCLK_EN		(1<<7)

/* IT_STATUS bit shifts */
#define V_UP		(1<<0)
#define V_DOWN		(1<<1)
#define H_UP		(1<<2)
#define H_DOWN		(1<<3)
#define FIFO_FULL	(1<<4)
#define DATA_XFER	(1<<5)

/* MODE bit shifts */
#define CAMOSC		(1<<0)
#define IMGSIZE_BIT	1
#define IMGSIZE_MASK	(0x3 << IMGSIZE_BIT)
#define	 IMGSIZE_CIF	(0x0 << IMGSIZE_BIT)	/* 352x288 */
#define	 IMGSIZE_QCIF	(0x1 << IMGSIZE_BIT)	/* 176x144 */
#define	 IMGSIZE_VGA	(0x2 << IMGSIZE_BIT)	/* 640x480 */
#define	 IMGSIZE_QVGA	(0x3 << IMGSIZE_BIT)	/* 320x240 */
#define ORDERCAMD	(1<<3)
#define EN_V_UP		(1<<4)
#define EN_V_DOWN	(1<<5)
#define EN_H_UP		(1<<6)
#define EN_H_DOWN	(1<<7)
#define EN_DMA		(1<<8)
#define THRESHOLD	(1<<9)
#define THRESHOLD_BIT	9
#define THRESHOLD_MASK	(0x7f<<9)
#define EN_NIRQ		(1<<16)
#define EN_FIFO_FULL	(1<<17)
#define RAZ_FIFO	(1<<18)

/* STATUS bit shifts */
#define VSTATUS		(1<<0)
#define HSTATUS		(1<<1)

/* GPIO bit shifts */
#define CAM_RST		(1<<0)


/*********************
 * Watchdog timer.
 *********************/
#define WDTIM_BASE	0xfffec800
#define WDTIM_CONTROL	(WDTIM_BASE+0x00)
#define WDTIM_LOAD	(WDTIM_BASE+0x04)
#define WDTIM_READ	(WDTIM_BASE+0x04)
#define WDTIM_MODE	(WDTIM_BASE+0x08)

/* Values to write to mode register to disable the watchdog function. */
#define DISABLE_SEQ1	0xF5
#define DISABLE_SEQ2	0xA0

/* WDTIM_CONTROL bit definitions. */
#define WDTIM_CONTROL_ST	BIT7


/* ---------------------------------------------------------------------------
 *  Differentiating processor versions for those who care.
 * ---------------------------------------------------------------------------
 *
 */
#define OMAP1509 0
#define OMAP1510 1

#define OMAP1510_ID_CODE_REG 0xfffed404

#ifndef __ASSEMBLY__
int cpu_type(void);
#endif

/*
 * EVM Implementation Specifics.
 *
 * *** NOTE ***
 * Any definitions in these files should be prefixed by an identifier -
 * eg. OMAP1510P1_FLASH0_BASE .
 *
 */
#ifdef CONFIG_OMAP_INNOVATOR
#include "innovator.h"
#endif

#ifdef CONFIG_OMAP_1510P1
#include "omap1510p1.h"
#endif

/*****************************************************************************/

#define CLKGEN_RESET_BASE (0xfffece00)
#define ARM_CKCTL	(volatile __u16 *)(CLKGEN_RESET_BASE + 0x0)
#define ARM_IDLECT1	(volatile __u16 *)(CLKGEN_RESET_BASE + 0x4)
#define ARM_IDLECT2	(volatile __u16 *)(CLKGEN_RESET_BASE + 0x8)
#define ARM_EWUPCT	(volatile __u16 *)(CLKGEN_RESET_BASE + 0xC)
#define ARM_RSTCT1	(volatile __u16 *)(CLKGEN_RESET_BASE + 0x10)
#define ARM_RSTCT2	(volatile __u16 *)(CLKGEN_RESET_BASE + 0x14)
#define ARM_SYSST	(volatile __u16 *)(CLKGEN_RESET_BASE + 0x18)


#define CK_CLKIN	12 /* MHz */
#define CK_RATEF	1
#define CK_IDLEF	2
#define CK_ENABLEF	4
#define CK_SELECTF	8
#ifndef __ASSEMBLER__
#define CK_DPLL1	((volatile __u16 *)0xfffecf00)
#else
#define CK_DPLL1 (0xfffecf00)
#endif
#define SETARM_IDLE_SHIFT

/* ARM_CKCTL bit shifts */
#define PERDIV		0
#define LCDDIV		2
#define ARMDIV		4
#define DSPDIV		6
#define TCDIV		8
#define DSPMMUDIV	10
#define ARM_TIMXO	12
#define EN_DSPCK	13
#define ARM_INTHCK_SEL	14 /* REVISIT -- where is this used? */

#define ARM_CKCTL_RSRVD_BIT15		(1 << 15)
#define ARM_CKCTL_ARM_INTHCK_SEL	(1 << 14)
#define ARM_CKCTL_EN_DSPCK		(1 << 13)
#define ARM_CKCTL_ARM_TIMXO		(1 << 12)
#define ARM_CKCTL_DSPMMU_DIV1		(1 << 11)
#define ARM_CKCTL_DSPMMU_DIV2		(1 << 10)
#define ARM_CKCTL_TCDIV1		(1 << 9)
#define ARM_CKCTL_TCDIV2		(1 << 8)
#define ARM_CKCTL_DSPDIV1		(1 << 7)
#define ARM_CKCTL_DSPDIV0		(1 << 6)
#define ARM_CKCTL_ARMDIV1		(1 << 5)
#define ARM_CKCTL_ARMDIV0		(1 << 4)
#define ARM_CKCTL_LCDDIV1		(1 << 3)
#define ARM_CKCTL_LCDDIV0		(1 << 2)
#define ARM_CKCTL_PERDIV1		(1 << 1)
#define ARM_CKCTL_PERDIV0		(1 << 0)

/* ARM_IDLECT1 bit shifts */
#define IDLWDT_ARM	0
#define IDLXORP_ARM	1
#define IDLPER_ARM	2
#define IDLLCD_ARM	3
#define IDLLB_ARM	4
#define IDLHSAB_ARM	5
#define IDLIF_ARM	6
#define IDLDPLL_ARM	7
#define IDLAPI_ARM	8
#define IDLTIM_ARM	9
#define SETARM_IDLE	11

/* ARM_IDLECT2 bit shifts */
#define EN_WDTCK	0
#define EN_XORPCK	1
#define EN_PERCK	2
#define EN_LCDCK	3
#define EN_LBCK		4
#define EN_HSABCK	5
#define EN_APICK	6
#define EN_TIMCK	7
#define DMACK_REQ	8
#define EN_GPIOCK	9
#define EN_LBFREECK	10

#define ARM_RSTCT1_SW_RST	(1 << 3)
#define ARM_RSTCT1_DSP_RST	(1 << 2)
#define ARM_RSTCT1_DSP_EN	(1 << 1)
#define ARM_RSTCT1_ARM_RST	(1 << 0)

/* ARM_RSTCT2 bit shifts */
#define EN_PER		0

#define ARM_SYSST_RSRVD_BIT15	(1 << 15)
#define ARM_SYSST_RSRVD_BIT14	(1 << 14)
#define ARM_SYSST_CLOCK_SELECT2 (1 << 13)
#define ARM_SYSST_CLOCK_SELECT1 (1 << 12)
#define ARM_SYSST_CLOCK_SELECT0 (1 << 11)
#define ARM_SYSST_RSRVD_BIT10	(1 << 10)
#define ARM_SYSST_RSRVD_BIT9	(1 << 9)
#define ARM_SYSST_RSRVD_BIT8	(1 << 8)
#define ARM_SYSST_RSRVD_BIT7	(1 << 7)
#define ARM_SYSST_IDLE_DSP	(1 << 6)
#define ARM_SYSST_POR		(1 << 5)
#define ARM_SYSST_EXT_RST	(1 << 4)
#define ARM_SYSST_ARM_MCRST	(1 << 3)
#define ARM_SYSST_ARM_WDRST	(1 << 2)
#define ARM_SYSST_GLOB_SWRST	(1 << 1)
#define ARM_SYSST_DSP_WDRST	(1 << 0)

/* Table 15-23. DPLL Control Registers: */
#define DPLL_CTL_REG		(volatile __u16 *)(0xfffecf00)

/* Table 15-24. Control Register (CTL_REG): */

#define DPLL_CTL_REG_IOB		(1 << 13)
#define DPLL_CTL_REG_PLL_MULT		Fld(5,0)

/*****************************************************************************/

/* OMAP INTERRUPT REGISTERS */
#define IRQ_ITR			0x00
#define IRQ_MIR			0x04
#define IRQ_SIR_IRQ		0x10
#define IRQ_SIR_FIQ		0x14
#define IRQ_CONTROL_REG		0x18
#define IRQ_ISR			0x9c
#define IRQ_ILR0		0x1c

#define REG_IHL1_MIR  (OMAP_IH1_BASE+IRQ_MIR)
#define REG_IHL2_MIR  (OMAP_IH2_BASE+IRQ_MIR)

/* INTERRUPT LEVEL REGISTER BITS */
#define ILR_PRIORITY_MASK	(0x3c)
#define ILR_PRIORITY_SHIFT	(2)
#define ILR_LEVEL_TRIGGER	(1<<1)
#define ILR_FIQ			(1<<0)

#define IRQ_LEVEL_INT		1
#define IRQ_EDGE_INT		0

/* Macros to access registers */
#define outb(v,p) *(volatile u8 *) (p) = v
#define outw(v,p) *(volatile u16 *) (p) = v
#define outl(v,p) *(volatile u32 *) (p) = v

#define inb(p)	*(volatile u8 *) (p)
#define inw(p)	*(volatile u16 *) (p)
#define inl(p)	*(volatile u32 *) (p)
