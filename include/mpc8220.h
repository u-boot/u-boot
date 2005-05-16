/*
 * include/mpc8220.h
 *
 * Prototypes, etc. for the Motorola MPC8220
 * embedded cpu chips
 *
 * 2004 (c) Freescale, Inc.
 * Author: TsiChung Liew <Tsi-Chung.Liew@freescale.com>
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
#ifndef __MPC8220_H__
#define __MPC8220_H__

/* Processor name */
#if defined(CONFIG_MPC8220)
#define CPU_ID_STR	    "MPC8220"
#endif

/* Exception offsets (PowerPC standard) */
#define EXC_OFF_SYS_RESET   0x0100

/* Internal memory map */
/* MPC8220 Internal Register MMAP */
#define MMAP_MBAR	(CFG_MBAR + 0x00000000) /* chip selects		     */
#define MMAP_MEMCTL	(CFG_MBAR + 0x00000100) /* sdram controller	     */
#define MMAP_XLBARB	(CFG_MBAR + 0x00000200) /* xlb arbitration control   */
#define MMAP_CDM	(CFG_MBAR + 0x00000300) /* clock distribution module */
#define MMAP_VDOPLL	(CFG_MBAR + 0x00000400) /* video PLL		     */
#define MMAP_FB		(CFG_MBAR + 0x00000500) /* flex bus controller	     */
#define MMAP_PCFG	(CFG_MBAR + 0x00000600) /* port config		     */
#define MMAP_ICTL	(CFG_MBAR + 0x00000700) /* interrupt controller	     */
#define MMAP_GPTMR	(CFG_MBAR + 0x00000800) /* general purpose timers    */
#define MMAP_SLTMR	(CFG_MBAR + 0x00000900) /* slice timers		     */
#define MMAP_GPIO	(CFG_MBAR + 0x00000A00) /* gpio module		     */
#define MMAP_XCPCI	(CFG_MBAR + 0x00000B00) /* pci controller	     */
#define MMAP_PCIARB	(CFG_MBAR + 0x00000C00) /* pci arbiter		     */
#define MMAP_EXTDMA1	(CFG_MBAR + 0x00000D00) /* external dma1	     */
#define MMAP_EXTDMA2	(CFG_MBAR + 0x00000E00) /* external dma1	     */
#define MMAP_USBH	(CFG_MBAR + 0x00001000) /* usb host		     */
#define MMAP_CMTMR	(CFG_MBAR + 0x00007f00) /* comm timers		     */
#define MMAP_DMA	(CFG_MBAR + 0x00008000) /* dma			     */
#define MMAP_USBD	(CFG_MBAR + 0x00008200) /* usb device		     */
#define MMAP_COMMPCI	(CFG_MBAR + 0x00008400) /* pci comm Bus regs	     */
#define MMAP_1284	(CFG_MBAR + 0x00008500) /* 1284			     */
#define MMAP_PEV	(CFG_MBAR + 0x00008600) /* print engine video	     */
#define MMAP_PSC1	(CFG_MBAR + 0x00008800) /* psc1 block		     */
#define MMAP_I2C	(CFG_MBAR + 0x00008f00) /* i2c controller	     */
#define MMAP_FEC1	(CFG_MBAR + 0x00009000) /* fast ethernet 1	     */
#define MMAP_FEC2	(CFG_MBAR + 0x00009800) /* fast ethernet 2	     */
#define MMAP_JBIGRAM	(CFG_MBAR + 0x0000a000) /* jbig RAM		     */
#define MMAP_JBIG	(CFG_MBAR + 0x0000c000) /* jbig			     */
#define MMAP_PDLA	(CFG_MBAR + 0x00010000) /*			     */
#define MMAP_SRAMCFG	(CFG_MBAR + 0x0001ff00) /* SRAM config		     */
#define MMAP_SRAM	(CFG_MBAR + 0x00020000) /* SRAM			     */

#define SRAM_SIZE	0x8000			/* 32 KB */

/* ------------------------------------------------------------------------ */
/*
 * Macro for Programmable Serial Channel
 */
/* equates for mode reg. 1 for channel	A or B */
#define PSC_MR1_RX_RTS		0x80000000    /* receiver RTS enabled */
#define PSC_MR1_RX_INT		0x40000000    /* receiver intrupt enabled */
#define PSC_MR1_ERR_MODE	0x20000000    /* block error mode */
#define PSC_MR1_PAR_MODE_MULTI	0x18000000    /* multi_drop mode */
#define PSC_MR1_NO_PARITY	0x10000000    /* no parity mode */
#define PSC_MR1_ALWAYS_0	0x08000000    /* force parity mode */
#define PSC_MR1_ALWAYS_1	0x0c000000    /* force parity mode */
#define PSC_MR1_EVEN_PARITY	0x00000000    /* parity mode */
#define PSC_MR1_ODD_PARITY	0x04000000    /* 0 = even, 1 = odd */
#define PSC_MR1_BITS_CHAR_8	0x03000000    /* 8 bits */
#define PSC_MR1_BITS_CHAR_7	0x02000000    /* 7 bits */
#define PSC_MR1_BITS_CHAR_6	0x01000000    /* 6 bits */
#define PSC_MR1_BITS_CHAR_5	0x00000000    /* 5 bits */

/* equates for mode reg. 2 for channel	A or B */
#define PSC_MR2_NORMAL_MODE	0x00000000    /* normal channel mode */
#define PSC_MR2_AUTO_MODE	0x40000000    /* automatic channel mode */
#define PSC_MR2_LOOPBACK_LOCL	0x80000000    /* local loopback channel mode */
#define PSC_MR2_LOOPBACK_REMT	0xc0000000    /* remote loopback channel mode */
#define PSC_MR2_TX_RTS		0x20000000    /* transmitter RTS enabled */
#define PSC_MR2_TX_CTS		0x10000000    /* transmitter CTS enabled */
#define PSC_MR2_STOP_BITS_2	0x0f000000    /* 2 stop bits */
#define PSC_MR2_STOP_BITS_1	0x07000000    /* 1 stop bit */

/* equates for status reg. A or B */
#define PSC_SR_BREAK		0x80000000    /* received break */
#define PSC_SR_NEOF		PSC_SR_BREAK  /* Next byte is EOF - MIR/FIR */
#define PSC_SR_FRAMING		0x40000000    /* framing error */
#define PSC_SR_PHYERR		PSC_SR_FRAMING/* Physical Layer error - MIR/FIR */
#define PSC_SR_PARITY		0x20000000    /* parity error */
#define PSC_SR_CRCERR		PSC_SR_PARITY /* CRC error */
#define PSC_SR_OVERRUN		0x10000000    /* overrun error */
#define PSC_SR_TXEMT		0x08000000    /* transmitter empty */
#define PSC_SR_TXRDY		0x04000000    /* transmitter ready*/
#define PSC_SR_FFULL		0x02000000    /* fifo full */
#define PSC_SR_RXRDY		0x01000000    /* receiver ready */
#define PSC_SR_DEOF		0x00800000    /* Detect EOF or RX-FIFO contain EOF */
#define PSC_SR_ERR		0x00400000    /* Error Status including FIFO */

/* equates for clock select reg. */
#define PSC_CSRX16EXT_CLK	0x1110	/* x 16 ext_clock */
#define PSC_CSRX1EXT_CLK	0x1111	/* x 1 ext_clock  */

/* equates for command reg. A or B */
#define PSC_CR_NO_COMMAND	0x00000000    /* no command */
#define PSC_CR_RST_MR_PTR_CMD	0x10000000    /* reset mr pointer command */
#define PSC_CR_RST_RX_CMD	0x20000000    /* reset receiver command */
#define PSC_CR_RST_TX_CMD	0x30000000    /* reset transmitter command */
#define PSC_CR_RST_ERR_STS_CMD	0x40000000    /* reset error status cmnd */
#define PSC_CR_RST_BRK_INT_CMD	0x50000000    /* reset break int. command */
#define PSC_CR_STR_BREAK_CMD	0x60000000    /* start break command */
#define PSC_CR_STP_BREAK_CMD	0x70000000    /* stop break command */
#define PSC_CR_RX_ENABLE	0x01000000    /* receiver enabled */
#define PSC_CR_RX_DISABLE	0x02000000    /* receiver disabled */
#define PSC_CR_TX_ENABLE	0x04000000    /* transmitter enabled */
#define PSC_CR_TX_DISABLE	0x08000000    /* transmitter disabled */

/* equates for input port change reg. */
#define PSC_IPCR_SYNC		0x80000000    /* Sync Detect */
#define PSC_IPCR_D_CTS		0x10000000    /* Delta CTS */
#define PSC_IPCR_CTS		0x01000000    /* CTS - current state of PSC_CTS */

/* equates for auxiliary control reg. (timer and counter clock selects) */
#define PSC_ACR_BRG		0x80000000    /* for 68681 compatibility
						 baud rate gen select
						 0 = set 1; 1 = set 2
						 equates are set 2 ONLY */
#define PSC_ACR_TMR_EXT_CLK_16	0x70000000    /* xtnl clock divided by 16 */
#define PSC_ACR_TMR_EXT_CLK	0x60000000    /* external clock */
#define PSC_ACR_TMR_IP2_16	0x50000000    /* ip2 divided by 16 */
#define PSC_ACR_TMR_IP2		0x40000000    /* ip2 */
#define PSC_ACR_CTR_EXT_CLK_16	0x30000000    /* xtnl clock divided by 16 */
#define PSC_ACR_CTR_TXCB	0x20000000    /* channel B xmitr clock */
#define PSC_ACR_CTR_TXCA	0x10000000    /* channel A xmitr clock */
#define PSC_ACR_CTR_IP2		0x00000000    /* ip2 */
#define PSC_ACR_IEC0		0x01000000    /* interrupt enable ctrl for D_CTS */

/* equates for int. status reg. */
#define PSC_ISR_IPC		0x80000000    /* input port change*/
#define PSC_ISR_BREAK		0x04000000    /* delta break */
#define PSC_ISR_RX_RDY		0x02000000    /* receiver rdy /fifo full */
#define PSC_ISR_TX_RDY		0x01000000    /* transmitter ready */
#define PSC_ISR_DEOF		0x00800000    /* Detect EOF / RX-FIFO contains EOF */
#define PSC_ISR_ERR		0x00400000    /* Error Status including FIFO */

/* equates for int. mask reg. */
#define PSC_IMR_CLEAR		0xff000000    /* Clear the imr */
#define PSC_IMR_IPC		0x80000000    /* input port change*/
#define PSC_IMR_BREAK		0x04000000    /* delta break */
#define PSC_IMR_RX_RDY		0x02000000    /* rcvr ready / fifo full */
#define PSC_IMR_TX_RDY		0x01000000    /* transmitter ready */
#define PSC_IMR_DEOF		0x00800000    /* Detect EOF / RX-FIFO contains EOF */
#define PSC_IMR_ERR		0x00400000    /* Error Status including FIFO */

/* equates for input port reg. */
#define PSC_IP_LPWRB		0x80000000    /* Low power mode in Ac97 */
#define PSC_IP_TGL		0x40000000    /* test usage */
#define PSC_IP_CTS		0x01000000    /* CTS */

/* equates for output port bit set reg. */
#define PSC_OPSET_RTS		0x01000000    /* Assert PSC_RTS output */

/* equates for output port bit reset reg. */
#define PSC_OPRESET_RTS		0x01000000    /* Assert PSC_RTS output */

/* equates for rx FIFO number of data reg. */
#define PSC_RFNUM(x)		((x&0xff)<<24)/* receive count */

/* equates for tx FIFO number of data reg. */
#define PSC_TFNUM(x)		((x&0xff)<<24)/* receive count */

/* equates for rx FIFO status reg */
#define PSC_RFSTAT_TAG(x)	((x&3)<<28)   /* tag */
#define PSC_RFSTAT_FRAME0	0x08	      /* Frame Indicator 0 */
#define PSC_RFSTAT_FRAME1	0x04	      /* Frame Indicator 1 */
#define PSC_RFSTAT_FRAME2	0x02	      /* Frame Indicator 2 */
#define PSC_RFSTAT_FRAME3	0x01	      /* Frame Indicator 3 */
#define PSC_RFSTAT_FRAME(x)	((x&0x0f)<<24)/* Frame indicator */
#define PSC_RFSTAT_ERR		0x00400000    /* Fifo err */
#define PSC_RFSTAT_UF		0x00200000    /* Underflow */
#define PSC_RFSTAT_OF		0x00100000    /* overflow */
#define PSC_RFSTAT_FR		0x00080000    /* frame ready */
#define PSC_RFSTAT_FULL		0x00040000    /* full */
#define PSC_RFSTAT_ALARM	0x00020000    /* alarm */
#define PSC_RFSTAT_EMPTY	0x00010000    /* empty */

/* equates for tx FIFO status reg */
#define PSC_TFSTAT_TAG(x)	((x&3)<<28)   /* tag */
#define PSC_TFSTAT_FRAME0	0x08	      /* Frame Indicator 0 */
#define PSC_TFSTAT_FRAME1	0x04	      /* Frame Indicator 1 */
#define PSC_TFSTAT_FRAME2	0x02	      /* Frame Indicator 2 */
#define PSC_TFSTAT_FRAME3	0x01	      /* Frame Indicator 3 */
#define PSC_TFSTAT_FRAME(x)	((x&0x0f)<<24)/* Frame indicator */
#define PSC_TFSTAT_ERR		0x00400000    /* Fifo err */
#define PSC_TFSTAT_UF		0x00200000    /* Underflow */
#define PSC_TFSTAT_OF		0x00100000    /* overflow */
#define PSC_TFSTAT_FR		0x00080000    /* frame ready */
#define PSC_TFSTAT_FULL		0x00040000    /* full */
#define PSC_TFSTAT_ALARM	0x00020000    /* alarm */
#define PSC_TFSTAT_EMPTY	0x00010000    /* empty */

/* equates for rx FIFO control reg. */
#define PSC_RFCNTL_WTAG(x)	((x&3)<<29)   /* Write tag */
#define PSC_RFCNTL_FRAME	0x08000000    /* Frame mode enable */
#define PSC_RFCNTL_GR(x)	((x&7)<<24)   /* Granularity */

/* equates for tx FIFO control reg. */
#define PSC_TFCNTL_WTAG(x)	((x&3)<<29)   /* Write tag */
#define PSC_TFCNTL_FRAME	0x08000000    /* Frame mode enable */
#define PSC_TFCNTL_GR(x)	((x&7)<<24)   /* Granularity */

/* equates for rx FIFO alarm reg */
#define PSC_RFALARM(x)		(x&0x1ff)     /* Alarm */

/* equates for tx FIFO alarm reg */
#define PSC_TFALARM(x)		(x&0x1ff)     /* Alarm */

/* equates for rx FIFO read pointer */
#define PSC_RFRPTR(x)		(x&0x1ff)     /* read pointer */

/* equates for tx FIFO read pointer */
#define PSC_TFRPTR(x)		(x&0x1ff)     /* read pointer */

/* equates for rx FIFO write pointer */
#define PSC_RFWPTR(x)		(x&0x1ff)     /* write pointer */

/* equates for rx FIFO write pointer */
#define PSC_TFWPTR(x)		(x&0x1ff)     /* write pointer */

/* equates for rx FIFO last read frame pointer reg */
#define PSC_RFLRFPTR(x)		(x&0x1ff)     /* last read frame pointer */

/* equates for tx FIFO last read frame pointer reg */
#define PSC_TFLRFPTR(x)		(x&0x1ff)     /* last read frame pointer */

/* equates for rx FIFO last write frame pointer reg */
#define PSC_RFLWFPTR(x)		(x&0x1ff)     /* last write frame pointer */

/* equates for tx FIFO last write frame pointer reg */
#define PSC_TFLWFPTR(x)		(x&0x1ff)     /* last write frame pointer */

/* PCI configuration (only for PLL determination)*/
#define PCI_REG_PCIGSCR		(MMAP_XCPCI + 0x60) /* Global status/control register */
#define PCI_REG_PCIGSCR_PCI2XLB_CLK_MASK	0x07000000
#define PCI_REG_PCIGSCR_PCI2XLB_CLK_BIT		24

#define PCI_REG_PCICAR		(MMAP_XCPCI + 0xF8) /* Configuration Address Register */

/* ------------------------------------------------------------------------ */
/*
 * Macro for General Purpose Timer
 */
/* Enable and Mode Select */
#define GPT_OCT(x)	    (x & 0x3)<<4/* Output Compare Type */
#define GPT_ICT(x)	    (x & 0x3)	/* Input Capture Type */
#define GPT_CTRL_WDEN	    0x80	/* Watchdog Enable */
#define GPT_CTRL_CE	    0x10	/* Counter Enable */
#define GPT_CTRL_STPCNT	    0x04	/* Stop continous */
#define GPT_CTRL_ODRAIN	    0x02	/* Open Drain */
#define GPT_CTRL_INTEN	    0x01	/* Interrupt Enable */
#define GPT_MODE_GPIO(x)    (x & 0x3)<<4/* Gpio Mode Type */
#define GPT_TMS_ICT	    0x01	/* Input Capture Enable */
#define GPT_TMS_OCT	    0x02	/* Output Capture Enable */
#define GPT_TMS_PWM	    0x03	/* PWM Capture Enable */
#define GPT_TMS_SGPIO	    0x04	/* PWM Capture Enable */

#define GPT_PWM_WIDTH(x)    (x & 0xffff)

/* Status */
#define GPT_STA_CAPTURE(x)  (x & 0xffff)/* Read of internal counter */

#define GPT_OVFPIN_OVF(x)   (x & 0x70)	/* Internal counter roll over */
#define GPT_OVFPIN_PIN	    0x01	/* Input pin - Timer 0 and 1 */

#define GPT_INT_TEXP	    0x08	/* Timer Expired in Internal Timer mode */
#define GPT_INT_PWMP	    0x04	/* PWM end of period occurred */
#define GPT_INT_COMP	    0x02	/* OC reference event occurred */
#define GPT_INT_CAPT	    0x01	/* IC reference event occurred */

/* ------------------------------------------------------------------------ */
/*
 * Port configuration
 */
#define CFG_FEC1_PORT0_CONFIG	0x00000000
#define CFG_FEC1_PORT1_CONFIG	0x00000000
#define CFG_1284_PORT0_CONFIG  0x00000000
#define CFG_1284_PORT1_CONFIG  0x00000000
#define CFG_FEC2_PORT2_CONFIG	0x00000000
#define CFG_PEV_PORT2_CONFIG   0x00000000
#define CFG_GP0_PORT0_CONFIG   0x00000000
#define CFG_GP1_PORT2_CONFIG   0xaaaaaac0
#define CFG_PSC_PORT3_CONFIG   0x00020000
#define CFG_CS1_PORT3_CONFIG   0x00000000
#define CFG_CS2_PORT3_CONFIG	0x10000000
#define CFG_CS3_PORT3_CONFIG	0x40000000
#define CFG_CS4_PORT3_CONFIG	0x00000400
#define CFG_CS5_PORT3_CONFIG	0x00000200
#define CFG_PCI_PORT3_CONFIG   0x01400180
#define CFG_I2C_PORT3_CONFIG   0x00000000
#define CFG_GP2_PORT3_CONFIG   0x000200a0

/* ------------------------------------------------------------------------ */
/*
 * DRAM configuration
 */

/* Field definitions for the control register */
#define CTL_MODE_ENABLE_SHIFT	    31
#define CTL_CKE_SHIFT		    30
#define CTL_DDR_SHIFT		    29
#define CTL_REFRESH_SHIFT	    28
#define CTL_ADDRMUX_SHIFT	    24
#define CTL_PRECHARGE_SHIFT	    23
#define CTL_DRIVE_RULE_SHIFT	    22
#define CTL_REFRESH_INTERVAL_SHIFT  16
#define CTL_DQSOEN_SHIFT	    8
#define CTL_BUFFERED_SHIFT	    4
#define CTL_REFRESH_CMD_SHIFT	    2
#define CTL_PRECHARGE_CMD_SHIFT	    1

#define CTL_MODE_ENABLE		    (1<<CTL_MODE_ENABLE_SHIFT)
#define CTL_CKE_HIGH		    (1<<CTL_CKE_SHIFT)
#define CTL_DDR_MODE		    (1<<CTL_DDR_SHIFT)
#define CTL_REFRESH_ENABLE	    (1<<CTL_REFRESH_SHIFT)
#define CTL_ADDRMUX(value)	    ((value)<<CTL_ADDRMUX_SHIFT)
#define CTL_A8PRECHARGE		    (1<<CTL_PRECHARGE_SHIFT)
#define CTL_REFRESH_INTERVAL(value) ((value)<<CTL_REFRESH_INTERVAL_SHIFT)
#define CTL_DQSOEN(value)	    ((value)<<CTL_DQSOEN_SHIFT)
#define CTL_BUFFERED		    (1<<CTL_BUFFERED_SHIFT)
#define CTL_REFRESH_CMD		    (1<<CTL_REFRESH_CMD_SHIFT)
#define CTL_PRECHARGE_CMD	    (1<<CTL_PRECHARGE_CMD_SHIFT)

/* Field definitions for config register 1 */

#define CFG1_SRD2RWP_SHIFT	    28
#define CFG1_SWT2RWP_SHIFT	    24
#define CFG1_RLATENCY_SHIFT	    20
#define CFG1_ACT2WR_SHIFT	    16
#define CFG1_PRE2ACT_SHIFT	    12
#define CFG1_REF2ACT_SHIFT	    8
#define CFG1_WLATENCY_SHIFT	    4

#define CFG1_SRD2RWP(value)	    ((value)<<CFG1_SRD2RWP_SHIFT)
#define CFG1_SWT2RWP(value)	    ((value)<<CFG1_SWT2RWP_SHIFT)
#define CFG1_RLATENCY(value)	    ((value)<<CFG1_RLATENCY_SHIFT)
#define CFG1_ACT2WR(value)	    ((value)<<CFG1_ACT2WR_SHIFT)
#define CFG1_PRE2ACT(value)	    ((value)<<CFG1_PRE2ACT_SHIFT)
#define CFG1_REF2ACT(value)	    ((value)<<CFG1_REF2ACT_SHIFT)
#define CFG1_WLATENCY(value)	    ((value)<<CFG1_WLATENCY_SHIFT)

/* Field definitions for config register 2 */
#define CFG2_BRD2RP_SHIFT	    28
#define CFG2_BWT2RWP_SHIFT	    24
#define CFG2_BRD2WT_SHIFT	    20
#define CFG2_BURSTLEN_SHIFT	    16

#define CFG2_BRD2RP(value)	    ((value)<<CFG2_BRD2RP_SHIFT)
#define CFG2_BWT2RWP(value)	    ((value)<<CFG2_BWT2RWP_SHIFT)
#define CFG2_BRD2WT(value)	    ((value)<<CFG2_BRD2WT_SHIFT)
#define CFG2_BURSTLEN(value)	    ((value)<<CFG2_BURSTLEN_SHIFT)

/* Field definitions for the mode/extended mode register - mode
 * register access
 */
#define MODE_REG_SHIFT		    30
#define MODE_OPMODE_SHIFT	    25
#define MODE_CL_SHIFT		    22
#define MODE_BT_SHIFT		    21
#define MODE_BURSTLEN_SHIFT	    18
#define MODE_CMD_SHIFT		    16

#define MODE_MODE		    0
#define MODE_OPMODE(value)	    ((value)<<MODE_OPMODE_SHIFT)
#define MODE_CL(value)		    ((value)<<MODE_CL_SHIFT)
#define MODE_BT_INTERLEAVED	    (1<<MODE_BT_SHIFT)
#define MODE_BT_SEQUENTIAL	    (0<<MODE_BT_SHIFT)
#define MODE_BURSTLEN(value)	    ((value)<<MODE_BURSTLEN_SHIFT)
#define MODE_CMD		    (1<<MODE_CMD_SHIFT)

#define MODE_BURSTLEN_8		    3
#define MODE_BURSTLEN_4		    2
#define MODE_BURSTLEN_2		    1

#define MODE_CL_2		    2
#define MODE_CL_2p5		    6
#define MODE_OPMODE_NORMAL	    0
#define MODE_OPMODE_RESETDLL	    2


/* Field definitions for the mode/extended mode register - extended
 * mode register access
 */
#define MODE_X_DLL_SHIFT	    18 /* DLL enable/disable */
#define MODE_X_DS_SHIFT		    19 /* Drive strength normal/reduced */
#define MODE_X_QFC_SHIFT	    20 /* QFC function (whatever that is) */
#define MODE_X_OPMODE_SHIFT	    21

#define MODE_EXTENDED		    (1<<MODE_REG_SHIFT)
#define MODE_X_DLL_ENABLE	    0
#define MODE_X_DLL_DISABLE	    (1<<MODE_X_DLL_SHIFT)
#define MODE_X_DS_NORMAL	    0
#define MODE_X_DS_REDUCED	    (1<<MODE_X_DS_SHIFT)
#define MODE_X_QFC_DISABLED	    0
#define MODE_X_OPMODE(value)	    ((value)<<MODE_X_OPMODE_SHIFT)

#ifndef __ASSEMBLY__
/*
 * DMA control/status registers.
 */
struct mpc8220_dma {
    u32 taskBar;	/* DMA + 0x00 */
    u32 currentPointer; /* DMA + 0x04 */
    u32 endPointer;	/* DMA + 0x08 */
    u32 variablePointer;/* DMA + 0x0c */

    u8 IntVect1;	/* DMA + 0x10 */
    u8 IntVect2;	/* DMA + 0x11 */
    u16 PtdCntrl;	/* DMA + 0x12 */

    u32 IntPend;	/* DMA + 0x14 */
    u32 IntMask;	/* DMA + 0x18 */

    u16 tcr_0;		/* DMA + 0x1c */
    u16 tcr_1;		/* DMA + 0x1e */
    u16 tcr_2;		/* DMA + 0x20 */
    u16 tcr_3;		/* DMA + 0x22 */
    u16 tcr_4;		/* DMA + 0x24 */
    u16 tcr_5;		/* DMA + 0x26 */
    u16 tcr_6;		/* DMA + 0x28 */
    u16 tcr_7;		/* DMA + 0x2a */
    u16 tcr_8;		/* DMA + 0x2c */
    u16 tcr_9;		/* DMA + 0x2e */
    u16 tcr_a;		/* DMA + 0x30 */
    u16 tcr_b;		/* DMA + 0x32 */
    u16 tcr_c;		/* DMA + 0x34 */
    u16 tcr_d;		/* DMA + 0x36 */
    u16 tcr_e;		/* DMA + 0x38 */
    u16 tcr_f;		/* DMA + 0x3a */

    u8 IPR0;		/* DMA + 0x3c */
    u8 IPR1;		/* DMA + 0x3d */
    u8 IPR2;		/* DMA + 0x3e */
    u8 IPR3;		/* DMA + 0x3f */
    u8 IPR4;		/* DMA + 0x40 */
    u8 IPR5;		/* DMA + 0x41 */
    u8 IPR6;		/* DMA + 0x42 */
    u8 IPR7;		/* DMA + 0x43 */
    u8 IPR8;		/* DMA + 0x44 */
    u8 IPR9;		/* DMA + 0x45 */
    u8 IPR10;		/* DMA + 0x46 */
    u8 IPR11;		/* DMA + 0x47 */
    u8 IPR12;		/* DMA + 0x48 */
    u8 IPR13;		/* DMA + 0x49 */
    u8 IPR14;		/* DMA + 0x4a */
    u8 IPR15;		/* DMA + 0x4b */
    u8 IPR16;		/* DMA + 0x4c */
    u8 IPR17;		/* DMA + 0x4d */
    u8 IPR18;		/* DMA + 0x4e */
    u8 IPR19;		/* DMA + 0x4f */
    u8 IPR20;		/* DMA + 0x50 */
    u8 IPR21;		/* DMA + 0x51 */
    u8 IPR22;		/* DMA + 0x52 */
    u8 IPR23;		/* DMA + 0x53 */
    u8 IPR24;		/* DMA + 0x54 */
    u8 IPR25;		/* DMA + 0x55 */
    u8 IPR26;		/* DMA + 0x56 */
    u8 IPR27;		/* DMA + 0x57 */
    u8 IPR28;		/* DMA + 0x58 */
    u8 IPR29;		/* DMA + 0x59 */
    u8 IPR30;		/* DMA + 0x5a */
    u8 IPR31;		/* DMA + 0x5b */

    u32 res1;		/* DMA + 0x5c */
    u32 res2;		/* DMA + 0x60 */
    u32 res3;		/* DMA + 0x64 */
    u32 MDEDebug;	/* DMA + 0x68 */
    u32 ADSDebug;	/* DMA + 0x6c */
    u32 Value1;		/* DMA + 0x70 */
    u32 Value2;		/* DMA + 0x74 */
    u32 Control;	/* DMA + 0x78 */
    u32 Status;		/* DMA + 0x7c */
    u32 EU00;		/* DMA + 0x80 */
    u32 EU01;		/* DMA + 0x84 */
    u32 EU02;		/* DMA + 0x88 */
    u32 EU03;		/* DMA + 0x8c */
    u32 EU04;		/* DMA + 0x90 */
    u32 EU05;		/* DMA + 0x94 */
    u32 EU06;		/* DMA + 0x98 */
    u32 EU07;		/* DMA + 0x9c */
    u32 EU10;		/* DMA + 0xa0 */
    u32 EU11;		/* DMA + 0xa4 */
    u32 EU12;		/* DMA + 0xa8 */
    u32 EU13;		/* DMA + 0xac */
    u32 EU14;		/* DMA + 0xb0 */
    u32 EU15;		/* DMA + 0xb4 */
    u32 EU16;		/* DMA + 0xb8 */
    u32 EU17;		/* DMA + 0xbc */
    u32 EU20;		/* DMA + 0xc0 */
    u32 EU21;		/* DMA + 0xc4 */
    u32 EU22;		/* DMA + 0xc8 */
    u32 EU23;		/* DMA + 0xcc */
    u32 EU24;		/* DMA + 0xd0 */
    u32 EU25;		/* DMA + 0xd4 */
    u32 EU26;		/* DMA + 0xd8 */
    u32 EU27;		/* DMA + 0xdc */
    u32 EU30;		/* DMA + 0xe0 */
    u32 EU31;		/* DMA + 0xe4 */
    u32 EU32;		/* DMA + 0xe8 */
    u32 EU33;		/* DMA + 0xec */
    u32 EU34;		/* DMA + 0xf0 */
    u32 EU35;		/* DMA + 0xf4 */
    u32 EU36;		/* DMA + 0xf8 */
    u32 EU37;		/* DMA + 0xfc */
};

/*
 * PCI Header Registers
 */
typedef struct mpc8220_xcpci {
	u32	dev_ven_id;		/* 0xb00 - device/vendor ID */
	u32	stat_cmd_reg;		/* 0xb04 - status command register */
	u32	class_code_rev_id;	/* 0xb08 - class code / revision ID */
	u32	bist_htyp_lat_cshl;	/* 0xb0c - BIST/HeaderType/Latency/cache line */
	u32	base0;			/* 0xb10 - base address 0 */
	u32	base1;			/* 0xb14 - base address 1 */
	u32	reserved1[4];		/* 0xb18->0xd27 - base address 2 - 5 */
	u32	cis;			/* 0xb28 - cardBus CIS pointer */
	u32	sub_sys_ven_id;		/* 0xb2c - sub system ID/ subsystem vendor ID */
	u32	reserved2;		/* 0xb30 - expansion ROM base address */
	u32	reserved3;		/* 0xb00 - reserved */
	u32	reserved4;		/* 0xb00 - reserved */
	u32	mlat_mgnt_ipl;		/* 0xb3c - MaxLat/MinGnt/ int pin/int line */
	u32	reserved5[8];
	/* MPC8220 specific - not accessible in PCI header space externally */
	u32	glb_stat_ctl;		/* 0xb60 - Global Status Control */
	u32	target_bar0;		/* 0xb64 - Target Base Address 0 */
	u32	target_bar1;		/* 0xb68 - Target Base Address 1 */
	u32	target_ctrl;		/* 0xb6c - Target Control */
	u32	init_win0;		/* 0xb70 - Initiator Window 0 Base/Translation */
	u32	init_win1;		/* 0xb74 - Initiator Window 1 Base/Translation */
	u32	init_win2;		/* 0xb78 - Initiator Window 2 Base/Translation */
	u32	reserved6;		/* 0xb7c - reserved  */
	u32	init_win_cfg;		/* 0xb80 */
	u32	init_ctrl;		/* 0xb84 */
	u32	init_stat;		/* 0xb88 */
	u32	reserved7[27];
	u32	cfg_adr;		/* 0xbf8 */
	u32	reserved8;
} mpc8220_xcpci_t;

/* PCI->XLB space translation (MPC8220 target), reg0 can address max 256MB,
   reg1 - 1GB */
#define PCI_BASE_ADDR_REG0			0x40000000
#define PCI_BASE_ADDR_REG1			(CFG_SDRAM_BASE)
#define PCI_TARGET_BASE_ADDR_REG0		(CFG_MBAR)
#define PCI_TARGET_BASE_ADDR_REG1		(CFG_SDRAM_BASE)
#define PCI_TARGET_BASE_ADDR_EN			1<<0


/* PCI Global Status/Control Register (PCIGSCR) */
#define PCI_GLB_STAT_CTRL_PE_SHIFT		29
#define PCI_GLB_STAT_CTRL_SE_SHIFT		28
#define PCI_GLB_STAT_CTRL_XLB_TO_PCI_CLK_SHIFT	24
#define PCI_GLB_STAT_CTRL_XLB_TO_PCI_CLK_MASK	0x7
#define PCI_GLB_STAT_CTRL_IPG_TO_PCI_CLK_SHIFT	16
#define PCI_GLB_STAT_CTRL_IPG_TO_PCI_CLK_MASK	0x7
#define PCI_GLB_STAT_CTRL_PEE_SHIFT		13
#define PCI_GLB_STAT_CTRL_SEE_SHIFT		12
#define PCI_GLB_STAT_CTRL_PR_SHIFT		0

#define PCI_GLB_STAT_CTRL_PE			(1<<PCI_GLB_STAT_CTRL_PE_SHIFT)
#define PCI_GLB_STAT_CTRL_SE			(1<<PCI_GLB_STAT_CTRL_SE_SHIFT)
#define PCI_GLB_STAT_CTRL_PEE			(1<<PCI_GLB_STAT_CTRL_PEE_SHIFT)
#define PCI_GLB_STAT_CTRL_SEE			(1<<PCI_GLB_STAT_CTRL_SEE_SHIFT)
#define PCI_GLB_STAT_CTRL_PR			(1<<PCI_GLB_STAT_CTRL_PR_SHIFT)

/* PCI Target Control Register (PCITCR) */
#define PCI_TARGET_CTRL_LD_SHIFT		24
#define PCI_TARGET_CTRL_P_SHIFT			16

#define PCI_TARGET_CTRL_LD			(1<<PCI_TARGET_CTRL_LD_SHIFT)
#define PCI_TARGET_CTRL_P			(1<<PCI_TARGET_CTRL_P_SHIFT)

/* PCI Initiator Window Configuration Register (PCIIWCR) */
#define PCI_INIT_WIN_CFG_WIN0_CTRL_IO_SHIFT	27
#define PCI_INIT_WIN_CFG_WIN0_CTRL_PRC_SHIFT	25
#define PCI_INIT_WIN_CFG_WIN0_CTRL_PRC_MASK	0x3
#define PCI_INIT_WIN_CFG_WIN0_CTRL_EN_SHIFT	24
#define PCI_INIT_WIN_CFG_WIN1_CTRL_IO_SHIFT	19
#define PCI_INIT_WIN_CFG_WIN1_CTRL_PRC_SHIFT	17
#define PCI_INIT_WIN_CFG_WIN1_CTRL_PRC_MASK	0x3
#define PCI_INIT_WIN_CFG_WIN1_CTRL_EN_SHIFT	16
#define PCI_INIT_WIN_CFG_WIN2_CTRL_IO_SHIFT	11
#define PCI_INIT_WIN_CFG_WIN2_CTRL_PRC_SHIFT	9
#define PCI_INIT_WIN_CFG_WIN2_CTRL_PRC_MASK	0x3
#define PCI_INIT_WIN_CFG_WIN2_CTRL_EN_SHIFT	8

#define PCI_INIT_WIN_CFG_WIN_MEM_READ		0x0
#define PCI_INIT_WIN_CFG_WIN_MEM_READ_LINE	0x1
#define PCI_INIT_WIN_CFG_WIN_MEM_READ_MULTIPLE	0x2

#define PCI_INIT_WIN_CFG_WIN0_CTRL_IO		(1<<PCI_INIT_WIN_CFG_WIN0_CTRL_IO_SHIFT)
#define PCI_INIT_WIN_CFG_WIN0_CTRL_EN		(1<<PCI_INIT_WIN_CFG_WIN0_CTRL_EN_SHIFT)
#define PCI_INIT_WIN_CFG_WIN1_CTRL_IO		(1<<PCI_INIT_WIN_CFG_WIN1_CTRL_IO_SHIFT)
#define PCI_INIT_WIN_CFG_WIN1_CTRL_EN		(1<<PCI_INIT_WIN_CFG_WIN1_CTRL_EN_SHIFT)
#define PCI_INIT_WIN_CFG_WIN2_CTRL_IO		(1<<PCI_INIT_WIN_CFG_WIN2_CTRL_IO_SHIFT)
#define PCI_INIT_WIN_CFG_WIN2_CTRL_EN		(1<<PCI_INIT_WIN_CFG_WIN2_CTRL_EN_SHIFT)

/* PCI Initiator Control Register (PCIICR) */
#define PCI_INIT_CTRL_REE_SHIFT			26
#define PCI_INIT_CTRL_IAE_SHIFT			25
#define PCI_INIT_CTRL_TAE_SHIFT			24
#define PCI_INIT_CTRL_MAX_RETRIES_SHIFT		0
#define PCI_INIT_CTRL_MAX_RETRIES_MASK		0xff

#define PCI_INIT_CTRL_REE			(1<<PCI_INIT_CTRL_REE_SHIFT)
#define PCI_INIT_CTRL_IAE			(1<<PCI_INIT_CTRL_IAE_SHIFT)
#define PCI_INIT_CTRL_TAE			(1<<PCI_INIT_CTRL_TAE_SHIFT)

/* PCI Status/Command Register (PCISCR) - PCI Dword 1 */
#define PCI_STAT_CMD_PE_SHIFT			31
#define PCI_STAT_CMD_SE_SHIFT			30
#define PCI_STAT_CMD_MA_SHIFT			29
#define PCI_STAT_CMD_TR_SHIFT			28
#define PCI_STAT_CMD_TS_SHIFT			27
#define PCI_STAT_CMD_DT_SHIFT			25
#define PCI_STAT_CMD_DT_MASK			0x3
#define PCI_STAT_CMD_DP_SHIFT			24
#define PCI_STAT_CMD_FC_SHIFT			23
#define PCI_STAT_CMD_R_SHIFT			22
#define PCI_STAT_CMD_66M_SHIFT			21
#define PCI_STAT_CMD_C_SHIFT			20
#define PCI_STAT_CMD_F_SHIFT			9
#define PCI_STAT_CMD_S_SHIFT			8
#define PCI_STAT_CMD_ST_SHIFT			7
#define PCI_STAT_CMD_PER_SHIFT			6
#define PCI_STAT_CMD_V_SHIFT			5
#define PCI_STAT_CMD_MW_SHIFT			4
#define PCI_STAT_CMD_SP_SHIFT			3
#define PCI_STAT_CMD_B_SHIFT			2
#define PCI_STAT_CMD_M_SHIFT			1
#define PCI_STAT_CMD_IO_SHIFT			0

#define PCI_STAT_CMD_PE			(1<<PCI_STAT_CMD_PE_SHIFT)
#define PCI_STAT_CMD_SE			(1<<PCI_STAT_CMD_SE_SHIFT)
#define PCI_STAT_CMD_MA			(1<<PCI_STAT_CMD_MA_SHIFT)
#define PCI_STAT_CMD_TR			(1<<PCI_STAT_CMD_TR_SHIFT)
#define PCI_STAT_CMD_TS			(1<<PCI_STAT_CMD_TS_SHIFT)
#define PCI_STAT_CMD_DP			(1<<PCI_STAT_CMD_DP_SHIFT)
#define PCI_STAT_CMD_FC			(1<<PCI_STAT_CMD_FC_SHIFT)
#define PCI_STAT_CMD_R				(1<<PCI_STAT_CMD_R_SHIFT)
#define PCI_STAT_CMD_66M			(1<<PCI_STAT_CMD_66M_SHIFT)
#define PCI_STAT_CMD_C				(1<<PCI_STAT_CMD_C_SHIFT)
#define PCI_STAT_CMD_F				(1<<PCI_STAT_CMD_F_SHIFT)
#define PCI_STAT_CMD_S				(1<<PCI_STAT_CMD_S_SHIFT)
#define PCI_STAT_CMD_ST			(1<<PCI_STAT_CMD_ST_SHIFT)
#define PCI_STAT_CMD_PER			(1<<PCI_STAT_CMD_PER_SHIFT)
#define PCI_STAT_CMD_V				(1<<PCI_STAT_CMD_V_SHIFT)
#define PCI_STAT_CMD_MW			(1<<PCI_STAT_CMD_MW_SHIFT)
#define PCI_STAT_CMD_SP			(1<<PCI_STAT_CMD_SP_SHIFT)
#define PCI_STAT_CMD_B				(1<<PCI_STAT_CMD_B_SHIFT)
#define PCI_STAT_CMD_M				(1<<PCI_STAT_CMD_M_SHIFT)
#define PCI_STAT_CMD_IO			(1<<PCI_STAT_CMD_IO_SHIFT)

/* PCI Configuration 1 Register (PCICR1) - PCI Dword 3 */
#define PCI_CFG1_HT_SHIFT			16
#define PCI_CFG1_HT_MASK			0xff
#define PCI_CFG1_LT_SHIFT			8
#define PCI_CFG1_LT_MASK			0xff
#define PCI_CFG1_CLS_SHIFT			0
#define PCI_CFG1_CLS_MASK			0xf

/* function prototypes */
void loadtask(int basetask, int tasks);
u32 dramSetup(void);

#if defined(CONFIG_PSC_CONSOLE)
int psc_serial_init (void);
void psc_serial_putc(const char c);
void psc_serial_puts (const char *s);
int psc_serial_getc(void);
int psc_serial_tstc(void);
void psc_serial_setbrg(void);
#endif

#if defined (CONFIG_EXTUART_CONSOLE)
int ext_serial_init (void);
void ext_serial_putc(const char c);
void ext_serial_puts (const char *s);
int ext_serial_getc(void);
int ext_serial_tstc(void);
void ext_serial_setbrg(void);
#endif

#endif /* __ASSEMBLY__ */

#endif /* __MPC8220_H__ */
