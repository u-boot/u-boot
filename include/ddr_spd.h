/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#ifndef _DDR_SPD_H_
#define _DDR_SPD_H_

/*
 * Format from "JEDEC Standard No. 21-C,
 * Appendix D: Rev 1.0: SPD's for DDR SDRAM
 */
typedef struct ddr1_spd_eeprom_s {
	unsigned char info_size;   /*  0 # bytes written into serial memory */
	unsigned char chip_size;   /*  1 Total # bytes of SPD memory device */
	unsigned char mem_type;    /*  2 Fundamental memory type */
	unsigned char nrow_addr;   /*  3 # of Row Addresses on this assembly */
	unsigned char ncol_addr;   /*  4 # of Column Addrs on this assembly */
	unsigned char nrows;       /*  5 Number of DIMM Banks */
	unsigned char dataw_lsb;   /*  6 Data Width of this assembly */
	unsigned char dataw_msb;   /*  7 ... Data Width continuation */
	unsigned char voltage;     /*  8 Voltage intf std of this assembly */
	unsigned char clk_cycle;   /*  9 SDRAM Cycle time @ CL=X */
	unsigned char clk_access;  /* 10 SDRAM Access from Clk @ CL=X (tAC) */
	unsigned char config;      /* 11 DIMM Configuration type */
	unsigned char refresh;     /* 12 Refresh Rate/Type */
	unsigned char primw;       /* 13 Primary SDRAM Width */
	unsigned char ecw;         /* 14 Error Checking SDRAM width */
	unsigned char min_delay;   /* 15 for Back to Back Random Address */
	unsigned char burstl;      /* 16 Burst Lengths Supported */
	unsigned char nbanks;      /* 17 # of Banks on SDRAM Device */
	unsigned char cas_lat;     /* 18 CAS# Latencies Supported */
	unsigned char cs_lat;      /* 19 CS# Latency */
	unsigned char write_lat;   /* 20 Write Latency (aka Write Recovery) */
	unsigned char mod_attr;    /* 21 SDRAM Module Attributes */
	unsigned char dev_attr;    /* 22 SDRAM Device Attributes */
	unsigned char clk_cycle2;  /* 23 Min SDRAM Cycle time @ CL=X-0.5 */
	unsigned char clk_access2; /* 24 SDRAM Access from
				         Clk @ CL=X-0.5 (tAC) */
	unsigned char clk_cycle3;  /* 25 Min SDRAM Cycle time @ CL=X-1 */
	unsigned char clk_access3; /* 26 Max Access from Clk @ CL=X-1 (tAC) */
	unsigned char trp;         /* 27 Min Row Precharge Time (tRP)*/
	unsigned char trrd;        /* 28 Min Row Active to Row Active (tRRD) */
	unsigned char trcd;        /* 29 Min RAS to CAS Delay (tRCD) */
	unsigned char tras;        /* 30 Minimum RAS Pulse Width (tRAS) */
	unsigned char bank_dens;   /* 31 Density of each bank on module */
	unsigned char ca_setup;    /* 32 Addr + Cmd Setup Time Before Clk */
	unsigned char ca_hold;     /* 33 Addr + Cmd Hold Time After Clk */
	unsigned char data_setup;  /* 34 Data Input Setup Time Before Strobe */
	unsigned char data_hold;   /* 35 Data Input Hold Time After Strobe */
	unsigned char res_36_40[5];/* 36-40 reserved for VCSDRAM */
	unsigned char trc;         /* 41 Min Active to Auto refresh time tRC */
	unsigned char trfc;        /* 42 Min Auto to Active period tRFC */
	unsigned char tckmax;      /* 43 Max device cycle time tCKmax */
	unsigned char tdqsq;       /* 44 Max DQS to DQ skew (tDQSQ max) */
	unsigned char tqhs;        /* 45 Max Read DataHold skew (tQHS) */
	unsigned char res_46;      /* 46 Reserved */
	unsigned char dimm_height; /* 47 DDR SDRAM DIMM Height */
	unsigned char res_48_61[14]; /* 48-61 Reserved */
	unsigned char spd_rev;     /* 62 SPD Data Revision Code */
	unsigned char cksum;       /* 63 Checksum for bytes 0-62 */
	unsigned char mid[8];      /* 64-71 Mfr's JEDEC ID code per JEP-106 */
	unsigned char mloc;        /* 72 Manufacturing Location */
	unsigned char mpart[18];   /* 73 Manufacturer's Part Number */
	unsigned char rev[2];      /* 91 Revision Code */
	unsigned char mdate[2];    /* 93 Manufacturing Date */
	unsigned char sernum[4];   /* 95 Assembly Serial Number */
	unsigned char mspec[27];   /* 99-127 Manufacturer Specific Data */

} ddr1_spd_eeprom_t;

/*
 * Format from "JEDEC Appendix X: Serial Presence Detects for DDR2 SDRAM",
 * SPD Revision 1.2
 */
typedef struct ddr2_spd_eeprom_s {
	unsigned char info_size;   /*  0 # bytes written into serial memory */
	unsigned char chip_size;   /*  1 Total # bytes of SPD memory device */
	unsigned char mem_type;    /*  2 Fundamental memory type */
	unsigned char nrow_addr;   /*  3 # of Row Addresses on this assembly */
	unsigned char ncol_addr;   /*  4 # of Column Addrs on this assembly */
	unsigned char mod_ranks;   /*  5 Number of DIMM Ranks */
	unsigned char dataw;       /*  6 Module Data Width */
	unsigned char res_7;       /*  7 Reserved */
	unsigned char voltage;     /*  8 Voltage intf std of this assembly */
	unsigned char clk_cycle;   /*  9 SDRAM Cycle time @ CL=X */
	unsigned char clk_access;  /* 10 SDRAM Access from Clk @ CL=X (tAC) */
	unsigned char config;      /* 11 DIMM Configuration type */
	unsigned char refresh;     /* 12 Refresh Rate/Type */
	unsigned char primw;       /* 13 Primary SDRAM Width */
	unsigned char ecw;         /* 14 Error Checking SDRAM width */
	unsigned char res_15;      /* 15 Reserved */
	unsigned char burstl;      /* 16 Burst Lengths Supported */
	unsigned char nbanks;      /* 17 # of Banks on Each SDRAM Device */
	unsigned char cas_lat;     /* 18 CAS# Latencies Supported */
	unsigned char mech_char;   /* 19 DIMM Mechanical Characteristics */
	unsigned char dimm_type;   /* 20 DIMM type information */
	unsigned char mod_attr;    /* 21 SDRAM Module Attributes */
	unsigned char dev_attr;    /* 22 SDRAM Device Attributes */
	unsigned char clk_cycle2;  /* 23 Min SDRAM Cycle time @ CL=X-1 */
	unsigned char clk_access2; /* 24 SDRAM Access from Clk @ CL=X-1 (tAC) */
	unsigned char clk_cycle3;  /* 25 Min SDRAM Cycle time @ CL=X-2 */
	unsigned char clk_access3; /* 26 Max Access from Clk @ CL=X-2 (tAC) */
	unsigned char trp;         /* 27 Min Row Precharge Time (tRP)*/
	unsigned char trrd;        /* 28 Min Row Active to Row Active (tRRD) */
	unsigned char trcd;        /* 29 Min RAS to CAS Delay (tRCD) */
	unsigned char tras;        /* 30 Minimum RAS Pulse Width (tRAS) */
	unsigned char rank_dens;   /* 31 Density of each rank on module */
	unsigned char ca_setup;    /* 32 Addr+Cmd Setup Time Before Clk (tIS) */
	unsigned char ca_hold;     /* 33 Addr+Cmd Hold Time After Clk (tIH) */
	unsigned char data_setup;  /* 34 Data Input Setup Time
				         Before Strobe (tDS) */
	unsigned char data_hold;   /* 35 Data Input Hold Time
				         After Strobe (tDH) */
	unsigned char twr;         /* 36 Write Recovery time tWR */
	unsigned char twtr;        /* 37 Int write to read delay tWTR */
	unsigned char trtp;        /* 38 Int read to precharge delay tRTP */
	unsigned char mem_probe;   /* 39 Mem analysis probe characteristics */
	unsigned char trctrfc_ext; /* 40 Extensions to trc and trfc */
	unsigned char trc;         /* 41 Min Active to Auto refresh time tRC */
	unsigned char trfc;        /* 42 Min Auto to Active period tRFC */
	unsigned char tckmax;      /* 43 Max device cycle time tCKmax */
	unsigned char tdqsq;       /* 44 Max DQS to DQ skew (tDQSQ max) */
	unsigned char tqhs;        /* 45 Max Read DataHold skew (tQHS) */
	unsigned char pll_relock;  /* 46 PLL Relock time */
	unsigned char Tcasemax;    /* 47 Tcasemax */
	unsigned char psiTAdram;   /* 48 Thermal Resistance of DRAM Package from
				         Top (Case) to Ambient (Psi T-A DRAM) */
	unsigned char dt0_mode;    /* 49 DRAM Case Temperature Rise from Ambient
				         due to Activate-Precharge/Mode Bits
					 (DT0/Mode Bits) */
	unsigned char dt2n_dt2q;   /* 50 DRAM Case Temperature Rise from Ambient
				         due to Precharge/Quiet Standby
					 (DT2N/DT2Q) */
	unsigned char dt2p;        /* 51 DRAM Case Temperature Rise from Ambient
				         due to Precharge Power-Down (DT2P) */
	unsigned char dt3n;        /* 52 DRAM Case Temperature Rise from Ambient
				         due to Active Standby (DT3N) */
	unsigned char dt3pfast;    /* 53 DRAM Case Temperature Rise from Ambient
				         due to Active Power-Down with
					 Fast PDN Exit (DT3Pfast) */
	unsigned char dt3pslow;    /* 54 DRAM Case Temperature Rise from Ambient
				         due to Active Power-Down with Slow
					 PDN Exit (DT3Pslow) */
	unsigned char dt4r_dt4r4w; /* 55 DRAM Case Temperature Rise from Ambient
				         due to Page Open Burst Read/DT4R4W
					 Mode Bit (DT4R/DT4R4W Mode Bit) */
	unsigned char dt5b;        /* 56 DRAM Case Temperature Rise from Ambient
				         due to Burst Refresh (DT5B) */
	unsigned char dt7;         /* 57 DRAM Case Temperature Rise from Ambient
				         due to Bank Interleave Reads with
					 Auto-Precharge (DT7) */
	unsigned char psiTApll;    /* 58 Thermal Resistance of PLL Package form
				         Top (Case) to Ambient (Psi T-A PLL) */
	unsigned char psiTAreg;    /* 59 Thermal Reisitance of Register Package
				         from Top (Case) to Ambient
					 (Psi T-A Register) */
	unsigned char dtpllactive; /* 60 PLL Case Temperature Rise from Ambient
				         due to PLL Active (DT PLL Active) */
	unsigned char dtregact;    /* 61 Register Case Temperature Rise from
				         Ambient due to Register Active/Mode Bit
					 (DT Register Active/Mode Bit) */
	unsigned char spd_rev;     /* 62 SPD Data Revision Code */
	unsigned char cksum;       /* 63 Checksum for bytes 0-62 */
	unsigned char mid[8];      /* 64 Mfr's JEDEC ID code per JEP-106 */
	unsigned char mloc;        /* 72 Manufacturing Location */
	unsigned char mpart[18];   /* 73 Manufacturer's Part Number */
	unsigned char rev[2];      /* 91 Revision Code */
	unsigned char mdate[2];    /* 93 Manufacturing Date */
	unsigned char sernum[4];   /* 95 Assembly Serial Number */
	unsigned char mspec[27];   /* 99-127 Manufacturer Specific Data */

} ddr2_spd_eeprom_t;

typedef struct ddr3_spd_eeprom_s {
	/* General Section: Bytes 0-59 */
	unsigned char info_size_crc;   /*  0 # bytes written into serial memory,
					     CRC coverage */
	unsigned char spd_rev;         /*  1 Total # bytes of SPD mem device */
	unsigned char mem_type;        /*  2 Key Byte / Fundamental mem type */
	unsigned char module_type;     /*  3 Key Byte / Module Type */
	unsigned char density_banks;   /*  4 SDRAM Density and Banks */
	unsigned char addressing;      /*  5 SDRAM Addressing */
	unsigned char module_vdd;      /*  6 Module nominal voltage, VDD */
	unsigned char organization;    /*  7 Module Organization */
	unsigned char bus_width;       /*  8 Module Memory Bus Width */
	unsigned char ftb_div;         /*  9 Fine Timebase (FTB)
					     Dividend / Divisor */
	unsigned char mtb_dividend;    /* 10 Medium Timebase (MTB) Dividend */
	unsigned char mtb_divisor;     /* 11 Medium Timebase (MTB) Divisor */
	unsigned char tCK_min;         /* 12 SDRAM Minimum Cycle Time */
	unsigned char res_13;          /* 13 Reserved */
	unsigned char caslat_lsb;      /* 14 CAS Latencies Supported,
					     Least Significant Byte */
	unsigned char caslat_msb;      /* 15 CAS Latencies Supported,
					     Most Significant Byte */
	unsigned char tAA_min;         /* 16 Min CAS Latency Time */
	unsigned char tWR_min;         /* 17 Min Write REcovery Time */
	unsigned char tRCD_min;        /* 18 Min RAS# to CAS# Delay Time */
	unsigned char tRRD_min;        /* 19 Min Row Active to
					     Row Active Delay Time */
	unsigned char tRP_min;         /* 20 Min Row Precharge Delay Time */
	unsigned char tRAS_tRC_ext;    /* 21 Upper Nibbles for tRAS and tRC */
	unsigned char tRAS_min_lsb;    /* 22 Min Active to Precharge
					     Delay Time */
	unsigned char tRC_min_lsb;     /* 23 Min Active to Active/Refresh
					     Delay Time, LSB */
	unsigned char tRFC_min_lsb;    /* 24 Min Refresh Recovery Delay Time */
	unsigned char tRFC_min_msb;    /* 25 Min Refresh Recovery Delay Time */
	unsigned char tWTR_min;        /* 26 Min Internal Write to
					     Read Command Delay Time */
	unsigned char tRTP_min;        /* 27 Min Internal Read to Precharge
					     Command Delay Time */
	unsigned char tFAW_msb;        /* 28 Upper Nibble for tFAW */
	unsigned char tFAW_min;        /* 29 Min Four Activate Window
					     Delay Time*/
	unsigned char opt_features;    /* 30 SDRAM Optional Features */
	unsigned char therm_ref_opt;   /* 31 SDRAM Thermal and Refresh Opts */
	unsigned char therm_sensor;    /* 32 Module Thermal Sensor */
	unsigned char device_type;     /* 33 SDRAM device type */
	unsigned char res_34_59[26];   /* 34-59 Reserved, General Section */

	/* Module-Specific Section: Bytes 60-116 */
	union {
		struct {
			/* 60 (Unbuffered) Module Nominal Height */
			unsigned char mod_height;
			/* 61 (Unbuffered) Module Maximum Thickness */
			unsigned char mod_thickness;
			/* 62 (Unbuffered) Reference Raw Card Used */
			unsigned char ref_raw_card;
			/* 63 (Unbuffered) Address Mapping from
			      Edge Connector to DRAM */
			unsigned char addr_mapping;
			/* 64-116 (Unbuffered) Reserved */
			unsigned char res_64_116[53];
		} unbuffered;
		struct {
			/* 60 (Registered) Module Nominal Height */
			unsigned char mod_height;
			/* 61 (Registered) Module Maximum Thickness */
			unsigned char mod_thickness;
			/* 62 (Registered) Reference Raw Card Used */
			unsigned char ref_raw_card;
			/* 63 DIMM Module Attributes */
			unsigned char modu_attr;
			/* 64 RDIMM Thermal Heat Spreader Solution */
			unsigned char thermal;
			/* 65 Register Manufacturer ID Code, Least Significant Byte */
			unsigned char reg_id_lo;
			/* 66 Register Manufacturer ID Code, Most Significant Byte */
			unsigned char reg_id_hi;
			/* 67 Register Revision Number */
			unsigned char reg_rev;
			/* 68 Register Type */
			unsigned char reg_type;
			/* 69-76 RC1,3,5...15 (MS Nibble) / RC0,2,4...14 (LS Nibble) */
			unsigned char rcw[8];
		} registered;
		unsigned char uc[57]; /* 60-116 Module-Specific Section */
	} mod_section;

	/* Unique Module ID: Bytes 117-125 */
	unsigned char mmid_lsb;        /* 117 Module MfgID Code LSB - JEP-106 */
	unsigned char mmid_msb;        /* 118 Module MfgID Code MSB - JEP-106 */
	unsigned char mloc;            /* 119 Mfg Location */
	unsigned char mdate[2];        /* 120-121 Mfg Date */
	unsigned char sernum[4];       /* 122-125 Module Serial Number */

	/* CRC: Bytes 126-127 */
	unsigned char crc[2];          /* 126-127 SPD CRC */

	/* Other Manufacturer Fields and User Space: Bytes 128-255 */
	unsigned char mpart[18];       /* 128-145 Mfg's Module Part Number */
	unsigned char mrev[2];         /* 146-147 Module Revision Code */

	unsigned char dmid_lsb;        /* 148 DRAM MfgID Code LSB - JEP-106 */
	unsigned char dmid_msb;        /* 149 DRAM MfgID Code MSB - JEP-106 */

	unsigned char msd[26];         /* 150-175 Mfg's Specific Data */
	unsigned char cust[80];        /* 176-255 Open for Customer Use */

} ddr3_spd_eeprom_t;

extern unsigned int ddr1_spd_check(const ddr1_spd_eeprom_t *spd);
extern void ddr1_spd_dump(const ddr1_spd_eeprom_t *spd);
extern unsigned int ddr2_spd_check(const ddr2_spd_eeprom_t *spd);
extern void ddr2_spd_dump(const ddr2_spd_eeprom_t *spd);
extern unsigned int ddr3_spd_check(const ddr3_spd_eeprom_t *spd);

/*
 * Byte 2 Fundamental Memory Types.
 */
#define SPD_MEMTYPE_FPM		(0x01)
#define SPD_MEMTYPE_EDO		(0x02)
#define SPD_MEMTYPE_PIPE_NIBBLE	(0x03)
#define SPD_MEMTYPE_SDRAM	(0x04)
#define SPD_MEMTYPE_ROM		(0x05)
#define SPD_MEMTYPE_SGRAM	(0x06)
#define SPD_MEMTYPE_DDR		(0x07)
#define SPD_MEMTYPE_DDR2	(0x08)
#define SPD_MEMTYPE_DDR2_FBDIMM	(0x09)
#define SPD_MEMTYPE_DDR2_FBDIMM_PROBE	(0x0A)
#define SPD_MEMTYPE_DDR3	(0x0B)

/* DIMM Type for DDR2 SPD (according to v1.3) */
#define DDR2_SPD_DIMMTYPE_UNDEFINED	(0x00)
#define DDR2_SPD_DIMMTYPE_RDIMM		(0x01)
#define DDR2_SPD_DIMMTYPE_UDIMM		(0x02)
#define DDR2_SPD_DIMMTYPE_SO_DIMM	(0x04)
#define DDR2_SPD_DIMMTYPE_72B_SO_CDIMM	(0x06)
#define DDR2_SPD_DIMMTYPE_72B_SO_RDIMM	(0x07)
#define DDR2_SPD_DIMMTYPE_MICRO_DIMM	(0x08)
#define DDR2_SPD_DIMMTYPE_MINI_RDIMM	(0x10)
#define DDR2_SPD_DIMMTYPE_MINI_UDIMM	(0x20)

/* Byte 3 Key Byte / Module Type for DDR3 SPD */
#define DDR3_SPD_MODULETYPE_MASK	(0x0f)
#define DDR3_SPD_MODULETYPE_RDIMM	(0x01)
#define DDR3_SPD_MODULETYPE_UDIMM	(0x02)
#define DDR3_SPD_MODULETYPE_SO_DIMM	(0x03)
#define DDR3_SPD_MODULETYPE_MICRO_DIMM	(0x04)
#define DDR3_SPD_MODULETYPE_MINI_RDIMM	(0x05)
#define DDR3_SPD_MODULETYPE_MINI_UDIMM	(0x06)

#endif /* _DDR_SPD_H_ */
