/*
 * ap1000.h: AP1000 (e.g. AP1070, AP1100) board specific definitions and functions that are needed globally
 *
 * Author : James MacAulay
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License (GPL) version 2, incorporated herein by
 * reference. Drivers based on or derived from this code fall under the GPL
 * and must retain the authorship, copyright and this license notice. This
 * file is not a complete program and may only be used when the entire
 * program is licensed under the GPL.
 *
 */

#ifndef __AP1000_H
#define __AP1000_H

/*
 *  Revision Register stuff
 */
#define AP1xx_FPGA_REV_ADDR 0x29000000

#define AP1xx_PLATFORM_MASK	 0xFF000000
#define AP100_BASELINE_PLATFORM	 0x01000000
#define AP1xx_QUADGE_PLATFORM	 0x02000000
#define AP1xx_MGT_REF_PLATFORM	 0x03000000
#define AP1xx_STANDARD_PLATFORM	 0x04000000
#define AP1xx_DUAL_PLATFORM	 0x05000000
#define AP1xx_BASE_SRAM_PLATFORM 0x06000000

#define AP1000_BASELINE_PLATFORM 0x21000000

#define AP1xx_TESTPLATFORM_MASK		0xC0000000
#define AP1xx_PCI_PCB_TESTPLATFORM	0xC0000000
#define AP1xx_DUAL_GE_MEZZ_TESTPLATFORM 0xC1000000
#define AP1xx_SFP_MEZZ_TESTPLATFORM	0xC2000000

#define AP1000_PCI_PCB_TESTPLATFORM	 0xC3000000

#define AP1xx_TARGET_MASK  0x00FF0000
#define AP1xx_AP107_TARGET 0x00010000
#define AP1xx_AP120_TARGET 0x00020000
#define AP1xx_AP130_TARGET 0x00030000
#define AP1xx_AP1070_TARGET 0x00040000
#define AP1xx_AP1100_TARGET 0x00050000

#define AP1xx_UNKNOWN_STR "Unknown"

#define AP1xx_PLATFORM_STR	     " Platform"
#define AP1xx_BASELINE_PLATFORM_STR  "Baseline"
#define AP1xx_QUADGE_PLATFORM_STR    "Quad GE"
#define AP1xx_MGT_REF_PLATFORM_STR   "MGT Reference"
#define AP1xx_STANDARD_PLATFORM_STR  "Standard"
#define AP1xx_DUAL_PLATFORM_STR	     "Dual"
#define AP1xx_BASE_SRAM_PLATFORM_STR "Baseline with SRAM"

#define AP1xx_TESTPLATFORM_STR		    " Test Platform"
#define AP1xx_PCI_PCB_TESTPLATFORM_STR	    "Base"
#define AP1xx_DUAL_GE_MEZZ_TESTPLATFORM_STR "Dual GE Mezzanine"
#define AP1xx_SFP_MEZZ_TESTPLATFORM_STR	    "SFP Mezzanine"

#define AP1xx_TARGET_STR       " Board"
#define AP1xx_AP107_TARGET_STR "AP107"
#define AP1xx_AP120_TARGET_STR "AP120"
#define AP1xx_AP130_TARGET_STR "AP130"

#define AP1xx_AP1070_TARGET_STR "AP1070"
#define AP1xx_AP1100_TARGET_STR "AP1100"

/*
 *  Flash Stuff
 */
#define AP1xx_PROGRAM_FLASH_INDEX   0
#define AP1xx_CONFIG_FLASH_INDEX    1

/*
 *  System Ace Stuff
 */
#define AP1000_SYSACE_REGBASE  0x28000000

#define SYSACE_STATREG0 0x04 /* 7:0 */
#define SYSACE_STATREG1 0x05 /* 15:8 */
#define SYSACE_STATREG2 0x06 /* 23:16 */
#define SYSACE_STATREG3 0x07 /* 31:24 */

#define SYSACE_ERRREG0 0x08 /* 7:0 */
#define SYSACE_ERRREG1 0x09 /* 15:8 */
#define SYSACE_ERRREG2 0x0a /* 23:16 */
#define SYSACE_ERRREG3 0x0b /* 31:24 */

#define SYSACE_CTRLREG0 0x18 /* 7:0 */
#define SYSACE_CTRLREG1 0x19 /* 15:8 */
#define SYSACE_CTRLREG2 0x1A /* 23:16 */
#define SYSACE_CTRLREG3 0x1B /* 31:24 */

/*
 *  Software reconfig thing
 */
#define SW_BYTE_SECTOR_ADDR	0x24FE0000
#define SW_BYTE_SECTOR_OFFSET	0x0001FFFF
#define SW_BYTE_SECTOR_SIZE	0x00020000
#define SW_BYTE_MASK		0x00000003

#define DEFAULT_TEMP_ADDR	0x00100000

#define AP1000_CPLD_BASE	0x26000000

/* PowerSpan II Stuff */
#define PSII_SYNC() asm("eieio")
#define PSPAN_BASEADDR 0x30000000
#define EEPROM_DEFAULT { 0x01,	     /* Byte 0 - Long Load = 0x02, short = 01, use 0xff for try no load */  \
			0x0,0x0,0x0, /* Bytes 1 - 3 Power span reserved */ \
			0x0,	     /* Byte 4 - Powerspan reserved  - start of short load */ \
			0x0F,	     /* Byte 5 - Enable PCI 1 & 2 as Bus masters and Memory targets. */ \
			0x0E,	     /* Byte 6 - PCI 1 Target image prefetch - on for image 0,1,2, off for i20 & 3. */ \
			0x00, 0x00,  /* Byte 7,8 - PCI-1 Subsystem ID - */ \
			0x00, 0x00,  /* Byte 9,10 - PCI-1 Subsystem Vendor Id -	 */ \
			0x00,	     /* Byte 11 - No PCI interrupt generation on PCI-1 PCI-2 int A */ \
			0x1F,	     /* Byte 12 - PCI-1 enable bridge registers, all target images */ \
			0xBA,	     /* Byte 13 - Target 0 image 128 Meg(Ram), Target 1 image 64 Meg. (config Flash/CPLD )*/ \
			0xA0,	     /* Byte 14 - Target 2 image 64 Meg(program Flash), target 3 64k. */ \
			0x00,	     /* Byte 15 - Vital Product Data Disabled. */ \
			0x88,	     /* Byte 16 - PCI arbiter config complete, all requests routed through PCI-1, Unlock PCI-1	*/ \
			0x40,	     /* Byte 17 - Interrupt direction control - PCI-1 Int A out, everything else in. */ \
			0x00,	     /* Byte 18 - I2O disabled */ \
			0x00,	     /* Byte 19 - PCI-2 Target image prefetch - off for all images. */ \
			0x00,0x00,   /* Bytes 20,21 - PCI 2 Subsystem Id */ \
			0x00,0x00,   /* Bytes 22,23 - PCI 2 Subsystem Vendor id */ \
			0x0C,	     /* Byte 24 - PCI-2 BAR enables, target image 0, & 1 */ \
			0xBB,	     /* Byte 25 - PCI-2 target 0 - 128 Meg(Ram), target 1  - 128 Meg (program/config flash) */ \
			0x00,	     /* Byte 26 - PCI-2 target 2 & 3 unused. */ \
			0x00,0x00,0x00,0x00,0x00, /* Bytes 27,28,29,30, 31 - Reserved */ \
			/* Long Load Information */ \
			0x82,0x60,   /* Bytes 32,33 - PCI-1 Device ID - Powerspan II */ \
			0x10,0xE3,   /* Bytes 24,35 - PCI-1 Vendor ID - Tundra */ \
			0x06,	     /* Byte 36 - PCI-1 Class Base - Bridge device. */ \
			0x80,	     /* Byte 37 - PCI-1 Class sub class - Other bridge. */ \
			0x00,	     /* Byte 38 - PCI-1 Class programing interface - Other bridge */ \
			0x01,	     /* Byte 39 - Power span revision 1. */ \
			0x6E,	     /* Byte 40 - PB SI0 enabled, translation enabled, decode enabled, 64 Meg */ \
			0x40,	     /* Byte 41 - PB SI0 memory command mode, PCI-1 dest */ \
			0x22,	     /* Byte 42 - Prefetch discard after read, PCI-little endian conversion, 32 byte prefetch */ \
			0x00,0x00,   /* Bytes 43, 44 - Translation address for SI0, set to zero for now. */ \
			0x0E,	     /* Byte 45 - Translation address (0) and PB bus master enables - all. */ \
			0x2c,00,00,  /* Bytes 46,47,48 - PB SI0 processor base address - 0x2C000000 */ \
			0x30,00,00,  /* Bytes 49,50,51 - PB Address for Powerspan registers - 0x30000000, big Endian */ \
			0x82,0x60,   /* Bytes 52, 53 - PCI-2 Device ID - Powerspan II */ \
			0x10,0xE3,   /* Bytes 54,55 - PCI 2 Vendor Id - Tundra */ \
			0x06,	     /* Byte 56 - PCI-2 Class Base - Bridge device */ \
			0x80,	     /* Byte 57 - PCI-2 Class sub class - Other Bridge. */ \
			0x00,	     /* Byte 58 - PCI-2 class programming interface - Other bridge */ \
			0x01,	     /* Byte 59 - PCI-2 class revision	1 */ \
			0x00,0x00,0x00,0x00 }; /* Bytes 60,61, 62, 63 - Powerspan reserved */


#define EEPROM_LENGTH	64  /* Long Load */

#define I2C_SENSOR_DEV	    0x9
#define I2C_SENSOR_CHIP_SEL 0x4

/*
 *  Board Functions
 */
void set_eat_machine_checks(int a_flag);
int get_eat_machine_checks(void);
unsigned int get_platform(void);
void* memcpyb(void * dest,const void *src,size_t count);
int process_bootflag(ulong bootflag);
void user_led_on(void);
void user_led_off(void);

#endif	/* __COMMON_H_ */
