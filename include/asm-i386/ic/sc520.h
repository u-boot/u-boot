/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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

#ifndef _ASM_IC_SC520_H_
#define _ASM_IC_SC520_H_ 1

/* Memory mapped configuration registers, MMCR */
#define SC520_REVID		0x0000		/* ElanSC520 Microcontroller Revision ID Register */
#define SC520_CPUCTL		0x0002		/* Am5x86 CPU Control Register */
#define SC520_DRCCTL            0x0010          /* SDRAM Control Register */
#define SC520_DRCTMCTL		0x0012		/* SDRAM Timing Control Register */
#define SC520_DRCCFG		0x0014		/* SDRAM Bank Configuration Register*/
#define SC520_DRCBENDADR	0x0018		/* SDRAM Bank 0-3 Ending Address Register*/
#define SC520_ECCCTL		0x0020		/* ECC Control Register */
#define SC520_ECCSTA		0x0021		/* ECC Status Register */
#define SC520_ECCCKBPOS		0x0022		/* ECC Check Bit Position Register */
#define SC520_ECCSBADD		0x0024		/* ECC Single-Bit Error Address Register */
#define SC520_DBCTL		0x0040		/* SDRAM Buffer Control Register */
#define SC520_BOOTCSCTL		0x0050		/* /BOOTCS Control Register */
#define SC520_ROMCS1CTL		0x0054		/* /ROMCS1 Control Register */
#define SC520_ROMCS2CTL		0x0056		/* /ROMCS2 Control Register */
#define SC520_HBCTL		0x0060 		/* Host Bridge Control Register */
#define SC520_HBTGTIRQCTL	0x0062		/* Host Bridge Target Interrupt Control Register */
#define SC520_HBTGTIRQSTA	0x0064		/* Host Bridge Target Interrupt Status Register */
#define SC520_HBMSTIRQCTL	0x0066		/* Host Bridge Target Interrupt Control Register */
#define SC520_HBMSTIRQSTA	0x0068		/* Host Bridge Master Interrupt Status Register */
#define SC520_MSTINTADD		0x006c		/* Host Bridge Master Interrupt Address Register */
#define SC520_SYSARBCTL		0x0070		/* System Arbiter Control Register */
#define SC520_PCIARBSTA		0x0071		/* PCI Bus Arbiter Status Register */
#define SC520_SYSARBMENB	0x0072		/* System Arbiter Master Enable Register */
#define SC520_ARBPRICTL		0x0074		/* Arbiter Priority Control Register */
#define SC520_ADDDECCTL     	0x0080 		/* Address Decode Control Register */
#define SC520_WPVSTA        	0x0082		/* Write-Protect Violation Status Register */
#define SC520_PAR0          	0x0088		/* Programmable Address Region 0 Register */
#define SC520_PAR1          	0x008c       	/* Programmable Address Region 1 Register */
#define SC520_PAR2          	0x0090       	/* Programmable Address Region 2 Register */
#define SC520_PAR3          	0x0094      	/* Programmable Address Region 3 Register */
#define SC520_PAR4          	0x0098       	/* Programmable Address Region 4 Register */
#define SC520_PAR5          	0x009c       	/* Programmable Address Region 5 Register */
#define SC520_PAR6         	0x00a0      	/* Programmable Address Region 6 Register */
#define SC520_PAR7          	0x00a4       	/* Programmable Address Region 7 Register */
#define SC520_PAR8          	0x00a8       	/* Programmable Address Region 8 Register */
#define SC520_PAR9          	0x00ac       	/* Programmable Address Region 9 Register */
#define SC520_PAR10         	0x00b0       	/* Programmable Address Region 10 Register */
#define SC520_PAR11         	0x00b4       	/* Programmable Address Region 11 Register */
#define SC520_PAR12         	0x00b8       	/* Programmable Address Region 12 Register */
#define SC520_PAR13         	0x00bc       	/* Programmable Address Region 13 Register */
#define SC520_PAR14         	0x00c0       	/* Programmable Address Region 14 Register */
#define SC520_PAR15		0x00c4		/* Programmable Address Region 15 Register */
#define SC520_GPECHO		0x0c00		/* GP Echo Mode Register */
#define SC520_GPCSDW		0x0c01		/* GP Chip Select Data Width Register */
#define SC520_GPCSQUAL		0x0c02		/* GP Chip Select Qualification Register */
#define SC520_GPCSRT		0x0c08		/* GP Chip Select Recovery Time Register */
#define SC520_GPCSPW		0x0c09		/* GP Chip Select Pulse Width Register */
#define SC520_GPCSOFF		0x0c0a		/* GP Chip Select Offset Register */
#define SC520_GPRDW		0x0c0b		/* GP Read Pulse Width Register */
#define SC520_GPRDOFF		0x0c0c		/* GP Read Offset Register */
#define SC520_GPWRW		0x0c0d		/* GP Write Pulse Width Register */
#define SC520_GPWROFF		0x0c0e		/* GP Write Offset Register */
#define SC520_GPALEW		0x0c0f		/* GP ALE Pulse Width Register */
#define SC520_GPALEOFF		0x0c10		/* GP ALE Offset Register */
#define SC520_PIOPFS15_0	0x0c20		/* PIO15-PIO0 Pin Function Select */
#define SC520_PIOPFS31_16	0x0c22		/* PIO31-PIO16 Pin Function Select */
#define SC520_CSPFS		0x0c24		/* Chip Select Pin Function Select */
#define SC520_CLKSEL		0x0c26		/* Clock Select */
#define SC520_DSCTL		0x0c28		/* Drive Strength Control */
#define SC520_PIODIR15_0	0x0c2a		/* PIO15-PIO0 Direction */
#define SC520_PIODIR31_16	0x0c2c		/* PIO31-PIO16 Direction */
#define SC520_PIODATA15_0	0x0c30		/* PIO15-PIO0 Data */
#define SC520_PIODATA31_16	0x0c32		/* PIO31-PIO16 Data */
#define SC520_PIOSET15_0	0x0c34		/* PIO15-PIO0 Set */
#define SC520_PIOSET31_16	0x0c36		/* PIO31-PIO16 Set */
#define SC520_PIOCLR15_0	0x0c38		/* PIO15-PIO0 Clear */
#define SC520_PIOCLR31_16	0x0c3a		/* PIO31-PIO16 Clear */
#define SC520_SWTMRMILLI 	0x0c60		/* Software Timer Millisecond Count */
#define SC520_SWTMRMICRO 	0x0c62		/* Software Timer Microsecond Count */
#define SC520_SWTMRCFG   	0x0c64		/* Software Timer Configuration */
#define SC520_GPTMRSTA		0x0c70		/* GP Timers Status Register */
#define SC520_GPTMR0CTL		0x0c72		/* GP Timer 0 Mode/Control Register */
#define SC520_GPTMR0CNT		0x0c74		/* GP Timer 0 Count Register */
#define SC520_GPTMR0MAXCMPA	0x0c76		/* GP Timer 0 Maxcount Compare A Register */
#define SC520_GPTMR0MAXCMPB	0x0c78		/* GP Timer 0 Maxcount Compare B Register */
#define SC520_GPTMR1CTL		0x0c7a		/* GP Timer 1 Mode/Control Register */
#define SC520_GPTMR1CNT		0x0c7c		/* GP Timer 1 Count Register */
#define SC520_GPTMR1MAXCMPA	0x0c7e		/* GP Timer 1 Maxcount Compare Register A */
#define SC520_GPTMR1MAXCMPB	0x0c80		/* GP Timer 1 Maxcount Compare B Register */
#define SC520_GPTMR2CTL		0x0c82		/* GP Timer 2 Mode/Control Register */
#define SC520_GPTMR2CNT		0x0c84		/* GP Timer 2 Count Register */
#define SC520_GPTMR2MAXCMPA	0x0c8e		/* GP Timer 2 Maxcount Compare A Register */
#define SC520_WDTMRCTL		0x0cb0		/* Watchdog Timer Control Register */
#define SC520_WDTMRCNTL		0x0cb2		/* Watchdog Timer Count Low Register */
#define SC520_WDTMRCNTH		0x0cb4		/* Watchdog Timer Count High Register */
#define SC520_UART1CTL		0x0cc0	        /* UART 1 General Control Register */
#define SC520_UART1STA		0x0cc1	        /* UART 1 General Status Register */
#define SC520_UART1FCRSHAD	0x0cc2	        /* UART 1 FIFO Control Shadow Register */
#define SC520_UART2CTL		0x0cc4	        /* UART 2 General Control Register */
#define SC520_UART2STA		0x0cc5	        /* UART 2 General Status Register */
#define SC520_UART2FCRSHAD	0x0cc6	        /* UART 2 FIFO Control Shadow Register */
#define SC520_SSICTL            0x0cd0          /* SSI Control */
#define SC520_SSIXMIT           0x0cd1          /* SSI Transmit */
#define SC520_SSICMD            0x0cd2          /* SSI Command */
#define SC520_SSISTA            0x0cd3          /* SSI Status */
#define SC520_SSIRCV            0x0cd4          /* SSI Receive */
#define SC520_PICICR		0x0d00		/* Interrupt Control Register */
#define SC520_MPICMODE		0x0d02		/* Master PIC Interrupt Mode Register */
#define SC520_SL1PICMODE	0x0d03		/* Slave 1 PIC Interrupt Mode Register */
#define SC520_SL2PICMODE	0x0d04		/* Slave 2 PIC Interrupt Mode Register */
#define SC520_SWINT16_1		0x0d08		/* Software Interrupt 16-1 Control Register */
#define SC520_SWINT22_17	0x0d0a		/* Software Interrupt 22-17/NMI Control Register */
#define SC520_INTPINPOL		0x0d10		/* Interrupt Pin Polarity Register */
#define SC520_PCIHOSTMAP	0x0d14		/* PCI Host Bridge Interrupt Mappin Register */
#define SC520_ECCMAP		0x0d18		/* ECC Interrupt Mapping Register */
#define SC520_GPTMR0MAP		0x0d1a		/* GP Timer 0 Interrupt Mapping Register */
#define SC520_GPTMR1MAP		0x0d1b		/* GP Timer 1 Interrupt Mapping Register */
#define SC520_GPTMR2MAP		0x0d1c		/* GP Timer 2 Interrupt Mapping Register */
#define SC520_PIT0MAP		0x0d20		/* PIT0 Interrupt Mapping Register */
#define SC520_PIT1MAP		0x0d21		/* PIT1 Interrupt Mapping Register */
#define SC520_PIT2MAP		0x0d22		/* PIT2 Interrupt Mapping Register */
#define SC520_UART1MAP		0x0d28		/* UART 1 Interrupt Mapping Register */
#define SC520_UART2MAP		0x0d29		/* UART 2 Interrupt Mapping Register */
#define SC520_PCIINTAMAP	0x0d30		/* PCI Interrupt A Mapping Register */
#define SC520_PCIINTBMAP	0x0d31		/* PCI Interrupt B Mapping Register */
#define SC520_PCIINTCMAP	0x0d32		/* PCI Interrupt C Mapping Register */
#define SC520_PCIINTDMAP	0x0d33		/* PCI Interrupt D Mapping Register */
#define SC520_DMABCINTMAP	0x0d40		/* DMA Buffer Chaining Interrupt Mapping Register */
#define SC520_SSIMAP		0x0d41		/* SSI Interrupt Mapping Register */
#define SC520_WDTMAP		0x0d42		/* Watchdog Timer Interrupt Mapping Register */
#define SC520_RTCMAP		0x0d43		/* RTC Interrupt Mapping Register */
#define SC520_WPVMAP		0x0d44		/* Write-Protect Interrupt Mapping Register */
#define SC520_ICEMAP		0x0d45		/* AMDebug JTAG RX/TX Interrupt Mapping Register */
#define SC520_FERRMAP		0x0d46		/* Floating Point Error Interrupt Mapping Register */
#define SC520_GP0IMAP		0x0d50		/* GPIRQ0 Interrupt Mapping Register */
#define SC520_GP1IMAP		0x0d51		/* GPIRQ1 Interrupt Mapping Register */
#define SC520_GP2IMAP		0x0d52		/* GPIRQ2 Interrupt Mapping Register */
#define SC520_GP3IMAP		0x0d53		/* GPIRQ3 Interrupt Mapping Register */
#define SC520_GP4IMAP		0x0d54		/* GPIRQ4 Interrupt Mapping Register */
#define SC520_GP5IMAP		0x0d55		/* GPIRQ5 Interrupt Mapping Register */
#define SC520_GP6IMAP		0x0d56		/* GPIRQ6 Interrupt Mapping Register */
#define SC520_GP7IMAP		0x0d57		/* GPIRQ7 Interrupt Mapping Register */
#define SC520_GP8IMAP		0x0d58		/* GPIRQ8 Interrupt Mapping Register */
#define SC520_GP9IMAP		0x0d59		/* GPIRQ9 Interrupt Mapping Register */
#define SC520_GP10IMAP		0x0d5a		/* GPIRQ10 Interrupt Mapping Register */
#define SC520_SYSINFO		0x0d70		/* System Board Information Register */
#define SC520_RESCFG		0x0d72		/* Reset Configuration Register */
#define SC520_RESSTA		0x0d74		/* Reset Status Register */
#define SC520_GPDMAMMIO		0x0d81		/* GP-DMA Memory-Mapped I/O Register */
#define SC520_GPDMAEXTCHMAPA	0x0d82		/* GP-DMA Resource Channel Map A */
#define SC520_GPDMAEXTCHMAPB	0x0d84		/* GP-DMA Resource Channel Map B */
#define SC520_GPDMAEXTPG0	0x0d86		/* GP-DMA Channel 0 Extended Page */
#define SC520_GPDMAEXTPG1	0x0d87		/* GP-DMA Channel 1 Extended Page */
#define SC520_GPDMAEXTPG2	0x0d88		/* GP-DMA Channel 2 Extended Page */
#define SC520_GPDMAEXTPG3	0x0d89		/* GP-DMA Channel 3 Extended Page */
#define SC520_GPDMAEXTPG5	0x0d8a		/* GP-DMA Channel 5 Extended Page */
#define SC520_GPDMAEXTPG6	0x0d8b		/* GP-DMA Channel 6 Extended Page */
#define SC520_GPDMAEXTPG7	0x0d8c		/* GP-DMA Channel 7 Extended Page */
#define SC520_GPDMAEXTTC3	0x0d90		/* GP-DMA Channel 3 Extender Transfer count */
#define SC520_GPDMAEXTTC5	0x0d91		/* GP-DMA Channel 5 Extender Transfer count */
#define SC520_GPDMAEXTTC6	0x0d92		/* GP-DMA Channel 6 Extender Transfer count */
#define SC520_GPDMAEXTTC7	0x0d93		/* GP-DMA Channel 7 Extender Transfer count */
#define SC520_GPDMABCCTL	0x0d98		/* Buffer Chaining Control */
#define SC520_GPDMABCSTA	0x0d99		/* Buffer Chaining Status */
#define SC520_GPDMABSINTENB	0x0d9a		/* Buffer Chaining Interrupt Enable */
#define SC520_GPDMABCVAL	0x0d9b		/* Buffer Chaining Valid */
#define SC520_GPDMANXTADDL3	0x0da0		/* GP-DMA Channel 3 Next Address Low */
#define SC520_GPDMANXTADDH3	0x0da2		/* GP-DMA Channel 3 Next Address High */
#define SC520_GPDMANXTADDL5	0x0da4		/* GP-DMA Channel 5 Next Address Low */
#define SC520_GPDMANXTADDH5	0x0da6		/* GP-DMA Channel 5 Next Address High */
#define SC520_GPDMANXTADDL6	0x0da8		/* GP-DMA Channel 6 Next Address Low */
#define SC520_GPDMANXTADDH6	0x0daa		/* GP-DMA Channel 6 Next Address High */
#define SC520_GPDMANXTADDL7	0x0dac		/* GP-DMA Channel 7 Next Address Low */
#define SC520_GPDMANXTADDH7	0x0dae		/* GP-DMA Channel 7 Next Address High */
#define SC520_GPDMANXTTCL3	0x0db0		/* GP-DMA Channel 3 Next Transfer Count Low */
#define SC520_GPDMANXTTCH3	0x0db2		/* GP-DMA Channel 3 Next Transfer Count High */
#define SC520_GPDMANXTTCL5	0x0db4		/* GP-DMA Channel 5 Next Transfer Count Low */
#define SC520_GPDMANXTTCH5	0x0db6		/* GP-DMA Channel 5 Next Transfer Count High */
#define SC520_GPDMANXTTCL6	0x0db8		/* GP-DMA Channel 6 Next Transfer Count Low */
#define SC520_GPDMANXTTCH6	0x0dba		/* GP-DMA Channel 6 Next Transfer Count High */
#define SC520_GPDMANXTTCL7	0x0dbc		/* GP-DMA Channel 7 Next Transfer Count Low */
#define SC520_GPDMANXTTCH7	0x0dbe		/* GP-DMA Channel 7 Next Transfer Count High */

/* MMCR Register bits (not all of them :) ) */

/* SSI Stuff */
#define CTL_CLK_SEL_4           0x00           /* Nominal Bit Rate = 8 MHz    */
#define CTL_CLK_SEL_8           0x10           /* Nominal Bit Rate = 4 MHz    */
#define CTL_CLK_SEL_16          0x20           /* Nominal Bit Rate = 2 MHz    */
#define CTL_CLK_SEL_32          0x30           /* Nominal Bit Rate = 1 MHz    */
#define CTL_CLK_SEL_64          0x40           /* Nominal Bit Rate = 512 KHz  */
#define CTL_CLK_SEL_128         0x50           /* Nominal Bit Rate = 256 KHz  */
#define CTL_CLK_SEL_256         0x60           /* Nominal Bit Rate = 128 KHz  */
#define CTL_CLK_SEL_512         0x70           /* Nominal Bit Rate = 64 KHz   */

#define TC_INT_ENB              0x08           /* Transaction Complete Interrupt Enable */
#define PHS_INV_ENB             0x04           /* SSI Inverted Phase Mode Enable */
#define CLK_INV_ENB    	        0x02           /* SSI Inverted Clock Mode Enable */
#define MSBF_ENB      	        0x01           /* SSI Most Significant Bit First Mode Enable */

#define SSICMD_CMD_SEL_XMITRCV  0x03           /* Simultaneous Transmit / Receive Transaction */
#define SSICMD_CMD_SEL_RCV      0x02           /* Receive Transaction */
#define SSICMD_CMD_SEL_XMIT     0x01           /* Transmit Transaction */
#define SSISTA_BSY              0x02           /* SSI Busy */
#define SSISTA_TC_INT           0x01           /* SSI Transaction Complete Interrupt */


/* BITS for SC520_ADDDECCTL: */
#define WPV_INT_ENB		0x80		/* Write-Protect Violation Interrupt Enable */
#define IO_HOLE_DEST_PCI	0x10		/* I/O Hole Access Destination */
#define RTC_DIS			0x04		/* RTC Disable */
#define UART2_DIS		0x02		/* UART2 Disable */
#define UART1_DIS		0x01		/* UART1 Disable */

/* bus mapping constants (used for PCI core initialization) */																																																 /* bus mapping constants */
#define SC520_REG_ADDR		0x00000cf8
#define SC520_REG_DATA		0x00000cfc


#define SC520_ISA_MEM_PHYS	0x00000000
#define SC520_ISA_MEM_BUS	0x00000000
#define SC520_ISA_MEM_SIZE	0x01000000

#define SC520_ISA_IO_PHYS	0x00000000
#define SC520_ISA_IO_BUS	0x00000000
#define SC520_ISA_IO_SIZE	0x00001000

/* PCI I/O space from 0x1000 to 0xdfff
 * (make 0xe000-0xfdff available for stuff like PCCard boot) */
#define SC520_PCI_IO_PHYS	0x00001000
#define SC520_PCI_IO_BUS	0x00001000
#define SC520_PCI_IO_SIZE	0x0000d000

/* system memory from 0x00000000 to 0x0fffffff */
#define	SC520_PCI_MEMORY_PHYS	0x00000000
#define	SC520_PCI_MEMORY_BUS	0x00000000
#define SC520_PCI_MEMORY_SIZE	0x10000000

/* PCI bus memory from 0x10000000 to 0x26ffffff
 * (make 0x27000000 - 0x27ffffff available for stuff like PCCard boot) */
#define SC520_PCI_MEM_PHYS	0x10000000
#define SC520_PCI_MEM_BUS       0x10000000
#define SC520_PCI_MEM_SIZE	0x17000000

/* 0x28000000 - 0x3fffffff is used by the flash banks */

/* 0x40000000 - 0xffffffff is not adressable by the SC520 */

/* priority numbers used for interrupt channel mappings */
#define SC520_IRQ_DISABLED 0
#define SC520_IRQ0  1
#define SC520_IRQ1  2
#define SC520_IRQ2  4  /* same as IRQ9 */
#define SC520_IRQ3  11
#define SC520_IRQ4  12
#define SC520_IRQ5  13
#define SC520_IRQ6  21
#define SC520_IRQ7  22
#define SC520_IRQ8  3
#define SC520_IRQ9  4
#define SC520_IRQ10 5
#define SC520_IRQ11 6
#define SC520_IRQ12 7
#define SC520_IRQ13 8
#define SC520_IRQ14 9
#define SC520_IRQ15 10


/* pin number used for PCI interrupt mappings */
#define SC520_PCI_INTA 0
#define SC520_PCI_INTB 1
#define SC520_PCI_INTC 2
#define SC520_PCI_INTD 3
#define SC520_PCI_GPIRQ0 4
#define SC520_PCI_GPIRQ1 5
#define SC520_PCI_GPIRQ2 6
#define SC520_PCI_GPIRQ3 7
#define SC520_PCI_GPIRQ4 8
#define SC520_PCI_GPIRQ5 9
#define SC520_PCI_GPIRQ6 10
#define SC520_PCI_GPIRQ7 11
#define SC520_PCI_GPIRQ8 12
#define SC520_PCI_GPIRQ9 13
#define SC520_PCI_GPIRQ10 14

/* utility functions */
void write_mmcr_byte(u16 mmcr, u8 data);
void write_mmcr_word(u16 mmcr, u16 data);
void write_mmcr_long(u16 mmcr, u32 data);
u8 read_mmcr_byte(u16 mmcr);
u16 read_mmcr_word(u16 mmcr);
u32 read_mmcr_long(u16 mmcr);

extern int sc520_pci_ints[];

void init_sc520(void);
unsigned long init_sc520_dram(void);
void pci_sc520_init(struct pci_controller *hose);
int pci_sc520_set_irq(int pci_pin, int irq);

#endif
