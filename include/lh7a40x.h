/*
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

/*
 * lh7a40x SoC series common interface
 */

#ifndef __LH7A40X_H__
#define __LH7A40X_H__

/* (SMC) Static Memory Controller (usersguide 4.2.1) */
typedef struct {
	volatile u32  attib;
	volatile u32  com;
	volatile u32  io;
	volatile u32  rsvd1;
} /*__attribute__((__packed__))*/ lh7a40x_pccard_t;

typedef struct {
	volatile u32      bcr[8];
	lh7a40x_pccard_t  pccard[2];
	volatile u32	  pcmciacon;
} /*__attribute__((__packed__))*/ lh7a40x_smc_t;
#define LH7A40X_SMC_BASE  (0x80002000)
#define LH7A40X_SMC_PTR   ((lh7a40x_smc_t*) LH7A40X_SMC_BASE)

/* (SDMC) Synchronous Dynamic Ram Controller (usersguide 5.3.1) */
typedef struct {
	volatile u32  rsvd1;
	volatile u32  gblcnfg;
	volatile u32  rfshtmr;
	volatile u32  bootstat;
	volatile u32  sdcsc[4];
} /*__attribute__((__packed__))*/ lh7a40x_sdmc_t;
#define LH7A40X_SDMC_BASE  (0x80002400)
#define LH7A40X_SDMC_PTR   ((lh7a40x_sdmc_t*) LH7A40X_SDMC_BASE)

/* (CSC) Clock and State Controller (userguide 6.2.1) */
typedef struct {
	volatile u32  pwrsr;
	volatile u32  pwrcnt;
	volatile u32  halt;
	volatile u32  stby;
	volatile u32  bleoi;
	volatile u32  mceoi;
	volatile u32  teoi;
	volatile u32  stfclr;
	volatile u32  clkset;
	volatile u32  scrreg[2];
	volatile u32  rsvd1;
	volatile u32  usbreset;
} /*__attribute__((__packed__))*/ lh7a40x_csc_t;
#define LH7A40X_STPWR_BASE  (0x80000400)
#define LH7A40X_CSC_PTR     ((lh7a40x_csc_t*) LH7A40X_STPWR_BASE)

#define CLKSET_SMCROM		(0x01000000)
#define CLKSET_PS		(0x000C0000)
#define CLKSET_PS_0		(0x00000000)
#define CLKSET_PS_1		(0x00040000)
#define CLKSET_PS_2		(0x00080000)
#define CLKSET_PS_3		(0x000C0000)
#define CLKSET_PCLKDIV		(0x00030000)
#define CLKSET_PCLKDIV_2	(0x00000000)
#define CLKSET_PCLKDIV_4	(0x00010000)
#define CLKSET_PCLKDIV_8	(0x00020000)
#define CLKSET_MAINDIV2		(0x0000f800)
#define CLKSET_MAINDIV1		(0x00000780)
#define CLKSET_PREDIV		(0x0000007C)
#define CLKSET_HCLKDIV		(0x00000003)

/* (DMA) Direct Memory Access Controller (userguide 9.2.1) */
typedef struct {
	volatile u32  maxcnt;
	volatile u32  base;
	volatile u32  current;
	volatile u32  rsvd1;
} lh7a40x_dmabuf_t;

typedef struct {
	volatile u32      control;
	volatile u32      interrupt;
	volatile u32      rsvd1;
	volatile u32      status;
	volatile u32      rsvd2;
	volatile u32      remain;
	volatile u32      rsvd3;
	volatile u32      rsvd4;
	lh7a40x_dmabuf_t  buf[2];
} /*__attribute__((__packed__))*/ lh7a40x_dmachan_t;


/* (WDT) Watchdog Timer (userguide 11.2.1) */
typedef struct {
	volatile u32  ctl;
	volatile u32  rst;
	volatile u32  status;
	volatile u32  count[4];
} /*__attribute__((__packed__))*/ lh7a40x_wdt_t;
#define LH7A40X_WDT_BASE    (0x80001400)
#define LH7A40X_WDT_PTR     ((lh7a40x_wdt_t*) LH7A40X_WDT_BASE)

/* (RTC) Real Time Clock (lh7a400 userguide 12.2.1, lh7a404 userguide 13.2.1) */
typedef struct {
	volatile u32  rtcdr;
	volatile u32  rtclr;
	volatile u32  rtcmr;
	volatile u32  unk1;
	volatile u32  rtcstat_eoi;
	volatile u32  rtccr;
	volatile u32  rsvd1[58];
} /*__attribute__((__packed__))*/ lh7a40x_rtc_t;
#define LH7A40X_RTC_BASE    (0x80000D00)
#define LH7A40X_RTC_PTR     ((lh7a40x_rtc_t*) LH7A40X_RTC_BASE)

/* Timers (lh7a400 userguide 13.2.1, lh7a404 userguide 11.2.1) */
typedef struct {
	volatile u32  load;
	volatile u32  value;
	volatile u32  control;
	volatile u32  tceoi;
} /*__attribute__((__packed__))*/ lh7a40x_timer_t;

typedef struct {
	lh7a40x_timer_t  timer1;
	volatile u32     rsvd1[4];
	lh7a40x_timer_t  timer2;
	volatile u32     unk1[4];
	volatile u32     bzcon;
	volatile u32     unk2[15];
	lh7a40x_timer_t  timer3;
	/*volatile u32     rsvd2;*/
} /*__attribute__((__packed__))*/ lh7a40x_timers_t;
#define LH7A40X_TIMERS_BASE    (0x80000C00)
#define LH7A40X_TIMERS_PTR     ((lh7a40x_timers_t*) LH7A40X_TIMERS_BASE)

#define TIMER_EN	(0x00000080)
#define TIMER_PER	(0x00000040)
#define TIMER_FREE	(0x00000000)
#define TIMER_CLK508K	(0x00000008)
#define TIMER_CLK2K	(0x00000000)

/* (SSP) Sychronous Serial Ports (lh7a400 userguide 14.2.1, lh7a404 userguide 14.2.1) */
typedef struct {
	volatile u32  cr0;
	volatile u32  cr1;
	volatile u32  irr_roeoi;
	volatile u32  dr;
	volatile u32  cpr;
	volatile u32  sr;
	/*volatile u32  rsvd1[58];*/
} /*__attribute__((__packed__))*/ lh7a40x_ssp_t;
#define LH7A40X_SSP_BASE    (0x80000B00)
#define LH7A40X_SSP_PTR     ((lh7a40x_ssp_t*) LH7A40X_SSP_BASE)

/* (UART) Universal Asychronous Receiver/Transmitter (lh7a400 userguide 15.2.1, lh7a404 userguide 15.2.1) */
typedef struct {
	volatile u32  data;
	volatile u32  fcon;
	volatile u32  brcon;
	volatile u32  con;
	volatile u32  status;
	volatile u32  rawisr;
	volatile u32  inten;
	volatile u32  isr;
	volatile u32  rsvd1[56];
} /*__attribute__((__packed__))*/ lh7a40x_uart_t;
#define LH7A40X_UART_BASE    (0x80000600)
#define LH7A40X_UART_PTR(n) \
	((lh7a40x_uart_t*) (LH7A40X_UART_BASE + ((n-1) * sizeof(lh7a40x_uart_t))))

#define UART_BE		(0x00000800)      /* the rx error bits */
#define UART_OE		(0x00000400)
#define UART_PE		(0x00000200)
#define UART_FE		(0x00000100)

#define UART_WLEN	(0x00000060)	/* fcon bits */
#define UART_WLEN_8	(0x00000060)
#define UART_WLEN_7	(0x00000040)
#define UART_WLEN_6	(0x00000020)
#define UART_WLEN_5	(0x00000000)
#define UART_FEN	(0x00000010)
#define UART_STP2	(0x00000008)
#define UART_STP2_2	(0x00000008)
#define UART_STP2_1	(0x00000000)
#define UART_EPS	(0x00000004)
#define UART_EPS_EVEN	(0x00000004)
#define UART_EPS_ODD	(0x00000000)
#define UART_PEN	(0x00000002)
#define UART_BRK	(0x00000001)

#define UART_BAUDDIV	(0x0000ffff)	/* brcon bits */

#define UART_SIRBD	(0x00000080)	/* con bits */
#define UART_LBE	(0x00000040)
#define UART_MXP	(0x00000020)
#define UART_TXP	(0x00000010)
#define UART_RXP	(0x00000008)
#define UART_SIRLP	(0x00000004)
#define UART_SIRD	(0x00000002)
#define UART_EN		(0x00000001)

#define UART_TXFE	(0x00000080)	/* status bits */
#define UART_RXFF	(0x00000040)
#define UART_TXFF	(0x00000020)
#define UART_RXFE	(0x00000010)
#define UART_BUSY	(0x00000008)
#define UART_DCD	(0x00000004)
#define UART_DSR	(0x00000002)
#define UART_CTS	(0x00000001)

#define UART_MSEOI	(0xfffffff0)	/* rawisr interrupt bits */

#define UART_RTI	(0x00000008)	/* generic interrupt bits */
#define UART_MI		(0x00000004)
#define UART_TI		(0x00000002)
#define UART_RI		(0x00000001)

/* (GPIO) General Purpose IO and External Interrupts (userguide 16.2.1) */
typedef struct {
	volatile u32  pad;
	volatile u32  pbd;
	volatile u32  pcd;
	volatile u32  pdd;
	volatile u32  padd;
	volatile u32  pbdd;
	volatile u32  pcdd;
	volatile u32  pddd;
	volatile u32  ped;
	volatile u32  pedd;
	volatile u32  kbdctl;
	volatile u32  pinmux;
	volatile u32  pfd;
	volatile u32  pfdd;
	volatile u32  pgd;
	volatile u32  pgdd;
	volatile u32  phd;
	volatile u32  phdd;
	volatile u32  rsvd1;
	volatile u32  inttype1;
	volatile u32  inttype2;
	volatile u32  gpiofeoi;
	volatile u32  gpiointen;
	volatile u32  intstatus;
	volatile u32  rawintstatus;
	volatile u32  gpiodb;
	volatile u32  papd;
	volatile u32  pbpd;
	volatile u32  pcpd;
	volatile u32  pdpd;
	volatile u32  pepd;
	volatile u32  pfpd;
	volatile u32  pgpd;
	volatile u32  phpd;
} /*__attribute__((__packed__))*/ lh7a40x_gpioint_t;
#define LH7A40X_GPIOINT_BASE    (0x80000E00)
#define LH7A40X_GPIOINT_PTR     ((lh7a40x_gpioint_t*) LH7A40X_GPIOINT_BASE)

/* Embedded SRAM */
#define CFG_SRAM_BASE	(0xB0000000)
#define CFG_SRAM_SIZE	(80*1024)	/* 80kB */

#endif  /* __LH7A40X_H__ */
