/*
 * (C) Copyright 2003 Intracom S.A.
 * Pantelis Antoniou <panto@intracom.gr>
 *
 * This little program makes an exhaustive search for the
 * correct terms of pdf, mfi, mfn, mfd, s, dbrmo, in PLPRCR.
 * The goal is to produce a gclk2 from a xin input, while respecting
 * all the restrictions on their combination.
 *
 * Generaly you select the first row of the produced table.
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


#include <stdio.h>
#include <stdlib.h>

#define DPREF_MIN	 10000000
#define DPREF_MAX	 32000000

#define DPGDCK_MAX	320000000
#define DPGDCK_MIN	160000000

#define S_MIN		0
#define S_MAX		2

#define MFI_MIN		5
#define MFI_MAX		15

#define MFN_MIN		0
#define MFN_MAX		15

#define MFD_MIN		0
#define MFD_MAX		31

#define MF_MIN		5
#define MF_MAX		15

#define PDF_MIN		0
#define PDF_MAX		15

#define GCLK2_MAX	150000000

static int calculate (int xin, int target_clock,
		      int ppm, int pdf, int mfi, int mfn, int mfd, int s,
		      int *dprefp, int *dpgdckp, int *jdbckp,
		      int *gclk2p, int *dbrmop)
{
	unsigned int dpref, dpgdck, jdbck, gclk2, t1, t2, dbrmo;

	/* valid MFI? */
	if (mfi < MFI_MIN)
		return -1;

	/* valid num, denum? */
	if (mfn > 0 && mfn >= mfd)
		return -1;

	dpref = xin / (pdf + 1);

	/* valid dpef? */
	if (dpref < DPREF_MIN || dpref > DPREF_MAX)
		return -1;

	if (mfn == 0) {
		dpgdck  = (2 * mfi * xin) / (pdf + 1) ;
		dbrmo = 0;
	} else {
		/* 5 <= mfi + (mfn / mfd + 1) <= 15 */
		t1 = mfd + 1;
		t2 = mfi * t1 + mfn;
		if ( MF_MIN * t1 > t2 || MF_MAX * t1 < t2)
			return -1;

		dpgdck  = (unsigned int)(2 * (mfi * mfd + mfi + mfn) *
				(unsigned int)xin) /
				((mfd + 1) * (pdf + 1));

		dbrmo = 10 * mfn < (mfd + 1);
	}

	/* valid dpgclk? */
	if (dpgdck < DPGDCK_MIN || dpgdck > DPGDCK_MAX)
		return -1;

	jdbck = dpgdck >> s;
	gclk2 = jdbck / 2;

	/* valid gclk2 */
	if (gclk2 > GCLK2_MAX)
		return -1;

	t1 = abs(gclk2 - target_clock);

	/* XXX max 1MHz dev. in clock */
	if (t1 > 1000000)
		return -1;

	/* dev within range (XXX gclk2 scaled to avoid overflow) */
	if (t1 * 1000 > (unsigned int)ppm * (gclk2 / 1000))
		return -1;

	*dprefp = dpref;
	*dpgdckp = dpgdck;
	*jdbckp = jdbck;
	*gclk2p = gclk2;
	*dbrmop = dbrmo;

	return gclk2;
}

int conf_clock(int xin, int target_clock, int ppm)
{
	int pdf, s, mfn, mfd, mfi;
	int dpref, dpgdck, jdbck, gclk2, xout, dbrmo;
	int found = 0;

	/* integer multipliers */
	for (pdf = PDF_MIN; pdf <= PDF_MAX; pdf++) {
		for (mfi = MFI_MIN; mfi <= MFI_MAX; mfi++) {
			for (s = 0; s <= S_MAX; s++) {
				xout = calculate(xin, target_clock,
						 ppm, pdf, mfi, 0, 0, s,
						 &dpref, &dpgdck, &jdbck,
						 &gclk2, &dbrmo);
				if (xout < 0)
					continue;

				if (found == 0) {
					printf("pdf mfi mfn mfd s dbrmo     dpref    dpgdck     jdbck     gclk2 exact?\n");
					printf("--- --- --- --- - -----     -----    ------     -----     ----- ------\n");
				}

				printf("%3d %3d --- --- %1d %5d %9d %9d %9d %9d%s\n",
					pdf, mfi, s, dbrmo,
					dpref, dpgdck, jdbck, gclk2,
					gclk2 == target_clock ? "    YES" : "");

				found++;
			}
		}
	}

	/* fractional multipliers */
	for (pdf = PDF_MIN; pdf <= PDF_MAX; pdf++) {
		for (mfi = MFI_MIN; mfi <= MFI_MAX; mfi++) {
			for (mfn = 1; mfn <= MFN_MAX; mfn++) {
				for (mfd = 1; mfd <= MFD_MAX; mfd++) {
					for (s = 0; s <= S_MAX; s++) {
						xout = calculate(xin, target_clock,
							    ppm, pdf, mfi, mfn, mfd, s,
							    &dpref, &dpgdck, &jdbck,
							    &gclk2, &dbrmo);
						if (xout < 0)
							continue;

						if (found == 0) {
							printf("pdf mfi mfn mfd s dbrmo     dpref    dpgdck     jdbck     gclk2 exact?\n");
							printf("--- --- --- --- - -----     -----    ------     -----     ----- ------\n");
						}

						printf("%3d %3d %3d %3d %1d %5d %9d %9d %9d %9d%s\n",
							pdf, mfi, mfn, mfd, s,
							dbrmo, dpref, dpgdck, jdbck, gclk2,
							gclk2 == target_clock ? "    YES" : "");

						found++;
					}
				}
			}

		}
	}

	return found;
}

int main(int argc, char *argv[])
{
	int xin, want_gclk2, found, ppm = 100;

	if (argc < 3) {
		fprintf(stderr, "usage: mpc86x_clk <xin> <want_gclk2> [ppm]\n");
		fprintf(stderr, "       default ppm is 100\n");
		return 10;
	}

	xin  = atoi(argv[1]);
	want_gclk2 = atoi(argv[2]);
	if (argc >= 4)
		ppm = atoi(argv[3]);

	found = conf_clock(xin, want_gclk2, ppm);
	if (found <= 0) {
		fprintf(stderr, "cannot produce gclk2 %d from xin %d\n",
			want_gclk2, xin);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
