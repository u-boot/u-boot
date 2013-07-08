/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _VPD_H_
#define _VPD_H_

/*
 * Main Flash Configuration.
 */
typedef struct flashCfg_s {
    unsigned short mfg;				/* Manufacture ID */
    unsigned short dev;				/* Device ID */
    unsigned char devWidth;			/* Device Width */
    unsigned char numDevs;			/* Number of devices */
    unsigned char numCols;			/* Number of columns */
    unsigned char colWidth;			/* Width of a column */
    unsigned char weDataWidth;			/* Write/Erase Data Width */
} flashCfg_t;

/*
 * Vital Product Data - VPD
 */
#define MAX_PROD_ID		15
#define MAX_ETH_ADDRS		10
typedef unsigned char EthAddr[6];
typedef struct vpd {
    unsigned char _devAddr;			/* Device address during read */
    char productId[MAX_PROD_ID];		/* Product ID */
    char revisionId;				/* Revision ID as a char */
    unsigned long serialNum;			/* Serial number */
    unsigned char  manuID;			/* Manufact ID - byte int */
    unsigned long configOpt;			/* Config Option - bit field */
    unsigned long sysClk;			/* System clock in Hertz */
    unsigned long serClk;			/* Ext. clock in Hertz */
    flashCfg_t flashCfg;			/* Flash configuration */
    unsigned long numPOTS;			/* Number of POTS lines */
    unsigned long numDS1;			/* Number of DS1 circuits */
    EthAddr ethAddrs[MAX_ETH_ADDRS];		/* Ethernet MAC, 1st = craft */
} VPD;


#define VPD_MAX_EEPROM_SIZE	512		/* Max size VPD EEPROM */
#define SDRAM_SPD_DATA_SIZE	128		/* Size SPD in VPD EEPROM */

/*
 * PIDs - Packet Identifiers
 */
#define VPD_PID_GI		0x0		/* Guaranted Illegal */
#define VPD_PID_PID		0x1		/* Product Identifier */
#define VPD_PID_REV		0x2		/* Product Revision */
#define VPD_PID_SN		0x3		/* Serial Number */
#define VPD_PID_MANID		0x4		/* Manufacture ID */
#define VPD_PID_PCO		0x5		/* Product configuration */
#define VPD_PID_SYSCLK		0x6		/* System Clock */
#define VPD_PID_SERCLK		0x7		/* Ser. Clk. Speed in Hertz */
#define VPD_PID_CRC		0x8		/* VPD CRC */
#define VPD_PID_FLASH		0x9		/* Flash Configuration */
#define VPD_PID_ETHADDR		0xA		/* Ethernet Address(es) */
#define VPD_PID_GAL		0xB		/* Galileo Switch Config */
#define VPD_PID_POTS		0xC		/* Number of POTS Lines */
#define VPD_PID_DS1		0xD		/* Number of DS1s */
#define VPD_PID_TERM		0xFF		/* Termination packet */

/*
 * VPD - Eyecatcher/Magic
 */
#define VPD_EYECATCHER		"W7O"
#define VPD_EYE_SIZE		3
typedef struct vpd_header {
    unsigned char eyecatcher[VPD_EYE_SIZE];	/* eyecatcher - "W7O" */
    unsigned short size __attribute__((packed)); /* size of EEPROM */
} vpd_header_t;


#define VPD_DATA_SIZE (VPD_MAX_EEPROM_SIZE - SDRAM_SPD_DATA_SIZE - \
			sizeof(vpd_header_t))
typedef struct vpd_s {
    vpd_header_t header;
    unsigned char packets[VPD_DATA_SIZE];
} vpd_t;

typedef struct vpd_packet {
    unsigned char identifier;
    unsigned char size;
    unsigned char data[1];
} vpd_packet_t;

/*
 * VPD configOpt bit mask
 */
#define VPD_HAS_BBRAM		0x1		/* Battery backed SRAM */
#define VPD_HAS_RTC		0x2		/* Battery backed RTC */
#define VPD_HAS_EXT_SER_CLK	0x4		/* External serial clock */
#define VPD_HAS_SER_TRANS_1	0x8		/* COM1 transceiver */
#define VPD_HAS_SER_TRANS_2	0x10		/* COM2 transceiver */
#define VPD_HAS_CRAFT_PHY	0x20		/* CRAFT Ethernet */
#define VPD_HAS_DTT_1		0x40		/* I2C Digital therm. #1 */
#define VPD_HAS_DTT_2		0x80		/* I2C Digital therm. #2 */
#define VPD_HAS_1000_UP_LASER	0x100		/* GMM - 1000Mbit Uplink */
#define VPD_HAS_70KM_UP_LASER	0x200		/* CMM - 70KM Uplink laser */
#define VPD_HAS_2_UPLINKS	0x400		/* CMM - 2 uplink lasers */
#define VPD_HAS_FPGA		0x800		/* Has 1 or more FPGAs */
#define VPD_HAS_DFA		0x1000		/* CLM - Has 2 Fiber Inter. */
#define VPD_HAS_GAL_SWITCH	0x2000		/* GMM - Has a Gal switch */
#define VPD_HAS_POTS_LINES	0x4000		/* GMM - Has POTS lines */
#define VPD_HAS_DS1_CHANNELS	0x8000		/* GMM - Has DS1 channels */
#define VPD_HAS_CABLE_RETURN	0x10000		/* GBM/GBR - Cable ret. path */

#define VPD_EEPROM_SIZE         (256 - SDRAM_SPD_DATA_SIZE) /* Size EEPROM */

extern int vpd_get_data(unsigned char dev_addr, VPD *vpd);
extern void vpd_print(VPD *vpdInfo);

#endif /* _VPD_H_ */
