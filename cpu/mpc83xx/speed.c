/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2004 Freescale Semiconductor, Inc.
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
 *
 * Change log:
 *
 * 20050101: Eran Liberty (liberty@freescale.com)
 *           Initial file creating (porting from 85XX & 8260)
 */

#include <common.h>
#include <mpc83xx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/* ----------------------------------------------------------------- */

typedef enum {
	_unk,
	_off,
	_byp,
	_x8,
	_x4,
	_x2,
	_x1,
	_1x,
	_1_5x,
	_2x,
	_2_5x,
	_3x
} mult_t;

typedef struct {
	mult_t core_csb_ratio;
	mult_t  vco_divider;
} corecnf_t;

corecnf_t corecnf_tab[] = {
	{ _byp, _byp},	/* 0x00 */
	{ _byp, _byp},	/* 0x01 */
	{ _byp, _byp},	/* 0x02 */
	{ _byp, _byp},	/* 0x03 */
	{ _byp, _byp},	/* 0x04 */
	{ _byp, _byp},	/* 0x05 */
	{ _byp, _byp},	/* 0x06 */
	{ _byp, _byp},	/* 0x07 */
	{  _1x,  _x2},	/* 0x08 */
	{  _1x,  _x4},	/* 0x09 */
	{  _1x,  _x8},	/* 0x0A */
	{  _1x,  _x8},	/* 0x0B */
	{_1_5x,  _x2},	/* 0x0C */
	{_1_5x,  _x4},	/* 0x0D */
	{_1_5x,  _x8},	/* 0x0E */
	{_1_5x,  _x8},	/* 0x0F */
	{  _2x,  _x2},	/* 0x10 */
	{  _2x,  _x4},	/* 0x11 */
	{  _2x,  _x8},	/* 0x12 */
	{  _2x,  _x8},	/* 0x13 */
	{_2_5x,  _x2},	/* 0x14 */
	{_2_5x,  _x4},	/* 0x15 */
	{_2_5x,  _x8},	/* 0x16 */
	{_2_5x,  _x8},	/* 0x17 */
	{  _3x,  _x2},	/* 0x18 */
	{  _3x,  _x4},	/* 0x19 */
	{  _3x,  _x8},	/* 0x1A */
	{  _3x,  _x8},	/* 0x1B */
};

/* ----------------------------------------------------------------- */

/*
 *
 */
int get_clocks (void)
{
	volatile immap_t *im = (immap_t *)CFG_IMMRBAR;
	u32 pci_sync_in;
	u8  spmf;
	u8  clkin_div;
	u32 sccr;
	u32 corecnf_tab_index;
	u8  corepll;
	u32 lcrr;

	u32 csb_clk;
	u32 tsec1_clk;
	u32 tsec2_clk;
	u32 core_clk;
	u32 usbmph_clk;
	u32 usbdr_clk;
	u32 i2c_clk;
	u32 enc_clk;
	u32 lbiu_clk;
	u32 lclk_clk;
	u32 ddr_clk;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -1;

#ifndef CFG_HRCW_HIGH
# error "CFG_HRCW_HIGH must be defined in board config file"
#endif /* CFG_HCWD_HIGH */

#if (CFG_HRCW_HIGH & HRCWH_PCI_HOST)

# ifndef CONFIG_83XX_CLKIN
#  error "In PCI Host Mode, CONFIG_83XX_CLKIN must be defined in board config file"
# endif /* CONFIG_83XX_CLKIN */
# ifdef CONFIG_83XX_PCICLK
#  warning "In PCI Host Mode, CONFIG_83XX_PCICLK in board config file is igonred"
# endif /* CONFIG_83XX_PCICLK */

	/* PCI Host Mode */
	if (!(im->reset.rcwh & RCWH_PCIHOST)) {
		/* though RCWH_PCIHOST is defined in CFG_HRCW_HIGH
		 * the im->reset.rcwhr PCI Host Mode is disabled
		 * FIXME: findout if there is a way to issue some warning */
		return -2;
	}
	if (im->clk.spmr & SPMR_CKID) {
		/* PCI Clock is half CONFIG_83XX_CLKIN */
		pci_sync_in = CONFIG_83XX_CLKIN / 2;
	}
	else {
		pci_sync_in = CONFIG_83XX_CLKIN;
	}

#else /* (CFG_HRCW_HIGH & HRCWH_PCI_HOST) */

# ifdef CONFIG_83XX_CLKIN
#  warning "In PCI Agent Mode, CONFIG_83XX_CLKIN in board config file is igonred"
# endif /* CONFIG_83XX_CLKIN */
# ifndef CONFIG_83XX_PCICLK
#  error "In PCI Agent Mode, CONFIG_83XX_PCICLK must be defined in board config file"
# endif /* CONFIG_83XX_PCICLK */

	/* PCI Agent Mode */
	if (im->reset.rcwh & RCWH_PCIHOST) {
		/* though RCWH_PCIHOST is not defined in CFG_HRCW_HIGH
		 * the im->reset.rcwhr PCI Host Mode is enabled */
		return -3;
	}
	pci_sync_in = CONFIG_83XX_PCICLK;

#endif /* (CFG_HRCW_HIGH | RCWH_PCIHOST) */

	/* we have up to date pci_sync_in */
	spmf = ((im->reset.rcwl & RCWL_SPMF) >> RCWL_SPMF_SHIFT);
	clkin_div = ((im->clk.spmr & SPMR_CKID) >> SPMR_CKID_SHIFT);

	if ((im->reset.rcwl & RCWL_LBIUCM) || (im->reset.rcwl & RCWL_DDRCM)) {
		csb_clk	= (pci_sync_in * spmf * (1 + clkin_div)) / 2;
	}
	else {
		csb_clk = pci_sync_in * spmf * (1 + clkin_div);
	}

	sccr = im->clk.sccr;
	switch ((sccr & SCCR_TSEC1CM) >> SCCR_TSEC1CM_SHIFT) {
	case 0:
		tsec1_clk = 0;
		break;
	case 1:
		tsec1_clk = csb_clk;
		break;
	case 2:
		tsec1_clk = csb_clk / 2;
		break;
	case 3:
		tsec1_clk = csb_clk / 3;
		break;
	default:
		/* unkown SCCR_TSEC1CM value */
		return -4;
	}

	switch ((sccr & SCCR_TSEC2CM) >> SCCR_TSEC2CM_SHIFT) {
	case 0:
		tsec2_clk = 0;
		break;
	case 1:
		tsec2_clk = csb_clk;
		break;
	case 2:
		tsec2_clk = csb_clk / 2;
		break;
	case 3:
		tsec2_clk = csb_clk / 3;
		break;
	default:
		/* unkown SCCR_TSEC2CM value */
		return -5;
	}
	i2c_clk = tsec2_clk;

	switch ((sccr & SCCR_ENCCM) >> SCCR_ENCCM_SHIFT) {
	case 0:
		enc_clk = 0;
		break;
	case 1:
		enc_clk = csb_clk;
		break;
	case 2:
		enc_clk = csb_clk / 2;
		break;
	case 3:
		enc_clk = csb_clk / 3;
		break;
	default:
		/* unkown SCCR_ENCCM value */
		return -6;
	}

	switch ((sccr & SCCR_USBMPHCM) >> SCCR_USBMPHCM_SHIFT) {
	case 0:
		usbmph_clk = 0;
		break;
	case 1:
		usbmph_clk = csb_clk;
		break;
	case 2:
		usbmph_clk = csb_clk / 2;
		break;
	case 3:
		usbmph_clk = csb_clk / 3;
		break;
	default:
		/* unkown SCCR_USBMPHCM value */
		return -7;
	}

	switch ((sccr & SCCR_USBDRCM) >> SCCR_USBDRCM_SHIFT) {
	case 0:
		usbdr_clk = 0;
		break;
	case 1:
		usbdr_clk = csb_clk;
		break;
	case 2:
		usbdr_clk = csb_clk / 2;
		break;
	case 3:
		usbdr_clk = csb_clk / 3;
		break;
	default:
		/* unkown SCCR_USBDRCM value */
		return -8;
	}

	if (usbmph_clk != 0
		&& usbdr_clk != 0
		&& usbmph_clk != usbdr_clk ) {
		/* if USB MPH clock is not disabled and USB DR clock is not disabled than USB MPH & USB DR must have the same rate */
		return -9;
	}

	lbiu_clk = csb_clk * (1 + ((im->reset.rcwl & RCWL_LBIUCM) >> RCWL_LBIUCM_SHIFT));
	lcrr = (im->lbus.lcrr & LCRR_CLKDIV) >> LCRR_CLKDIV_SHIFT;
	switch (lcrr) {
	case 2:
	case 4:
	case 8:
		lclk_clk = lbiu_clk / lcrr;
		break;
	default:
		/* unknown lcrr */
		return -10;
	}

	ddr_clk = csb_clk * (1 + ((im->reset.rcwl & RCWL_DDRCM) >> RCWL_DDRCM_SHIFT));

	corepll = (im->reset.rcwl & RCWL_COREPLL) >> RCWL_COREPLL_SHIFT;
	corecnf_tab_index = ((corepll & 0x1F) << 2) | ((corepll & 0x60) >> 5);
	if (corecnf_tab_index > (sizeof(corecnf_tab)/sizeof(corecnf_t)) ) {
		/* corecnf_tab_index is too high, possibly worng value */
		return -11;
	}
	switch (corecnf_tab[corecnf_tab_index].core_csb_ratio) {
	case _byp:
	case _x1:
	case _1x:
		core_clk = csb_clk;
		break;
	case _1_5x:
		core_clk = (3 * csb_clk) / 2;
		break;
	case _2x:
		core_clk = 2 * csb_clk;
		break;
	case _2_5x:
		core_clk = ( 5 * csb_clk) / 2;
		break;
	case _3x:
		core_clk = 3 * csb_clk;
		break;
	default:
		/* unkown core to csb ratio */
		return -12;
	}

	gd->csb_clk    = csb_clk   ;
	gd->tsec1_clk  = tsec1_clk ;
	gd->tsec2_clk  = tsec2_clk ;
	gd->core_clk   = core_clk  ;
	gd->usbmph_clk = usbmph_clk;
	gd->usbdr_clk  = usbdr_clk ;
	gd->i2c_clk    = i2c_clk   ;
	gd->enc_clk    = enc_clk   ;
	gd->lbiu_clk   = lbiu_clk  ;
	gd->lclk_clk   = lclk_clk  ;
	gd->ddr_clk    = ddr_clk   ;
	gd->pci_clk    = pci_sync_in;

	gd->cpu_clk = gd->core_clk;
	gd->bus_clk = gd->lbiu_clk;
	return 0;
}

/********************************************
 * get_bus_freq
 * return system bus freq in Hz
 *********************************************/
ulong get_bus_freq (ulong dummy)
{
	return gd->csb_clk;
}

int print_clock_conf (void)
{
	printf("Clock configuration:\n");
	printf("  Coherent System Bus: %4d MHz\n",gd->csb_clk/1000000);
	printf("  Core:                %4d MHz\n",gd->core_clk/1000000);
	debug("  Local Bus Controller:%4d MHz\n",gd->lbiu_clk/1000000);
	printf("  Local Bus:           %4d MHz\n",gd->lclk_clk/1000000);
	debug("  DDR:                 %4d MHz\n",gd->ddr_clk/1000000);
	debug("  I2C:                 %4d MHz\n",gd->i2c_clk/1000000);
	debug("  TSEC1:               %4d MHz\n",gd->tsec1_clk/1000000);
	debug("  TSEC2:               %4d MHz\n",gd->tsec2_clk/1000000);
	debug("  USB MPH:             %4d MHz\n",gd->usbmph_clk/1000000);
	debug("  USB DR:              %4d MHz\n",gd->usbdr_clk/1000000);

	return 0;
}
