/*
 * MCF5272 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
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

#ifndef __IMMAP_5272__
#define __IMMAP_5272__

/* System configuration registers
*/
typedef	struct sys_ctrl {
	uint	sc_mbar;
	ushort	sc_scr;
	ushort	sc_spr;
	uint	sc_pmr;
	char	res1[2];
	ushort	sc_alpr;
	uint	sc_dir;
	char	res2[12];
} sysctrl_t;

/* Interrupt module registers
*/
typedef struct int_ctrl {
	uint	int_icr1;
	uint	int_icr2;
	uint	int_icr3;
	uint	int_icr4;
	uint	int_isr;
	uint	int_pitr;
	uint	int_piwr;
	uchar	res1[3];
	uchar	int_pivr;
} intctrl_t;

/* Chip select module registers.
*/
typedef struct	cs_ctlr {
	uint	cs_br0;
	uint	cs_or0;
	uint	cs_br1;
	uint	cs_or1;
	uint	cs_br2;
	uint	cs_or2;
	uint	cs_br3;
	uint	cs_or3;
	uint	cs_br4;
	uint	cs_or4;
	uint	cs_br5;
	uint	cs_or5;
	uint	cs_br6;
	uint	cs_or6;
	uint	cs_br7;
	uint	cs_or7;
} csctrl_t;

/* GPIO port registers
*/
typedef struct	gpio_ctrl {
	uint	gpio_pacnt;
	ushort	gpio_paddr;
	ushort	gpio_padat;
	uint	gpio_pbcnt;
	ushort	gpio_pbddr;
	ushort	gpio_pbdat;
	uchar	res1[4];
	ushort	gpio_pcddr;
	ushort	gpio_pcdat;
	uint	gpio_pdcnt;
	uchar	res2[4];
} gpio_t;

/* QSPI module registers
 */
typedef struct	qspi_ctrl {
	ushort	qspi_qmr;
	uchar	res1[2];
	ushort	qspi_qdlyr;
	uchar	res2[2];
	ushort	qspi_qwr;
	uchar	res3[2];
	ushort	qspi_qir;
	uchar	res4[2];
	ushort	qspi_qar;
	uchar	res5[2];
	ushort	qspi_qdr;
	uchar	res6[10];
} qspi_t;

/* PWM module registers
 */
typedef struct	pwm_ctrl {
	uchar	pwm_pwcr0;
	uchar	res1[3];
	uchar	pwm_pwcr1;
	uchar	res2[3];
	uchar	pwm_pwcr2;
	uchar	res3[7];
	uchar	pwm_pwwd0;
	uchar	res4[3];
	uchar	pwm_pwwd1;
	uchar	res5[3];
	uchar	pwm_pwwd2;
	uchar	res6[7];
} pwm_t;

/* DMA module registers
 */
typedef struct	dma_ctrl {
	ulong	dma_dmr;
	uchar	res1[2];
	ushort	dma_dir;
	ulong	dma_dbcr;
	ulong	dma_dsar;
	ulong	dma_ddar;
	uchar	res2[12];
} dma_t;

/* UART module registers
 */
typedef struct uart_ctrl {
	uchar	uart_umr;
	uchar	res1[3];
	uchar	uart_usr_ucsr;
	uchar	res2[3];
	uchar	uart_ucr;
	uchar	res3[3];
	uchar	uart_urb_utb;
	uchar	res4[3];
	uchar	uart_uipcr_uacr;
	uchar	res5[3];
	uchar	uart_uisr_uimr;
	uchar	res6[3];
	uchar	uart_udu;
	uchar	res7[3];
	uchar	uart_udl;
	uchar	res8[3];
	uchar	uart_uabu;
	uchar	res9[3];
	uchar	uart_uabl;
	uchar	res10[3];
	uchar	uart_utf;
	uchar	res11[3];
	uchar	uart_urf;
	uchar	res12[3];
	uchar	uart_ufpd;
	uchar	res13[3];
	uchar	uart_uip;
	uchar	res14[3];
	uchar	uart_uop1;
	uchar	res15[3];
	uchar	uart_uop0;
	uchar	res16[3];
} uart_t;

/* SDRAM controller registers, offset: 0x180
 */
typedef struct sdram_ctrl {
	uchar   res1[2];
	ushort	sdram_sdcr;
	uchar	res2[2];
	ushort	sdram_sdtr;
	uchar	res3[120];
} sdramctrl_t;

/* Timer module registers
 */
typedef struct timer_ctrl {
	ushort	timer_tmr;
	ushort	res1;
	ushort	timer_trr;
	ushort	res2;
	ushort	timer_tcap;
	ushort	res3;
	ushort	timer_tcn;
	ushort	res4;
	ushort	timer_ter;
	uchar	res5[14];
} timer_t;

/* Watchdog registers
 */
typedef struct wdog_ctrl {
	ushort	wdog_wrrr;
	ushort	res1;
	ushort	wdog_wirr;
	ushort	res2;
	ushort	wdog_wcr;
	ushort	res3;
	ushort	wdog_wer;
	uchar	res4[114];
} wdog_t;

/* PLIC module registers
 */
typedef struct plic_ctrl {
	ulong	plic_p0b1rr;
	ulong	plic_p1b1rr;
	ulong	plic_p2b1rr;
	ulong	plic_p3b1rr;
	ulong	plic_p0b2rr;
	ulong	plic_p1b2rr;
	ulong	plic_p2b2rr;
	ulong	plic_p3b2rr;
	uchar	plic_p0drr;
	uchar	plic_p1drr;
	uchar	plic_p2drr;
	uchar	plic_p3drr;
	uchar	res1[4];
	ulong	plic_p0b1tr;
	ulong	plic_p1b1tr;
	ulong	plic_p2b1tr;
	ulong	plic_p3b1tr;
	ulong	plic_p0b2tr;
	ulong	plic_p1b2tr;
	ulong	plic_p2b2tr;
	ulong	plic_p3b2tr;
	uchar	plic_p0dtr;
	uchar	plic_p1dtr;
	uchar	plic_p2dtr;
	uchar	plic_p3dtr;
	uchar	res2[4];
	ushort	plic_p0cr;
	ushort	plic_p1cr;
	ushort	plic_p2cr;
	ushort	plic_p3cr;
	ushort	plic_p0icr;
	ushort	plic_p1icr;
	ushort	plic_p2icr;
	ushort	plic_p3icr;
	ushort	plic_p0gmr;
	ushort	plic_p1gmr;
	ushort	plic_p2gmr;
	ushort	plic_p3gmr;
	ushort	plic_p0gmt;
	ushort	plic_p1gmt;
	ushort	plic_p2gmt;
	ushort	plic_p3gmt;
	uchar	res3;
	uchar	plic_pgmts;
	uchar	plic_pgmta;
	uchar	res4;
	uchar	plic_p0gcir;
	uchar	plic_p1gcir;
	uchar	plic_p2gcir;
	uchar	plic_p3gcir;
	uchar	plic_p0gcit;
	uchar	plic_p1gcit;
	uchar	plic_p2gcit;
	uchar	plic_p3gcit;
	uchar	res5[3];
	uchar	plic_pgcitsr;
	uchar	res6[3];
	uchar	plic_pdcsr;
	ushort	plic_p0psr;
	ushort	plic_p1psr;
	ushort	plic_p2psr;
	ushort	plic_p3psr;
	ushort	plic_pasr;
	uchar	res7;
	uchar	plic_plcr;
	ushort	res8;
	ushort	plic_pdrqr;
	ushort	plic_p0sdr;
	ushort	plic_p1sdr;
	ushort	plic_p2sdr;
	ushort	plic_p3sdr;
	ushort	res9;
	ushort	plic_pcsr;
	uchar	res10[1184];
} plic_t;

/* Fast ethernet controller registers
 */
typedef struct fec {
	uint	fec_ecntrl;		/* ethernet control register		*/
	uint	fec_ievent;		/* interrupt event register		*/
	uint	fec_imask;		/* interrupt mask register		*/
	uint	fec_ivec;		/* interrupt level and vector status	*/
	uint	fec_r_des_active;	/* Rx ring updated flag			*/
	uint	fec_x_des_active;	/* Tx ring updated flag			*/
	uint	res3[10];		/* reserved				*/
	uint	fec_mii_data;		/* MII data register			*/
	uint	fec_mii_speed;		/* MII speed control register		*/
	uint	res4[17];		/* reserved				*/
	uint	fec_r_bound;		/* end of RAM (read-only)		*/
	uint	fec_r_fstart;		/* Rx FIFO start address		*/
	uint	res5[6];		/* reserved				*/
	uint	fec_x_fstart;		/* Tx FIFO start address		*/
	uint	res7[21];		/* reserved				*/
	uint	fec_r_cntrl;		/* Rx control register			*/
	uint	fec_r_hash;		/* Rx hash register			*/
	uint	res8[14];		/* reserved				*/
	uint	fec_x_cntrl;		/* Tx control register			*/
	uint	res9[0x9e];		/* reserved				*/
	uint	fec_addr_low;		/* lower 32 bits of station address	*/
	uint	fec_addr_high;		/* upper 16 bits of station address	*/
	uint	fec_hash_table_high;	/* upper 32-bits of hash table		*/
	uint	fec_hash_table_low;	/* lower 32-bits of hash table		*/
	uint	fec_r_des_start;	/* beginning of Rx descriptor ring	*/
	uint	fec_x_des_start;	/* beginning of Tx descriptor ring	*/
	uint	fec_r_buff_size;	/* Rx buffer size			*/
	uint	res2[9];		/* reserved				*/
	uchar	fec_fifo[960];		/* fifo RAM				*/
} fec_t;

/* USB module registers
*/
typedef struct usb {
	ushort	res1;
	ushort	usb_fnr;
	ushort	res2;
	ushort	usb_fnmr;
	ushort	res3;
	ushort	usb_rfmr;
	ushort	res4;
	ushort	usb_rfmmr;
	uchar	res5[3];
	uchar	usb_far;
	ulong	usb_asr;
	ulong	usb_drr1;
	ulong	usb_drr2;
	ushort	res6;
	ushort	usb_specr;
	ushort	res7;
	ushort	usb_ep0sr;
	ulong	usb_iep0cfg;
	ulong	usb_oep0cfg;
	ulong	usb_ep1cfg;
	ulong	usb_ep2cfg;
	ulong	usb_ep3cfg;
	ulong	usb_ep4cfg;
	ulong	usb_ep5cfg;
	ulong	usb_ep6cfg;
	ulong	usb_ep7cfg;
	ulong	usb_ep0ctl;
	ushort	res8;
	ushort	usb_ep1ctl;
	ushort	res9;
	ushort	usb_ep2ctl;
	ushort	res10;
	ushort	usb_ep3ctl;
	ushort	res11;
	ushort	usb_ep4ctl;
	ushort	res12;
	ushort	usb_ep5ctl;
	ushort	res13;
	ushort	usb_ep6ctl;
	ushort	res14;
	ushort	usb_ep7ctl;
	ulong	usb_ep0isr;
	ushort	res15;
	ushort	usb_ep1isr;
	ushort	res16;
	ushort	usb_ep2isr;
	ushort	res17;
	ushort	usb_ep3isr;
	ushort	res18;
	ushort	usb_ep4isr;
	ushort	res19;
	ushort	usb_ep5isr;
	ushort	res20;
	ushort	usb_ep6isr;
	ushort	res21;
	ushort	usb_ep7isr;
	ulong	usb_ep0imr;
	ushort	res22;
	ushort	usb_ep1imr;
	ushort	res23;
	ushort	usb_ep2imr;
	ushort	res24;
	ushort	usb_ep3imr;
	ushort	res25;
	ushort	usb_ep4imr;
	ushort	res26;
	ushort	usb_ep5imr;
	ushort	res27;
	ushort	usb_ep6imr;
	ushort	res28;
	ushort	usb_ep7imr;
	ulong	usb_ep0dr;
	ulong	usb_ep1dr;
	ulong	usb_ep2dr;
	ulong	usb_ep3dr;
	ulong	usb_ep4dr;
	ulong	usb_ep5dr;
	ulong	usb_ep6dr;
	ulong	usb_ep7dr;
	ushort	res29;
	ushort	usb_ep0dpr;
	ushort	res30;
	ushort	usb_ep1dpr;
	ushort	res31;
	ushort	usb_ep2dpr;
	ushort	res32;
	ushort	usb_ep3dpr;
	ushort	res33;
	ushort	usb_ep4dpr;
	ushort	res34;
	ushort	usb_ep5dpr;
	ushort	res35;
	ushort	usb_ep6dpr;
	ushort	res36;
	ushort	usb_ep7dpr;
	uchar	res37[788];
	uchar	usb_cfgram[1024];
} usb_t;

/* Internal memory map.
*/
typedef struct immap {
	sysctrl_t	sysctrl_reg;	/* System configuration registers */
	intctrl_t	intctrl_reg;	/* Interrupt controller registers */
	csctrl_t	csctrl_reg;	/* Chip select controller registers */
	gpio_t		gpio_reg;	/* GPIO controller registers */
	qspi_t		qspi_reg;	/* QSPI controller registers */
	pwm_t		pwm_reg;	/* Pulse width modulation registers */
	dma_t		dma_reg;	/* DMA registers */
	uart_t		uart_reg[2];	/* UART registers */
	sdramctrl_t	sdram_reg;	/* SDRAM controller registers */
	timer_t		timer_reg[4];	/* Timer registers */
	wdog_t		wdog_reg;	/* Watchdog registers */
	plic_t		plic_reg;	/* Physical layer interface registers */
	fec_t		fec_reg;	/* Fast ethernet controller registers */
	usb_t		usb_reg;	/* USB controller registers */
} immap_t;

#endif /* __IMMAP_5272__ */
