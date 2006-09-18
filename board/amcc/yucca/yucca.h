/*
 * (C) Copyright 2006
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

#ifndef __YUCCA_H_
#define __YUCCA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------+
| Defines
+----------------------------------------------------------------------------*/

#define TMR_FREQ_EXT		25000000
#define BOARD_UART_CLOCK	11059200

#define BOARD_OPTION_SELECTED		1
#define BOARD_OPTION_NOT_SELECTED	0

#define ENGINEERING_CLOCK_CHECKING "clk_chk"
#define ENGINEERING_EXTERNAL_CLOCK "ext_clk"

#define ENGINEERING_CLOCK_CHECKING_DATA	1
#define ENGINEERING_EXTERNAL_CLOCK_DATA	2

/* ethernet definition */
#define MAX_ENETMODE_PARM	3
#define ENETMODE_NEG		0
#define ENETMODE_SPEED		1
#define ENETMODE_DUPLEX		2

#define ENETMODE_AUTONEG	0
#define ENETMODE_NO_AUTONEG	1
#define ENETMODE_10		2
#define ENETMODE_100		3
#define ENETMODE_1000		4
#define ENETMODE_HALF		5
#define ENETMODE_FULL		6

#define NUM_TLB_ENTRIES          64

/* MICRON SPD JEDEC ID Code (first byte) - SPD data byte [64] */
#define MICRON_SPD_JEDEC_ID 0x2c

/*----------------------------------------------------------------------------+
| TLB specific defines.
+----------------------------------------------------------------------------*/
#define TLB_256MB_ALIGN_MASK	0xF0000000
#define TLB_16MB_ALIGN_MASK	0xFF000000
#define TLB_1MB_ALIGN_MASK	0xFFF00000
#define TLB_256KB_ALIGN_MASK	0xFFFC0000
#define TLB_64KB_ALIGN_MASK	0xFFFF0000
#define TLB_16KB_ALIGN_MASK	0xFFFFC000
#define TLB_4KB_ALIGN_MASK	0xFFFFF000
#define TLB_1KB_ALIGN_MASK	0xFFFFFC00
#define TLB_256MB_SIZE		0x10000000
#define TLB_16MB_SIZE		0x01000000
#define TLB_1MB_SIZE		0x00100000
#define TLB_256KB_SIZE		0x00040000
#define TLB_64KB_SIZE		0x00010000
#define TLB_16KB_SIZE		0x00004000
#define TLB_4KB_SIZE		0x00001000
#define TLB_1KB_SIZE		0x00000400

#define TLB_WORD0_EPN_MASK	0xFFFFFC00
#define TLB_WORD0_EPN_ENCODE(n) (((unsigned long)(n))&0xFFFFFC00)
#define TLB_WORD0_EPN_DECODE(n) (((unsigned long)(n))&0xFFFFFC00)
#define TLB_WORD0_V_MASK	0x00000200
#define TLB_WORD0_V_ENABLE	0x00000200
#define TLB_WORD0_V_DISABLE	0x00000000
#define TLB_WORD0_TS_MASK	0x00000100
#define TLB_WORD0_TS_1		0x00000100
#define TLB_WORD0_TS_0		0x00000000
#define TLB_WORD0_SIZE_MASK	0x000000F0
#define TLB_WORD0_SIZE_1KB	0x00000000
#define TLB_WORD0_SIZE_4KB	0x00000010
#define TLB_WORD0_SIZE_16KB	0x00000020
#define TLB_WORD0_SIZE_64KB	0x00000030
#define TLB_WORD0_SIZE_256KB	0x00000040
#define TLB_WORD0_SIZE_1MB	0x00000050
#define TLB_WORD0_SIZE_16MB	0x00000070
#define TLB_WORD0_SIZE_256MB	0x00000090
#define TLB_WORD0_TPAR_MASK	0x0000000F
#define TLB_WORD0_TPAR_ENCODE(n) ((((unsigned long)(n))&0x0F)<<0)
#define TLB_WORD0_TPAR_DECODE(n) ((((unsigned long)(n))>>0)&0x0F)

#define TLB_WORD1_RPN_MASK	0xFFFFFC00
#define TLB_WORD1_RPN_ENCODE(n) (((unsigned long)(n))&0xFFFFFC00)
#define TLB_WORD1_RPN_DECODE(n) (((unsigned long)(n))&0xFFFFFC00)
#define TLB_WORD1_PAR1_MASK	0x00000300
#define TLB_WORD1_PAR1_ENCODE(n) ((((unsigned long)(n))&0x03)<<8)
#define TLB_WORD1_PAR1_DECODE(n) ((((unsigned long)(n))>>8)&0x03)
#define TLB_WORD1_PAR1_0	0x00000000
#define TLB_WORD1_PAR1_1	0x00000100
#define TLB_WORD1_PAR1_2	0x00000200
#define TLB_WORD1_PAR1_3	0x00000300
#define TLB_WORD1_ERPN_MASK	0x0000000F
#define TLB_WORD1_ERPN_ENCODE(n) ((((unsigned long)(n))&0x0F)<<0)
#define TLB_WORD1_ERPN_DECODE(n) ((((unsigned long)(n))>>0)&0x0F)

#define TLB_WORD2_PAR2_MASK	0xC0000000
#define TLB_WORD2_PAR2_ENCODE(n) ((((unsigned long)(n))&0x03)<<30)
#define TLB_WORD2_PAR2_DECODE(n) ((((unsigned long)(n))>>30)&0x03)
#define TLB_WORD2_PAR2_0	0x00000000
#define TLB_WORD2_PAR2_1	0x40000000
#define TLB_WORD2_PAR2_2	0x80000000
#define TLB_WORD2_PAR2_3	0xC0000000
#define TLB_WORD2_U0_MASK	0x00008000
#define TLB_WORD2_U0_ENABLE	0x00008000
#define TLB_WORD2_U0_DISABLE	0x00000000
#define TLB_WORD2_U1_MASK	0x00004000
#define TLB_WORD2_U1_ENABLE	0x00004000
#define TLB_WORD2_U1_DISABLE	0x00000000
#define TLB_WORD2_U2_MASK	0x00002000
#define TLB_WORD2_U2_ENABLE	0x00002000
#define TLB_WORD2_U2_DISABLE	0x00000000
#define TLB_WORD2_U3_MASK	0x00001000
#define TLB_WORD2_U3_ENABLE	0x00001000
#define TLB_WORD2_U3_DISABLE	0x00000000
#define TLB_WORD2_W_MASK	0x00000800
#define TLB_WORD2_W_ENABLE	0x00000800
#define TLB_WORD2_W_DISABLE	0x00000000
#define TLB_WORD2_I_MASK	0x00000400
#define TLB_WORD2_I_ENABLE	0x00000400
#define TLB_WORD2_I_DISABLE	0x00000000
#define TLB_WORD2_M_MASK	0x00000200
#define TLB_WORD2_M_ENABLE	0x00000200
#define TLB_WORD2_M_DISABLE	0x00000000
#define TLB_WORD2_G_MASK	0x00000100
#define TLB_WORD2_G_ENABLE	0x00000100
#define TLB_WORD2_G_DISABLE	0x00000000
#define TLB_WORD2_E_MASK	0x00000080
#define TLB_WORD2_E_ENABLE	0x00000080
#define TLB_WORD2_E_DISABLE	0x00000000
#define TLB_WORD2_UX_MASK	0x00000020
#define TLB_WORD2_UX_ENABLE	0x00000020
#define TLB_WORD2_UX_DISABLE	0x00000000
#define TLB_WORD2_UW_MASK	0x00000010
#define TLB_WORD2_UW_ENABLE	0x00000010
#define TLB_WORD2_UW_DISABLE	0x00000000
#define TLB_WORD2_UR_MASK	0x00000008
#define TLB_WORD2_UR_ENABLE	0x00000008
#define TLB_WORD2_UR_DISABLE	0x00000000
#define TLB_WORD2_SX_MASK	0x00000004
#define TLB_WORD2_SX_ENABLE	0x00000004
#define TLB_WORD2_SX_DISABLE	0x00000000
#define TLB_WORD2_SW_MASK	0x00000002
#define TLB_WORD2_SW_ENABLE	0x00000002
#define TLB_WORD2_SW_DISABLE	0x00000000
#define TLB_WORD2_SR_MASK	0x00000001
#define TLB_WORD2_SR_ENABLE	0x00000001
#define TLB_WORD2_SR_DISABLE	0x00000000

/*----------------------------------------------------------------------------+
| Board specific defines.
+----------------------------------------------------------------------------*/
#define NONCACHE_MEMORY_SIZE     (64*1024)
#define NONCACHE_AREA0_ENDOFFSET (64*1024)
#define NONCACHE_AREA1_ENDOFFSET (32*1024)

#define FLASH_SECTORSIZE	0x00010000

/* SDRAM MICRON */
#define SDRAM_MICRON		0x2C

#define SDRAM_TRUE		1
#define SDRAM_FALSE		0
#define SDRAM_DDR1		1
#define SDRAM_DDR2		2
#define SDRAM_NONE		0
#define MAXDIMMS		2		/* Changes le 12/01/05 pour 1.6 */
#define MAXRANKS		4		/* Changes le 12/01/05 pour 1.6 */
#define MAXBANKSPERDIMM		2
#define MAXRANKSPERDIMM		2
#define MAXBXCF			4		/* Changes le 12/01/05 pour 1.6 */
#define MAXSDRAMMEMORY		0xFFFFFFFF	/* 4GB */
#define ERROR_STR_LENGTH	256
#define MAX_SPD_BYTES		256		/* Max number of bytes on the DIMM's SPD EEPROM */

/*----------------------------------------------------------------------------+
| SDR Configuration registers
+----------------------------------------------------------------------------*/
/* Serial Device Strap Reg 0 */
#define sdr_pstrp0	0x0040

#define	SDR0_SDSTP1_EBC_ROM_BS_MASK	0x00000080 /* EBC Boot bus width Mask */
#define	SDR0_SDSTP1_EBC_ROM_BS_16BIT	0x00000080 /* EBC 16 Bits */
#define	SDR0_SDSTP1_EBC_ROM_BS_8BIT	0x00000000 /* EBC  8 Bits */

#define	SDR0_SDSTP1_BOOT_SEL_MASK	0x00080000 /* Boot device Selection Mask */
#define	SDR0_SDSTP1_BOOT_SEL_EBC	0x00000000 /* EBC */
#define	SDR0_SDSTP1_BOOT_SEL_PCI	0x00080000 /* PCI */

#define	SDR0_SDSTP1_EBC_SIZE_MASK	0x00000060 /* Boot rom size Mask */
#define	SDR0_SDSTP1_BOOT_SIZE_16MB	0x00000060 /* 16 MB */
#define	SDR0_SDSTP1_BOOT_SIZE_8MB	0x00000040 /*  8 MB */
#define	SDR0_SDSTP1_BOOT_SIZE_4MB	0x00000020 /*  4 MB */
#define	SDR0_SDSTP1_BOOT_SIZE_2MB	0x00000000 /*  2 MB */

/* Serial Device Enabled - Addr = 0xA8 */
#define SDR0_PSTRP0_BOOTSTRAP_IIC_A8_EN SDR0_PSTRP0_BOOTSTRAP_SETTINGS5
/* Serial Device Enabled - Addr = 0xA4 */
#define SDR0_PSTRP0_BOOTSTRAP_IIC_A4_EN SDR0_PSTRP0_BOOTSTRAP_SETTINGS7

/* Pin Straps Reg */
#define SDR0_PSTRP0			0x0040
#define SDR0_PSTRP0_BOOTSTRAP_MASK	0xE0000000  /* Strap Bits */

#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS0	0x00000000  /* Default strap settings 0 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS1	0x20000000  /* Default strap settings 1 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS2	0x40000000  /* Default strap settings 2 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS3	0x60000000  /* Default strap settings 3 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS4	0x80000000  /* Default strap settings 4 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS5	0xA0000000  /* Default strap settings 5 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS6	0xC0000000  /* Default strap settings 6 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS7	0xE0000000  /* Default strap settings 7 */

/* fpgareg - defines are in include/config/YUCCA.h */

#define SDR0_CUST0_ENET3_MASK		0x00000080
#define SDR0_CUST0_ENET3_COPPER		0x00000000
#define SDR0_CUST0_ENET3_FIBER		0x00000080
#define SDR0_CUST0_RGMII3_MASK		0x00000070
#define SDR0_CUST0_RGMII3_ENCODE(n)	((((unsigned long)(n))&0x7)<<4)
#define SDR0_CUST0_RGMII3_DECODE(n)	((((unsigned long)(n))>>4)&0x07)
#define SDR0_CUST0_RGMII3_DISAB		0x00000000
#define SDR0_CUST0_RGMII3_RTBI		0x00000040
#define SDR0_CUST0_RGMII3_RGMII		0x00000050
#define SDR0_CUST0_RGMII3_TBI		0x00000060
#define SDR0_CUST0_RGMII3_GMII		0x00000070
#define SDR0_CUST0_ENET2_MASK		0x00000008
#define SDR0_CUST0_ENET2_COPPER		0x00000000
#define SDR0_CUST0_ENET2_FIBER		0x00000008
#define SDR0_CUST0_RGMII2_MASK		0x00000007
#define SDR0_CUST0_RGMII2_ENCODE(n)	((((unsigned long)(n))&0x7)<<0)
#define SDR0_CUST0_RGMII2_DECODE(n)	((((unsigned long)(n))>>0)&0x07)
#define SDR0_CUST0_RGMII2_DISAB		0x00000000
#define SDR0_CUST0_RGMII2_RTBI		0x00000004
#define SDR0_CUST0_RGMII2_RGMII		0x00000005
#define SDR0_CUST0_RGMII2_TBI		0x00000006
#define SDR0_CUST0_RGMII2_GMII		0x00000007

#define ONE_MILLION			1000000
#define ONE_BILLION			1000000000

/*----------------------------------------------------------------------------+
|                               X
|                              XX
| XX  XXX   XXXXX   XX XXX    XXXXX
| XX  XX        X    XXX XX    XX
| XX  XX   XXXXXX    XX        XX
| XX  XX   X   XX    XX        XX XX
|  XXX XX  XXXXX X  XXXX        XXX
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
| Declare Configuration values
+----------------------------------------------------------------------------*/

typedef enum config_selection {
	CONFIG_NOT_SELECTED,
	CONFIG_SELECTED
} config_selection_t;

typedef enum config_list {
	UART2_IN_SERVICE_MODE,
	CPU_TRACE_MODE,
	UART1_CTS_RTS,
	CONFIG_NB
} config_list_t;

#define MAX_CONFIG_SELECT_NB			3

#define BOARD_INFO_UART2_IN_SERVICE_MODE	1
#define BOARD_INFO_CPU_TRACE_MODE		2
#define BOARD_INFO_UART1_CTS_RTS_MODE		4

void force_bup_config_selection(config_selection_t *confgi_select_P);
void update_config_selection_table(config_selection_t *config_select_P);
void display_config_selection(config_selection_t *config_select_P);

/*----------------------------------------------------------------------------+
|                     XX
|
|   XXXX    XX XXX   XXX     XXXX
|  XX        XX  XX   XX    XX  XX
|  XX  XXX   XX  XX   XX    XX  XX
|  XX  XX    XXXXX    XX    XX  XX
|   XXXX     XX      XXXX    XXXX
|           XXXX
|
|
|
| +------------------------------------------------------------------+
| |  GPIO/Secondary func | Primary Function | I/O | Alternate1 | I/O |
| +----------------------+------------------+-----+------------+-----+
| |                      |                  |     |            |     |
| | GPIO0_0              | PCIX0REQ2_N      | I/O |  TRCCLK    |     |
| | GPIO0_1              | PCIX0REQ3_N      | I/O |  TRCBS0    |     |
| | GPIO0_2              | PCIX0GNT2_N      | I/O |  TRCBS1    |     |
| | GPIO0_3              | PCIX0GNT3_N      | I/O |  TRCBS2    |     |
| | GPIO0_4              | PCIX1REQ2_N      | I/O |  TRCES0    |     |
| | GPIO0_5              | PCIX1REQ3_N      | I/O |  TRCES1    |     |
| | GPIO0_6              | PCIX1GNT2_N      | I/O |  TRCES2    | NA  |
| | GPIO0_7              | PCIX1GNT3_N      | I/O |  TRCES3    | NA  |
| | GPIO0_8              | PERREADY         |  I  |  TRCES4    | NA  |
| | GPIO0_9              | PERCS1_N         |  O  |  TRCTS0    | NA  |
| | GPIO0_10             | PERCS2_N         |  O  |  TRCTS1    | NA  |
| | GPIO0_11             | IRQ0             |  I  |  TRCTS2    | NA  |
| | GPIO0_12             | IRQ1             |  I  |  TRCTS3    | NA  |
| | GPIO0_13             | IRQ2             |  I  |  TRCTS4    | NA  |
| | GPIO0_14             | IRQ3             |  I  |  TRCTS5    | NA  |
| | GPIO0_15             | IRQ4             |  I  |  TRCTS6    | NA  |
| | GPIO0_16             | IRQ5             |  I  |  UART2RX   |  I  |
| | GPIO0_17             | PERBE0_N         |  O  |  UART2TX   |  O  |
| | GPIO0_18             | PCI0GNT0_N       | I/O |  NA        | NA  |
| | GPIO0_19             | PCI0GNT1_N       | I/O |  NA        | NA  |
| | GPIO0_20             | PCI0REQ0_N       | I/O |  NA        | NA  |
| | GPIO0_21             | PCI0REQ1_N       | I/O |  NA        | NA  |
| | GPIO0_22             | PCI1GNT0_N       | I/O |  NA        | NA  |
| | GPIO0_23             | PCI1GNT1_N       | I/O |  NA        | NA  |
| | GPIO0_24             | PCI1REQ0_N       | I/O |  NA        | NA  |
| | GPIO0_25             | PCI1REQ1_N       | I/O |  NA        | NA  |
| | GPIO0_26             | PCI2GNT0_N       | I/O |  NA        | NA  |
| | GPIO0_27             | PCI2GNT1_N       | I/O |  NA        | NA  |
| | GPIO0_28             | PCI2REQ0_N       | I/O |  NA        | NA  |
| | GPIO0_29             | PCI2REQ1_N       | I/O |  NA        | NA  |
| | GPIO0_30             | UART1RX          |  I  |  NA        | NA  |
| | GPIO0_31             | UART1TX          |  O  |  NA        | NA  |
| |                      |                  |     |            |     |
| +----------------------+------------------+-----+------------+-----+
|
+----------------------------------------------------------------------------*/

unsigned long auto_calc_speed(void);
/*----------------------------------------------------------------------------+
| Prototypes
+----------------------------------------------------------------------------*/
void print_evb440spe_info(void);

int onboard_pci_arbiter_selected(int core_pci);

#ifdef __cplusplus
}
#endif
#endif /* __YUCCA_H_ */
