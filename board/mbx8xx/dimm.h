#ifndef __dimm_h
#define __dimm_h

/*
 * Module name: %M%
 * Description:
 * Serial Presence Detect Definitions Module
 * SCCS identification: %I%
 * Branch: %B%
 * Sequence: %S%
 * Date newest applied delta was created (MM/DD/YY): %G%
 * Time newest applied delta was created (HH:MM:SS): %U%
 * SCCS file name %F%
 * Fully qualified SCCS file name:
 * %P%
 * Copyright:
 * (C) COPYRIGHT MOTOROLA, INC. 1996
 * ALL RIGHTS RESERVED
 * Notes:
 * 1. All data was taken from an IBM application note titled
 * "Serial Presence Detect Definitions".
 * History:
 * Date Who
 *
 * 10/24/96 Rob Baxter
 * Initial release.
 *
 */

/*
 * serial PD byte assignment address map (256 byte EEPROM)
 */
typedef struct dimm
{
	uchar n_bytes; /* 00 number of bytes written/used */
	uchar t_bytes; /* 01 total number of bytes in serial PD device */
	uchar fmt; /* 02 fundamental memory type (FPM/EDO/SDRAM) */
	uchar n_row; /* 03 number of rows */
	uchar n_col; /* 04 number of columns */
	uchar n_banks; /* 05 number of banks */
	uchar data_w_lo; /* 06 data width */
	uchar data_w_hi; /* 07 data width */
	uchar ifl; /* 08 interface levels */
	uchar a_ras; /* 09 RAS access */
	uchar a_cas; /* 0A CAS access */
	uchar ct; /* 0B configuration type (non-parity/parity/ECC) */
	uchar refresh_rt; /* 0C refresh rate/type */
	uchar p_dram_o; /* 0D primary DRAM organization */
	uchar s_dram_o; /* 0E secondary DRAM organization (parity/ECC-checkbits) */
	uchar reserved[17]; /* 0F reserved fields for future offerings */
	uchar ss_info[32]; /* 20 superset information (may be used in the future) */
	uchar m_info[64]; /* 40 manufacturer information (optional) */
	uchar unused[128]; /* 80 unused storage locations */
} dimm_t;

/*
 * memory type definitions
 */
#define DIMM_MT_FPM 1 /* standard FPM (fast page mode) DRAM */
#define DIMM_MT_EDO 2 /* EDO (extended data out) */
#define DIMM_MT_PN 3 /* pipelined nibble */
#define DIMM_MT_SDRAM 4 /* SDRAM (synchronous DRAM) */

/*
 * row addresses definitions
 */
#define DIMM_RA_RDNDNT (1<<7) /* redundant addressing */
#define DIMM_RA_MASK 0x7f /* number of row addresses mask */

/*
 * module interface levels definitions
 */
#define DIMM_IFL_TTL 0 /* TTL/5V tolerant */
#define DIMM_IFL_LVTTL 1 /* LVTTL (not 5V tolerant) */
#define DIMM_IFL_HSTL15 2 /* HSTL 1.5 */
#define DIMM_IFL_SSTL33 3 /* SSTL 3.3 */
#define DIMM_IFL_SSTL25 4 /* SSTL 2.5 */

/*
 * DIMM configuration type definitions
 */
#define DIMM_CT_NONE 0 /* none */
#define DIMM_CT_PARITY 1 /* parity */
#define DIMM_CT_ECC 2 /* ECC */

/*
 * row addresses definitions
 */
#define DIMM_RRT_SR (1<<7) /* self refresh flag */
#define DIMM_RRT_MASK 0x7f /* refresh rate mask */
#define DIMM_RRT_NRML 0x00 /* normal (15.625us) */
#define DIMM_RRT_R_3_9 0x01 /* reduced .25x (3.9us) */
#define DIMM_RRT_R_7_8 0x02 /* reduced .5x (7.8us) */
#define DIMM_RRT_E_31_3 0x03 /* extended 2x (31.3us) */
#define DIMM_RRT_E_62_5 0x04 /* extended 4x (62.5us) */
#define DIMM_RRT_E_125 0x05 /* extended 8x (125us) */

#endif /* __dimm_h */
