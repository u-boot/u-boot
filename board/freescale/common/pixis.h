/*
 * Copyright 2010 Freescale Semiconductor, Inc.
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
#ifndef __PIXIS_H_
#define __PIXIS_H_	1

/* PIXIS register set. */
#if defined(CONFIG_MPC8536DS)
typedef struct pixis {
	u8 id;
	u8 ver;
	u8 pver;
	u8 csr;
	u8 rst;
	u8 rst2;
	u8 aux1;
	u8 spd;
	u8 aux2;
	u8 csr2;
	u8 watch;
	u8 led;
	u8 pwr;
	u8 res[3];
	u8 vctl;
	u8 vstat;
	u8 vcfgen0;
	u8 vcfgen1;
	u8 vcore0;
	u8 res1;
	u8 vboot;
	u8 vspeed[3];
	u8 sclk[3];
	u8 dclk[3];
	u8 i2cdacr;
	u8 vcoreacc[4];
	u8 vcorecnt[3];
	u8 vcoremax[2];
	u8 vplatacc[4];
	u8 vplatcnt[3];
	u8 vplatmax[2];
	u8 vtempacc[4];
	u8 vtempcnt[3];
	u8 vtempmax[2];
	u8 res2[4];
} __attribute__ ((packed)) pixis_t;

#elif defined(CONFIG_MPC8544DS)
typedef struct pixis {
	u8 id;
	u8 ver;
	u8 pver;
	u8 csr;
	u8 rst;
	u8 pwr;
	u8 aux1;
	u8 spd;
	u8 res[8];
	u8 vctl;
	u8 vstat;
	u8 vcfgen0;
	u8 vcfgen1;
	u8 vcore0;
	u8 res1;
	u8 vboot;
	u8 vspeed[2];
	u8 vclkh;
	u8 vclkl;
	u8 watch;
	u8 led;
	u8 vspeed2;
	u8 res2[34];
} __attribute__ ((packed)) pixis_t;

#elif defined(CONFIG_MPC8572DS)
typedef struct pixis {
	u8 id;
	u8 ver;
	u8 pver;
	u8 csr;
	u8 rst;
	u8 pwr1;
	u8 aux1;
	u8 spd;
	u8 aux2;
	u8 res[7];
	u8 vctl;
	u8 vstat;
	u8 vcfgen0;
	u8 vcfgen1;
	u8 vcore0;
	u8 res1;
	u8 vboot;
	u8 vspeed[3];
	u8 res2[2];
	u8 sclk[3];
	u8 dclk[3];
	u8 res3[2];
	u8 watch;
	u8 led;
	u8 res4[25];
} __attribute__ ((packed)) pixis_t;

#elif defined(CONFIG_MPC8610HPCD)
typedef struct pixis {
	u8 id;
	u8 ver;	/* also called arch */
	u8 pver;
	u8 csr;
	u8 rst;
	u8 pwr;
	u8 aux;
	u8 spd;
	u8 brdcfg0;
	u8 brdcfg1;
	u8 res[4];
	u8 led;
	u8 serno;
	u8 vctl;
	u8 vstat;
	u8 vcfgen0;
	u8 vcfgen1;
	u8 vcore0;
	u8 res1;
	u8 vboot;
	u8 vspeed[2];
	u8 res2;
	u8 sclk[3];
	u8 res3;
	u8 watch;
	u8 res4[33];
} __attribute__ ((packed)) pixis_t;

#elif defined(CONFIG_MPC8641HPCN)
typedef struct pixis {
	u8 id;
	u8 ver;
	u8 pver;
	u8 csr;
	u8 rst;
	u8 pwr;
	u8 aux;
	u8 spd;
	u8 res[8];
	u8 vctl;
	u8 vstat;
	u8 vcfgen0;
	u8 vcfgen1;
	u8 vcore0;
	u8 res1;
	u8 vboot;
	u8 vspeed[2];
	u8 vclkh;
	u8 vclkl;
	u8 watch;
	u8 res3[36];
} __attribute__ ((packed)) pixis_t;
#else
#error Need to define pixis_t for this board
#endif

/* Pointer to the PIXIS register set */
#define pixis ((pixis_t *)PIXIS_BASE)

#endif	/* __PIXIS_H_ */
