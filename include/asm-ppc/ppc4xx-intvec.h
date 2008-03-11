/*
*  Copyright (C) 2002 Scott McNutt <smcnutt@artesyncp.com>
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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,
* MA 02111-1307 USA
*/

/*
 * Interrupt vector number definitions to ease the
 * 405 -- 440 porting pain ;-)
 *
 * NOTE: They're not all here yet ... update as needed.
 *
 */

#ifndef _VECNUMS_H_
#define _VECNUMS_H_

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

/* UIC 0 */
#define VECNUM_U0                   0  /* UART 0                        */
#define VECNUM_U1                   1  /* UART 1                        */
#define VECNUM_IIC0                 2  /* IIC                           */
#define VECNUM_KRD                  3  /* Kasumi Ready for data         */
#define VECNUM_KDA                  4  /* Kasumi Data Available         */
#define VECNUM_PCRW                 5  /* PCI command register write    */
#define VECNUM_PPM                  6  /* PCI power management          */
#define VECNUM_IIC1                 7  /* IIC                           */
#define VECNUM_SPI                  8  /* SPI                           */
#define VECNUM_EPCISER              9  /* External PCI SERR             */
#define VECNUM_MTE                 10  /* MAL TXEOB                     */
#define VECNUM_MRE                 11  /* MAL RXEOB                     */
#define VECNUM_D0                  12  /* DMA channel 0                 */
#define VECNUM_D1                  13  /* DMA channel 1                 */
#define VECNUM_D2                  14  /* DMA channel 2                 */
#define VECNUM_D3                  15  /* DMA channel 3                 */
#define VECNUM_UD0                 16  /* UDMA irq 0                    */
#define VECNUM_UD1                 17  /* UDMA irq 1                    */
#define VECNUM_UD2                 18  /* UDMA irq 2                    */
#define VECNUM_UD3                 19  /* UDMA irq 3                    */
#define VECNUM_HSB2D               20  /* USB2.0 Device                 */
#define VECNUM_USBDEV		   20  /* USB 1.1/USB 2.0 Device        */
#define VECNUM_OHCI1               21  /* USB2.0 Host OHCI irq 1        */
#define VECNUM_OHCI2               22  /* USB2.0 Host OHCI irq 2        */
#define VECNUM_EIP94               23  /* Security EIP94                */
#define VECNUM_ETH0                24  /* Emac 0                        */
#define VECNUM_ETH1                25  /* Emac 1                        */
#define VECNUM_EHCI                26  /* USB2.0 Host EHCI              */
#define VECNUM_EIR4                27  /* External interrupt 4          */
#define VECNUM_UIC2NC              28  /* UIC2 non-critical interrupt   */
#define VECNUM_UIC2C               29  /* UIC2 critical interrupt       */
#define VECNUM_UIC1NC              30  /* UIC1 non-critical interrupt   */
#define VECNUM_UIC1C               31  /* UIC1 critical interrupt       */

/* UIC 1 */
#define VECNUM_MS           (32 +  0)  /* MAL SERR                      */
#define VECNUM_MTDE         (32 +  1)  /* MAL TXDE                      */
#define VECNUM_MRDE         (32 +  2)  /* MAL RXDE                      */
#define VECNUM_U2           (32 +  3)  /* UART 2                        */
#define VECNUM_U3           (32 +  4)  /* UART 3                        */
#define VECNUM_EBCO         (32 +  5)  /* EBCO interrupt status         */
#define VECNUM_NDFC         (32 +  6)  /* NDFC                          */
#define VECNUM_KSLE         (32 +  7)  /* KASUMI slave error            */
#define VECNUM_CT5          (32 +  8)  /* GPT compare timer 5           */
#define VECNUM_CT6          (32 +  9)  /* GPT compare timer 6           */
#define VECNUM_PLB34I0      (32 + 10)  /* PLB3X4X MIRQ0                 */
#define VECNUM_PLB34I1      (32 + 11)  /* PLB3X4X MIRQ1                 */
#define VECNUM_PLB34I2      (32 + 12)  /* PLB3X4X MIRQ2                 */
#define VECNUM_PLB34I3      (32 + 13)  /* PLB3X4X MIRQ3                 */
#define VECNUM_PLB34I4      (32 + 14)  /* PLB3X4X MIRQ4                 */
#define VECNUM_PLB34I5      (32 + 15)  /* PLB3X4X MIRQ5                 */
#define VECNUM_CT0          (32 + 16)  /* GPT compare timer 0           */
#define VECNUM_CT1          (32 + 17)  /* GPT compare timer 1           */
#define VECNUM_EIR7         (32 + 18)  /* External interrupt 7          */
#define VECNUM_EIR8         (32 + 19)  /* External interrupt 8          */
#define VECNUM_EIR9         (32 + 20)  /* External interrupt 9          */
#define VECNUM_CT2          (32 + 21)  /* GPT compare timer 2           */
#define VECNUM_CT3          (32 + 22)  /* GPT compare timer 3           */
#define VECNUM_CT4          (32 + 23)  /* GPT compare timer 4           */
#define VECNUM_SRE          (32 + 24)  /* Serial ROM error              */
#define VECNUM_GPTDC        (32 + 25)  /* GPT decrementer pulse         */
#define VECNUM_RSVD0        (32 + 26)  /* Reserved                      */
#define VECNUM_EPCIPER      (32 + 27)  /* External PCI PERR             */
#define VECNUM_EIR0         (32 + 28)  /* External interrupt 0          */
#define VECNUM_EWU0         (32 + 29)  /* Ethernet 0 wakeup             */
#define VECNUM_EIR1         (32 + 30)  /* External interrupt 1          */
#define VECNUM_EWU1         (32 + 31)  /* Ethernet 1 wakeup             */

#define VECNUM_TXDE         VECNUM_MTDE
#define VECNUM_RXDE         VECNUM_MRDE

/* UIC 2 */
#define VECNUM_EIR5         (64 +  0)  /* External interrupt 5          */
#define VECNUM_EIR6         (64 +  1)  /* External interrupt 6          */
#define VECNUM_OPB          (64 +  2)  /* OPB to PLB bridge int stat    */
#define VECNUM_EIR2         (64 +  3)  /* External interrupt 2          */
#define VECNUM_EIR3         (64 +  4)  /* External interrupt 3          */
#define VECNUM_DDR2         (64 +  5)  /* DDR2 sdram                    */
#define VECNUM_MCTX0        (64 +  6)  /* MAl intp coalescence TX0      */
#define VECNUM_MCTX1        (64 +  7)  /* MAl intp coalescence TX1      */
#define VECNUM_MCTR0        (64 +  8)  /* MAl intp coalescence TR0      */
#define VECNUM_MCTR1        (64 +  9)  /* MAl intp coalescence TR1      */

#elif defined(CONFIG_460EX) || defined(CONFIG_460GT)

/* UIC 0 */
#define VECNUM_U1	1		/* UART1			*/
#define VECNUM_IIC0	2		/* IIC0				*/
#define VECNUM_IIC1	3		/* IIC1				*/
#define VECNUM_PIM	4		/* PCI inbound message		*/
#define VECNUM_PCRW	5		/* PCI command reg write	*/
#define VECNUM_PPM	6		/* PCI power management		*/
#define VECNUM_MSI0	8		/* PCI MSI level 0		*/
#define VECNUM_EIR0	9		/* External interrupt 0		*/
#define VECNUM_UIC2NC	10		/* UIC2 non-critical interrupt	*/
#define VECNUM_UIC2C	11		/* UIC2 critical interrupt	*/
#define VECNUM_D0	12		/* DMA channel 0		*/
#define VECNUM_D1	13		/* DMA channel 1		*/
#define VECNUM_D2	14		/* DMA channel 2		*/
#define VECNUM_D3	15		/* DMA channel 3		*/
#define VECNUM_UIC3NC	16		/* UIC3 non-critical interrupt	*/
#define VECNUM_UIC3C	17		/* UIC3 critical interrupt	*/
#define VECNUM_EIR1	9		/* External interrupt 1		*/
#define VECNUM_UIC1NC	30		/* UIC1 non-critical interrupt	*/
#define VECNUM_UIC1C	31		/* UIC1 critical interrupt	*/

/* UIC 1 */
#define VECNUM_EIR2	(32 + 0)	/* External interrupt 0		*/
#define VECNUM_U0	(32 + 1)	/* UART0			*/
#define VECNUM_EIR3	(32 + 20)	/* External interrupt 3		*/
#define VECNUM_EIR4	(32 + 21)	/* External interrupt 4		*/
#define VECNUM_EIR5	(32 + 26)	/* External interrupt 5		*/
#define VECNUM_EIR6	(32 + 27)	/* External interrupt 6		*/
#define VECNUM_U2	(32 + 28)	/* UART2			*/
#define VECNUM_U3	(32 + 29)	/* UART3			*/
#define VECNUM_EIR7	(32 + 30)	/* External interrupt 7		*/
#define VECNUM_EIR8	(32 + 31)	/* External interrupt 8		*/

/* UIC 2 */
#define VECNUM_EIR9	(64 + 2)	/* External interrupt 9		*/
#define VECNUM_MS	(64 + 3)	/* MAL SERR			*/
#define	VECNUM_TXDE	(64 + 4)	/* MAL TXDE			*/
#define	VECNUM_RXDE	(64 + 5)	/* MAL RXDE			*/
#define VECNUM_MTE	(64 + 6)	/* MAL TXEOB			*/
#define	VECNUM_MRE	(64 + 7)	/* MAL RXEOB			*/
#define	VECNUM_ETH0	(64 + 16)	/* Ethernet 0			*/
#define	VECNUM_ETH1	(64 + 17)	/* Ethernet 1			*/
#define	VECNUM_ETH2	(64 + 18)	/* Ethernet 2			*/
#define	VECNUM_ETH3	(64 + 19)	/* Ethernet 3			*/
#define VECNUM_EWU0	(64 + 20)	/* Emac 0 wakeup		*/
#define VECNUM_EWU1	(64 + 21)	/* Emac 1 wakeup		*/
#define VECNUM_EWU2	(64 + 22)	/* Emac 2 wakeup		*/
#define VECNUM_EWU3	(64 + 23)	/* Emac 3 wakeup		*/
#define VECNUM_EIR10	(64 + 24)	/* External interrupt 10	*/
#define VECNUM_EIR11	(64 + 25)	/* External interrupt 11	*/

/* UIC 3 */
#define VECNUM_EIR12	(96 + 20)	/* External interrupt 20	*/
#define VECNUM_EIR13	(96 + 21)	/* External interrupt 21	*/
#define VECNUM_EIR14	(96 + 22)	/* External interrupt 22	*/
#define VECNUM_EIR15	(96 + 23)	/* External interrupt 23	*/
#define VECNUM_PCIEMSI0	(96 + 24)	/* PCI Express MSI level 0	*/
#define VECNUM_PCIEMSI1	(96 + 25)	/* PCI Express MSI level 1	*/
#define VECNUM_PCIEMSI2	(96 + 26)	/* PCI Express MSI level 2	*/
#define VECNUM_PCIEMSI3	(96 + 27)	/* PCI Express MSI level 3	*/
#define VECNUM_PCIEMSI4	(96 + 28)	/* PCI Express MSI level 4	*/
#define VECNUM_PCIEMSI5	(96 + 29)	/* PCI Express MSI level 5	*/
#define VECNUM_PCIEMSI6	(96 + 30)	/* PCI Express MSI level 6	*/
#define VECNUM_PCIEMSI7	(96 + 31)	/* PCI Express MSI level 7	*/

#elif defined(CONFIG_440SPE)

/* UIC 0 */
#define VECNUM_U0           0           /* UART0                        */
#define VECNUM_U1           1           /* UART1                        */
#define VECNUM_IIC0         2           /* IIC0                         */
#define VECNUM_IIC1         3           /* IIC1                         */
#define VECNUM_PIM          4           /* PCI inbound message          */
#define VECNUM_PCRW         5           /* PCI command reg write        */
#define VECNUM_PPM          6           /* PCI power management         */
#define VECNUM_MSI0         7           /* PCI MSI level 0              */
#define VECNUM_MSI1         8           /* PCI MSI level 0              */
#define VECNUM_MSI2         9           /* PCI MSI level 0              */
#define VECNUM_UIC2NC       10          /* UIC2 non-critical interrupt  */
#define VECNUM_UIC2C        11          /* UIC2 critical interrupt      */
#define VECNUM_D0           12          /* DMA channel 0                */
#define VECNUM_D1           13          /* DMA channel 1                */
#define VECNUM_D2           14          /* DMA channel 2                */
#define VECNUM_D3           15          /* DMA channel 3                */
#define VECNUM_UIC3NC       16          /* UIC3 non-critical interrupt  */
#define VECNUM_UIC3C        17          /* UIC3 critical interrupt      */
#define VECNUM_UIC1NC       30          /* UIC1 non-critical interrupt  */
#define VECNUM_UIC1C        31          /* UIC1 critical interrupt      */

/* UIC 1 */
#define VECNUM_MS           (32 + 1 )   /* MAL SERR                     */
#define VECNUM_TXDE         (32 + 2 )   /* MAL TXDE                     */
#define VECNUM_RXDE         (32 + 3 )   /* MAL RXDE                     */
#define VECNUM_MTE          (32 + 6 )   /* MAL Tx EOB                   */
#define VECNUM_MRE          (32 + 7 )   /* MAL Rx EOB                   */
#define VECNUM_CT0          (32 + 12 )  /* GPT compare timer 0          */
#define VECNUM_CT1          (32 + 13 )  /* GPT compare timer 1          */
#define VECNUM_CT2          (32 + 14 )  /* GPT compare timer 2          */
#define VECNUM_CT3          (32 + 15 )  /* GPT compare timer 3          */
#define VECNUM_CT4          (32 + 16 )  /* GPT compare timer 4          */
#define VECNUM_ETH0         (32 + 28)   /* Ethernet interrupt status    */
#define VECNUM_EWU0         (32 + 29)   /* Emac  wakeup                 */

/* UIC 2 */
#define VECNUM_EIR5         (64 + 24)   /* External interrupt 5         */
#define VECNUM_EIR4         (64 + 25)   /* External interrupt 4         */
#define VECNUM_EIR3         (64 + 26)   /* External interrupt 3         */
#define VECNUM_EIR2         (64 + 27)   /* External interrupt 2         */
#define VECNUM_EIR1         (64 + 28)   /* External interrupt 1         */
#define VECNUM_EIR0         (64 + 29)   /* External interrupt 0         */

#elif defined(CONFIG_440SP)

/* UIC 0 */
#define VECNUM_U0           0           /* UART0                        */
#define VECNUM_U1           1           /* UART1                        */
#define VECNUM_IIC0         2           /* IIC0                         */
#define VECNUM_IIC1         3           /* IIC1                         */
#define VECNUM_PIM          4           /* PCI inbound message          */
#define VECNUM_PCRW         5           /* PCI command reg write        */
#define VECNUM_PPM          6           /* PCI power management         */
#define VECNUM_UIC1NC       30          /* UIC1 non-critical interrupt  */
#define VECNUM_UIC1C        31          /* UIC1 critical interrupt      */

/* UIC 1 */
#define VECNUM_EIR0         (32 + 0)	/* External interrupt 0         */
#define VECNUM_MS           (32 + 1)	/* MAL SERR                     */
#define VECNUM_TXDE         (32 + 2)	/* MAL TXDE                     */
#define VECNUM_RXDE         (32 + 3)	/* MAL RXDE                     */
#define VECNUM_MTE          (32 + 6)	/* MAL Tx EOB                   */
#define VECNUM_MRE          (32 + 7)	/* MAL Rx EOB                   */
#define VECNUM_CT0          (32 + 12)	/* GPT compare timer 0          */
#define VECNUM_CT1          (32 + 13)	/* GPT compare timer 1          */
#define VECNUM_CT2          (32 + 14)	/* GPT compare timer 2          */
#define VECNUM_CT3          (32 + 15)	/* GPT compare timer 3          */
#define VECNUM_CT4          (32 + 16)	/* GPT compare timer 4          */
#define VECNUM_ETH0         (32 + 28)	/* Ethernet interrupt status    */
#define VECNUM_EWU0         (32 + 29)	/* Emac  wakeup                 */

#elif defined(CONFIG_440)

/* UIC 0 */
#define VECNUM_U0           0           /* UART0                        */
#define VECNUM_U1           1           /* UART1                        */
#define VECNUM_IIC0         2           /* IIC0                         */
#define VECNUM_IIC1         3           /* IIC1                         */
#define VECNUM_PIM          4           /* PCI inbound message          */
#define VECNUM_PCRW         5           /* PCI command reg write        */
#define VECNUM_PPM          6           /* PCI power management         */
#define VECNUM_MSI0         7           /* PCI MSI level 0              */
#define VECNUM_MSI1         8           /* PCI MSI level 0              */
#define VECNUM_MSI2         9           /* PCI MSI level 0              */
#define VECNUM_MTE          10          /* MAL TXEOB                    */
#define VECNUM_MRE          11          /* MAL RXEOB                    */
#define VECNUM_D0           12          /* DMA channel 0                */
#define VECNUM_D1           13          /* DMA channel 1                */
#define VECNUM_D2           14          /* DMA channel 2                */
#define VECNUM_D3           15          /* DMA channel 3                */
#define VECNUM_CT0          18          /* GPT compare timer 0          */
#define VECNUM_CT1          19          /* GPT compare timer 1          */
#define VECNUM_CT2          20          /* GPT compare timer 2          */
#define VECNUM_CT3          21          /* GPT compare timer 3          */
#define VECNUM_CT4          22          /* GPT compare timer 4          */
#define VECNUM_EIR0         23          /* External interrupt 0         */
#define VECNUM_EIR1         24          /* External interrupt 1         */
#define VECNUM_EIR2         25          /* External interrupt 2         */
#define VECNUM_EIR3         26          /* External interrupt 3         */
#define VECNUM_EIR4         27          /* External interrupt 4         */
#define VECNUM_EIR5         28          /* External interrupt 5         */
#define VECNUM_EIR6         29          /* External interrupt 6         */
#define VECNUM_UIC1NC       30          /* UIC1 non-critical interrupt  */
#define VECNUM_UIC1C        31          /* UIC1 critical interrupt      */

/* UIC 1 */
#define VECNUM_MS           (32 + 0 )   /* MAL SERR                     */
#define VECNUM_TXDE         (32 + 1 )   /* MAL TXDE                     */
#define VECNUM_RXDE         (32 + 2 )   /* MAL RXDE                     */
#define VECNUM_USBDEV	    (32 + 23)   /* USB 1.1/USB 2.0 Device       */
#define VECNUM_ETH0         (32 + 28)   /* Ethernet 0 interrupt status  */
#define VECNUM_EWU0         (32 + 29)   /* Ethernet 0 wakeup            */

#else /* !defined(CONFIG_440) */

#if defined(CONFIG_405EZ)
#define VECNUM_D0		0	/* DMA channel 0		*/
#define VECNUM_D1		1	/* DMA channel 1		*/
#define VECNUM_D2		2	/* DMA channel 2		*/
#define VECNUM_D3		3	/* DMA channel 3		*/
#define VECNUM_1588		4	/* IEEE 1588 network synchronization */
#define VECNUM_U0		5	/* UART0			*/
#define VECNUM_U1		6	/* UART1			*/
#define VECNUM_CAN0		7	/* CAN 0			*/
#define VECNUM_CAN1		8	/* CAN 1			*/
#define VECNUM_SPI		9	/* SPI				*/
#define VECNUM_IIC0		10	/* I2C				*/
#define VECNUM_CHT0		11	/* Chameleon timer high pri interrupt */
#define VECNUM_CHT1		12	/* Chameleon timer high pri interrupt */
#define VECNUM_USBH1		13	/* USB Host 1			*/
#define VECNUM_USBH2		14	/* USB Host 2			*/
#define VECNUM_USBDEV		15	/* USB Device			*/
#define VECNUM_ETH0		16	/* 10/100 Ethernet interrupt status */
#define VECNUM_EWU0		17	/* Ethernet wakeup sequence detected */

#define VECNUM_MADMAL		18	/* Logical OR of following MadMAL int */
#define VECNUM_MS		18	/*	MAL_SERR_INT 		*/
#define VECNUM_TXDE		18	/* 	MAL_TXDE_INT 		*/
#define VECNUM_RXDE		18	/*	MAL_RXDE_INT 		*/

#define VECNUM_MTE		19	/* MAL TXEOB			*/
#define VECNUM_MTE1		20	/* MAL TXEOB1			*/
#define VECNUM_MRE		21	/* MAL RXEOB			*/
#define VECNUM_NAND		22	/* NAND Flash controller	*/
#define VECNUM_ADC		23	/* ADC				*/
#define VECNUM_DAC		24	/* DAC				*/
#define VECNUM_OPB2PLB		25	/* OPB to PLB bridge interrupt	*/
#define VECNUM_RESERVED0	26	/* Reserved			*/
#define VECNUM_EIR0		27	/* External interrupt 0		*/
#define VECNUM_EIR1		28	/* External interrupt 1		*/
#define VECNUM_EIR2		29	/* External interrupt 2		*/
#define VECNUM_EIR3		30	/* External interrupt 3		*/
#define VECNUM_EIR4		31	/* External interrupt 4		*/

#elif defined(CONFIG_405EX)

/* UIC 0 */
#define VECNUM_U0		00
#define VECNUM_U1		01
#define VECNUM_IIC0		02
#define VECNUM_PKA		03
#define VECNUM_TRNG		04
#define VECNUM_EBM		05
#define VECNUM_BGI		06
#define VECNUM_IIC1		07
#define VECNUM_SPI		08
#define VECNUM_EIR0		09
#define VECNUM_MTE		10	/* MAL Tx EOB */
#define VECNUM_MRE		11	/* MAL Rx EOB */
#define VECNUM_DMA0		12
#define VECNUM_DMA1		13
#define VECNUM_DMA2		14
#define VECNUM_DMA3		15
#define VECNUM_PCIE0AL		16
#define VECNUM_PCIE0VPD		17
#define VECNUM_RPCIE0HRST	18
#define VECNUM_FPCIE0HRST	19
#define VECNUM_PCIE0TCR		20
#define VECNUM_PCIEMSI0		21
#define VECNUM_PCIEMSI1		22
#define VECNUM_SECURITY		23
#define VECNUM_ETH0		24
#define VECNUM_ETH1		25
#define VECNUM_PCIEMSI2		26
#define VECNUM_EIR4		27
#define VECNUM_UIC2NC		28
#define VECNUM_UIC2C		29
#define VECNUM_UIC1NC		30
#define VECNUM_UIC1C		31

/* UIC 1 */
#define VECNUM_MS		(32 + 00)	/* MAL SERR */
#define VECNUM_TXDE		(32 + 01)	/* MAL TXDE */
#define VECNUM_RXDE		(32 + 02)	/* MAL RXDE */
#define VECNUM_PCIE0BMVC0	(32 + 03)
#define VECNUM_PCIE0DCRERR	(32 + 04)
#define VECNUM_EBC		(32 + 05)
#define VECNUM_NDFC		(32 + 06)
#define VECNUM_PCEI1DCRERR	(32 + 07)
#define VECNUM_CT8		(32 + 08)
#define VECNUM_CT9		(32 + 09)
#define VECNUM_PCIE1AL		(32 + 10)
#define VECNUM_PCIE1VPD		(32 + 11)
#define VECNUM_RPCE1HRST	(32 + 12)
#define VECNUM_FPCE1HRST	(32 + 13)
#define VECNUM_PCIE1TCR		(32 + 14)
#define VECNUM_PCIE1VC0		(32 + 15)
#define VECNUM_CT3		(32 + 16)
#define VECNUM_CT4		(32 + 17)
#define VECNUM_EIR7		(32 + 18)
#define VECNUM_EIR8		(32 + 19)
#define VECNUM_EIR9		(32 + 20)
#define VECNUM_CT5		(32 + 21)
#define VECNUM_CT6		(32 + 22)
#define VECNUM_CT7		(32 + 23)
#define VECNUM_SROM		(32 + 24)	/* SERIAL ROM */
#define VECNUM_GPTDECPULS	(32 + 25)	/* GPT Decrement pulse */
#define VECNUM_EIR2		(32 + 26)
#define VECNUM_EIR5		(32 + 27)
#define VECNUM_EIR6		(32 + 28)
#define VECNUM_EMAC0WAKE	(32 + 29)
#define VECNUM_EIR1		(32 + 30)
#define VECNUM_EMAC1WAKE	(32 + 31)

/* UIC 2 */
#define VECNUM_PCIE0INTA	(64 + 00)	/* PCIE0 INTA */
#define VECNUM_PCIE0INTB	(64 + 01)	/* PCIE0 INTB */
#define VECNUM_PCIE0INTC	(64 + 02)	/* PCIE0 INTC */
#define VECNUM_PCIE0INTD	(64 + 03)	/* PCIE0 INTD */
#define VECNUM_EIR3		(64 + 04)	/* External IRQ 3 */
#define VECNUM_DDRMCUE		(64 + 05)
#define VECNUM_DDRMCCE		(64 + 06)
#define VECNUM_MALINTCOATX0	(64 + 07)	/* Interrupt coalecence TX0 */
#define VECNUM_MALINTCOATX1	(64 + 08)	/* Interrupt coalecence TX1 */
#define VECNUM_MALINTCOARX0	(64 + 09)	/* Interrupt coalecence RX0 */
#define VECNUM_MALINTCOARX1	(64 + 10)	/* Interrupt coalecence RX1 */
#define VECNUM_PCIE1INTA	(64 + 11)	/* PCIE0 INTA */
#define VECNUM_PCIE1INTB	(64 + 12)	/* PCIE0 INTB */
#define VECNUM_PCIE1INTC	(64 + 13)	/* PCIE0 INTC */
#define VECNUM_PCIE1INTD	(64 + 14)	/* PCIE0 INTD */
#define VECNUM_RPCIEMSI2	(64 + 15)	/* MSI level 2 */
#define VECNUM_PCIEMSI3		(64 + 16)	/* MSI level 2 */
#define VECNUM_PCIEMSI4		(64 + 17)	/* MSI level 2 */
#define VECNUM_PCIEMSI5		(64 + 18)	/* MSI level 2 */
#define VECNUM_PCIEMSI6		(64 + 19)	/* MSI level 2 */
#define VECNUM_PCIEMSI7		(64 + 20)	/* MSI level 2 */
#define VECNUM_PCIEMSI8		(64 + 21)	/* MSI level 2 */
#define VECNUM_PCIEMSI9		(64 + 22)	/* MSI level 2 */
#define VECNUM_PCIEMSI10	(64 + 23)	/* MSI level 2 */
#define VECNUM_PCIEMSI11	(64 + 24)	/* MSI level 2 */
#define VECNUM_PCIEMSI12	(64 + 25)	/* MSI level 2 */
#define VECNUM_PCIEMSI13	(64 + 26)	/* MSI level 2 */
#define VECNUM_PCIEMSI14	(64 + 27)	/* MSI level 2 */
#define VECNUM_PCIEMSI15	(64 + 28)	/* MSI level 2 */
#define VECNUM_PLB4XAHB		(64 + 29)	/* PLBxAHB bridge */
#define VECNUM_USBWAKE		(64 + 30)	/* USB wakup */
#define VECNUM_USBOTG		(64 + 31)	/* USB OTG */

#else	/* !CONFIG_405EZ */

#define VECNUM_U0           0           /* UART0                        */
#define VECNUM_U1           1           /* UART1                        */
#define VECNUM_D0           5           /* DMA channel 0                */
#define VECNUM_D1           6           /* DMA channel 1                */
#define VECNUM_D2           7           /* DMA channel 2                */
#define VECNUM_D3           8           /* DMA channel 3                */
#define VECNUM_EWU0         9           /* Ethernet wakeup              */
#define VECNUM_MS           10          /* MAL SERR                     */
#define VECNUM_MTE          11          /* MAL TXEOB                    */
#define VECNUM_MRE          12          /* MAL RXEOB                    */
#define VECNUM_TXDE         13          /* MAL TXDE                     */
#define VECNUM_RXDE         14          /* MAL RXDE                     */
#define VECNUM_ETH0         15          /* Ethernet interrupt status    */
#define VECNUM_EIR0         25          /* External interrupt 0         */
#define VECNUM_EIR1         26          /* External interrupt 1         */
#define VECNUM_EIR2         27          /* External interrupt 2         */
#define VECNUM_EIR3         28          /* External interrupt 3         */
#define VECNUM_EIR4         29          /* External interrupt 4         */
#define VECNUM_EIR5         30          /* External interrupt 5         */
#define VECNUM_EIR6         31          /* External interrupt 6         */
#endif	/* defined(CONFIG_405EZ) */

#endif /* defined(CONFIG_440) */

#endif /* _VECNUMS_H_ */
