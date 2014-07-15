/*
 * (C) Copyright 2001
 * Murray Jensen, CSIRO-MIT, <Murray.Jensen@csiro.au>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _HYMOD_H_
#define _HYMOD_H_

#ifdef CONFIG_MPC8260
#include <asm/iopin_8260.h>
#endif

/*
 * hymod configuration data - passed by boot code via the board information
 * structure (only U-Boot has support for this at the moment)
 *
 * there are three types of data passed up from the boot monitor. the first
 * (type hymod_eeprom_t) is the eeprom data that was read off both the main
 * (or mother) board and the mezzanine board (if any). this data defines how
 * many Xilinx fpgas are on each board, and their types (among other things).
 * the second type of data (type xlx_mmap_t, one per Xilinx fpga) defines where
 * in the physical address space the various Xilinx fpga access regions have
 * been mapped by the boot rom. the third type of data (type xlx_iopins_t,
 * one per Xilinx fpga) defines which io port pins are connected to the various
 * signals required to program a Xilinx fpga.
 *
 * A ram/flash "bank" refers to memory controlled by the same chip select.
 *
 * the eeprom contents are defined as in technical note #2 - basically,
 * a header, zero or more records in no particular order, and a 32 bit crc
 * a record is 1 or more type bytes, a length byte and "length" bytes.
 */

#define HYMOD_EEPROM_ID		0xAA	/* eeprom id byte */
#define HYMOD_EEPROM_VER	1	/* eeprom contents version (0-127) */
#define HYMOD_EEPROM_SIZE	256	/* number of bytes in the eeprom */

/* eeprom header */
typedef
    struct {
	unsigned char id;		/* eeprom id byte */
	unsigned char :1;
	unsigned char ver:7;		/* eeprom contents version number */
	unsigned long len;		/* total # of bytes btw hdr and crc */
    }
hymod_eehdr_t;

/* maximum number of bytes available for eeprom data records */
#define HYMOD_EEPROM_MAXLEN	(HYMOD_EEPROM_SIZE \
					- sizeof (hymod_eehdr_t) \
					- sizeof (unsigned long))

/* eeprom data record */
typedef
    union {
	struct {
	    unsigned char topbit:1;
	    unsigned char type:7;
	    unsigned char len;
	    unsigned char data[1];	/* variable length */
	} small;
	struct {
	    unsigned short topbit:1;
	    unsigned short nxtbit:1;
	    unsigned short type:14;
	    unsigned short len;
	    unsigned char data[1];	/* variable length */
	} medium;
	struct {
	    unsigned long topbit:1;
	    unsigned long nxtbit:1;
	    unsigned long type:30;
	    unsigned long len;
	    unsigned char data[1];	/* variable length */
	} large;
    }
hymod_eerec_t;

#define HYMOD_EEOFF_MAIN	0x00	/* i2c addr offset for main eeprom */
#define HYMOD_EEOFF_MEZZ	0x04	/* i2c addr offset for mezz eepomr */

/* eeprom record types */
#define HYMOD_EEREC_SERNO	1	/* serial number */
#define HYMOD_EEREC_DATE	2	/* date */
#define HYMOD_EEREC_BATCH	3	/* batch id */
#define HYMOD_EEREC_TYPE	4	/* board type */
#define HYMOD_EEREC_REV		5	/* revision number */
#define HYMOD_EEREC_SDRAM	6	/* sdram sizes */
#define HYMOD_EEREC_FLASH	7	/* flash sizes */
#define HYMOD_EEREC_ZBT		8	/* zbt ram sizes */
#define HYMOD_EEREC_XLXTYP	9	/* Xilinx fpga types */
#define HYMOD_EEREC_XLXSPD	10	/* Xilinx fpga speeds */
#define HYMOD_EEREC_XLXTMP	11	/* Xilinx fpga temperatures */
#define HYMOD_EEREC_XLXGRD	12	/* Xilinx fpga grades */
#define HYMOD_EEREC_CPUTYP	13	/* Motorola CPU type */
#define HYMOD_EEREC_CPUSPD	14	/* CPU speed */
#define HYMOD_EEREC_BUSSPD	15	/* bus speed */
#define HYMOD_EEREC_CPMSPD	16	/* CPM speed */
#define HYMOD_EEREC_HSTYPE	17	/* high-speed serial chip type */
#define HYMOD_EEREC_HSCHIN	18	/* high-speed serial input channels */
#define HYMOD_EEREC_HSCHOUT	19	/* high-speed serial output channels */

/* some dimensions */
#define HYMOD_MAX_BATCH		32	/* max no. of bytes in batch id */
#define HYMOD_MAX_SDRAM		4	/* max sdram "banks" on any board */
#define HYMOD_MAX_FLASH		4	/* max flash "banks" on any board */
#define HYMOD_MAX_ZBT		16	/* max ZBT rams on any board */
#define HYMOD_MAX_XLX		4	/* max Xilinx fpgas on any board */

#define HYMOD_MAX_BYTES		16	/* enough to store any bytes array */

/* board types */
#define HYMOD_BDTYPE_NONE	0	/* information not present */
#define HYMOD_BDTYPE_IO		1	/* I/O main board */
#define HYMOD_BDTYPE_CLP	2	/* CLP main board */
#define HYMOD_BDTYPE_DSP	3	/* DSP main board */
#define HYMOD_BDTYPE_INPUT	4	/* video input mezzanine board */
#define HYMOD_BDTYPE_ALTINPUT	5	/* video input mezzanine board */
#define HYMOD_BDTYPE_DISPLAY	6	/* video display mezzanine board */
#define HYMOD_BDTYPE_MAX	7	/* first invalid value */

/* Xilinx fpga types */
#define HYMOD_XTYP_NONE		0	/* information not present */
#define HYMOD_XTYP_XCV300E	1	/* Xilinx Virtex 300 */
#define HYMOD_XTYP_XCV400E	2	/* Xilinx Virtex 400 */
#define HYMOD_XTYP_XCV600E	3	/* Xilinx Virtex 600 */
#define HYMOD_XTYP_MAX		4	/* first invalid value */

/* Xilinx fpga speeds */
#define HYMOD_XSPD_NONE		0	/* information not present */
#define HYMOD_XSPD_SIX		1
#define HYMOD_XSPD_SEVEN	2
#define HYMOD_XSPD_EIGHT	3
#define HYMOD_XSPD_MAX		4	/* first invalid value */

/* Xilinx fpga temperatures */
#define HYMOD_XTMP_NONE		0	/* information not present */
#define HYMOD_XTMP_COM		1
#define HYMOD_XTMP_IND		2
#define HYMOD_XTMP_MAX		3	/* first invalid value */

/* Xilinx fpga grades */
#define HYMOD_XTMP_NONE		0	/* information not present */
#define HYMOD_XTMP_NORMAL	1
#define HYMOD_XTMP_ENGSAMP	2
#define HYMOD_XTMP_MAX		3	/* first invalid value */

/* CPU types */
#define HYMOD_CPUTYPE_NONE	0	/* information not present */
#define HYMOD_CPUTYPE_MPC8260	1	/* Motorola MPC8260 embedded powerpc */
#define HYMOD_CPUTYPE_MAX	2	/* first invalid value */

/* CPU/BUS/CPM clock speeds */
#define HYMOD_CLKSPD_NONE	0	/* information not present */
#define HYMOD_CLKSPD_33MHZ	1
#define HYMOD_CLKSPD_66MHZ	2
#define HYMOD_CLKSPD_100MHZ	3
#define HYMOD_CLKSPD_133MHZ	4
#define HYMOD_CLKSPD_166MHZ	5
#define HYMOD_CLKSPD_200MHZ	6
#define HYMOD_CLKSPD_MAX	7	/* first invalid value */

/* high speed serial chip types */
#define HYMOD_HSSTYPE_NONE	0	/* information not present */
#define HYMOD_HSSTYPE_AMCC52064	1
#define HYMOD_HSSTYPE_MAX	2	/* first invalid value */

/* a date (yyyy-mm-dd) */
typedef
    struct {
	unsigned short year;
	unsigned char month;
	unsigned char day;
    }
hymod_date_t;

/* describes a Xilinx fpga */
typedef
    struct {
	unsigned char type;		/* chip type */
	unsigned char speed;		/* chip speed rating */
	unsigned char temp;		/* chip temperature rating */
	unsigned char grade;		/* chip grade */
    }
hymod_xlx_t;

/* describes a Motorola embedded processor */
typedef
    struct {
	unsigned char type;		/* CPU type */
	unsigned char cpuspd;		/* speed of the PowerPC core */
	unsigned char busspd;		/* speed of the system and 60x bus */
	unsigned char cpmspd;		/* speed of the CPM co-processor */
    }
hymod_mpc_t;

/* info about high-speed (1Gbit) serial interface */
typedef
    struct {
	unsigned char type;		/* high-speed serial chip type */
	unsigned char nchin;		/* number of input channels mounted */
	unsigned char nchout;		/* number of output channels mounted */
    }
hymod_hss_t;

/*
 * this defines the contents of the serial eeprom that exists on every
 * hymod board, including mezzanine boards (the serial eeprom will be
 * faked for early development boards that don't have one)
 */

typedef
    struct {
	unsigned char valid:1;		/* contents of this struct is valid */
	unsigned char ver:7;		/* eeprom contents version */
	unsigned char bdtype;		/* board type */
	unsigned char bdrev;		/* board revision */
	unsigned char batchlen;		/* length of batch string below */
	unsigned long serno;		/* serial number */
	hymod_date_t date;		/* manufacture date */
	unsigned char batch[32];	/* manufacturer specific batch id */
	unsigned char nsdram;		/* # of ram "banks" */
	unsigned char nflash;		/* # of flash "banks" */
	unsigned char nzbt;		/* # of ZBT rams */
	unsigned char nxlx;		/* # of Xilinx fpgas */
	unsigned char sdramsz[HYMOD_MAX_SDRAM];	/* log2 of sdram size */
	unsigned char flashsz[HYMOD_MAX_FLASH];	/* log2 of flash size */
	unsigned char zbtsz[HYMOD_MAX_ZBT];	/* log2 of ZBT ram size */
	hymod_xlx_t xlx[HYMOD_MAX_XLX];	/* Xilinx fpga info */
	hymod_mpc_t mpc;		/* Motorola MPC CPU info */
	hymod_hss_t hss;		/* high-speed serial info */
    }
hymod_eeprom_t;

/*
 * this defines a region in the processor's physical address space
 */
typedef
    struct {
	unsigned long exists:1;		/* 1 if the region exists, 0 if not */
	unsigned long size:31;		/* size in bytes */
	unsigned long base;		/* base address */
    }
xlx_prgn_t;

/*
 * this defines where the various Xilinx fpga access regions are mapped
 * into the physical address space of the processor
 */
typedef
    struct {
	xlx_prgn_t prog;		/* program access region */
	xlx_prgn_t reg;			/* register access region */
	xlx_prgn_t port;		/* port access region */
    }
xlx_mmap_t;

/*
 * this defines which 8260 i/o port pins are connected to the various
 * signals required for programming a Xilinx fpga
 */
typedef
    struct {
	iopin_t prog_pin;		/* assert for >= 300ns to program */
	iopin_t init_pin;		/* goes high when fpga is cleared */
	iopin_t done_pin;		/* goes high when program is done */
	iopin_t enable_pin;		/* some fpgas need enabling */
    }
xlx_iopins_t;

/* all info about one Xilinx chip */
typedef
    struct {
	xlx_mmap_t mmap;
	xlx_iopins_t iopins;
	unsigned long irq:8;		/* h/w intr req number for this fpga */
    }
xlx_info_t;

/* all info about one hymod board */
typedef
    struct {
	hymod_eeprom_t eeprom;
	xlx_info_t xlx[HYMOD_MAX_XLX];
    }
hymod_board_t;

/*
 * this defines the configuration information of a hymod board-set
 * (main board + possible mezzanine board). In future, there may be
 * more than one mezzanine board (stackable?) - if so, add a "mezz2"
 * field, and so on... or make mezz an array?
 */
typedef
    struct {
	unsigned long ver:8;		/* version control */
	hymod_board_t main;		/* main board info */
	hymod_board_t mezz;		/* mezzanine board info */
	unsigned long crc;		/* ensures kernel and boot prom agree */
    }
hymod_conf_t;

#endif /* _HYMOD_H_ */
