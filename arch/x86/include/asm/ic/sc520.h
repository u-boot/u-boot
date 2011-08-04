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

#ifndef __ASSEMBLY__

void init_sc520(void);
unsigned long init_sc520_dram(void);
void sc520_udelay(unsigned long usec);

/* Memory mapped configuration registers */
typedef struct sc520_mmcr {
	u16 revid;	/* ElanSC520 microcontroller revision id */
	u8  cpuctl;	/* am5x86 CPU control  */

	u8  pad_0x003[0x0d];

	u8  drcctl;		/* SDRAM control */
	u8  pad_0x011[0x01];
	u8  drctmctl;		/* SDRAM timing control */
	u8  pad_0x013[0x01];
	u16 drccfg;		/* SDRAM bank configuration*/
	u8  pad_0x016[0x02];
	u32 drcbendadr;		/* SDRAM bank 0-3 ending address*/
	u8  pad_0x01c[0x04];
	u8  eccctl;		/* ECC control */
	u8  eccsta;		/* ECC status */
	u8  eccckbpos;		/* ECC check bit position */
	u8  ecccktest;		/* ECC Check Code Test */
	u32 eccsbadd;		/* ECC single-bit error address */
	u32 eccmbadd;		/* ECC multi-bit error address */

	u8  pad_0x02c[0x14];

	u8  dbctl;		/* SDRAM buffer control */

	u8  pad_0x041[0x0f];

	u16 bootcsctl;		/* /BOOTCS control */
	u8  pad_0x052[0x02];
	u16 romcs1ctl;		/* /ROMCS1 control */
	u16 romcs2ctl;		/* /ROMCS2 control */

	u8  pad_0x058[0x08];

	u16 hbctl;		/* host bridge control */
	u16 hbtgtirqctl;	/* host bridge target interrupt control */
	u16 hbtgtirqsta;	/* host bridge target interrupt status */
	u16 hbmstirqctl;	/* host bridge target interrupt control */
	u16 hbmstirqsta;	/* host bridge master interrupt status */
	u8  pad_0x06a[0x02];
	u32 mstintadd;		/* host bridge master interrupt address */

	u8  sysarbctl;		/* system arbiter control */
	u8  pciarbsta;		/* PCI bus arbiter status */
	u16 sysarbmenb;		/* system arbiter master enable */
	u32 arbprictl;		/* arbiter priority control */

	u8  pad_0x078[0x08];

	u8  adddecctl;		/* address decode control */
	u8  pad_0x081[0x01];
	u16 wpvsta;		/* write-protect violation status */
	u8  pad_0x084[0x04];
	u32 par[16];		/* programmable address regions */

	u8  pad_0x0c8[0x0b38];

	u8  gpecho;		/* GP echo mode */
	u8  gpcsdw;		/* GP chip select data width */
	u16 gpcsqual;		/* GP chip select qualification */
	u8  pad_0xc04[0x4];
	u8  gpcsrt;		/* GP chip select recovery time */
	u8  gpcspw;		/* GP chip select pulse width */
	u8  gpcsoff;		/* GP chip select offset */
	u8  gprdw;		/* GP read pulse width */
	u8  gprdoff;		/* GP read offset */
	u8  gpwrw;		/* GP write pulse width */
	u8  gpwroff;		/* GP write offset */
	u8  gpalew;		/* GP ale pulse width */
	u8  gpaleoff;		/* GP ale offset */

	u8  pad_0xc11[0x0f];

	u16 piopfs15_0;		/* PIO15-PIO0 pin function select */
	u16 piopfs31_16;	/* PIO31-PIO16 pin function select */
	u8  cspfs;		/* chip select pin function select */
	u8  pad_0xc25[0x01];
	u8  clksel;		/* clock select */
	u8  pad_0xc27[0x01];
	u16 dsctl;		/* drive strength control */
	u16 piodir15_0;		/* PIO15-PIO0 direction */
	u16 piodir31_16;	/* PIO31-PIO16 direction */
	u8  pad_0xc2e[0x02];
	u16 piodata15_0	;	/* PIO15-PIO0 data */
	u16 piodata31_16;	/* PIO31-PIO16 data */
	u16 pioset15_0;		/* PIO15-PIO0 set */
	u16 pioset31_16;	/* PIO31-PIO16 set */
	u16 pioclr15_0;		/* PIO15-PIO0 clear */
	u16 pioclr31_16;	/* PIO31-PIO16 clear */

	u8  pad_0xc3c[0x24];

	u16 swtmrmilli;		/* software timer millisecond count */
	u16 swtmrmicro;		/* software timer microsecond count */
	u8  swtmrcfg;		/* software timer configuration */

	u8  pad_0xc65[0x0b];

	u8  gptmrsta;		/* GP timers status register */
	u8  pad_0xc71;
	u16 gptmr0ctl;		/* GP timer 0 mode/control */
	u16 gptmr0cnt;		/* GP timer 0 count */
	u16 gptmr0maxcmpa;	/* GP timer 0 maxcount compare A */
	u16 gptmr0maxcmpb;	/* GP timer 0 maxcount compare B */
	u16 gptmr1ctl;		/* GP timer 1 mode/control */
	u16 gptmr1cnt;		/* GP timer 1 count */
	u16 gptmr1maxcmpa;	/* GP timer 1 maxcount compare A */
	u16 gptmr1maxcmpb;	/* GP timer 1 maxcount compare B*/
	u16 gptmr2ctl;		/* GP timer 2 mode/control */
	u16 gptmr2cnt;		/* GP timer 2 count */
	u8  pad_0xc86[0x08];
	u16 gptmr2maxcmpa;	/* GP timer 2 maxcount compare A */

	u8  pad_0xc90[0x20];

	u16 wdtmrctl;		/* watchdog timer control */
	u16 wdtmrcntl;		/* watchdog timer count low */
	u16 wdtmrcnth;		/* watchdog timer count high */

	u8  pad_0xcb6[0x0a];

	u8  uart1ctl;		/* UART 1 general control */
	u8  uart1sta;		/* UART 1 general status */
	u8  uart1fcrshad;	/* UART 1 FIFO control shadow */
	u8  pad_0xcc3[0x01];
	u8  uart2ctl;		/* UART 2 general control */
	u8  uart2sta;		/* UART 2 general status */
	u8  uart2fcrshad;	/* UART 2 FIFO control shadow */

	u8  pad_0xcc7[0x09];

	u8  ssictl;		/* SSI control */
	u8  ssixmit;		/* SSI transmit */
	u8  ssicmd;		/* SSI command */
	u8  ssista;		/* SSI status */
	u8  ssircv;		/* SSI receive */

	u8  pad_0xcd5[0x2b];

	u8  picicr;		/* interrupt control */
	u8  pad_0xd01[0x01];
	u8  pic_mode[3];	/* PIC interrupt mode */
	u8  pad_0xd05[0x03];
	u16 swint16_1;		/* software interrupt 16-1 control */
	u8  swint22_17;		/* software interrupt 22-17/NMI control */
	u8  pad_0xd0b[0x05];
	u16 intpinpol;		/* interrupt pin polarity */
	u8  pad_0xd12[0x02];
	u16 pcihostmap;		/* PCI host bridge interrupt mapping */
	u8  pad_0xd16[0x02];
	u16 eccmap;		/* ECC interrupt mapping */
	u8  gp_tmr_int_map[3];	/* GP timer interrupt mapping */
	u8  pad_0xd1d[0x03];
	u8  pit_int_map[3];	/* PIT interrupt mapping */
	u8  pad_0xd23[0x05];
	u8  uart_int_map[2];	/* UART interrupt mapping */
	u8  pad_0xd2a[0x06];
	u8  pci_int_map[4];	/* PCI interrupt mapping (A through D)*/
	u8  pad_0xd34[0x0c];
	u8  dmabcintmap;	/* DMA buffer chaining interrupt mapping */
	u8  ssimap;		/* SSI interrupt mapping register */
	u8  wdtmap;		/* watchdog timer interrupt mapping */
	u8  rtcmap;		/* RTC interrupt mapping register */
	u8  wpvmap;		/* write-protect interrupt mapping */
	u8  icemap;		/* AMDebug JTAG Rx/Tx interrupt mapping */
	u8  ferrmap;		/* floating point error interrupt mapping */
	u8  pad_0xd47[0x09];
	u8  gp_int_map[11];	/* GP IRQ interrupt mapping */

	u8  pad_0xd5b[0x15];

	u8  sysinfo;		/* system board information */
	u8  pad_0xd71[0x01];
	u8  rescfg;		/* reset configuration */
	u8  pad_0xd73[0x01];
	u8  ressta;		/* reset status */

	u8  pad_0xd75[0x0b];

	u8  gpdmactl;		/* GP-DMA Control */
	u8  gpdmammio;		/* GP-DMA memory-mapped I/O */
	u16 gpdmaextchmapa;	/* GP-DMA resource channel map a */
	u16 gpdmaextchmapb;	/* GP-DMA resource channel map b */
	u8  gp_dma_ext_pg_0;	/* GP-DMA channel extended page 0 */
	u8  gp_dma_ext_pg_1;	/* GP-DMA channel extended page 0 */
	u8  gp_dma_ext_pg_2;	/* GP-DMA channel extended page 0 */
	u8  gp_dma_ext_pg_3;	/* GP-DMA channel extended page 0 */
	u8  gp_dma_ext_pg_5;	/* GP-DMA channel extended page 0 */
	u8  gp_dma_ext_pg_6;	/* GP-DMA channel extended page 0 */
	u8  gp_dma_ext_pg_7;	/* GP-DMA channel extended page 0 */
	u8  pad_0xd8d[0x03];
	u8  gpdmaexttc3;	/* GP-DMA channel 3 extender transfer count */
	u8  gpdmaexttc5;	/* GP-DMA channel 5 extender transfer count */
	u8  gpdmaexttc6;	/* GP-DMA channel 6 extender transfer count */
	u8  gpdmaexttc7;	/* GP-DMA channel 7 extender transfer count */
	u8  pad_0xd94[0x4];
	u8  gpdmabcctl;		/* buffer chaining control */
	u8  gpdmabcsta;		/* buffer chaining status */
	u8  gpdmabsintenb;	/* buffer chaining interrupt enable */
	u8  gpdmabcval;		/* buffer chaining valid */
	u8  pad_0xd9c[0x04];
	u16 gpdmanxtaddl3;	/* GP-DMA channel 3 next address low */
	u16 gpdmanxtaddh3;	/* GP-DMA channel 3 next address high */
	u16 gpdmanxtaddl5;	/* GP-DMA channel 5 next address low */
	u16 gpdmanxtaddh5;	/* GP-DMA channel 5 next address high */
	u16 gpdmanxtaddl6;	/* GP-DMA channel 6 next address low */
	u16 gpdmanxtaddh6;	/* GP-DMA channel 6 next address high */
	u16 gpdmanxtaddl7;	/* GP-DMA channel 7 next address low */
	u16 gpdmanxtaddh7;	/* GP-DMA channel 7 next address high */
	u16 gpdmanxttcl3;	/* GP-DMA channel 3 next transfer count low */
	u16 gpdmanxttch3;	/* GP-DMA channel 3 next transfer count high */
	u16 gpdmanxttcl5;	/* GP-DMA channel 5 next transfer count low */
	u16 gpdmanxttch5;	/* GP-DMA channel 5 next transfer count high */
	u16 gpdmanxttcl6;	/* GP-DMA channel 6 next transfer count low */
	u16 gpdmanxttch6;	/* GP-DMA channel 6 next transfer count high */
	u16 gpdmanxttcl7;	/* GP-DMA channel 7 next transfer count low */
	u16 gpdmanxttch7;	/* GP-DMA channel 7 next transfer count high */

	u8  pad_0xdc0[0x0240];
} sc520_mmcr_t;

extern sc520_mmcr_t *sc520_mmcr;

#endif

/* Memory Mapped Control Registers (MMCR) Base Address */
#define SC520_MMCR_BASE		0xfffef000

/* MMCR Addresses (required for assembler code) */
#define SC520_DRCCTL		(SC520_MMCR_BASE + 0x010)
#define SC520_DRCTMCTL		(SC520_MMCR_BASE + 0x012)
#define SC520_DRCCFG		(SC520_MMCR_BASE + 0x014)
#define SC520_DRCBENDADR	(SC520_MMCR_BASE + 0x018)
#define SC520_ECCCTL		(SC520_MMCR_BASE + 0x020)
#define SC520_DBCTL		(SC520_MMCR_BASE + 0x040)
#define SC520_ECCINT		(SC520_MMCR_BASE + 0xd18)

#define SC520_PAR0		(SC520_MMCR_BASE + 0x088)
#define SC520_PAR1		(SC520_PAR0 + (0x04 * 1))
#define SC520_PAR2		(SC520_PAR0 + (0x04 * 2))
#define SC520_PAR3		(SC520_PAR0 + (0x04 * 3))
#define SC520_PAR4		(SC520_PAR0 + (0x04 * 4))
#define SC520_PAR5		(SC520_PAR0 + (0x04 * 5))
#define SC520_PAR6		(SC520_PAR0 + (0x04 * 6))
#define SC520_PAR7		(SC520_PAR0 + (0x04 * 7))
#define SC520_PAR8		(SC520_PAR0 + (0x04 * 8))
#define SC520_PAR9		(SC520_PAR0 + (0x04 * 9))
#define SC520_PAR10		(SC520_PAR0 + (0x04 * 10))
#define SC520_PAR11		(SC520_PAR0 + (0x04 * 11))
#define SC520_PAR12		(SC520_PAR0 + (0x04 * 12))
#define SC520_PAR13		(SC520_PAR0 + (0x04 * 13))
#define SC520_PAR14		(SC520_PAR0 + (0x04 * 14))
#define SC520_PAR15		(SC520_PAR0 + (0x04 * 15))

/*
 * PARs for maximum allowable 256MB of SDRAM @ 0x00000000
 * Two PARs are required due to maximum PAR size of 128MB
 * These are used in the SDRAM sizing code to disable caching
 *
 * 111 0 0 0 1 11111111111 00000000000000 }- 0xe3ffc000
 * 111 0 0 0 1 11111111111 00100000000000 }- 0xe3ffc800
 * \ / | | | | \----+----/ \-----+------/
 *  |  | | | |      |            +---------- Start at 0x00000000
 *  |  | | | |      |                                 0x08000000
 *  |  | | | |      +----------------------- 128MB Region Size
 *  |  | | | |                               ((2047 + 1) * 64kB)
 *  |  | | | +------------------------------ 64kB Page Size
 *  |  | | +-------------------------------- Writes Enabled
 *  |  | +---------------------------------- Caching Enabled
 *  |  +------------------------------------ Execution Enabled
 *  +--------------------------------------- SDRAM
 */
#define SC520_SDRAM1_PAR	0xe3ffc000
#define SC520_SDRAM2_PAR	0xe3ffc800

#define SC520_PAR_WRITE_DIS	0x04000000
#define SC520_PAR_CACHE_DIS	0x08000000
#define SC520_PAR_EXEC_DIS	0x10000000

/*
 * Programmable Address Regions to cover 256MB SDRAM (Maximum supported)
 * required for DRAM sizing code
 */

/* MMCR Register bits (not all of them :) ) */

/* SSI Stuff */
#define CTL_CLK_SEL_4		0x00	/* Nominal Bit Rate = 8 MHz    */
#define CTL_CLK_SEL_8		0x10	/* Nominal Bit Rate = 4 MHz    */
#define CTL_CLK_SEL_16		0x20	/* Nominal Bit Rate = 2 MHz    */
#define CTL_CLK_SEL_32		0x30	/* Nominal Bit Rate = 1 MHz    */
#define CTL_CLK_SEL_64		0x40	/* Nominal Bit Rate = 512 KHz  */
#define CTL_CLK_SEL_128		0x50	/* Nominal Bit Rate = 256 KHz  */
#define CTL_CLK_SEL_256		0x60	/* Nominal Bit Rate = 128 KHz  */
#define CTL_CLK_SEL_512		0x70	/* Nominal Bit Rate = 64 KHz   */

#define TC_INT_ENB		0x08	/* Transaction Complete Interrupt Enable */
#define PHS_INV_ENB		0x04	/* SSI Inverted Phase Mode Enable */
#define CLK_INV_ENB		0x02	/* SSI Inverted Clock Mode Enable */
#define MSBF_ENB		0x01	/* SSI Most Significant Bit First Mode Enable */

#define SSICMD_CMD_SEL_XMITRCV	0x03	/* Simultaneous Transmit / Receive Transaction */
#define SSICMD_CMD_SEL_RCV	0x02	/* Receive Transaction */
#define SSICMD_CMD_SEL_XMIT	0x01	/* Transmit Transaction */
#define SSISTA_BSY		0x02	/* SSI Busy */
#define SSISTA_TC_INT		0x01	/* SSI Transaction Complete Interrupt */

/* BITS for SC520_ADDDECCTL: */
#define WPV_INT_ENB		0x80	/* Write-Protect Violation Interrupt Enable */
#define IO_HOLE_DEST_PCI	0x10	/* I/O Hole Access Destination */
#define RTC_DIS			0x04	/* RTC Disable */
#define UART2_DIS		0x02	/* UART2 Disable */
#define UART1_DIS		0x01	/* UART1 Disable */

/*
 * Defines used for SDRAM Sizing (number of columns and rows)
 * Refer to section 10.6.4 - SDRAM Sizing Algorithm in the
 * Elan SC520 Microcontroller User's Manual (Order #22004B)
 */
#define CACHELINESZ		0x00000010

#define COL11_ADR		0x0e001e00
#define COL10_ADR		0x0e000e00
#define COL09_ADR		0x0e000600
#define COL08_ADR		0x0e000200
#define COL11_DATA		0x0b0b0b0b
#define COL10_DATA		0x0a0a0a0a
#define COL09_DATA		0x09090909
#define COL08_DATA		0x08080808

#define ROW14_ADR		0x0f000000
#define ROW13_ADR		0x07000000
#define ROW12_ADR		0x03000000
#define ROW11_ADR		0x01000000
#define ROW10_ADR		0x00000000
#define ROW14_DATA		0x3f3f3f3f
#define ROW13_DATA		0x1f1f1f1f
#define ROW12_DATA		0x0f0f0f0f
#define ROW11_DATA		0x07070707
#define ROW10_DATA		0xaaaaaaaa

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

#endif
