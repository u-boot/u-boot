/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <mpc8260.h>
#include <asm/processor.h>

/* ------------------------------------------------------------------------- */

/* Bus-to-Core Multiplier */
#define _1x	2
#define _1_5x	3
#define _2x	4
#define _2_5x	5
#define _3x	6
#define _3_5x	7
#define _4x	8
#define _4_5x	9
#define _5x	10
#define _5_5x	11
#define _6x	12
#define _6_5x	13
#define _7x	14
#define _7_5x	15
#define _8x	16
#define _byp	-1
#define _off	-2
#define _unk	-3

typedef struct {
	int b2c_mult;
	int vco_div;
	char *freq_60x;
	char *freq_core;
} corecnf_t;

/*
 * this table based on "Errata to MPC8260 PowerQUICC II User's Manual",
 * Rev. 1, 8/2000, page 10.
 */
corecnf_t corecnf_tab[] = {
	{ _1_5x,  4, " 33-100", " 33-100" },	/* 0x00 */
	{   _1x,  4, " 50-150", " 50-150" },	/* 0x01 */
	{   _1x,  8, " 25-75 ", " 25-75 " },	/* 0x02 */
	{  _byp, -1, "  ?-?  ", "  ?-?  " },	/* 0x03 */
	{   _2x,  2, " 50-150", "100-300" },	/* 0x04 */
	{   _2x,  4, " 25-75 ", " 50-150" },	/* 0x05 */
	{ _2_5x,  2, " 40-120", "100-240" },	/* 0x06 */
	{ _4_5x,  2, " 22-65 ", "100-300" },	/* 0x07 */
	{   _3x,  2, " 33-100", "100-300" },	/* 0x08 */
	{ _5_5x,  2, " 18-55 ", "100-300" },	/* 0x09 */
	{   _4x,  2, " 25-75 ", "100-300" },	/* 0x0A */
	{   _5x,  2, " 20-60 ", "100-300" },	/* 0x0B */
	{ _1_5x,  8, " 16-50 ", " 16-50 " },	/* 0x0C */
	{   _6x,  2, " 16-50 ", "100-300" },	/* 0x0D */
	{ _3_5x,  2, " 30-85 ", "100-300" },	/* 0x0E */
	{  _off, -1, "  ?-?  ", "  ?-?  " },	/* 0x0F */
	{   _3x,  4, " 16-50 ", " 50-150" },	/* 0x10 */
	{ _2_5x,  4, " 20-60 ", " 50-120" },	/* 0x11 */
	{ _6_5x,  2, " 15-46 ", "100-300" },	/* 0x12 */
	{  _byp, -1, "  ?-?  ", "  ?-?  " },	/* 0x13 */
	{   _7x,  2, " 14-43 ", "100-300" },	/* 0x14 */
	{   _2x,  4, " 25-75 ", " 50-150" },	/* 0x15 */
	{ _7_5x,  2, " 13-40 ", "100-300" },	/* 0x16 */
	{ _4_5x,  2, " 22-65 ", "100-300" },	/* 0x17 */
	{  _unk, -1, "  ?-?  ", "  ?-?  " },	/* 0x18 */
	{ _5_5x,  2, " 18-55 ", "100-300" },	/* 0x19 */
	{   _4x,  2, " 25-75 ", "100-300" },	/* 0x1A */
	{   _5x,  2, " 20-60 ", "100-300" },	/* 0x1B */
	{   _8x,  2, " 12-38 ", "100-300" },	/* 0x1C */
	{   _6x,  2, " 16-50 ", "100-300" },	/* 0x1D */
	{ _3_5x,  2, " 30-85 ", "100-300" },	/* 0x1E */
	{  _off, -1, "  ?-?  ", "  ?-?  " },	/* 0x1F */
};

/* ------------------------------------------------------------------------- */

/*
 *
 */

int get_clocks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	ulong clkin;
	ulong sccr, dfbrg;
	ulong scmr, corecnf, busdf, cpmdf, plldf, pllmf;
	corecnf_t *cp;

#if !defined(CONFIG_8260_CLKIN)
#error clock measuring not implemented yet - define CONFIG_8260_CLKIN
#else
	clkin = CONFIG_8260_CLKIN;
#endif

	sccr = immap->im_clkrst.car_sccr;
	dfbrg = (sccr & SCCR_DFBRG_MSK) >> SCCR_DFBRG_SHIFT;

	scmr = immap->im_clkrst.car_scmr;
	corecnf = (scmr & SCMR_CORECNF_MSK) >> SCMR_CORECNF_SHIFT;
	cp = &corecnf_tab[corecnf];

	busdf = (scmr & SCMR_BUSDF_MSK) >> SCMR_BUSDF_SHIFT;
	cpmdf = (scmr & SCMR_CPMDF_MSK) >> SCMR_CPMDF_SHIFT;

	/* HiP7, HiP7 Rev01, HiP7 RevA */
	if ((get_pvr () == PVR_8260_HIP7) ||
	    (get_pvr () == PVR_8260_HIP7R1) ||
	    (get_pvr () == PVR_8260_HIP7RA)) {
		pllmf = (scmr & SCMR_PLLMF_MSKH7) >> SCMR_PLLMF_SHIFT;
		gd->vco_out = clkin * (pllmf + 1);
	} else {                        /* HiP3, HiP4 */
		pllmf = (scmr & SCMR_PLLMF_MSK) >> SCMR_PLLMF_SHIFT;
		plldf = (scmr & SCMR_PLLDF) ? 1 : 0;
		gd->vco_out = (clkin * 2 * (pllmf + 1)) / (plldf + 1);
	}
#if 0
	if (gd->vco_out / (busdf + 1) != clkin) {
		/* aaarrrggghhh!!! */
		return (1);
	}
#endif

	gd->cpm_clk = gd->vco_out / 2;
	gd->bus_clk = clkin;
	gd->scc_clk = gd->vco_out / 4;
	gd->brg_clk = gd->vco_out / (1 << (2 * (dfbrg + 1)));

	if (cp->b2c_mult > 0) {
		gd->cpu_clk = (clkin * cp->b2c_mult) / 2;
	} else {
		gd->cpu_clk = clkin;
	}

	return (0);
}

int prt_8260_clks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	ulong sccr, dfbrg;
	ulong scmr, corecnf, busdf, cpmdf, plldf, pllmf;
	corecnf_t *cp;

	sccr = immap->im_clkrst.car_sccr;
	dfbrg = (sccr & SCCR_DFBRG_MSK) >> SCCR_DFBRG_SHIFT;

	scmr = immap->im_clkrst.car_scmr;
	corecnf = (scmr & SCMR_CORECNF_MSK) >> SCMR_CORECNF_SHIFT;
	busdf = (scmr & SCMR_BUSDF_MSK) >> SCMR_BUSDF_SHIFT;
	cpmdf = (scmr & SCMR_CPMDF_MSK) >> SCMR_CPMDF_SHIFT;
	plldf = (scmr & SCMR_PLLDF) ? 1 : 0;
	pllmf = (scmr & SCMR_PLLMF_MSK) >> SCMR_PLLMF_SHIFT;

	cp = &corecnf_tab[corecnf];

	puts (CPU_ID_STR " Clock Configuration\n - Bus-to-Core Mult ");

	switch (cp->b2c_mult) {
	case _byp:
		puts ("BYPASS");
		break;

	case _off:
		puts ("OFF");
		break;

	case _unk:
		puts ("UNKNOWN");
		break;

	default:
		printf ("%d%sx",
			cp->b2c_mult / 2,
			(cp->b2c_mult % 2) ? ".5" : "");
		break;
	}

	printf (", VCO Div %d, 60x Bus Freq %s, Core Freq %s\n",
			cp->vco_div, cp->freq_60x, cp->freq_core);

	printf (" - dfbrg %ld, corecnf 0x%02lx, busdf %ld, cpmdf %ld, "
			"plldf %ld, pllmf %ld\n", dfbrg, corecnf, busdf, cpmdf, plldf,
			pllmf);

	printf (" - vco_out %10ld, scc_clk %10ld, brg_clk %10ld\n",
			gd->vco_out, gd->scc_clk, gd->brg_clk);

	printf (" - cpu_clk %10ld, cpm_clk %10ld, bus_clk %10ld\n",
			gd->cpu_clk, gd->cpm_clk, gd->bus_clk);

	if (sccr & SCCR_PCI_MODE) {
		uint pci_div;

		pci_div = ( (sccr & SCCR_PCI_MODCK) ? 2 : 1) *
			( ( (sccr & SCCR_PCIDF_MSK) >> SCCR_PCIDF_SHIFT) + 1);

		printf (" - pci_clk %10ld\n", (gd->cpm_clk * 2) / pci_div);
	}
	putc ('\n');

	return (0);
}

/* ------------------------------------------------------------------------- */
