/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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

/*************************************************************************
 * adaption for the Marvell DB64360 Board
 * Ingo Assmus (ingo.assmus@keymile.com)
 *
 * adaption for the cpci750 Board
 * Reinhard Arlt (reinhard.arlt@esd-electronics.com)
 *************************************************************************/


/* sdram_init.c - automatic memory sizing */

#include <common.h>
#include <74xx_7xx.h>
#include "../../Marvell/include/memory.h"
#include "../../Marvell/include/pci.h"
#include "../../Marvell/include/mv_gen_reg.h"
#include <net.h>

#include "eth.h"
#include "mpsc.h"
#include "../../Marvell/common/i2c.h"
#include "64360.h"
#include "mv_regs.h"


#undef	DEBUG
/* #define DEBUG */
#ifdef CONFIG_PCI
#define	MAP_PCI
#endif /* of CONFIG_PCI */

#ifdef DEBUG
#define DP(x) x
#else
#define DP(x)
#endif

int set_dfcdlInit(void);	/* setup delay line of Mv64360 */

/* ------------------------------------------------------------------------- */

int
memory_map_bank(unsigned int bankNo,
		unsigned int bankBase,
		unsigned int bankLength)
{
#ifdef MAP_PCI
	PCI_HOST host;
#endif


#ifdef DEBUG
	if (bankLength > 0) {
		printf("mapping bank %d at %08x - %08x\n",
		       bankNo, bankBase, bankBase + bankLength - 1);
	} else {
		printf("unmapping bank %d\n", bankNo);
	}
#endif

	memoryMapBank(bankNo, bankBase, bankLength);

#ifdef MAP_PCI
	for (host=PCI_HOST0;host<=PCI_HOST1;host++) {
		const int features=
			PREFETCH_ENABLE |
			DELAYED_READ_ENABLE |
			AGGRESSIVE_PREFETCH |
			READ_LINE_AGGRESSIVE_PREFETCH |
			READ_MULTI_AGGRESSIVE_PREFETCH |
			MAX_BURST_4 |
			PCI_NO_SWAP;

		pciMapMemoryBank(host, bankNo, bankBase, bankLength);

		pciSetRegionSnoopMode(host, bankNo, PCI_SNOOP_WB, bankBase,
				bankLength);

		pciSetRegionFeatures(host, bankNo, features, bankBase, bankLength);
	}
#endif
	return 0;
}

#define GB         (1 << 30)

/* much of this code is based on (or is) the code in the pip405 port */
/* thanks go to the authors of said port - Josh */

/* structure to store the relevant information about an sdram bank */
typedef struct sdram_info {
	uchar drb_size;
	uchar registered, ecc;
	uchar tpar;
	uchar tras_clocks;
	uchar burst_len;
	uchar banks, slot;
} sdram_info_t;

/* Typedefs for 'gtAuxilGetDIMMinfo' function */

typedef enum _memoryType {SDRAM, DDR} MEMORY_TYPE;

typedef enum _voltageInterface {TTL_5V_TOLERANT, LVTTL, HSTL_1_5V,
				SSTL_3_3V, SSTL_2_5V, VOLTAGE_UNKNOWN,
			       } VOLTAGE_INTERFACE;

typedef enum _max_CL_supported_DDR {DDR_CL_1=1, DDR_CL_1_5=2, DDR_CL_2=4, DDR_CL_2_5=8, DDR_CL_3=16, DDR_CL_3_5=32, DDR_CL_FAULT} MAX_CL_SUPPORTED_DDR;
typedef enum _max_CL_supported_SD {SD_CL_1=1,  SD_CL_2,  SD_CL_3, SD_CL_4, SD_CL_5, SD_CL_6, SD_CL_7, SD_FAULT} MAX_CL_SUPPORTED_SD;


/* SDRAM/DDR information struct */
typedef struct _gtMemoryDimmInfo
{
    MEMORY_TYPE          memoryType;
    unsigned int         numOfRowAddresses;
    unsigned int         numOfColAddresses;
    unsigned int         numOfModuleBanks;
    unsigned int         dataWidth;
    VOLTAGE_INTERFACE    voltageInterface;
    unsigned int         errorCheckType;                                /* ECC , PARITY..*/
    unsigned int         sdramWidth;                                    /* 4,8,16 or 32 */;
    unsigned int         errorCheckDataWidth;                           /* 0 - no, 1 - Yes */
    unsigned int         minClkDelay;
    unsigned int         burstLengthSupported;
    unsigned int         numOfBanksOnEachDevice;
    unsigned int         suportedCasLatencies;
    unsigned int     	 RefreshInterval;
    unsigned int   	 maxCASlatencySupported_LoP;			/* LoP left of point (measured in ns) */
    unsigned int   	 maxCASlatencySupported_RoP;			/* RoP right of point (measured in ns)*/
    MAX_CL_SUPPORTED_DDR maxClSupported_DDR;
    MAX_CL_SUPPORTED_SD  maxClSupported_SD;
    unsigned int         moduleBankDensity;
    /* module attributes (true for yes) */
    bool                 bufferedAddrAndControlInputs;
    bool                 registeredAddrAndControlInputs;
    bool                 onCardPLL;
    bool                 bufferedDQMBinputs;
    bool                 registeredDQMBinputs;
    bool	         differentialClockInput;
    bool                 redundantRowAddressing;

    /* module general attributes */
    bool                 suportedAutoPreCharge;
    bool                 suportedPreChargeAll;
    bool                 suportedEarlyRasPreCharge;
    bool                 suportedWrite1ReadBurst;
    bool                 suported5PercentLowVCC;
    bool                 suported5PercentUpperVCC;
    /* module timing parameters */
    unsigned int         minRasToCasDelay;
    unsigned int         minRowActiveRowActiveDelay;
    unsigned int         minRasPulseWidth;
    unsigned int         minRowPrechargeTime;   			/* measured in ns */

    int   	         addrAndCommandHoldTime;			/* LoP left of point (measured in ns) */
    int   	         addrAndCommandSetupTime;				/* (measured in ns/100) */
    int   	         dataInputSetupTime;				/* LoP left of point (measured in ns) */
    int   	         dataInputHoldTime;				/* LoP left of point (measured in ns) */
/* tAC times for highest 2nd and 3rd highest CAS Latency values */
    unsigned int   	 clockToDataOut_LoP;				/* LoP left of point (measured in ns) */
    unsigned int   	 clockToDataOut_RoP;				/* RoP right of point (measured in ns)*/
    unsigned int   	 clockToDataOutMinus1_LoP;				/* LoP left of point (measured in ns) */
    unsigned int   	 clockToDataOutMinus1_RoP;			/* RoP right of point (measured in ns)*/
    unsigned int   	 clockToDataOutMinus2_LoP;				/* LoP left of point (measured in ns) */
    unsigned int   	 clockToDataOutMinus2_RoP;			/* RoP right of point (measured in ns)*/

    unsigned int   	 minimumCycleTimeAtMaxCasLatancy_LoP;		/* LoP left of point (measured in ns) */
    unsigned int   	 minimumCycleTimeAtMaxCasLatancy_RoP;		/* RoP right of point (measured in ns)*/

    unsigned int   	 minimumCycleTimeAtMaxCasLatancyMinus1_LoP;	/* LoP left of point (measured in ns) */
    unsigned int   	 minimumCycleTimeAtMaxCasLatancyMinus1_RoP;	/* RoP right of point (measured in ns)*/

    unsigned int   	 minimumCycleTimeAtMaxCasLatancyMinus2_LoP;	/* LoP left of point (measured in ns) */
    unsigned int   	 minimumCycleTimeAtMaxCasLatancyMinus2_RoP;	/* RoP right of point (measured in ns)*/

    /* Parameters calculated from
       the extracted DIMM information */
    unsigned int         size;
    unsigned int         deviceDensity;           		       	/* 16,64,128,256 or 512 Mbit */
    unsigned int         numberOfDevices;
    uchar 		 drb_size;				       	/* DRAM size in n*64Mbit */
    uchar 		 slot;						/* Slot Number this module is inserted in */
    uchar 		 spd_raw_data[128];			       	/* Content of SPD-EEPROM copied 1:1 */
#ifdef DEBUG
    uchar 		 manufactura[8];				/* Content of SPD-EEPROM Byte 64-71 */
    uchar 		 modul_id[18];					/* Content of SPD-EEPROM Byte 73-90 */
    uchar 		 vendor_data[27];			       	/* Content of SPD-EEPROM Byte 99-125 */
    unsigned long	 modul_serial_no;			       	/* Content of SPD-EEPROM Byte 95-98 */
    unsigned int         manufac_date;					/* Content of SPD-EEPROM Byte 93-94 */
    unsigned int         modul_revision;				/* Content of SPD-EEPROM Byte 91-92 */
    uchar 		 manufac_place;					/* Content of SPD-EEPROM Byte 72 */

#endif
} AUX_MEM_DIMM_INFO;


/*
 * translate ns.ns/10 coding of SPD timing values
 * into 10 ps unit values
 */
static inline unsigned short
NS10to10PS(unsigned char spd_byte)
{
	unsigned short ns, ns10;

	/* isolate upper nibble */
	ns = (spd_byte >> 4) & 0x0F;
	/* isolate lower nibble */
	ns10 = (spd_byte & 0x0F);

	return(ns*100 + ns10*10);
}

/*
 * translate ns coding of SPD timing values
 * into 10 ps unit values
 */
static inline unsigned short
NSto10PS(unsigned char spd_byte)
{
	return(spd_byte*100);
}

/* This code reads the SPD chip on the sdram and populates
 * the array which is passed in with the relevant information */
/* static int check_dimm(uchar slot, AUX_MEM_DIMM_INFO *info) */
static int check_dimm (uchar slot, AUX_MEM_DIMM_INFO * dimmInfo)
{
	DECLARE_GLOBAL_DATA_PTR;

	unsigned long spd_checksum;

	uchar addr = slot == 0 ? DIMM0_I2C_ADDR : DIMM1_I2C_ADDR;
	int ret;
	unsigned int i, j, density = 1, devicesForErrCheck = 0;

#ifdef DEBUG
	unsigned int k;
#endif
	unsigned int rightOfPoint = 0, leftOfPoint = 0, mult, div, time_tmp;
	int sign = 1, shift, maskLeftOfPoint, maskRightOfPoint;
	uchar supp_cal, cal_val;
	ulong memclk, tmemclk;
	ulong tmp;
	uchar trp_clocks = 0, trcd_clocks, tras_clocks, trrd_clocks;
	uchar data[128];

	memclk = gd->bus_clk;
	tmemclk = 1000000000 / (memclk / 100);	/* in 10 ps units */

	memset (data, 0, sizeof (data));


	ret = 0;

	DP (puts ("before i2c read\n"));

	ret = i2c_read (addr, 0, 2, data, 128);

	DP (puts ("after i2c read\n"));

	if ((data[64] != 'e') || (data[65] != 's') || (data[66] != 'd')
	    || (data[67] != '-') || (data[68] != 'g') || (data[69] != 'm')
	    || (data[70] != 'b') || (data[71] != 'h')) {
		ret = -1;
	}

	if ((ret != 0) && (slot == 0)) {
		memset (data, 0, sizeof (data));
		data[0] = 0x80;
		data[1] = 0x08;
		data[2] = 0x07;
		data[3] = 0x0c;
		data[4] = 0x09;
		data[5] = 0x01;
		data[6] = 0x48;
		data[7] = 0x00;
		data[8] = 0x04;
		data[9] = 0x75;
		data[10] = 0x80;
		data[11] = 0x02;
		data[12] = 0x80;
		data[13] = 0x10;
		data[14] = 0x08;
		data[15] = 0x01;
		data[16] = 0x0e;
		data[17] = 0x04;
		data[18] = 0x0c;
		data[19] = 0x01;
		data[20] = 0x02;
		data[21] = 0x20;
		data[22] = 0x00;
		data[23] = 0xa0;
		data[24] = 0x80;
		data[25] = 0x00;
		data[26] = 0x00;
		data[27] = 0x50;
		data[28] = 0x3c;
		data[29] = 0x50;
		data[30] = 0x32;
		data[31] = 0x10;
		data[32] = 0xb0;
		data[33] = 0xb0;
		data[34] = 0x60;
		data[35] = 0x60;
		data[64] = 'e';
		data[65] = 's';
		data[66] = 'd';
		data[67] = '-';
		data[68] = 'g';
		data[69] = 'm';
		data[70] = 'b';
		data[71] = 'h';
		ret = 0;
	}

	/* zero all the values */
	memset (dimmInfo, 0, sizeof (*dimmInfo));

	/* copy the SPD content 1:1 into the dimmInfo structure */
	for (i = 0; i <= 127; i++) {
		dimmInfo->spd_raw_data[i] = data[i];
	}

	if (ret) {
		DP (printf ("No DIMM in slot %d [err = %x]\n", slot, ret));
		return 0;
	} else
		dimmInfo->slot = slot;	/* start to fill up dimminfo for this "slot" */

#ifdef CFG_DISPLAY_DIMM_SPD_CONTENT

	for (i = 0; i <= 127; i++) {
		printf ("SPD-EEPROM Byte %3d = %3x (%3d)\n", i, data[i],
			data[i]);
	}

#endif
#ifdef DEBUG
	/* find Manufacturer of Dimm Module */
	for (i = 0; i < sizeof (dimmInfo->manufactura); i++) {
		dimmInfo->manufactura[i] = data[64 + i];
	}
	printf ("\nThis RAM-Module is produced by: 		%s\n",
		dimmInfo->manufactura);

	/* find Manul-ID of Dimm Module */
	for (i = 0; i < sizeof (dimmInfo->modul_id); i++) {
		dimmInfo->modul_id[i] = data[73 + i];
	}
	printf ("The Module-ID of this RAM-Module is: 		%s\n",
		dimmInfo->modul_id);

	/* find Vendor-Data of Dimm Module */
	for (i = 0; i < sizeof (dimmInfo->vendor_data); i++) {
		dimmInfo->vendor_data[i] = data[99 + i];
	}
	printf ("Vendor Data of this RAM-Module is: 		%s\n",
		dimmInfo->vendor_data);

	/* find modul_serial_no of Dimm Module */
	dimmInfo->modul_serial_no = (*((unsigned long *) (&data[95])));
	printf ("Serial No. of this RAM-Module is: 		%ld (%lx)\n",
		dimmInfo->modul_serial_no, dimmInfo->modul_serial_no);

	/* find Manufac-Data of Dimm Module */
	dimmInfo->manufac_date = (*((unsigned int *) (&data[93])));
	printf ("Manufactoring Date of this RAM-Module is: 	%d.%d\n", data[93], data[94]);	/*dimmInfo->manufac_date */

	/* find modul_revision of Dimm Module */
	dimmInfo->modul_revision = (*((unsigned int *) (&data[91])));
	printf ("Module Revision of this RAM-Module is: 		%d.%d\n", data[91], data[92]);	/* dimmInfo->modul_revision */

	/* find manufac_place of Dimm Module */
	dimmInfo->manufac_place = (*((unsigned char *) (&data[72])));
	printf ("manufac_place of this RAM-Module is: 		%d\n",
		dimmInfo->manufac_place);

#endif
/*------------------------------------------------------------------------------------------------------------------------------*/
/* calculate SPD checksum */
/*------------------------------------------------------------------------------------------------------------------------------*/
	spd_checksum = 0;
#if 0				/* test-only */
	for (i = 0; i <= 62; i++) {
		spd_checksum += data[i];
	}

	if ((spd_checksum & 0xff) != data[63]) {
		printf ("### Error in SPD Checksum !!! Is_value: %2x should value %2x\n", (unsigned int) (spd_checksum & 0xff), data[63]);
		hang ();
	}

	else
		printf ("SPD Checksum ok!\n");
#endif /* test-only */

/*------------------------------------------------------------------------------------------------------------------------------*/
	for (i = 2; i <= 35; i++) {
		switch (i) {
		case 2:	/* Memory type (DDR / SDRAM) */
			dimmInfo->memoryType = (data[i] == 0x7) ? DDR : SDRAM;
#ifdef DEBUG
			if (dimmInfo->memoryType == 0)
				DP (printf
				    ("Dram_type in slot %d is: 			SDRAM\n",
				     dimmInfo->slot));
			if (dimmInfo->memoryType == 1)
				DP (printf
				    ("Dram_type in slot %d is: 			DDRAM\n",
				     dimmInfo->slot));
#endif
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 3:	/* Number Of Row Addresses */
			dimmInfo->numOfRowAddresses = data[i];
			DP (printf
			    ("Module Number of row addresses: 		%d\n",
			     dimmInfo->numOfRowAddresses));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 4:	/* Number Of Column Addresses */
			dimmInfo->numOfColAddresses = data[i];
			DP (printf
			    ("Module Number of col addresses: 		%d\n",
			     dimmInfo->numOfColAddresses));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 5:	/* Number Of Module Banks */
			dimmInfo->numOfModuleBanks = data[i];
			DP (printf
			    ("Number of Banks on Mod. : 				%d\n",
			     dimmInfo->numOfModuleBanks));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 6:	/* Data Width */
			dimmInfo->dataWidth = data[i];
			DP (printf
			    ("Module Data Width: 				%d\n",
			     dimmInfo->dataWidth));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 8:	/* Voltage Interface */
			switch (data[i]) {
			case 0x0:
				dimmInfo->voltageInterface = TTL_5V_TOLERANT;
				DP (printf
				    ("Module is 					TTL_5V_TOLERANT\n"));
				break;
			case 0x1:
				dimmInfo->voltageInterface = LVTTL;
				DP (printf
				    ("Module is 					LVTTL\n"));
				break;
			case 0x2:
				dimmInfo->voltageInterface = HSTL_1_5V;
				DP (printf
				    ("Module is 					TTL_5V_TOLERANT\n"));
				break;
			case 0x3:
				dimmInfo->voltageInterface = SSTL_3_3V;
				DP (printf
				    ("Module is 					HSTL_1_5V\n"));
				break;
			case 0x4:
				dimmInfo->voltageInterface = SSTL_2_5V;
				DP (printf
				    ("Module is 					SSTL_2_5V\n"));
				break;
			default:
				dimmInfo->voltageInterface = VOLTAGE_UNKNOWN;
				DP (printf
				    ("Module is 					VOLTAGE_UNKNOWN\n"));
				break;
			}
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 9:	/* Minimum Cycle Time At Max CasLatancy */
			shift = (dimmInfo->memoryType == DDR) ? 4 : 2;
			mult = (dimmInfo->memoryType == DDR) ? 10 : 25;
			maskLeftOfPoint =
				(dimmInfo->memoryType == DDR) ? 0xf0 : 0xfc;
			maskRightOfPoint =
				(dimmInfo->memoryType == DDR) ? 0xf : 0x03;
			leftOfPoint = (data[i] & maskLeftOfPoint) >> shift;
			rightOfPoint = (data[i] & maskRightOfPoint) * mult;
			dimmInfo->minimumCycleTimeAtMaxCasLatancy_LoP =
				leftOfPoint;
			dimmInfo->minimumCycleTimeAtMaxCasLatancy_RoP =
				rightOfPoint;
			DP (printf
			    ("Minimum Cycle Time At Max CasLatancy: 		%d.%d [ns]\n",
			     leftOfPoint, rightOfPoint));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 10:	/* Clock To Data Out */
			div = (dimmInfo->memoryType == DDR) ? 100 : 10;
			time_tmp =
				(((data[i] & 0xf0) >> 4) * 10) +
				((data[i] & 0x0f));
			leftOfPoint = time_tmp / div;
			rightOfPoint = time_tmp % div;
			dimmInfo->clockToDataOut_LoP = leftOfPoint;
			dimmInfo->clockToDataOut_RoP = rightOfPoint;
			DP (printf
			    ("Clock To Data Out: 				%d.%2d [ns]\n",
			     leftOfPoint, rightOfPoint));
			/*dimmInfo->clockToDataOut */
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

#ifdef CONFIG_ECC
		case 11:	/* Error Check Type */
			dimmInfo->errorCheckType = data[i];
			DP (printf
			    ("Error Check Type (0=NONE): 			%d\n",
			     dimmInfo->errorCheckType));
			break;
#endif
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 12:	/* Refresh Interval */
			dimmInfo->RefreshInterval = data[i];
			DP (printf
			    ("RefreshInterval (80= Self refresh Normal, 15.625us) : %x\n",
			     dimmInfo->RefreshInterval));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 13:	/* Sdram Width */
			dimmInfo->sdramWidth = data[i];
			DP (printf
			    ("Sdram Width: 					%d\n",
			     dimmInfo->sdramWidth));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 14:	/* Error Check Data Width */
			dimmInfo->errorCheckDataWidth = data[i];
			DP (printf
			    ("Error Check Data Width: 			%d\n",
			     dimmInfo->errorCheckDataWidth));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 15:	/* Minimum Clock Delay */
			dimmInfo->minClkDelay = data[i];
			DP (printf
			    ("Minimum Clock Delay: 				%d\n",
			     dimmInfo->minClkDelay));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 16:	/* Burst Length Supported */
			   /******-******-******-*******
			   * bit3 | bit2 | bit1 | bit0 *
			   *******-******-******-*******
	    burst length = *  8   |  4   |   2  |   1  *
			   *****************************

	    If for example bit0 and bit2 are set, the burst
	    length supported are 1 and 4. */

			dimmInfo->burstLengthSupported = data[i];
#ifdef DEBUG
			DP (printf
			    ("Burst Length Supported: 			"));
			if (dimmInfo->burstLengthSupported & 0x01)
				DP (printf ("1, "));
			if (dimmInfo->burstLengthSupported & 0x02)
				DP (printf ("2, "));
			if (dimmInfo->burstLengthSupported & 0x04)
				DP (printf ("4, "));
			if (dimmInfo->burstLengthSupported & 0x08)
				DP (printf ("8, "));
			DP (printf (" Bit \n"));
#endif
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 17:	/* Number Of Banks On Each Device */
			dimmInfo->numOfBanksOnEachDevice = data[i];
			DP (printf
			    ("Number Of Banks On Each Chip: 			%d\n",
			     dimmInfo->numOfBanksOnEachDevice));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 18:	/* Suported Cas Latencies */

			/*     DDR:
			 *******-******-******-******-******-******-******-*******
			 * bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0 *
			 *******-******-******-******-******-******-******-*******
			 CAS =   * TBD  | TBD  | 3.5  |   3  | 2.5  |  2   | 1.5  |   1  *
			 *********************************************************
			 SDRAM:
			 *******-******-******-******-******-******-******-*******
			 * bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0 *
			 *******-******-******-******-******-******-******-*******
			 CAS =   * TBD  |  7   |  6   |  5   |  4   |  3   |   2  |   1  *
			 ********************************************************/
			dimmInfo->suportedCasLatencies = data[i];
#ifdef DEBUG
			DP (printf
			    ("Suported Cas Latencies: (CL) 			"));
			if (dimmInfo->memoryType == 0) {	/* SDRAM */
				for (k = 0; k <= 7; k++) {
					if (dimmInfo->
					    suportedCasLatencies & (1 << k))
						DP (printf
						    ("%d, 			",
						     k + 1));
				}

			} else {	/* DDR-RAM */

				if (dimmInfo->suportedCasLatencies & 1)
					DP (printf ("1, "));
				if (dimmInfo->suportedCasLatencies & 2)
					DP (printf ("1.5, "));
				if (dimmInfo->suportedCasLatencies & 4)
					DP (printf ("2, "));
				if (dimmInfo->suportedCasLatencies & 8)
					DP (printf ("2.5, "));
				if (dimmInfo->suportedCasLatencies & 16)
					DP (printf ("3, "));
				if (dimmInfo->suportedCasLatencies & 32)
					DP (printf ("3.5, "));

			}
			DP (printf ("\n"));
#endif
			/* Calculating MAX CAS latency */
			for (j = 7; j > 0; j--) {
				if (((dimmInfo->
				      suportedCasLatencies >> j) & 0x1) ==
				    1) {
					switch (dimmInfo->memoryType) {
					case DDR:
						/* CAS latency 1, 1.5, 2, 2.5, 3, 3.5 */
						switch (j) {
						case 7:
							DP (printf
							    ("Max. Cas Latencies (DDR): 			ERROR !!!\n"));
							dimmInfo->
								maxClSupported_DDR
								=
								DDR_CL_FAULT;
							hang ();
							break;
						case 6:
							DP (printf
							    ("Max. Cas Latencies (DDR): 			ERROR !!!\n"));
							dimmInfo->
								maxClSupported_DDR
								=
								DDR_CL_FAULT;
							hang ();
							break;
						case 5:
							DP (printf
							    ("Max. Cas Latencies (DDR): 			3.5 clk's\n"));
							dimmInfo->
								maxClSupported_DDR
								= DDR_CL_3_5;
							break;
						case 4:
							DP (printf
							    ("Max. Cas Latencies (DDR): 			3 clk's \n"));
							dimmInfo->
								maxClSupported_DDR
								= DDR_CL_3;
							break;
						case 3:
							DP (printf
							    ("Max. Cas Latencies (DDR): 			2.5 clk's \n"));
							dimmInfo->
								maxClSupported_DDR
								= DDR_CL_2_5;
							break;
						case 2:
							DP (printf
							    ("Max. Cas Latencies (DDR): 			2 clk's \n"));
							dimmInfo->
								maxClSupported_DDR
								= DDR_CL_2;
							break;
						case 1:
							DP (printf
							    ("Max. Cas Latencies (DDR): 			1.5 clk's \n"));
							dimmInfo->
								maxClSupported_DDR
								= DDR_CL_1_5;
							break;
						}
						dimmInfo->
							maxCASlatencySupported_LoP
							=
							1 +
							(int) (5 * j / 10);
						if (((5 * j) % 10) != 0)
							dimmInfo->
								maxCASlatencySupported_RoP
								= 5;
						else
							dimmInfo->
								maxCASlatencySupported_RoP
								= 0;
						DP (printf
						    ("Max. Cas Latencies (DDR LoP.RoP Notation): 	%d.%d \n",
						     dimmInfo->
						     maxCASlatencySupported_LoP,
						     dimmInfo->
						     maxCASlatencySupported_RoP));
						break;
					case SDRAM:
						/* CAS latency 1, 2, 3, 4, 5, 6, 7 */
						dimmInfo->maxClSupported_SD = j;	/*  Cas Latency DDR-RAM Coded                   */
						DP (printf
						    ("Max. Cas Latencies (SD): %d\n",
						     dimmInfo->
						     maxClSupported_SD));
						dimmInfo->
							maxCASlatencySupported_LoP
							= j;
						dimmInfo->
							maxCASlatencySupported_RoP
							= 0;
						DP (printf
						    ("Max. Cas Latencies (DDR LoP.RoP Notation): %d.%d \n",
						     dimmInfo->
						     maxCASlatencySupported_LoP,
						     dimmInfo->
						     maxCASlatencySupported_RoP));
						break;
					}
					break;
				}
			}
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 21:	/* Buffered Address And Control Inputs */
			DP (printf ("\nModul Attributes (SPD Byte 21): \n"));
			dimmInfo->bufferedAddrAndControlInputs =
				data[i] & BIT0;
			dimmInfo->registeredAddrAndControlInputs =
				(data[i] & BIT1) >> 1;
			dimmInfo->onCardPLL = (data[i] & BIT2) >> 2;
			dimmInfo->bufferedDQMBinputs = (data[i] & BIT3) >> 3;
			dimmInfo->registeredDQMBinputs =
				(data[i] & BIT4) >> 4;
			dimmInfo->differentialClockInput =
				(data[i] & BIT5) >> 5;
			dimmInfo->redundantRowAddressing =
				(data[i] & BIT6) >> 6;
#ifdef DEBUG
			if (dimmInfo->bufferedAddrAndControlInputs == 1)
				DP (printf
				    (" - Buffered Address/Control Input:		Yes \n"));
			else
				DP (printf
				    (" - Buffered Address/Control Input:		No \n"));

			if (dimmInfo->registeredAddrAndControlInputs == 1)
				DP (printf
				    (" - Registered Address/Control Input:		Yes \n"));
			else
				DP (printf
				    (" - Registered Address/Control Input:		No \n"));

			if (dimmInfo->onCardPLL == 1)
				DP (printf
				    (" - On-Card PLL (clock):				Yes \n"));
			else
				DP (printf
				    (" - On-Card PLL (clock):				No \n"));

			if (dimmInfo->bufferedDQMBinputs == 1)
				DP (printf
				    (" - Bufferd DQMB Inputs:				Yes \n"));
			else
				DP (printf
				    (" - Bufferd DQMB Inputs:				No \n"));

			if (dimmInfo->registeredDQMBinputs == 1)
				DP (printf
				    (" - Registered DQMB Inputs:			Yes \n"));
			else
				DP (printf
				    (" - Registered DQMB Inputs:			No \n"));

			if (dimmInfo->differentialClockInput == 1)
				DP (printf
				    (" - Differential Clock Input:			Yes \n"));
			else
				DP (printf
				    (" - Differential Clock Input:			No \n"));

			if (dimmInfo->redundantRowAddressing == 1)
				DP (printf
				    (" - redundant Row Addressing:			Yes \n"));
			else
				DP (printf
				    (" - redundant Row Addressing:			No \n"));

#endif
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 22:	/* Suported AutoPreCharge */
			DP (printf ("\nModul Attributes (SPD Byte 22): \n"));
			dimmInfo->suportedEarlyRasPreCharge = data[i] & BIT0;
			dimmInfo->suportedAutoPreCharge =
				(data[i] & BIT1) >> 1;
			dimmInfo->suportedPreChargeAll =
				(data[i] & BIT2) >> 2;
			dimmInfo->suportedWrite1ReadBurst =
				(data[i] & BIT3) >> 3;
			dimmInfo->suported5PercentLowVCC =
				(data[i] & BIT4) >> 4;
			dimmInfo->suported5PercentUpperVCC =
				(data[i] & BIT5) >> 5;
#ifdef DEBUG
			if (dimmInfo->suportedEarlyRasPreCharge == 1)
				DP (printf
				    (" - Early Ras Precharge:			Yes \n"));
			else
				DP (printf
				    (" -  Early Ras Precharge:			No \n"));

			if (dimmInfo->suportedAutoPreCharge == 1)
				DP (printf
				    (" - AutoPreCharge:				Yes \n"));
			else
				DP (printf
				    (" -  AutoPreCharge:				No \n"));

			if (dimmInfo->suportedPreChargeAll == 1)
				DP (printf
				    (" - Precharge All:				Yes \n"));
			else
				DP (printf
				    (" -  Precharge All:				No \n"));

			if (dimmInfo->suportedWrite1ReadBurst == 1)
				DP (printf
				    (" - Write 1/ReadBurst:				Yes \n"));
			else
				DP (printf
				    (" -  Write 1/ReadBurst:				No \n"));

			if (dimmInfo->suported5PercentLowVCC == 1)
				DP (printf
				    (" - lower VCC tolerance:			5 Percent \n"));
			else
				DP (printf
				    ("  - lower VCC tolerance:			10 Percent \n"));

			if (dimmInfo->suported5PercentUpperVCC == 1)
				DP (printf
				    (" - upper VCC tolerance:			5 Percent \n"));
			else
				DP (printf
				    (" -  upper VCC tolerance:			10 Percent \n"));

#endif
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 23:	/* Minimum Cycle Time At Maximum Cas Latancy Minus 1 (2nd highest CL) */
			shift = (dimmInfo->memoryType == DDR) ? 4 : 2;
			mult = (dimmInfo->memoryType == DDR) ? 10 : 25;
			maskLeftOfPoint =
				(dimmInfo->memoryType == DDR) ? 0xf0 : 0xfc;
			maskRightOfPoint =
				(dimmInfo->memoryType == DDR) ? 0xf : 0x03;
			leftOfPoint = (data[i] & maskLeftOfPoint) >> shift;
			rightOfPoint = (data[i] & maskRightOfPoint) * mult;
			dimmInfo->minimumCycleTimeAtMaxCasLatancyMinus1_LoP =
				leftOfPoint;
			dimmInfo->minimumCycleTimeAtMaxCasLatancyMinus1_RoP =
				rightOfPoint;
			DP (printf
			    ("Minimum Cycle Time At 2nd highest CasLatancy (0 = Not supported): %d.%d [ns]\n",
			     leftOfPoint, rightOfPoint));
			/*dimmInfo->minimumCycleTimeAtMaxCasLatancy */
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 24:	/* Clock To Data Out 2nd highest Cas Latency Value */
			div = (dimmInfo->memoryType == DDR) ? 100 : 10;
			time_tmp =
				(((data[i] & 0xf0) >> 4) * 10) +
				((data[i] & 0x0f));
			leftOfPoint = time_tmp / div;
			rightOfPoint = time_tmp % div;
			dimmInfo->clockToDataOutMinus1_LoP = leftOfPoint;
			dimmInfo->clockToDataOutMinus1_RoP = rightOfPoint;
			DP (printf
			    ("Clock To Data Out (2nd CL value): 		%d.%2d [ns]\n",
			     leftOfPoint, rightOfPoint));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 25:	/* Minimum Cycle Time At Maximum Cas Latancy Minus 2 (3rd highest CL) */
			shift = (dimmInfo->memoryType == DDR) ? 4 : 2;
			mult = (dimmInfo->memoryType == DDR) ? 10 : 25;
			maskLeftOfPoint =
				(dimmInfo->memoryType == DDR) ? 0xf0 : 0xfc;
			maskRightOfPoint =
				(dimmInfo->memoryType == DDR) ? 0xf : 0x03;
			leftOfPoint = (data[i] & maskLeftOfPoint) >> shift;
			rightOfPoint = (data[i] & maskRightOfPoint) * mult;
			dimmInfo->minimumCycleTimeAtMaxCasLatancyMinus2_LoP =
				leftOfPoint;
			dimmInfo->minimumCycleTimeAtMaxCasLatancyMinus2_RoP =
				rightOfPoint;
			DP (printf
			    ("Minimum Cycle Time At 3rd highest CasLatancy (0 = Not supported): %d.%d [ns]\n",
			     leftOfPoint, rightOfPoint));
			/*dimmInfo->minimumCycleTimeAtMaxCasLatancy */
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 26:	/* Clock To Data Out 3rd highest Cas Latency Value */
			div = (dimmInfo->memoryType == DDR) ? 100 : 10;
			time_tmp =
				(((data[i] & 0xf0) >> 4) * 10) +
				((data[i] & 0x0f));
			leftOfPoint = time_tmp / div;
			rightOfPoint = time_tmp % div;
			dimmInfo->clockToDataOutMinus2_LoP = leftOfPoint;
			dimmInfo->clockToDataOutMinus2_RoP = rightOfPoint;
			DP (printf
			    ("Clock To Data Out (3rd CL value): 		%d.%2d [ns]\n",
			     leftOfPoint, rightOfPoint));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 27:	/* Minimum Row Precharge Time */
			shift = (dimmInfo->memoryType == DDR) ? 2 : 0;
			maskLeftOfPoint =
				(dimmInfo->memoryType == DDR) ? 0xfc : 0xff;
			maskRightOfPoint =
				(dimmInfo->memoryType == DDR) ? 0x03 : 0x00;
			leftOfPoint = ((data[i] & maskLeftOfPoint) >> shift);
			rightOfPoint = (data[i] & maskRightOfPoint) * 25;

			dimmInfo->minRowPrechargeTime = ((leftOfPoint * 100) + rightOfPoint);	/* measured in n times 10ps Intervals */
			trp_clocks =
				(dimmInfo->minRowPrechargeTime +
				 (tmemclk - 1)) / tmemclk;
			DP (printf
			    ("*** 1 clock cycle = %ld  10ps intervalls = %ld.%ld ns****\n",
			     tmemclk, tmemclk / 100, tmemclk % 100));
			DP (printf
			    ("Minimum Row Precharge Time [ns]: 		%d.%2d = in Clk cycles %d\n",
			     leftOfPoint, rightOfPoint, trp_clocks));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 28:	/* Minimum Row Active to Row Active Time */
			shift = (dimmInfo->memoryType == DDR) ? 2 : 0;
			maskLeftOfPoint =
				(dimmInfo->memoryType == DDR) ? 0xfc : 0xff;
			maskRightOfPoint =
				(dimmInfo->memoryType == DDR) ? 0x03 : 0x00;
			leftOfPoint = ((data[i] & maskLeftOfPoint) >> shift);
			rightOfPoint = (data[i] & maskRightOfPoint) * 25;

			dimmInfo->minRowActiveRowActiveDelay = ((leftOfPoint * 100) + rightOfPoint);	/* measured in 100ns Intervals */
			trrd_clocks =
				(dimmInfo->minRowActiveRowActiveDelay +
				 (tmemclk - 1)) / tmemclk;
			DP (printf
			    ("Minimum Row Active -To- Row Active Delay [ns]: 	%d.%2d = in Clk cycles %d\n",
			     leftOfPoint, rightOfPoint, trp_clocks));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 29:	/* Minimum Ras-To-Cas Delay */
			shift = (dimmInfo->memoryType == DDR) ? 2 : 0;
			maskLeftOfPoint =
				(dimmInfo->memoryType == DDR) ? 0xfc : 0xff;
			maskRightOfPoint =
				(dimmInfo->memoryType == DDR) ? 0x03 : 0x00;
			leftOfPoint = ((data[i] & maskLeftOfPoint) >> shift);
			rightOfPoint = (data[i] & maskRightOfPoint) * 25;

			dimmInfo->minRowActiveRowActiveDelay = ((leftOfPoint * 100) + rightOfPoint);	/* measured in 100ns Intervals */
			trcd_clocks =
				(dimmInfo->minRowActiveRowActiveDelay +
				 (tmemclk - 1)) / tmemclk;
			DP (printf
			    ("Minimum Ras-To-Cas Delay [ns]: 			%d.%2d = in Clk cycles %d\n",
			     leftOfPoint, rightOfPoint, trp_clocks));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 30:	/* Minimum Ras Pulse Width */
			dimmInfo->minRasPulseWidth = data[i];
			tras_clocks =
				(NSto10PS (data[i]) +
				 (tmemclk - 1)) / tmemclk;
			DP (printf
			    ("Minimum Ras Pulse Width [ns]: 			%d = in Clk cycles %d\n",
			     dimmInfo->minRasPulseWidth, tras_clocks));

			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 31:	/* Module Bank Density */
			dimmInfo->moduleBankDensity = data[i];
			DP (printf
			    ("Module Bank Density: 				%d\n",
			     dimmInfo->moduleBankDensity));
#ifdef DEBUG
			DP (printf
			    ("*** Offered Densities (more than 1 = Multisize-Module): "));
			{
				if (dimmInfo->moduleBankDensity & 1)
					DP (printf ("4MB, "));
				if (dimmInfo->moduleBankDensity & 2)
					DP (printf ("8MB, "));
				if (dimmInfo->moduleBankDensity & 4)
					DP (printf ("16MB, "));
				if (dimmInfo->moduleBankDensity & 8)
					DP (printf ("32MB, "));
				if (dimmInfo->moduleBankDensity & 16)
					DP (printf ("64MB, "));
				if (dimmInfo->moduleBankDensity & 32)
					DP (printf ("128MB, "));
				if ((dimmInfo->moduleBankDensity & 64)
				    || (dimmInfo->moduleBankDensity & 128)) {
					DP (printf ("ERROR, "));
					hang ();
				}
			}
			DP (printf ("\n"));
#endif
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 32:	/* Address And Command Setup Time (measured in ns/1000) */
			sign = 1;
			switch (dimmInfo->memoryType) {
			case DDR:
				time_tmp =
					(((data[i] & 0xf0) >> 4) * 10) +
					((data[i] & 0x0f));
				leftOfPoint = time_tmp / 100;
				rightOfPoint = time_tmp % 100;
				break;
			case SDRAM:
				leftOfPoint = (data[i] & 0xf0) >> 4;
				if (leftOfPoint > 7) {
					leftOfPoint = data[i] & 0x70 >> 4;
					sign = -1;
				}
				rightOfPoint = (data[i] & 0x0f);
				break;
			}
			dimmInfo->addrAndCommandSetupTime =
				(leftOfPoint * 100 + rightOfPoint) * sign;
			DP (printf
			    ("Address And Command Setup Time [ns]: 		%d.%d\n",
			     sign * leftOfPoint, rightOfPoint));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 33:	/* Address And Command Hold Time */
			sign = 1;
			switch (dimmInfo->memoryType) {
			case DDR:
				time_tmp =
					(((data[i] & 0xf0) >> 4) * 10) +
					((data[i] & 0x0f));
				leftOfPoint = time_tmp / 100;
				rightOfPoint = time_tmp % 100;
				break;
			case SDRAM:
				leftOfPoint = (data[i] & 0xf0) >> 4;
				if (leftOfPoint > 7) {
					leftOfPoint = data[i] & 0x70 >> 4;
					sign = -1;
				}
				rightOfPoint = (data[i] & 0x0f);
				break;
			}
			dimmInfo->addrAndCommandHoldTime =
				(leftOfPoint * 100 + rightOfPoint) * sign;
			DP (printf
			    ("Address And Command Hold Time [ns]: 		%d.%d\n",
			     sign * leftOfPoint, rightOfPoint));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 34:	/* Data Input Setup Time */
			sign = 1;
			switch (dimmInfo->memoryType) {
			case DDR:
				time_tmp =
					(((data[i] & 0xf0) >> 4) * 10) +
					((data[i] & 0x0f));
				leftOfPoint = time_tmp / 100;
				rightOfPoint = time_tmp % 100;
				break;
			case SDRAM:
				leftOfPoint = (data[i] & 0xf0) >> 4;
				if (leftOfPoint > 7) {
					leftOfPoint = data[i] & 0x70 >> 4;
					sign = -1;
				}
				rightOfPoint = (data[i] & 0x0f);
				break;
			}
			dimmInfo->dataInputSetupTime =
				(leftOfPoint * 100 + rightOfPoint) * sign;
			DP (printf
			    ("Data Input Setup Time [ns]: 			%d.%d\n",
			     sign * leftOfPoint, rightOfPoint));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/

		case 35:	/* Data Input Hold Time */
			sign = 1;
			switch (dimmInfo->memoryType) {
			case DDR:
				time_tmp =
					(((data[i] & 0xf0) >> 4) * 10) +
					((data[i] & 0x0f));
				leftOfPoint = time_tmp / 100;
				rightOfPoint = time_tmp % 100;
				break;
			case SDRAM:
				leftOfPoint = (data[i] & 0xf0) >> 4;
				if (leftOfPoint > 7) {
					leftOfPoint = data[i] & 0x70 >> 4;
					sign = -1;
				}
				rightOfPoint = (data[i] & 0x0f);
				break;
			}
			dimmInfo->dataInputHoldTime =
				(leftOfPoint * 100 + rightOfPoint) * sign;
			DP (printf
			    ("Data Input Hold Time [ns]: 			%d.%d\n\n",
			     sign * leftOfPoint, rightOfPoint));
			break;
/*------------------------------------------------------------------------------------------------------------------------------*/
		}
	}
	/* calculating the sdram density */
	for (i = 0;
	     i < dimmInfo->numOfRowAddresses + dimmInfo->numOfColAddresses;
	     i++) {
		density = density * 2;
	}
	dimmInfo->deviceDensity = density * dimmInfo->numOfBanksOnEachDevice *
		dimmInfo->sdramWidth;
	dimmInfo->numberOfDevices =
		(dimmInfo->dataWidth / dimmInfo->sdramWidth) *
		dimmInfo->numOfModuleBanks;
	devicesForErrCheck =
		(dimmInfo->dataWidth - 64) / dimmInfo->sdramWidth;
	if ((dimmInfo->errorCheckType == 0x1)
	    || (dimmInfo->errorCheckType == 0x2)
	    || (dimmInfo->errorCheckType == 0x3)) {
		dimmInfo->size =
			(dimmInfo->deviceDensity / 8) *
			(dimmInfo->numberOfDevices - devicesForErrCheck);
	} else {
		dimmInfo->size =
			(dimmInfo->deviceDensity / 8) *
			dimmInfo->numberOfDevices;
	}

	/* compute the module DRB size */
	tmp = (1 <<
	       (dimmInfo->numOfRowAddresses + dimmInfo->numOfColAddresses));
	tmp *= dimmInfo->numOfModuleBanks;
	tmp *= dimmInfo->sdramWidth;
	tmp = tmp >> 24;	/* div by 0x4000000 (64M)       */
	dimmInfo->drb_size = (uchar) tmp;
	DP (printf ("Module DRB size (n*64Mbit): %d\n", dimmInfo->drb_size));

	/* try a CAS latency of 3 first... */

	/* bit 1 is CL2, bit 2 is CL3 */
	supp_cal = (dimmInfo->suportedCasLatencies & 0x1c) >> 1;

	cal_val = 0;
	if (supp_cal & 8) {
		if (NS10to10PS (data[9]) <= tmemclk)
			cal_val = 6;
	}
	if (supp_cal & 4) {
		if (NS10to10PS (data[9]) <= tmemclk)
			cal_val = 5;
	}

	/* then 2... */
	if (supp_cal & 2) {
		if (NS10to10PS (data[23]) <= tmemclk)
			cal_val = 4;
	}

	DP (printf ("cal_val = %d\n", cal_val * 5));

	/* bummer, did't work... */
	if (cal_val == 0) {
		DP (printf ("Couldn't find a good CAS latency\n"));
		hang ();
		return 0;
	}

	return true;
}

/* sets up the GT properly with information passed in */
int setup_sdram (AUX_MEM_DIMM_INFO * info)
{
	ulong tmp, check;
	ulong tmp_sdram_mode = 0;	/* 0x141c */
	ulong tmp_dunit_control_low = 0;	/* 0x1404 */
	int i;

	/* sanity checking */
	if (!info->numOfModuleBanks) {
		printf ("setup_sdram called with 0 banks\n");
		return 1;
	}

	/* delay line */

	/* Program the GT with the discovered data */
	if (info->registeredAddrAndControlInputs == true)
		DP (printf
		    ("Module is registered, but we do not support registered Modules !!!\n"));


	/* delay line */
	set_dfcdlInit ();	/* may be its not needed */
	DP (printf ("Delay line set done\n"));

	/* set SDRAM mode NOP */ /* To_do check it */
	GT_REG_WRITE (SDRAM_OPERATION, 0x5);
	while (GTREGREAD (SDRAM_OPERATION) != 0) {
		DP (printf
		    ("\n*** SDRAM_OPERATION 1418: Module still busy ... please wait... ***\n"));
	}

	/* SDRAM configuration */
	GT_REG_WRITE (SDRAM_CONFIG, 0x58200400);
	DP (printf ("sdram_conf 0x1400: %08x\n", GTREGREAD (SDRAM_CONFIG)));

	/* SDRAM open pages controll keep open as much as I can */
	GT_REG_WRITE (SDRAM_OPEN_PAGES_CONTROL, 0x0);
	DP (printf
	    ("sdram_open_pages_controll 0x1414: %08x\n",
	     GTREGREAD (SDRAM_OPEN_PAGES_CONTROL)));


	/* SDRAM D_UNIT_CONTROL_LOW 0x1404 */
	tmp = (GTREGREAD (D_UNIT_CONTROL_LOW) & 0x01);	/* Clock Domain Sync from power on reset */
	if (tmp == 0)
		DP (printf ("Core Signals are sync (by HW-Setting)!!!\n"));
	else
		DP (printf
		    ("Core Signals syncs. are bypassed (by HW-Setting)!!!\n"));

	/* SDRAM set CAS Lentency according to SPD information */
	switch (info->memoryType) {
	case SDRAM:
		DP (printf ("### SD-RAM not supported yet !!!\n"));
		hang ();
		/* ToDo fill SD-RAM if needed !!!!! */
		break;

	case DDR:
		DP (printf ("### SET-CL for DDR-RAM\n"));

		switch (info->maxClSupported_DDR) {
		case DDR_CL_3:
			tmp_dunit_control_low = 0x3c000000;	/* Read-Data sampled on falling edge of Clk */
			tmp_sdram_mode = 0x32;	/* CL=3 Burstlength = 4 */
			DP (printf
			    ("Max. CL is 3 CLKs 0x141c= %08lx, 0x1404 = %08lx\n",
			     tmp_sdram_mode, tmp_dunit_control_low));
			break;

		case DDR_CL_2_5:
			if (tmp == 1) {	/* clocks sync */
				tmp_dunit_control_low = 0x24000000;	/* Read-Data sampled on falling edge of Clk */
				tmp_sdram_mode = 0x62;	/* CL=2,5 Burstlength = 4 */
				DP (printf
				    ("Max. CL is 2,5s CLKs 0x141c= %08lx, 0x1404 = %08lx\n",
				     tmp_sdram_mode, tmp_dunit_control_low));
			} else {	/* clk sync. bypassed     */

				tmp_dunit_control_low = 0x03000000;	/* Read-Data sampled on rising edge of Clk */
				tmp_sdram_mode = 0x62;	/* CL=2,5 Burstlength = 4 */
				DP (printf
				    ("Max. CL is 2,5 CLKs 0x141c= %08lx, 0x1404 = %08lx\n",
				     tmp_sdram_mode, tmp_dunit_control_low));
			}
			break;

		case DDR_CL_2:
			if (tmp == 1) {	/* Sync */
				tmp_dunit_control_low = 0x03000000;	/* Read-Data sampled on rising edge of Clk */
				tmp_sdram_mode = 0x22;	/* CL=2 Burstlength = 4 */
				DP (printf
				    ("Max. CL is 2s CLKs 0x141c= %08lx, 0x1404 = %08lx\n",
				     tmp_sdram_mode, tmp_dunit_control_low));
			} else {	/* Not sync.      */

				tmp_dunit_control_low = 0x3b000000;	/* Read-Data sampled on rising edge of Clk */
				tmp_sdram_mode = 0x22;	/* CL=2 Burstlength = 4 */
				DP (printf
				    ("Max. CL is 2 CLKs 0x141c= %08lx, 0x1404 = %08lx\n",
				     tmp_sdram_mode, tmp_dunit_control_low));
			}
			break;

		case DDR_CL_1_5:
			if (tmp == 1) {	/* Sync */
				tmp_dunit_control_low = 0x23000000;	/* Read-Data sampled on falling edge of Clk */
				tmp_sdram_mode = 0x52;	/* CL=1,5 Burstlength = 4 */
				DP (printf
				    ("Max. CL is 1,5s CLKs 0x141c= %08lx, 0x1404 = %08lx\n",
				     tmp_sdram_mode, tmp_dunit_control_low));
			} else {	/* not sync */

				tmp_dunit_control_low = 0x1a000000;	/* Read-Data sampled on rising edge of Clk */
				tmp_sdram_mode = 0x52;	/* CL=1,5 Burstlength = 4 */
				DP (printf
				    ("Max. CL is 1,5 CLKs 0x141c= %08lx, 0x1404 = %08lx\n",
				     tmp_sdram_mode, tmp_dunit_control_low));
			}
			break;

		default:
			printf ("Max. CL is out of range %d\n",
				info->maxClSupported_DDR);
			hang ();
			break;
		}
		break;
	}

	/* Write results of CL detection procedure */
	GT_REG_WRITE (SDRAM_MODE, tmp_sdram_mode);
	/* set SDRAM mode SetCommand 0x1418 */
	GT_REG_WRITE (SDRAM_OPERATION, 0x3);
	while (GTREGREAD (SDRAM_OPERATION) != 0) {
		DP (printf
		    ("\n*** SDRAM_OPERATION 1418 after SDRAM_MODE: Module still busy ... please wait... ***\n"));
	}


	/* SDRAM D_UNIT_CONTROL_LOW 0x1404 */
	tmp = (GTREGREAD (D_UNIT_CONTROL_LOW) & 0x01);	/* Clock Domain Sync from power on reset */
	if (tmp != 1) {		/*clocks are not sync */
		/* asyncmode */
		GT_REG_WRITE (D_UNIT_CONTROL_LOW,
			      (GTREGREAD (D_UNIT_CONTROL_LOW) & 0x7F) |
			      0x18110780 | tmp_dunit_control_low);
	} else {
		/* syncmode */
		GT_REG_WRITE (D_UNIT_CONTROL_LOW,
			      (GTREGREAD (D_UNIT_CONTROL_LOW) & 0x7F) |
			      0x00110000 | tmp_dunit_control_low);
	}

	/* set SDRAM mode SetCommand 0x1418 */
	GT_REG_WRITE (SDRAM_OPERATION, 0x3);
	while (GTREGREAD (SDRAM_OPERATION) != 0) {
		DP (printf
		    ("\n*** SDRAM_OPERATION 1418 after D_UNIT_CONTROL_LOW: Module still busy ... please wait... ***\n"));
	}

/*------------------------------------------------------------------------------ */


	/* bank parameters */
	/* SDRAM address decode register */
	/* program this with the default value */
	tmp = 0x02;


	DP (printf ("drb_size (n*64Mbit): %d\n", info->drb_size));
	switch (info->drb_size) {
	case 1:		/* 64 Mbit */
	case 2:		/* 128 Mbit */
		DP (printf ("RAM-Device_size 64Mbit or 128Mbit)\n"));
		tmp |= (0x00 << 4);
		break;
	case 4:		/* 256 Mbit */
	case 8:		/* 512 Mbit */
		DP (printf ("RAM-Device_size 256Mbit or 512Mbit)\n"));
		tmp |= (0x01 << 4);
		break;
	case 16:		/* 1 Gbit */
	case 32:		/* 2 Gbit */
		DP (printf ("RAM-Device_size 1Gbit or 2Gbit)\n"));
		tmp |= (0x02 << 4);
		break;
	default:
		printf ("Error in dram size calculation\n");
		DP (printf ("Assume: RAM-Device_size 1Gbit or 2Gbit)\n"));
		tmp |= (0x02 << 4);
		return 1;
	}

	/* SDRAM bank parameters */
	/* the param registers for slot 1 (banks 2+3) are offset by 0x8 */
	DP (printf
	    ("setting up slot %d config with: %08lx \n", info->slot, tmp));
	GT_REG_WRITE (SDRAM_ADDR_CONTROL, tmp);

/* ------------------------------------------------------------------------------ */

	DP (printf
	    ("setting up sdram_timing_control_low with: %08x \n",
	     0x11511220));
	GT_REG_WRITE (SDRAM_TIMING_CONTROL_LOW, 0x11511220);


/* ------------------------------------------------------------------------------ */

	/* SDRAM configuration */
	tmp = GTREGREAD (SDRAM_CONFIG);

	if (info->registeredAddrAndControlInputs
	    || info->registeredDQMBinputs) {
		tmp |= (1 << 17);
		DP (printf
		    ("SPD says: registered Addr. and Cont.: %d; registered DQMBinputs: %d\n",
		     info->registeredAddrAndControlInputs,
		     info->registeredDQMBinputs));
	}

	/* Use buffer 1 to return read data to the CPU
	 * Page 426 MV64360 */
	tmp |= (1 << 26);
	DP (printf
	    ("Before Buffer assignment - sdram_conf: %08x\n",
	     GTREGREAD (SDRAM_CONFIG)));
	DP (printf
	    ("After Buffer assignment - sdram_conf: %08x\n",
	     GTREGREAD (SDRAM_CONFIG)));

	/* SDRAM timing To_do: */


	tmp = GTREGREAD (SDRAM_TIMING_CONTROL_HIGH);
	DP (printf ("# sdram_timing_control_high is : %08lx \n", tmp));

	/* SDRAM address decode register */
	/* program this with the default value */
	tmp = GTREGREAD (SDRAM_ADDR_CONTROL);
	DP (printf
	    ("SDRAM address control (before: decode): %08x  ",
	     GTREGREAD (SDRAM_ADDR_CONTROL)));
	GT_REG_WRITE (SDRAM_ADDR_CONTROL, (tmp | 0x2));
	DP (printf
	    ("SDRAM address control (after: decode): %08x\n",
	     GTREGREAD (SDRAM_ADDR_CONTROL)));

	/* set the SDRAM configuration for each bank */

/*	for (i = info->slot * 2; i < ((info->slot * 2) + info->banks); i++) */
	{
		i = info->slot;
		DP (printf
		    ("\n*** Running a MRS cycle for bank %d ***\n", i));

		/* map the bank */
		memory_map_bank (i, 0, GB / 4);
#if 1				/* test only */
		/* set SDRAM mode */ /* To_do check it */
		GT_REG_WRITE (SDRAM_OPERATION, 0x3);
		check = GTREGREAD (SDRAM_OPERATION);
		DP (printf
		    ("\n*** SDRAM_OPERATION 1418 (0 = Normal Operation) = %08lx ***\n",
		     check));


		/* switch back to normal operation mode */
		GT_REG_WRITE (SDRAM_OPERATION, 0);
		check = GTREGREAD (SDRAM_OPERATION);
		DP (printf
		    ("\n*** SDRAM_OPERATION 1418 (0 = Normal Operation) = %08lx ***\n",
		     check));
#endif /* test only */
		/* unmap the bank */
		memory_map_bank (i, 0, 0);
	}

	return 0;
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */
long int
dram_size(long int *base, long int maxsize)
{
    volatile long int	 *addr, *b=base;
    long int	 cnt, val, save1, save2;

#define STARTVAL (1<<20)	/* start test at 1M */
    for (cnt = STARTVAL/sizeof(long); cnt < maxsize/sizeof(long); cnt <<= 1) {
	    addr = base + cnt;	/* pointer arith! */

	    save1=*addr;		/* save contents of addr */
	    save2=*b;		/* save contents of base */

	    *addr=cnt;		/* write cnt to addr */
	    *b=0;			/* put null at base */

	    /* check at base address */
	    if ((*b) != 0) {
		*addr=save1;	/* restore *addr */
		*b=save2;	/* restore *b */
		return (0);
	    }
	    val = *addr;		/* read *addr */
	    val = *addr;		/* read *addr */

	    *addr=save1;
	    *b=save2;

	    if (val != cnt) {
		    DP(printf("Found %08x  at Address %08x (failure)\n", (unsigned int)val, (unsigned int) addr));
		    /* fix boundary condition.. STARTVAL means zero */
		    if(cnt==STARTVAL/sizeof(long)) cnt=0;
		    return (cnt * sizeof(long));
	    }
    }
    return maxsize;
}

/* ------------------------------------------------------------------------- */

/* ppcboot interface function to SDRAM init - this is where all the
 * controlling logic happens */
long int
initdram(int board_type)
{
	int s0 = 0, s1 = 0;
	int checkbank[4] = { [0 ... 3] = 0 };
		ulong bank_no, realsize, total, check;
	AUX_MEM_DIMM_INFO dimmInfo1;
	AUX_MEM_DIMM_INFO dimmInfo2;
	int nhr;

	/* first, use the SPD to get info about the SDRAM/ DDRRAM */

	/* check the NHR bit and skip mem init if it's already done */
	nhr = get_hid0() & (1 << 16);

	if (nhr) {
		printf("Skipping SD- DDRRAM setup due to NHR bit being set\n");
	} else {
		/* DIMM0 */
		s0 = check_dimm(0, &dimmInfo1);

		/* DIMM1 */
		s1 = check_dimm(1, &dimmInfo2);

		memory_map_bank(0, 0, 0);
		memory_map_bank(1, 0, 0);
		memory_map_bank(2, 0, 0);
		memory_map_bank(3, 0, 0);

		if (dimmInfo1.numOfModuleBanks && setup_sdram(&dimmInfo1)) {
			printf("Setup for DIMM1 failed.\n");
		}

		if (dimmInfo2.numOfModuleBanks && setup_sdram(&dimmInfo2)) {
			printf("Setup for DIMM2 failed.\n");
		}

		/* set the NHR bit */
		set_hid0(get_hid0() | (1 << 16));
	}
	/* next, size the SDRAM banks */

	realsize = total = 0;
	check = GB/4;
	if (dimmInfo1.numOfModuleBanks > 0) {checkbank[0] = 1; printf("-- DIMM1 has 1 bank\n");}
	if (dimmInfo1.numOfModuleBanks > 1) {checkbank[1] = 1; printf("-- DIMM1 has 2 banks\n");}
	if (dimmInfo1.numOfModuleBanks > 2)
		printf("Error, SPD claims DIMM1 has >2 banks\n");

	if (dimmInfo2.numOfModuleBanks > 0) {checkbank[2] = 1; printf("-- DIMM2 has 1 bank\n");}
	if (dimmInfo2.numOfModuleBanks > 1) {checkbank[3] = 1; printf("-- DIMM2 has 2 banks\n");}
	if (dimmInfo2.numOfModuleBanks > 2)
		printf("Error, SPD claims DIMM2 has >2 banks\n");

	for (bank_no = 0; bank_no < CFG_DRAM_BANKS; bank_no++) {
		/* skip over banks that are not populated */
		if (! checkbank[bank_no])
			continue;

		if ((total + check) > CFG_GT_REGS)
			check = CFG_GT_REGS - total;

		memory_map_bank(bank_no, total, check);
		realsize = dram_size((long int *)total, check);
		memory_map_bank(bank_no, total, realsize);

		total += realsize;
	}

/*	Setup Ethernet DMA Adress window to DRAM Area */
	return(total);
}

/* ***************************************************************************************
! *                             SDRAM INIT                                              *
! *  This procedure detect all Sdram types: 64, 128, 256, 512 Mbit, 1Gbit and 2Gb       *
! *               This procedure fits only the Atlantis                                *
! *                                                                                     *
! *************************************************************************************** */


/* ***************************************************************************************
! *                             DFCDL initialize MV643xx Design Considerations             *
! *                                                                                     *
! *************************************************************************************** */
int set_dfcdlInit (void)
{
	int i;
	unsigned int dfcdl_word = 0x0000014f;

	for (i = 0; i < 64; i++) {
		GT_REG_WRITE (SRAM_DATA0, dfcdl_word);
	}
	GT_REG_WRITE (DFCDL_CONFIG0, 0x00300000);	/* enable dynamic delay line updating */


	return (0);
}
