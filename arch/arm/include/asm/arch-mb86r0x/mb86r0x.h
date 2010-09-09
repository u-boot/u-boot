/*
 * (C) Copyright 2007
 *
 * mb86r0x definitions
 *
 * Author : Carsten Schneider, mycable GmbH
 *          <cs@mycable.de>
 *
 * (C) Copyright 2010
 * Matthias Weisser <weisserm@arcor.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef MB86R0X_H
#define MB86R0X_H

#ifndef __ASSEMBLY__

/* GPIO registers */
struct mb86r0x_gpio {
	uint32_t gpdr0;
	uint32_t gpdr1;
	uint32_t gpdr2;
	uint32_t res;
	uint32_t gpddr0;
	uint32_t gpddr1;
	uint32_t gpddr2;
};

/* PWM registers */
struct mb86r0x_pwm {
	uint32_t bcr;
	uint32_t tpr;
	uint32_t pr;
	uint32_t dr;
	uint32_t cr;
	uint32_t sr;
	uint32_t ccr;
	uint32_t ir;
};

/* The mb86r0x chip control (CCNT) register set. */
struct mb86r0x_ccnt {
	uint32_t ccid;
	uint32_t csrst;
	uint32_t pad0[2];
	uint32_t cist;
	uint32_t cistm;
	uint32_t cgpio_ist;
	uint32_t cgpio_istm;
	uint32_t cgpio_ip;
	uint32_t cgpio_im;
	uint32_t caxi_bw;
	uint32_t caxi_ps;
	uint32_t cmux_md;
	uint32_t cex_pin_st;
	uint32_t cmlb;
	uint32_t pad1[1];
	uint32_t cusb;
	uint32_t pad2[41];
	uint32_t cbsc;
	uint32_t cdcrc;
	uint32_t cmsr0;
	uint32_t cmsr1;
	uint32_t pad3[2];
};

/* The mb86r0x clock reset generator */
struct mb86r0x_crg {
	uint32_t crpr;
	uint32_t pad0;
	uint32_t crwr;
	uint32_t crsr;
	uint32_t crda;
	uint32_t crdb;
	uint32_t crha;
	uint32_t crpa;
	uint32_t crpb;
	uint32_t crhb;
	uint32_t cram;
};

/* The mb86r0x timer */
struct mb86r0x_timer {
	uint32_t load;
	uint32_t value;
	uint32_t control;
	uint32_t intclr;
	uint32_t ris;
	uint32_t mis;
	uint32_t bgload;
};

/* mb86r0x gdc display controller */
struct mb86r0x_gdc_dsp {
	/* Display settings */
	uint32_t dcm0;
	uint16_t pad00;
	uint16_t htp;
	uint16_t hdp;
	uint16_t hdb;
	uint16_t hsp;
	uint8_t  hsw;
	uint8_t  vsw;
	uint16_t pad01;
	uint16_t vtr;
	uint16_t vsp;
	uint16_t vdp;
	uint16_t wx;
	uint16_t wy;
	uint16_t ww;
	uint16_t wh;

	/* Layer 0 */
	uint32_t l0m;
	uint32_t l0oa;
	uint32_t l0da;
	uint16_t l0dx;
	uint16_t l0dy;

	/* Layer 1 */
	uint32_t l1m;
	uint32_t cbda0;
	uint32_t cbda1;
	uint32_t pad02;

	/* Layer 2 */
	uint32_t l2m;
	uint32_t l2oa0;
	uint32_t l2da0;
	uint32_t l2oa1;
	uint32_t l2da1;
	uint16_t l2dx;
	uint16_t l2dy;

	/* Layer 3 */
	uint32_t l3m;
	uint32_t l3oa0;
	uint32_t l3da0;
	uint32_t l3oa1;
	uint32_t l3da1;
	uint16_t l3dx;
	uint16_t l3dy;

	/* Layer 4 */
	uint32_t l4m;
	uint32_t l4oa0;
	uint32_t l4da0;
	uint32_t l4oa1;
	uint32_t l4da1;
	uint16_t l4dx;
	uint16_t l4dy;

	/* Layer 5 */
	uint32_t l5m;
	uint32_t l5oa0;
	uint32_t l5da0;
	uint32_t l5oa1;
	uint32_t l5da1;
	uint16_t l5dx;
	uint16_t l5dy;

	/* Cursor */
	uint16_t cutc;
	uint8_t  cpm;
	uint8_t  csize;
	uint32_t cuoa0;
	uint16_t cux0;
	uint16_t cuy0;
	uint32_t cuoa1;
	uint16_t cux1;
	uint16_t cuy1;

	/* Layer blending */
	uint32_t l0bld;
	uint32_t pad03;
	uint32_t l0tc;
	uint16_t l3tc;
	uint16_t l2tc;
	uint32_t pad04[15];

	/* Display settings */
	uint32_t dcm1;
	uint32_t dcm2;
	uint32_t dcm3;
	uint32_t pad05;

	/* Layer 0 extended */
	uint32_t l0em;
	uint16_t l0wx;
	uint16_t l0wy;
	uint16_t l0ww;
	uint16_t l0wh;
	uint32_t pad06;

	/* Layer 1 extended */
	uint32_t l1em;
	uint16_t l1wx;
	uint16_t l1wy;
	uint16_t l1ww;
	uint16_t l1wh;
	uint32_t pad07;

	/* Layer 2 extended */
	uint32_t l2em;
	uint16_t l2wx;
	uint16_t l2wy;
	uint16_t l2ww;
	uint16_t l2wh;
	uint32_t pad08;

	/* Layer 3 extended */
	uint32_t l3em;
	uint16_t l3wx;
	uint16_t l3wy;
	uint16_t l3ww;
	uint16_t l3wh;
	uint32_t pad09;

	/* Layer 4 extended */
	uint32_t l4em;
	uint16_t l4wx;
	uint16_t l4wy;
	uint16_t l4ww;
	uint16_t l4wh;
	uint32_t pad10;

	/* Layer 5 extended */
	uint32_t l5em;
	uint16_t l5wx;
	uint16_t l5wy;
	uint16_t l5ww;
	uint16_t l5wh;
	uint32_t pad11;

	/* Multi screen control */
	uint32_t msc;
	uint32_t pad12[3];
	uint32_t dls;
	uint32_t dbgc;

	/* Layer blending */
	uint32_t l1bld;
	uint32_t l2bld;
	uint32_t l3bld;
	uint32_t l4bld;
	uint32_t l5bld;
	uint32_t pad13;

	/* Extended transparency control */
	uint32_t l0etc;
	uint32_t l1etc;
	uint32_t l2etc;
	uint32_t l3etc;
	uint32_t l4etc;
	uint32_t l5etc;
	uint32_t pad14[10];

	/* YUV coefficients */
	uint32_t l1ycr0;
	uint32_t l1ycr1;
	uint32_t l1ycg0;
	uint32_t l1ycg1;
	uint32_t l1ycb0;
	uint32_t l1ycb1;
	uint32_t pad15[130];

	/* Layer palletes */
	uint32_t l0pal[256];
	uint32_t l1pal[256];
	uint32_t pad16[256];
	uint32_t l2pal[256];
	uint32_t l3pal[256];
	uint32_t pad17[256];

	/* PWM settings */
	uint32_t vpwmm;
	uint16_t vpwms;
	uint16_t vpwme;
	uint32_t vpwmc;
	uint32_t pad18[253];
};

/* mb86r0x gdc capture controller */
struct mb86r0x_gdc_cap {
	uint32_t vcm;
	uint32_t csc;
	uint32_t vcs;
	uint32_t pad01;

	uint32_t cbm;
	uint32_t cboa;
	uint32_t cbla;
	uint16_t cihstr;
	uint16_t civstr;
	uint16_t cihend;
	uint16_t civend;
	uint32_t pad02;

	uint32_t chp;
	uint32_t cvp;
	uint32_t pad03[4];

	uint32_t clpf;
	uint32_t pad04;
	uint32_t cmss;
	uint32_t cmds;
	uint32_t pad05[12];

	uint32_t rgbhc;
	uint32_t rgbhen;
	uint32_t rgbven;
	uint32_t pad06;
	uint32_t rgbs;
	uint32_t pad07[11];

	uint32_t rgbcmy;
	uint32_t rgbcmcb;
	uint32_t rgbcmcr;
	uint32_t rgbcmb;
	uint32_t pad08[12 + 1984];
};

/* mb86r0x gdc draw */
struct mb86r0x_gdc_draw {
	uint32_t ys;
	uint32_t xs;
	uint32_t dxdy;
	uint32_t xus;
	uint32_t dxudy;
	uint32_t xls;
	uint32_t dxldy;
	uint32_t usn;
	uint32_t lsn;
	uint32_t pad01[7];
	uint32_t rs;
	uint32_t drdx;
	uint32_t drdy;
	uint32_t gs;
	uint32_t dgdx;
	uint32_t dgdy;
	uint32_t bs;
	uint32_t dbdx;
	uint32_t dbdy;
	uint32_t pad02[7];
	uint32_t zs;
	uint32_t dzdx;
	uint32_t dzdy;
	uint32_t pad03[13];
	uint32_t ss;
	uint32_t dsdx;
	uint32_t dsdy;
	uint32_t ts;
	uint32_t dtdx;
	uint32_t dtdy;
	uint32_t qs;
	uint32_t dqdx;
	uint32_t dqdy;
	uint32_t pad04[23];
	uint32_t lpn;
	uint32_t lxs;
	uint32_t lxde;
	uint32_t lys;
	uint32_t lyde;
	uint32_t lzs;
	uint32_t lzde;
	uint32_t pad05[13];
	uint32_t pxdc;
	uint32_t pydc;
	uint32_t pzdc;
	uint32_t pad06[25];
	uint32_t rxs;
	uint32_t rys;
	uint32_t rsizex;
	uint32_t rsizey;
	uint32_t pad07[12];
	uint32_t saddr;
	uint32_t sstride;
	uint32_t srx;
	uint32_t sry;
	uint32_t daddr;
	uint32_t dstride;
	uint32_t drx;
	uint32_t dry;
	uint32_t brsizex;
	uint32_t brsizey;
	uint32_t tcolor;
	uint32_t pad08[93];
	uint32_t blpo;
	uint32_t pad09[7];
	uint32_t ctr;
	uint32_t ifsr;
	uint32_t ifcnt;
	uint32_t sst;
	uint32_t ds;
	uint32_t pst;
	uint32_t est;
	uint32_t pad10;
	uint32_t mdr0;
	uint32_t mdr1;
	uint32_t mdr2;
	uint32_t mdr3;
	uint32_t mdr4;
	uint32_t pad14[2];
	uint32_t mdr7;
	uint32_t fbr;
	uint32_t xres;
	uint32_t zbr;
	uint32_t tbr;
	uint32_t pfbr;
	uint32_t cxmin;
	uint32_t cxmax;
	uint32_t cymin;
	uint32_t cymax;
	uint32_t txs;
	uint32_t tis;
	uint32_t toa;
	uint32_t sho;
	uint32_t abr;
	uint32_t pad15[2];
	uint32_t fc;
	uint32_t bc;
	uint32_t alf;
	uint32_t blp;
	uint32_t pad16;
	uint32_t tbc;
	uint32_t pad11[42];
	uint32_t lx0dc;
	uint32_t ly0dc;
	uint32_t lx1dc;
	uint32_t ly1dc;
	uint32_t pad12[12];
	uint32_t x0dc;
	uint32_t y0dc;
	uint32_t x1dc;
	uint32_t y1dc;
	uint32_t x2dc;
	uint32_t y2dc;
	uint32_t pad13[666];
};

/* mb86r0x gdc geometry engine */
struct mb86r0x_gdc_geom {
	uint32_t gctr;
	uint32_t pad00[15];
	uint32_t gmdr0;
	uint32_t gmdr1;
	uint32_t gmdr2;
	uint32_t pad01[237];
	uint32_t dfifog;
	uint32_t pad02[767];
};

/* mb86r0x gdc */
struct mb86r0x_gdc {
	uint32_t pad00[2];
	uint32_t lts;
	uint32_t pad01;
	uint32_t lsta;
	uint32_t pad02[3];
	uint32_t ist;
	uint32_t imask;
	uint32_t pad03[6];
	uint32_t lsa;
	uint32_t lco;
	uint32_t lreq;

	uint32_t pad04[16*1024 - 19];
	struct mb86r0x_gdc_dsp dsp0;
	struct mb86r0x_gdc_dsp dsp1;
	uint32_t pad05[4*1024 - 2];
	uint32_t vccc;
	uint32_t vcsr;
	struct mb86r0x_gdc_cap cap0;
	struct mb86r0x_gdc_cap cap1;
	uint32_t pad06[4*1024];
	uint32_t texture_base[16*1024];
	struct mb86r0x_gdc_draw draw;
	uint32_t pad07[7*1024];
	struct mb86r0x_gdc_geom geom;
	uint32_t pad08[7*1024];
};

#endif /* __ASSEMBLY__ */

/*
 * Physical Address Defines
 */
#define MB86R0x_DDR2_BASE		0xf3000000
#define MB86R0x_GDC_BASE		0xf1fc0000
#define MB86R0x_CCNT_BASE		0xfff42000
#define MB86R0x_CAN0_BASE		0xfff54000
#define MB86R0x_CAN1_BASE		0xfff55000
#define MB86R0x_I2C0_BASE		0xfff56000
#define MB86R0x_I2C1_BASE		0xfff57000
#define MB86R0x_EHCI_BASE		0xfff80000
#define MB86R0x_OHCI_BASE		0xfff81000
#define MB86R0x_IRC1_BASE		0xfffb0000
#define MB86R0x_MEMC_BASE		0xfffc0000
#define MB86R0x_TIMER_BASE		0xfffe0000
#define MB86R0x_UART0_BASE		0xfffe1000
#define MB86R0x_UART1_BASE		0xfffe2000
#define MB86R0x_IRCE_BASE		0xfffe4000
#define MB86R0x_CRG_BASE		0xfffe7000
#define MB86R0x_IRC0_BASE		0xfffe8000
#define MB86R0x_GPIO_BASE		0xfffe9000
#define MB86R0x_PWM0_BASE		0xfff41000
#define MB86R0x_PWM1_BASE		0xfff41100

#define MB86R0x_CRSR_SWRSTREQ 		(1 << 1)

/*
 * Timer register bits
 */
#define MB86R0x_TIMER_ENABLE		(1 << 7)
#define MB86R0x_TIMER_MODE_MSK		(1 << 6)
#define MB86R0x_TIMER_MODE_FR		(0 << 6)
#define MB86R0x_TIMER_MODE_PD		(1 << 6)

#define MB86R0x_TIMER_INT_EN		(1 << 5)
#define MB86R0x_TIMER_PRS_MSK		(3 << 2)
#define MB86R0x_TIMER_PRS_4S		(1 << 2)
#define MB86R0x_TIMER_PRS_8S		(1 << 3)
#define MB86R0x_TIMER_SIZE_32		(1 << 1)
#define MB86R0x_TIMER_ONE_SHT		(1 << 0)

/*
 * Clock reset generator bits
 */
#define MB86R0x_CRG_CRPR_PLLRDY		(1 << 8)
#define MB86R0x_CRG_CRPR_PLLMODE	(0x1f << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X49	(0 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X46	(1 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X37	(2 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X20	(3 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X47	(4 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X44	(5 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X36	(6 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X19	(7 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X39	(8 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X38	(9 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X30	(10 << 0)
#define MB86R0x_CRG_CRPR_PLLMODE_X15	(11 << 0)
/*
 * DDR2 controller bits
 */
#define MB86R0x_DDR2_DRCI_DRINI		(1 << 15)
#define MB86R0x_DDR2_DRCI_CKEN		(1 << 14)
#define MB86R0x_DDR2_DRCI_DRCMD		(1 << 0)
#define MB86R0x_DDR2_DRCI_CMD		(MB86R0x_DDR2_DRCI_DRINI | \
					MB86R0x_DDR2_DRCI_CKEN | \
					MB86R0x_DDR2_DRCI_DRCMD)
#define MB86R0x_DDR2_DRCI_INIT		(MB86R0x_DDR2_DRCI_DRINI | \
					MB86R0x_DDR2_DRCI_CKEN)
#define MB86R0x_DDR2_DRCI_NORMAL	MB86R0x_DDR2_DRCI_CKEN
#endif /* MB86R0X_H */
