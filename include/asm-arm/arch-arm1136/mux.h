/*
 * (C) Copyright 2004
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _OMAP2420_MUX_H_
#define _OMAP2420_MUX_H_

#ifndef __ASSEMBLY__
typedef  unsigned char uint8;
typedef  unsigned int uint32;

void muxSetupSDRC(void);
void muxSetupGPMC(void);
void muxSetupUsb0(void);
void muxSetupUart3(void);
void muxSetupI2C1(void);
void muxSetupUART1(void);
void muxSetupLCD(void);
void muxSetupCamera(void);
void muxSetupMMCSD(void) ;
void muxSetupTouchScreen(void) ;
void muxSetupHDQ(void);
#endif

#define USB_OTG_CTRL			        ((volatile uint32 *)0x4805E30C)

/* Pin Muxing registers used for HDQ (Smart battery) */
#define CONTROL_PADCONF_HDQ_SIO         ((volatile unsigned char *)0x48000115)

/* Pin Muxing registers used for GPMC */
#define CONTROL_PADCONF_GPMC_D2_BYTE0	((volatile unsigned char *)0x48000088)
#define CONTROL_PADCONF_GPMC_D2_BYTE1	((volatile unsigned char *)0x48000089)
#define CONTROL_PADCONF_GPMC_D2_BYTE2	((volatile unsigned char *)0x4800008A)
#define CONTROL_PADCONF_GPMC_D2_BYTE3	((volatile unsigned char *)0x4800008B)

#define CONTROL_PADCONF_GPMC_NCS0_BYTE0	((volatile unsigned char *)0x4800008C)
#define CONTROL_PADCONF_GPMC_NCS0_BYTE1	((volatile unsigned char *)0x4800008D)
#define CONTROL_PADCONF_GPMC_NCS0_BYTE2	((volatile unsigned char *)0x4800008E)
#define CONTROL_PADCONF_GPMC_NCS0_BYTE3	((volatile unsigned char *)0x4800008F)

/* Pin Muxing registers used for SDRC */
#define CONTROL_PADCONF_SDRC_NCS0_BYTE0 ((volatile unsigned char *)0x480000A0)
#define CONTROL_PADCONF_SDRC_NCS0_BYTE1 ((volatile unsigned char *)0x480000A1)
#define CONTROL_PADCONF_SDRC_NCS0_BYTE2 ((volatile unsigned char *)0x480000A2)
#define CONTROL_PADCONF_SDRC_NCS0_BYTE3 ((volatile unsigned char *)0x480000A3)

#define CONTROL_PADCONF_SDRC_A14_BYTE0	((volatile unsigned char *)0x48000030)
#define CONTROL_PADCONF_SDRC_A14_BYTE1	((volatile unsigned char *)0x48000031)
#define CONTROL_PADCONF_SDRC_A14_BYTE2	((volatile unsigned char *)0x48000032)
#define CONTROL_PADCONF_SDRC_A14_BYTE3	((volatile unsigned char *)0x48000033)

/* Pin Muxing registers used for Touch Screen (SPI) */
#define CONTROL_PADCONF_SPI1_CLK        ((volatile unsigned char *)0x480000FF)
#define CONTROL_PADCONF_SPI1_SIMO       ((volatile unsigned char *)0x48000100)
#define CONTROL_PADCONF_SPI1_SOMI       ((volatile unsigned char *)0x48000101)
#define CONTROL_PADCONF_SPI1_NCS0       ((volatile unsigned char *)0x48000102)

#define CONTROL_PADCONF_MCBSP1_FSR      ((volatile unsigned char *)0x4800010B)

/* Pin Muxing registers used for MMCSD */
#define CONTROL_PADCONF_MMC_CLKI        ((volatile unsigned char *)0x480000FE)
#define CONTROL_PADCONF_MMC_CLKO        ((volatile unsigned char *)0x480000F3)
#define CONTROL_PADCONF_MMC_CMD         ((volatile unsigned char *)0x480000F4)
#define CONTROL_PADCONF_MMC_DAT0        ((volatile unsigned char *)0x480000F5)
#define CONTROL_PADCONF_MMC_DAT1        ((volatile unsigned char *)0x480000F6)
#define CONTROL_PADCONF_MMC_DAT2        ((volatile unsigned char *)0x480000F7)
#define CONTROL_PADCONF_MMC_DAT3        ((volatile unsigned char *)0x480000F8)
#define CONTROL_PADCONF_MMC_DAT_DIR0    ((volatile unsigned char *)0x480000F9)
#define CONTROL_PADCONF_MMC_DAT_DIR1    ((volatile unsigned char *)0x480000FA)
#define CONTROL_PADCONF_MMC_DAT_DIR2    ((volatile unsigned char *)0x480000FB)
#define CONTROL_PADCONF_MMC_DAT_DIR3    ((volatile unsigned char *)0x480000FC)
#define CONTROL_PADCONF_MMC_CMD_DIR     ((volatile unsigned char *)0x480000FD)

#define CONTROL_PADCONF_SDRC_A14        ((volatile unsigned char *)0x48000030)
#define CONTROL_PADCONF_SDRC_A13        ((volatile unsigned char *)0x48000031)

/* Pin Muxing registers used for CAMERA */
#define CONTROL_PADCONF_SYS_NRESWARM    ((volatile unsigned char *)0x4800012B)

#define CONTROL_PADCONF_CAM_XCLK        ((volatile unsigned char *)0x480000DC)
#define CONTROL_PADCONF_CAM_LCLK        ((volatile unsigned char *)0x480000DB)
#define CONTROL_PADCONF_CAM_VS          ((volatile unsigned char *)0x480000DA)
#define CONTROL_PADCONF_CAM_HS          ((volatile unsigned char *)0x480000D9)
#define CONTROL_PADCONF_CAM_D0          ((volatile unsigned char *)0x480000D8)
#define CONTROL_PADCONF_CAM_D1          ((volatile unsigned char *)0x480000D7)
#define CONTROL_PADCONF_CAM_D2          ((volatile unsigned char *)0x480000D6)
#define CONTROL_PADCONF_CAM_D3          ((volatile unsigned char *)0x480000D5)
#define CONTROL_PADCONF_CAM_D4          ((volatile unsigned char *)0x480000D4)
#define CONTROL_PADCONF_CAM_D5          ((volatile unsigned char *)0x480000D3)
#define CONTROL_PADCONF_CAM_D6          ((volatile unsigned char *)0x480000D2)
#define CONTROL_PADCONF_CAM_D7          ((volatile unsigned char *)0x480000D1)
#define CONTROL_PADCONF_CAM_D8          ((volatile unsigned char *)0x480000D0)
#define CONTROL_PADCONF_CAM_D9          ((volatile unsigned char *)0x480000CF)

/* Pin Muxing registers used for LCD */
#define CONTROL_PADCONF_DSS_D0          ((volatile unsigned char *)0x480000B3)
#define CONTROL_PADCONF_DSS_D1          ((volatile unsigned char *)0x480000B4)
#define CONTROL_PADCONF_DSS_D2          ((volatile unsigned char *)0x480000B5)
#define CONTROL_PADCONF_DSS_D3          ((volatile unsigned char *)0x480000B6)
#define CONTROL_PADCONF_DSS_D4          ((volatile unsigned char *)0x480000B7)
#define CONTROL_PADCONF_DSS_D5          ((volatile unsigned char *)0x480000B8)
#define CONTROL_PADCONF_DSS_D6          ((volatile unsigned char *)0x480000B9)
#define CONTROL_PADCONF_DSS_D7          ((volatile unsigned char *)0x480000BA)
#define CONTROL_PADCONF_DSS_D8          ((volatile unsigned char *)0x480000BB)
#define CONTROL_PADCONF_DSS_D9          ((volatile unsigned char *)0x480000BC)
#define CONTROL_PADCONF_DSS_D10         ((volatile unsigned char *)0x480000BD)
#define CONTROL_PADCONF_DSS_D11         ((volatile unsigned char *)0x480000BE)
#define CONTROL_PADCONF_DSS_D12         ((volatile unsigned char *)0x480000BF)
#define CONTROL_PADCONF_DSS_D13         ((volatile unsigned char *)0x480000C0)
#define CONTROL_PADCONF_DSS_D14         ((volatile unsigned char *)0x480000C1)
#define CONTROL_PADCONF_DSS_D15         ((volatile unsigned char *)0x480000C2)
#define CONTROL_PADCONF_DSS_D16         ((volatile unsigned char *)0x480000C3)
#define CONTROL_PADCONF_DSS_D17         ((volatile unsigned char *)0x480000C4)
#define CONTROL_PADCONF_DSS_PCLK        ((volatile unsigned char *)0x480000CB)
#define CONTROL_PADCONF_DSS_VSYNC       ((volatile unsigned char *)0x480000CC)
#define CONTROL_PADCONF_DSS_HSYNC       ((volatile unsigned char *)0x480000CD)
#define CONTROL_PADCONF_DSS_ACBIAS      ((volatile unsigned char *)0x480000CE)

/* Pin Muxing registers used for UART1 */
#define CONTROL_PADCONF_UART1_CTS       ((volatile unsigned char *)0x480000C5)
#define CONTROL_PADCONF_UART1_RTS       ((volatile unsigned char *)0x480000C6)
#define CONTROL_PADCONF_UART1_TX        ((volatile unsigned char *)0x480000C7)
#define CONTROL_PADCONF_UART1_RX        ((volatile unsigned char *)0x480000C8)

/* Pin Muxing registers used for I2C1 */
#define CONTROL_PADCONF_I2C1_SCL        ((volatile unsigned char *)0x48000111)
#define CONTROL_PADCONF_I2C1_SDA        ((volatile unsigned char *)0x48000112)

/* Pin Muxing registres used for USB0. */
#define CONTROL_PADCONF_USB0_PUEN		((volatile uint8 *)0x4800011D)
#define CONTROL_PADCONF_USB0_VP			((volatile uint8 *)0x4800011E)
#define CONTROL_PADCONF_USB0_VM			((volatile uint8 *)0x4800011F)
#define CONTROL_PADCONF_USB0_RCV		((volatile uint8 *)0x48000120)
#define CONTROL_PADCONF_USB0_TXEN		((volatile uint8 *)0x48000121)
#define CONTROL_PADCONF_USB0_SE0		((volatile uint8 *)0x48000122)
#define CONTROL_PADCONF_USB0_DAT		((volatile uint8 *)0x48000123)

/* Pin Muxing registers used for UART3/IRDA */
#define CONTROL_PADCONF_UART3_TX_IRTX	((volatile uint8 *)0x48000118)
#define CONTROL_PADCONF_UART3_RX_IRRX	((volatile uint8 *)0x48000119)

#endif
