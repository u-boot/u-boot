#ifndef __vpd_h
#define __vpd_h

/*
 * Module name: %M%
 * Description:
 * Vital Product Data (VPD) Header Module
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
 * History:
 * Date Who
 *
 * 10/24/96 Rob Baxter
 * Initial release.
 *
 */

#define VPD_EEPROM_SIZE 256 /* EEPROM size in bytes */

/*
 * packet tuple identifiers
 *
 * 0x0D - 0xBF reserved
 * 0xC0 - 0xFE user defined
 */
#define VPD_PID_GI   0x00 /* guaranteed illegal */
#define VPD_PID_PID  0x01 /* product identifier (ASCII) */
#define VPD_PID_FAN  0x02 /* factory assembly-number (ASCII) */
#define VPD_PID_SN   0x03 /* serial-number (ASCII) */
#define VPD_PID_PCO  0x04 /* product configuration options(binary) */
#define VPD_PID_ICS  0x05 /* internal clock speed in HZ (integer) */
#define VPD_PID_ECS  0x06 /* external clock speed in HZ (integer) */
#define VPD_PID_RCS  0x07 /* reference clock speed in HZ(integer) */
#define VPD_PID_EA   0x08 /* ethernet address (binary) */
#define VPD_PID_MT   0x09 /* microprocessor type (ASCII) */
#define VPD_PID_CRC  0x0A /* EEPROM CRC (integer) */
#define VPD_PID_FMC  0x0B /* FLASH memory configuration (binary) */
#define VPD_PID_VLSI 0x0C /* VLSI revisions/versions (binary) */
#define VPD_PID_TERM 0xFF /* termination */

/*
 * VPD structure (format)
 */
#define VPD_EYE_SIZE 8 /* eyecatcher size */
typedef struct vpd_header
{
	uchar eyecatcher[VPD_EYE_SIZE]; /* eyecatcher - "MOTOROLA" */
	ushort size; /* size of EEPROM */
} vpd_header_t;

#define VPD_DATA_SIZE (VPD_EEPROM_SIZE-sizeof(vpd_header_t))
typedef struct vpd
{
	vpd_header_t header; /* header */
	uchar packets[VPD_DATA_SIZE]; /* data */
} vpd_t;

/*
 * packet tuple structure (format)
 */
typedef struct vpd_packet
{
    uchar identifier; /* identifier (PIDs above) */
    uchar size;       /* size of the following data area */
    uchar data[1];    /* data (size is dependent upon PID) */
} vpd_packet_t;

/*
 * MBX product configuration options bit definitions
 *
 * Notes:
 * 1. The bit numbering is reversed in perspective with the C compiler.
 */
#define PCO_BBRAM    (1<<0)  /* battery-backed RAM (BBRAM) and socket */
#define PCO_BOOTROM  (1<<1)  /* boot ROM and socket (i.e., socketed FLASH) */
#define PCO_KAPWR    (1<<2)  /* keep alive power source (lithium battey) and control circuit */
#define PCO_ENET_TP  (1<<3)  /* ethernet twisted pair (TP) connector (RJ45) */
#define PCO_ENET_AUI (1<<4)  /* ethernet attachment unit interface (AUI) header */
#define PCO_PCMCIA   (1<<5)  /* PCMCIA socket */
#define PCO_DIMM     (1<<6)  /* DIMM module socket */
#define PCO_DTT      (1<<7)  /* digital thermometer and thermostat (DTT) device */
#define PCO_LCD      (1<<8)  /* liquid crystal display (LCD) device */
#define PCO_PCI      (1<<9)  /* PCI-Bus bridge device (QSpan) and ISA-Bus bridge device (Winbond) */
#define PCO_PCIO     (1<<10) /* PC I/O (COM1, COM2, FDC, LPT, Keyboard/Mouse) */
#define PCO_EIDE     (1<<11) /* enhanced IDE (EIDE) header */
#define PCO_FDC      (1<<12) /* floppy disk controller (FDC) header */
#define PCO_LPT_8XX  (1<<13) /* parallel port header via MPC8xx */
#define PCO_LPT_PCIO (1<<14) /* parallel port header via PC I/O */

/*
 * FLASH memory configuration packet data
 */
typedef struct vpd_fmc
{
    ushort mid; /* manufacturer's idenitfier */
    ushort did; /* manufacturer's device idenitfier */
    uchar ddw;  /* device data width (e.g., 8-bits, 16-bits) */
    uchar nod;  /* number of devices present */
    uchar noc;  /* number of columns */
    uchar cw;   /* column width in bits */
    uchar wedw; /* write/erase data width */
} vpd_fmc_t;

/* function prototypes */
extern void vpd_init(void);
extern int  vpd_read(uint iic_device, uchar *buf, int count, int offset);
extern      vpd_packet_t *vpd_find_packet(u_char ident);

#endif /* __vpd_h */
