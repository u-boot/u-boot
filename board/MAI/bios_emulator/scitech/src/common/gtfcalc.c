/****************************************************************************
*
*                   VESA Generalized Timing Formula (GTF)
*                               Version 1.1
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
* Developed by: SciTech Software, Inc.
*
* Language:     ANSI C
* Environment:  Any.
*
* Description:  C module for generating GTF compatible timings given a set
*               of input requirements. Translated from the original GTF
*               1.14 spreadsheet definition.
*
*               Compile with #define TESTING to build a command line test
*               program.
*
*               NOTE: The code in here has been written for clarity and
*                     to follow the original GTF spec as closely as
*                     possible.
*
****************************************************************************/

#include "gtf.h"
#ifndef __WIN32_VXD__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#endif

/*------------------------- Global Variables ------------------------------*/

static GTF_constants GC = {
    1.8,                    /* Margin size as percentage of display     */
    8,                      /* Character cell granularity               */
    1,                      /* Minimum front porch in lines/chars       */
    3,                      /* Width of V sync in lines                 */
    8,                      /* Width of H sync as percent of total      */
    550,                    /* Minimum vertical sync + back porch (us)  */
    600,                    /* Blanking formula gradient                */
    40,                     /* Blanking formula offset                  */
    128,                    /* Blanking formula scaling factor          */
    20,                     /* Blanking formula scaling factor weight   */
    };

/*-------------------------- Implementation -------------------------------*/

#ifdef __WIN32_VXD__
/* These functions are not supported in a VxD, so we stub them out so this
 * module will at least compile. Calling the functions in here will do
 * something wierd!
 */
double sqrt(double x)
{ return x; }

double floor(double x)
{ return x; }

double pow(double x,double y)
{ return x*y; }
#endif

static double round(double v)
{
    return floor(v + 0.5);
}

static void GetInternalConstants(GTF_constants *c)
/****************************************************************************
*
* Function:     GetInternalConstants
* Parameters:   c   - Place to store the internal constants
*
* Description:  Calculates the rounded, internal set of GTF constants.
*               These constants are different to the real GTF constants
*               that can be set up for the monitor. The calculations to
*               get these real constants are defined in the 'Work Area'
*               after the constants are defined in the Excel spreadsheet.
*
****************************************************************************/
{
    c->margin = GC.margin;
    c->cellGran = round(GC.cellGran);
    c->minPorch = round(GC.minPorch);
    c->vSyncRqd = round(GC.vSyncRqd);
    c->hSync = GC.hSync;
    c->minVSyncBP = GC.minVSyncBP;
    if (GC.k == 0)
	c->k = 0.001;
    else
	c->k = GC.k;
    c->m = (c->k / 256) * GC.m;
    c->c = (GC.c - GC.j) * (c->k / 256) + GC.j;
    c->j = GC.j;
}

void GTF_calcTimings(double hPixels,double vLines,double freq,
    int type,ibool wantMargins,ibool wantInterlace,GTF_timings *t)
/****************************************************************************
*
* Function:     GTF_calcTimings
* Parameters:   hPixels     - X resolution
*               vLines      - Y resolution
*               freq        - Frequency (Hz, KHz or MHz depending on type)
*               type        - 1 - vertical, 2 - horizontal, 3 - dot clock
*               margins     - True if margins should be generated
*               interlace   - True if interlaced timings to be generated
*               t           - Place to store the resulting timings
*
* Description:  Calculates a set of GTF timing parameters given a specified
*               resolution and vertical frequency. The horizontal frequency
*               and dot clock will be automatically generated by this
*               routines.
*
*               For interlaced modes the CRTC parameters are calculated for
*               a single field, so will be half what would be used in
*               a non-interlaced mode.
*
****************************************************************************/
{
    double          interlace,vFieldRate,hPeriod;
    double          topMarginLines,botMarginLines;
    double          leftMarginPixels,rightMarginPixels;
    double          hPeriodEst,vSyncBP,vBackPorch;
    double          vTotalLines,vFieldRateEst;
    double          hTotalPixels,hTotalActivePixels,hBlankPixels;
    double          idealDutyCycle,hSyncWidth,hSyncBP,hBackPorch;
    double          idealHPeriod;
    double          vFreq,hFreq,dotClock;
    GTF_constants   c;

    /* Get rounded GTF constants used for internal calculations */
    GetInternalConstants(&c);

    /* Move input parameters into appropriate variables */
    vFreq = hFreq = dotClock = freq;

    /* Round pixels to character cell granularity */
    hPixels = round(hPixels / c.cellGran) * c.cellGran;

    /* For interlaced mode halve the vertical parameters, and double
     * the required field refresh rate.
     */
    vFieldRate = vFreq;
    interlace = 0;
    if (wantInterlace)
	dotClock *= 2;

    /* Determine the lines for margins */
    if (wantMargins) {
	topMarginLines = round(c.margin / 100 * vLines);
	botMarginLines = round(c.margin / 100 * vLines);
	}
    else {
	topMarginLines = 0;
	botMarginLines = 0;
	}

    if (type != GTF_lockPF) {
	if (type == GTF_lockVF) {
	    /* Estimate the horizontal period */
	    hPeriodEst = ((1/vFieldRate) - (c.minVSyncBP/1000000)) /
		(vLines + (2*topMarginLines) + c.minPorch + interlace) * 1000000;

	    /* Find the number of lines in vSync + back porch */
	    vSyncBP = round(c.minVSyncBP / hPeriodEst);
	    }
	else if (type == GTF_lockHF) {
	    /* Find the number of lines in vSync + back porch */
	    vSyncBP = round((c.minVSyncBP * hFreq) / 1000);
	    }

	/* Find the number of lines in the V back porch alone */
	vBackPorch = vSyncBP - c.vSyncRqd;

	/* Find the total number of lines in the vertical period */
	vTotalLines = vLines + topMarginLines + botMarginLines + vSyncBP
	    + interlace + c.minPorch;

	if (type == GTF_lockVF) {
	    /* Estimate the vertical frequency */
	    vFieldRateEst = 1000000 / (hPeriodEst * vTotalLines);

	    /* Find the actual horizontal period */
	    hPeriod = (hPeriodEst * vFieldRateEst) / vFieldRate;

	    /* Find the actual vertical field frequency */
	    vFieldRate = 1000000 / (hPeriod * vTotalLines);
	    }
	else if (type == GTF_lockHF) {
	    /* Find the actual vertical field frequency */
	    vFieldRate = (hFreq / vTotalLines) * 1000;
	    }
	}

    /* Find the number of pixels in the left and right margins */
    if (wantMargins) {
	leftMarginPixels = round(hPixels * c.margin) / (100 * c.cellGran);
	rightMarginPixels = round(hPixels * c.margin) / (100 * c.cellGran);
	}
    else {
	leftMarginPixels = 0;
	rightMarginPixels = 0;
	}

    /* Find the total number of active pixels in image + margins */
    hTotalActivePixels = hPixels + leftMarginPixels + rightMarginPixels;

    if (type == GTF_lockVF) {
	/* Find the ideal blanking duty cycle */
	idealDutyCycle = c.c - ((c.m * hPeriod) / 1000);
	}
    else if (type == GTF_lockHF) {
	/* Find the ideal blanking duty cycle */
	idealDutyCycle = c.c - (c.m / hFreq);
	}
    else if (type == GTF_lockPF) {
	/* Find ideal horizontal period from blanking duty cycle formula */
	idealHPeriod = (((c.c - 100) + (sqrt((pow(100-c.c,2)) +
	    (0.4 * c.m * (hTotalActivePixels + rightMarginPixels +
	    leftMarginPixels) / dotClock)))) / (2 * c.m)) * 1000;

	/* Find the ideal blanking duty cycle */
	idealDutyCycle = c.c - ((c.m * idealHPeriod) / 1000);
	}

    /* Find the number of pixels in blanking time */
    hBlankPixels = round((hTotalActivePixels * idealDutyCycle) /
	((100 - idealDutyCycle) * c.cellGran)) * c.cellGran;

    /* Find the total number of pixels */
    hTotalPixels = hTotalActivePixels + hBlankPixels;

    /* Find the horizontal back porch */
    hBackPorch = round((hBlankPixels / 2) / c.cellGran) * c.cellGran;

    /* Find the horizontal sync width */
    hSyncWidth = round(((c.hSync/100) * hTotalPixels) / c.cellGran) * c.cellGran;

    /* Find the horizontal sync + back porch */
    hSyncBP = hBackPorch + hSyncWidth;

    if (type == GTF_lockPF) {
	/* Find the horizontal frequency */
	hFreq = (dotClock / hTotalPixels) * 1000;

	/* Find the number of lines in vSync + back porch */
	vSyncBP = round((c.minVSyncBP * hFreq) / 1000);

	/* Find the number of lines in the V back porch alone */
	vBackPorch = vSyncBP - c.vSyncRqd;

	/* Find the total number of lines in the vertical period */
	vTotalLines = vLines + topMarginLines + botMarginLines + vSyncBP
	    + interlace + c.minPorch;

	/* Find the actual vertical field frequency */
	vFieldRate = (hFreq / vTotalLines) * 1000;
	}
    else {
	if (type == GTF_lockVF) {
	    /* Find the horizontal frequency */
	    hFreq = 1000 / hPeriod;
	    }
	else if (type == GTF_lockHF) {
	    /* Find the horizontal frequency */
	    hPeriod = 1000 / hFreq;
	    }

	/* Find the pixel clock frequency */
	dotClock = hTotalPixels / hPeriod;
	}

    /* Return the computed frequencies */
    t->vFreq = vFieldRate;
    t->hFreq = hFreq;
    t->dotClock = dotClock;

    /* Determine the vertical timing parameters */
    t->h.hTotal = (int)hTotalPixels;
    t->h.hDisp = (int)hTotalActivePixels;
    t->h.hSyncStart = t->h.hTotal - (int)hSyncBP;
    t->h.hSyncEnd = t->h.hTotal - (int)hBackPorch;
    t->h.hFrontPorch = t->h.hSyncStart - t->h.hDisp;
    t->h.hSyncWidth = (int)hSyncWidth;
    t->h.hBackPorch = (int)hBackPorch;

    /* Determine the vertical timing parameters */
    t->v.vTotal = (int)vTotalLines;
    t->v.vDisp = (int)vLines;
    t->v.vSyncStart = t->v.vTotal - (int)vSyncBP;
    t->v.vSyncEnd = t->v.vTotal - (int)vBackPorch;
    t->v.vFrontPorch = t->v.vSyncStart - t->v.vDisp;
    t->v.vSyncWidth = (int)c.vSyncRqd;
    t->v.vBackPorch = (int)vBackPorch;
    if (wantInterlace) {
	/* Halve the timings for interlaced modes */
	t->v.vTotal /= 2;
	t->v.vDisp /= 2;
	t->v.vSyncStart /= 2;
	t->v.vSyncEnd /= 2;
	t->v.vFrontPorch /= 2;
	t->v.vSyncWidth /= 2;
	t->v.vBackPorch /= 2;
	t->dotClock /= 2;
	}

    /* Mark as GTF timing using the sync polarities */
    t->interlace = (wantInterlace) ? 'I' : 'N';
    t->hSyncPol = '-';
    t->vSyncPol = '+';
}

void GTF_getConstants(GTF_constants *constants)
{ *constants = GC; }

void GTF_setConstants(GTF_constants *constants)
{ GC = *constants; }

#ifdef  TESTING_GTF

void main(int argc,char *argv[])
{
    FILE        *f;
    double      xPixels,yPixels,freq;
    ibool       interlace;
    GTF_timings t;

    if (argc != 5 && argc != 6) {
	printf("Usage: GTFCALC <xPixels> <yPixels> <freq> [[Hz] [KHz] [MHz]] [I]\n");
	printf("\n");
	printf("where <xPixels> is the horizontal resolution of the mode, <yPixels> is the\n");
	printf("vertical resolution of the mode. The <freq> value will be the frequency to\n");
	printf("drive the calculations, and will be either the vertical frequency (in Hz)\n");
	printf("the horizontal frequency (in KHz) or the dot clock (in MHz). To generate\n");
	printf("timings for an interlaced mode, add 'I' to the end of the command line.\n");
	printf("\n");
	printf("For example to generate timings for 640x480 at 60Hz vertical:\n");
	printf("\n");
	printf("    GTFCALC 640 480 60 Hz\n");
	printf("\n");
	printf("For example to generate timings for 640x480 at 31.5KHz horizontal:\n");
	printf("\n");
	printf("    GTFCALC 640 480 31.5 KHz\n");
	printf("\n");
	printf("For example to generate timings for 640x480 with a 25.175Mhz dot clock:\n");
	printf("\n");
	printf("    GTFCALC 640 480 25.175 MHz\n");
	printf("\n");
	printf("GTFCALC will print a summary of the results found, and dump the CRTC\n");
	printf("values to the UVCONFIG.CRT file in the format used by SciTech Display Doctor.\n");
	exit(1);
	}

    /* Get values from command line */
    xPixels = atof(argv[1]);
    yPixels = atof(argv[2]);
    freq = atof(argv[3]);
    interlace = ((argc == 6) && (argv[5][0] == 'I'));

    /* Compute the CRTC timings */
    if (toupper(argv[4][0]) == 'H')
	GTF_calcTimings(xPixels,yPixels,freq,GTF_lockVF,false,interlace,&t);
    else if (toupper(argv[4][0]) == 'K')
	GTF_calcTimings(xPixels,yPixels,freq,GTF_lockHF,false,interlace,&t);
    else if (toupper(argv[4][0]) == 'M')
	GTF_calcTimings(xPixels,yPixels,freq,GTF_lockPF,false,interlace,&t);
    else {
	printf("Unknown command line!\n");
	exit(1);
	}

    /* Dump summary info to standard output */
    printf("CRTC values for %.0fx%.0f @ %.2f %s\n", xPixels, yPixels, freq, argv[4]);
    printf("\n");
    printf("  hTotal      = %-4d    vTotal      = %-4d\n",
	t.h.hTotal, t.v.vTotal);
    printf("  hDisp       = %-4d    vDisp       = %-4d\n",
	t.h.hDisp, t.v.vDisp);
    printf("  hSyncStart  = %-4d    vSyncStart  = %-4d\n",
	t.h.hSyncStart, t.v.vSyncStart);
    printf("  hSyncEnd    = %-4d    vSyncEnd    = %-4d\n",
	t.h.hSyncEnd, t.v.vSyncEnd);
    printf("  hFrontPorch = %-4d    vFrontPorch = %-4d\n",
	t.h.hFrontPorch, t.v.vFrontPorch);
    printf("  hSyncWidth  = %-4d    vSyncWidth  = %-4d\n",
	t.h.hSyncWidth, t.v.vSyncWidth);
    printf("  hBackPorch  = %-4d    vBackPorch  = %-4d\n",
	t.h.hBackPorch, t.v.vBackPorch);
    printf("\n");
    printf("  Interlaced  = %s\n", (t.interlace == 'I') ? "Yes" : "No");
    printf("  H sync pol  = %c\n", t.hSyncPol);
    printf("  V sync pol  = %c\n", t.vSyncPol);
    printf("\n");
    printf("  Vert freq   = %.2f Hz\n", t.vFreq);
    printf("  Horiz freq  = %.2f KHz\n", t.hFreq);
    printf("  Dot Clock   = %.2f Mhz\n",    t.dotClock);

    /* Dump to file in format used by SciTech Display Doctor */
    if ((f = fopen("UVCONFIG.CRT","w")) != NULL) {
	fprintf(f, "[%.0f %.0f]\n", xPixels, yPixels);
	fprintf(f, "%d %d %d %d '%c' %s\n",
	    t.h.hTotal, t.h.hDisp,
	    t.h.hSyncStart, t.h.hSyncEnd,
	    t.hSyncPol, (t.interlace == 'I') ? "I" : "NI");
	fprintf(f, "%d %d %d %d '%c'\n",
	    t.v.vTotal, t.v.vDisp,
	    t.v.vSyncStart, t.v.vSyncEnd,
	    t.vSyncPol);
	fprintf(f, "%.2f\n", t.dotClock);
	fclose(f);
	}
}

#endif  /* TESTING */
