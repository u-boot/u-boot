/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
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

#include <common.h>
#include <mpc83xx.h>
#include <command.h>
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
	mult_t vco_divider;
} corecnf_t;

corecnf_t corecnf_tab[] = {
	{_byp, _byp},		/* 0x00 */
	{_byp, _byp},		/* 0x01 */
	{_byp, _byp},		/* 0x02 */
	{_byp, _byp},		/* 0x03 */
	{_byp, _byp},		/* 0x04 */
	{_byp, _byp},		/* 0x05 */
	{_byp, _byp},		/* 0x06 */
	{_byp, _byp},		/* 0x07 */
	{_1x, _x2},		/* 0x08 */
	{_1x, _x4},		/* 0x09 */
	{_1x, _x8},		/* 0x0A */
	{_1x, _x8},		/* 0x0B */
	{_1_5x, _x2},		/* 0x0C */
	{_1_5x, _x4},		/* 0x0D */
	{_1_5x, _x8},		/* 0x0E */
	{_1_5x, _x8},		/* 0x0F */
	{_2x, _x2},		/* 0x10 */
	{_2x, _x4},		/* 0x11 */
	{_2x, _x8},		/* 0x12 */
	{_2x, _x8},		/* 0x13 */
	{_2_5x, _x2},		/* 0x14 */
	{_2_5x, _x4},		/* 0x15 */
	{_2_5x, _x8},		/* 0x16 */
	{_2_5x, _x8},		/* 0x17 */
	{_3x, _x2},		/* 0x18 */
	{_3x, _x4},		/* 0x19 */
	{_3x, _x8},		/* 0x1A */
	{_3x, _x8},		/* 0x1B */
};

/* ----------------------------------------------------------------- */

/*
 *
 */
int get_clocks(void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 pci_sync_in;
	u8 spmf;
	u8 clkin_div;
	u32 sccr;
	u32 corecnf_tab_index;
	u8 corepll;
	u32 lcrr;

	u32 csb_clk;
#if defined(CONFIG_MPC834X) || defined(CONFIG_MPC831X) || defined(CONFIG_MPC837X)
	u32 tsec1_clk;
	u32 tsec2_clk;
	u32 usbdr_clk;
#endif
#ifdef CONFIG_MPC834X
	u32 usbmph_clk;
#endif
	u32 core_clk;
	u32 i2c1_clk;
#if !defined(CONFIG_MPC832X)
	u32 i2c2_clk;
#endif
#if defined(CONFIG_MPC837X)
	u32 sdhc_clk;
#endif
	u32 enc_clk;
	u32 lbiu_clk;
	u32 lclk_clk;
	u32 ddr_clk;
#if defined(CONFIG_MPC8360)
	u32 ddr_sec_clk;
#endif
#if defined(CONFIG_MPC8360) || defined(CONFIG_MPC832X)
	u32 qepmf;
	u32 qepdf;
	u32 qe_clk;
	u32 brg_clk;
#endif
#if defined(CONFIG_MPC837X)
	u32 pciexp1_clk;
	u32 pciexp2_clk;
	u32 sata_clk;
#endif

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	clkin_div = ((im->clk.spmr & SPMR_CKID) >> SPMR_CKID_SHIFT);

	if (im->reset.rcwh & HRCWH_PCI_HOST) {
#if defined(CONFIG_83XX_CLKIN)
		pci_sync_in = CONFIG_83XX_CLKIN / (1 + clkin_div);
#else
		pci_sync_in = 0xDEADBEEF;
#endif
	} else {
#if defined(CONFIG_83XX_PCICLK)
		pci_sync_in = CONFIG_83XX_PCICLK;
#else
		pci_sync_in = 0xDEADBEEF;
#endif
	}

	spmf = ((im->reset.rcwl & HRCWL_SPMF) >> HRCWL_SPMF_SHIFT);
	csb_clk = pci_sync_in * (1 + clkin_div) * spmf;

	sccr = im->clk.sccr;

#if defined(CONFIG_MPC834X) || defined(CONFIG_MPC831X) || defined(CONFIG_MPC837X)
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
		return -2;
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
		return -3;
	}
#endif

#if defined(CONFIG_MPC834X) || defined(CONFIG_MPC837X)
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
		return -4;
	}
#elif defined(CONFIG_MPC831X)
	tsec2_clk = tsec1_clk;

	if (!(sccr & SCCR_TSEC1ON))
		tsec1_clk = 0;
	if (!(sccr & SCCR_TSEC2ON))
		tsec2_clk = 0;
#endif

#if defined(CONFIG_MPC834X)
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
		return -5;
	}

	if (usbmph_clk != 0 && usbdr_clk != 0 && usbmph_clk != usbdr_clk) {
		/* if USB MPH clock is not disabled and
		 * USB DR clock is not disabled then
		 * USB MPH & USB DR must have the same rate
		 */
		return -6;
	}
#endif
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
		return -7;
	}

#if defined(CONFIG_MPC837X)
	switch ((sccr & SCCR_SDHCCM) >> SCCR_SDHCCM_SHIFT) {
	case 0:
		sdhc_clk = 0;
		break;
	case 1:
		sdhc_clk = csb_clk;
		break;
	case 2:
		sdhc_clk = csb_clk / 2;
		break;
	case 3:
		sdhc_clk = csb_clk / 3;
		break;
	default:
		/* unkown SCCR_SDHCCM value */
		return -8;
	}
#endif

#if defined(CONFIG_MPC834X)
	i2c1_clk = tsec2_clk;
#elif defined(CONFIG_MPC8360)
	i2c1_clk = csb_clk;
#elif defined(CONFIG_MPC832X)
	i2c1_clk = enc_clk;
#elif defined(CONFIG_MPC831X)
	i2c1_clk = enc_clk;
#elif defined(CONFIG_MPC837X)
	i2c1_clk = sdhc_clk;
#endif
#if !defined(CONFIG_MPC832X)
	i2c2_clk = csb_clk; /* i2c-2 clk is equal to csb clk */
#endif

#if defined(CONFIG_MPC837X)
	switch ((sccr & SCCR_PCIEXP1CM) >> SCCR_PCIEXP1CM_SHIFT) {
	case 0:
		pciexp1_clk = 0;
		break;
	case 1:
		pciexp1_clk = csb_clk;
		break;
	case 2:
		pciexp1_clk = csb_clk / 2;
		break;
	case 3:
		pciexp1_clk = csb_clk / 3;
		break;
	default:
		/* unkown SCCR_PCIEXP1CM value */
		return -9;
	}

	switch ((sccr & SCCR_PCIEXP2CM) >> SCCR_PCIEXP2CM_SHIFT) {
	case 0:
		pciexp2_clk = 0;
		break;
	case 1:
		pciexp2_clk = csb_clk;
		break;
	case 2:
		pciexp2_clk = csb_clk / 2;
		break;
	case 3:
		pciexp2_clk = csb_clk / 3;
		break;
	default:
		/* unkown SCCR_PCIEXP2CM value */
		return -10;
	}
#endif

#if defined(CONFIG_MPC837X)
	switch ((sccr & SCCR_SATA1CM) >> SCCR_SATA1CM_SHIFT) {
	case 0:
		sata_clk = 0;
		break;
	case 1:
		sata_clk = csb_clk;
		break;
	case 2:
		sata_clk = csb_clk / 2;
		break;
	case 3:
		sata_clk = csb_clk / 3;
		break;
	default:
		/* unkown SCCR_SATA1CM value */
		return -11;
	}
#endif

	lbiu_clk = csb_clk *
	           (1 + ((im->reset.rcwl & HRCWL_LBIUCM) >> HRCWL_LBIUCM_SHIFT));
	lcrr = (im->lbus.lcrr & LCRR_CLKDIV) >> LCRR_CLKDIV_SHIFT;
	switch (lcrr) {
	case 2:
	case 4:
	case 8:
		lclk_clk = lbiu_clk / lcrr;
		break;
	default:
		/* unknown lcrr */
		return -12;
	}

	ddr_clk = csb_clk *
		  (1 + ((im->reset.rcwl & HRCWL_DDRCM) >> HRCWL_DDRCM_SHIFT));
	corepll = (im->reset.rcwl & HRCWL_COREPLL) >> HRCWL_COREPLL_SHIFT;
#if defined(CONFIG_MPC8360)
	ddr_sec_clk = csb_clk * (1 +
		       ((im->reset.rcwl & HRCWL_LBIUCM) >> HRCWL_LBIUCM_SHIFT));
#endif

	corecnf_tab_index = ((corepll & 0x1F) << 2) | ((corepll & 0x60) >> 5);
	if (corecnf_tab_index > (sizeof(corecnf_tab) / sizeof(corecnf_t))) {
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
		core_clk = (5 * csb_clk) / 2;
		break;
	case _3x:
		core_clk = 3 * csb_clk;
		break;
	default:
		/* unkown core to csb ratio */
		return -13;
	}

#if defined(CONFIG_MPC8360) || defined(CONFIG_MPC832X)
	qepmf = (im->reset.rcwl & HRCWL_CEPMF) >> HRCWL_CEPMF_SHIFT;
	qepdf = (im->reset.rcwl & HRCWL_CEPDF) >> HRCWL_CEPDF_SHIFT;
	qe_clk = (pci_sync_in * qepmf) / (1 + qepdf);
	brg_clk = qe_clk / 2;
#endif

	gd->csb_clk = csb_clk;
#if defined(CONFIG_MPC834X) || defined(CONFIG_MPC831X) || defined(CONFIG_MPC837X)
	gd->tsec1_clk = tsec1_clk;
	gd->tsec2_clk = tsec2_clk;
	gd->usbdr_clk = usbdr_clk;
#endif
#if defined(CONFIG_MPC834X)
	gd->usbmph_clk = usbmph_clk;
#endif
#if defined(CONFIG_MPC837X)
	gd->sdhc_clk = sdhc_clk;
#endif
	gd->core_clk = core_clk;
	gd->i2c1_clk = i2c1_clk;
#if !defined(CONFIG_MPC832X)
	gd->i2c2_clk = i2c2_clk;
#endif
	gd->enc_clk = enc_clk;
	gd->lbiu_clk = lbiu_clk;
	gd->lclk_clk = lclk_clk;
	gd->ddr_clk = ddr_clk;
#if defined(CONFIG_MPC8360)
	gd->ddr_sec_clk = ddr_sec_clk;
#endif
#if defined(CONFIG_MPC8360) || defined(CONFIG_MPC832X)
	gd->qe_clk = qe_clk;
	gd->brg_clk = brg_clk;
#endif
#if defined(CONFIG_MPC837X)
	gd->pciexp1_clk = pciexp1_clk;
	gd->pciexp2_clk = pciexp2_clk;
	gd->sata_clk = sata_clk;
#endif
	gd->pci_clk = pci_sync_in;
	gd->cpu_clk = gd->core_clk;
	gd->bus_clk = gd->csb_clk;
	return 0;

}

/********************************************
 * get_bus_freq
 * return system bus freq in Hz
 *********************************************/
ulong get_bus_freq(ulong dummy)
{
	return gd->csb_clk;
}

int do_clocks (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	printf("Clock configuration:\n");
	printf("  Core:                %4d MHz\n", gd->core_clk / 1000000);
	printf("  Coherent System Bus: %4d MHz\n", gd->csb_clk / 1000000);
#if defined(CONFIG_MPC8360) || defined(CONFIG_MPC832X)
	printf("  QE:                  %4d MHz\n", gd->qe_clk / 1000000);
	printf("  BRG:                 %4d MHz\n", gd->brg_clk / 1000000);
#endif
	printf("  Local Bus Controller:%4d MHz\n", gd->lbiu_clk / 1000000);
	printf("  Local Bus:           %4d MHz\n", gd->lclk_clk / 1000000);
	printf("  DDR:                 %4d MHz\n", gd->ddr_clk / 1000000);
#if defined(CONFIG_MPC8360)
	printf("  DDR Secondary:       %4d MHz\n", gd->ddr_sec_clk / 1000000);
#endif
	printf("  SEC:                 %4d MHz\n", gd->enc_clk / 1000000);
	printf("  I2C1:                %4d MHz\n", gd->i2c1_clk / 1000000);
#if !defined(CONFIG_MPC832X)
	printf("  I2C2:                %4d MHz\n", gd->i2c2_clk / 1000000);
#endif
#if defined(CONFIG_MPC837X)
	printf("  SDHC:                %4d MHz\n", gd->sdhc_clk / 1000000);
#endif
#if defined(CONFIG_MPC834X) || defined(CONFIG_MPC831X) || defined(CONFIG_MPC837X)
	printf("  TSEC1:               %4d MHz\n", gd->tsec1_clk / 1000000);
	printf("  TSEC2:               %4d MHz\n", gd->tsec2_clk / 1000000);
	printf("  USB DR:              %4d MHz\n", gd->usbdr_clk / 1000000);
#endif
#if defined(CONFIG_MPC834X)
	printf("  USB MPH:             %4d MHz\n", gd->usbmph_clk / 1000000);
#endif
#if defined(CONFIG_MPC837X)
	printf("  PCIEXP1:             %4d MHz\n", gd->pciexp1_clk / 1000000);
	printf("  PCIEXP2:             %4d MHz\n", gd->pciexp2_clk / 1000000);
	printf("  SATA:                %4d MHz\n", gd->sata_clk / 1000000);
#endif
	return 0;
}

U_BOOT_CMD(clocks, 1, 0, do_clocks,
	"clocks  - print clock configuration\n",
	"    clocks\n"
);
