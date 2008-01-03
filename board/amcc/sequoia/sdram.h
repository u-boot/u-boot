/*
 * (C) Copyright 2006
 * Sylvie Gohl,             AMCC/IBM, gohl.sylvie@fr.ibm.com
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Thierry Roman,           AMCC/IBM, thierry_roman@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 * Robert Snyder,           AMCC/IBM, rob.snyder@fr.ibm.com
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

#ifndef _SPD_SDRAM_DENALI_H_
#define _SPD_SDRAM_DENALI_H_

#define ppcMsync	sync
#define ppcMbar		eieio

/* General definitions */
#define MAX_SPD_BYTE        128         /* highest SPD byte # to read */
#define DENALI_REG_NUMBER   45          /* 45 Regs in PPC440EPx Denali Core */
#define SUPPORTED_DIMMS_NB  7           /* Number of supported DIMM modules types */
#define SDRAM_NONE          0           /* No DIMM detected in Slot */
#define MAXRANKS            2           /* 2 ranks maximum */

/* Supported PLB Frequencies */
#define PLB_FREQ_133MHZ     133333333
#define PLB_FREQ_152MHZ     152000000
#define PLB_FREQ_160MHZ     160000000
#define PLB_FREQ_166MHZ     166666666

/* Denali Core Registers */
#define SDRAM_DCR_BASE 0x10

#define DDR_DCR_BASE 0x10
#define ddrcfga  (DDR_DCR_BASE+0x0)   /* DDR configuration address reg */
#define ddrcfgd  (DDR_DCR_BASE+0x1)   /* DDR configuration data reg    */

/*-----------------------------------------------------------------------------+
  | Values for ddrcfga register - indirect addressing of these regs
  +-----------------------------------------------------------------------------*/

#define DDR0_00                         0x00
#define DDR0_00_INT_ACK_MASK              0x7F000000 /* Write only */
#define DDR0_00_INT_ACK_ALL               0x7F000000
#define DDR0_00_INT_ACK_ENCODE(n)           ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_00_INT_ACK_DECODE(n)           ((((unsigned long)(n))>>24)&0x7F)
/* Status */
#define DDR0_00_INT_STATUS_MASK           0x00FF0000 /* Read only */
/* Bit0. A single access outside the defined PHYSICAL memory space detected. */
#define DDR0_00_INT_STATUS_BIT0           0x00010000
/* Bit1. Multiple accesses outside the defined PHYSICAL memory space detected. */
#define DDR0_00_INT_STATUS_BIT1           0x00020000
/* Bit2. Single correctable ECC event detected */
#define DDR0_00_INT_STATUS_BIT2           0x00040000
/* Bit3. Multiple correctable ECC events detected. */
#define DDR0_00_INT_STATUS_BIT3           0x00080000
/* Bit4. Single uncorrectable ECC event detected. */
#define DDR0_00_INT_STATUS_BIT4           0x00100000
/* Bit5. Multiple uncorrectable ECC events detected. */
#define DDR0_00_INT_STATUS_BIT5           0x00200000
/* Bit6. DRAM initialization complete. */
#define DDR0_00_INT_STATUS_BIT6           0x00400000
/* Bit7. Logical OR of all lower bits. */
#define DDR0_00_INT_STATUS_BIT7           0x00800000

#define DDR0_00_INT_STATUS_ENCODE(n)        ((((unsigned long)(n))&0xFF)<<16)
#define DDR0_00_INT_STATUS_DECODE(n)        ((((unsigned long)(n))>>16)&0xFF)
#define DDR0_00_DLL_INCREMENT_MASK        0x00007F00
#define DDR0_00_DLL_INCREMENT_ENCODE(n)     ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_00_DLL_INCREMENT_DECODE(n)     ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_00_DLL_START_POINT_MASK      0x0000007F
#define DDR0_00_DLL_START_POINT_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_00_DLL_START_POINT_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)


#define DDR0_01                         0x01
#define DDR0_01_PLB0_DB_CS_LOWER_MASK     0x1F000000
#define DDR0_01_PLB0_DB_CS_LOWER_ENCODE(n)  ((((unsigned long)(n))&0x1F)<<24)
#define DDR0_01_PLB0_DB_CS_LOWER_DECODE(n)  ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_01_PLB0_DB_CS_UPPER_MASK     0x001F0000
#define DDR0_01_PLB0_DB_CS_UPPER_ENCODE(n)  ((((unsigned long)(n))&0x1F)<<16)
#define DDR0_01_PLB0_DB_CS_UPPER_DECODE(n)  ((((unsigned long)(n))>>16)&0x1F)
#define DDR0_01_OUT_OF_RANGE_TYPE_MASK    0x00000700 /* Read only */
#define DDR0_01_OUT_OF_RANGE_TYPE_ENCODE(n)               ((((unsigned long)(n))&0x7)<<8)
#define DDR0_01_OUT_OF_RANGE_TYPE_DECODE(n)               ((((unsigned long)(n))>>8)&0x7)
#define DDR0_01_INT_MASK_MASK             0x000000FF
#define DDR0_01_INT_MASK_ENCODE(n)          ((((unsigned long)(n))&0xFF)<<0)
#define DDR0_01_INT_MASK_DECODE(n)          ((((unsigned long)(n))>>0)&0xFF)
#define DDR0_01_INT_MASK_ALL_ON           0x000000FF
#define DDR0_01_INT_MASK_ALL_OFF          0x00000000

#define DDR0_02                         0x02
#define DDR0_02_MAX_CS_REG_MASK           0x02000000 /* Read only */
#define DDR0_02_MAX_CS_REG_ENCODE(n)        ((((unsigned long)(n))&0x2)<<24)
#define DDR0_02_MAX_CS_REG_DECODE(n)        ((((unsigned long)(n))>>24)&0x2)
#define DDR0_02_MAX_COL_REG_MASK          0x000F0000 /* Read only */
#define DDR0_02_MAX_COL_REG_ENCODE(n)       ((((unsigned long)(n))&0xF)<<16)
#define DDR0_02_MAX_COL_REG_DECODE(n)       ((((unsigned long)(n))>>16)&0xF)
#define DDR0_02_MAX_ROW_REG_MASK          0x00000F00 /* Read only */
#define DDR0_02_MAX_ROW_REG_ENCODE(n)       ((((unsigned long)(n))&0xF)<<8)
#define DDR0_02_MAX_ROW_REG_DECODE(n)       ((((unsigned long)(n))>>8)&0xF)
#define DDR0_02_START_MASK                0x00000001
#define DDR0_02_START_ENCODE(n)             ((((unsigned long)(n))&0x1)<<0)
#define DDR0_02_START_DECODE(n)             ((((unsigned long)(n))>>0)&0x1)
#define DDR0_02_START_OFF                 0x00000000
#define DDR0_02_START_ON                  0x00000001

#define DDR0_03                         0x03
#define DDR0_03_BSTLEN_MASK               0x07000000
#define DDR0_03_BSTLEN_ENCODE(n)            ((((unsigned long)(n))&0x7)<<24)
#define DDR0_03_BSTLEN_DECODE(n)            ((((unsigned long)(n))>>24)&0x7)
#define DDR0_03_CASLAT_MASK               0x00070000
#define DDR0_03_CASLAT_ENCODE(n)            ((((unsigned long)(n))&0x7)<<16)
#define DDR0_03_CASLAT_DECODE(n)            ((((unsigned long)(n))>>16)&0x7)
#define DDR0_03_CASLAT_LIN_MASK           0x00000F00
#define DDR0_03_CASLAT_LIN_ENCODE(n)        ((((unsigned long)(n))&0xF)<<8)
#define DDR0_03_CASLAT_LIN_DECODE(n)        ((((unsigned long)(n))>>8)&0xF)
#define DDR0_03_INITAREF_MASK             0x0000000F
#define DDR0_03_INITAREF_ENCODE(n)          ((((unsigned long)(n))&0xF)<<0)
#define DDR0_03_INITAREF_DECODE(n)          ((((unsigned long)(n))>>0)&0xF)

#define DDR0_04                         0x04
#define DDR0_04_TRC_MASK                  0x1F000000
#define DDR0_04_TRC_ENCODE(n)               ((((unsigned long)(n))&0x1F)<<24)
#define DDR0_04_TRC_DECODE(n)               ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_04_TRRD_MASK                 0x00070000
#define DDR0_04_TRRD_ENCODE(n)              ((((unsigned long)(n))&0x7)<<16)
#define DDR0_04_TRRD_DECODE(n)              ((((unsigned long)(n))>>16)&0x7)
#define DDR0_04_TRTP_MASK                 0x00000700
#define DDR0_04_TRTP_ENCODE(n)              ((((unsigned long)(n))&0x7)<<8)
#define DDR0_04_TRTP_DECODE(n)              ((((unsigned long)(n))>>8)&0x7)

#define DDR0_05                         0x05
#define DDR0_05_TMRD_MASK                 0x1F000000
#define DDR0_05_TMRD_ENCODE(n)              ((((unsigned long)(n))&0x1F)<<24)
#define DDR0_05_TMRD_DECODE(n)              ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_05_TEMRS_MASK                0x00070000
#define DDR0_05_TEMRS_ENCODE(n)             ((((unsigned long)(n))&0x7)<<16)
#define DDR0_05_TEMRS_DECODE(n)             ((((unsigned long)(n))>>16)&0x7)
#define DDR0_05_TRP_MASK                  0x00000F00
#define DDR0_05_TRP_ENCODE(n)               ((((unsigned long)(n))&0xF)<<8)
#define DDR0_05_TRP_DECODE(n)               ((((unsigned long)(n))>>8)&0xF)
#define DDR0_05_TRAS_MIN_MASK             0x000000FF
#define DDR0_05_TRAS_MIN_ENCODE(n)          ((((unsigned long)(n))&0xFF)<<0)
#define DDR0_05_TRAS_MIN_DECODE(n)          ((((unsigned long)(n))>>0)&0xFF)

#define DDR0_06                         0x06
#define DDR0_06_WRITEINTERP_MASK          0x01000000
#define DDR0_06_WRITEINTERP_ENCODE(n)       ((((unsigned long)(n))&0x1)<<24)
#define DDR0_06_WRITEINTERP_DECODE(n)       ((((unsigned long)(n))>>24)&0x1)
#define DDR0_06_TWTR_MASK                 0x00070000
#define DDR0_06_TWTR_ENCODE(n)              ((((unsigned long)(n))&0x7)<<16)
#define DDR0_06_TWTR_DECODE(n)              ((((unsigned long)(n))>>16)&0x7)
#define DDR0_06_TDLL_MASK                 0x0000FF00
#define DDR0_06_TDLL_ENCODE(n)              ((((unsigned long)(n))&0xFF)<<8)
#define DDR0_06_TDLL_DECODE(n)              ((((unsigned long)(n))>>8)&0xFF)
#define DDR0_06_TRFC_MASK                 0x0000007F
#define DDR0_06_TRFC_ENCODE(n)              ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_06_TRFC_DECODE(n)              ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_07                         0x07
#define DDR0_07_NO_CMD_INIT_MASK          0x01000000
#define DDR0_07_NO_CMD_INIT_ENCODE(n)       ((((unsigned long)(n))&0x1)<<24)
#define DDR0_07_NO_CMD_INIT_DECODE(n)       ((((unsigned long)(n))>>24)&0x1)
#define DDR0_07_TFAW_MASK                 0x001F0000
#define DDR0_07_TFAW_ENCODE(n)              ((((unsigned long)(n))&0x1F)<<16)
#define DDR0_07_TFAW_DECODE(n)              ((((unsigned long)(n))>>16)&0x1F)
#define DDR0_07_AUTO_REFRESH_MODE_MASK    0x00000100
#define DDR0_07_AUTO_REFRESH_MODE_ENCODE(n) ((((unsigned long)(n))&0x1)<<8)
#define DDR0_07_AUTO_REFRESH_MODE_DECODE(n) ((((unsigned long)(n))>>8)&0x1)
#define DDR0_07_AREFRESH_MASK             0x00000001
#define DDR0_07_AREFRESH_ENCODE(n)          ((((unsigned long)(n))&0x1)<<0)
#define DDR0_07_AREFRESH_DECODE(n)          ((((unsigned long)(n))>>0)&0x1)

#define DDR0_08                         0x08
#define DDR0_08_WRLAT_MASK                0x07000000
#define DDR0_08_WRLAT_ENCODE(n)             ((((unsigned long)(n))&0x7)<<24)
#define DDR0_08_WRLAT_DECODE(n)             ((((unsigned long)(n))>>24)&0x7)
#define DDR0_08_TCPD_MASK                 0x00FF0000
#define DDR0_08_TCPD_ENCODE(n)              ((((unsigned long)(n))&0xFF)<<16)
#define DDR0_08_TCPD_DECODE(n)              ((((unsigned long)(n))>>16)&0xFF)
#define DDR0_08_DQS_N_EN_MASK             0x00000100
#define DDR0_08_DQS_N_EN_ENCODE(n)          ((((unsigned long)(n))&0x1)<<8)
#define DDR0_08_DQS_N_EN_DECODE(n)          ((((unsigned long)(n))>>8)&0x1)
#define DDR0_08_DDRII_SDRAM_MODE_MASK     0x00000001
#define DDR0_08_DDRII_ENCODE(n)             ((((unsigned long)(n))&0x1)<<0)
#define DDR0_08_DDRII_DECODE(n)             ((((unsigned long)(n))>>0)&0x1)

#define DDR0_09                         0x09
#define DDR0_09_OCD_ADJUST_PDN_CS_0_MASK  0x1F000000
#define DDR0_09_OCD_ADJUST_PDN_CS_0_ENCODE(n) ((((unsigned long)(n))&0x1F)<<24)
#define DDR0_09_OCD_ADJUST_PDN_CS_0_DECODE(n) ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_09_RTT_0_MASK                0x00030000
#define DDR0_09_RTT_0_ENCODE(n)             ((((unsigned long)(n))&0x3)<<16)
#define DDR0_09_RTT_0_DECODE(n)             ((((unsigned long)(n))>>16)&0x3)
#define DDR0_09_WR_DQS_SHIFT_BYPASS_MASK  0x00007F00
#define DDR0_09_WR_DQS_SHIFT_BYPASS_ENCODE(n) ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_09_WR_DQS_SHIFT_BYPASS_DECODE(n) ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_09_WR_DQS_SHIFT_MASK         0x0000007F
#define DDR0_09_WR_DQS_SHIFT_ENCODE(n)      ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_09_WR_DQS_SHIFT_DECODE(n)      ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_10                         0x0A
#define DDR0_10_WRITE_MODEREG_MASK        0x00010000 /* Write only */
#define DDR0_10_WRITE_MODEREG_ENCODE(n)     ((((unsigned long)(n))&0x1)<<16)
#define DDR0_10_WRITE_MODEREG_DECODE(n)     ((((unsigned long)(n))>>16)&0x1)
#define DDR0_10_CS_MAP_MASK               0x00000300
#define DDR0_10_CS_MAP_NO_MEM             0x00000000
#define DDR0_10_CS_MAP_RANK0_INSTALLED    0x00000100
#define DDR0_10_CS_MAP_RANK1_INSTALLED    0x00000200
#define DDR0_10_CS_MAP_ENCODE(n)            ((((unsigned long)(n))&0x3)<<8)
#define DDR0_10_CS_MAP_DECODE(n)            ((((unsigned long)(n))>>8)&0x3)
#define DDR0_10_OCD_ADJUST_PUP_CS_0_MASK  0x0000001F
#define DDR0_10_OCD_ADJUST_PUP_CS_0_ENCODE(n) ((((unsigned long)(n))&0x1F)<<0)
#define DDR0_10_OCD_ADJUST_PUP_CS_0_DECODE(n) ((((unsigned long)(n))>>0)&0x1F)

#define DDR0_11                         0x0B
#define DDR0_11_SREFRESH_MASK             0x01000000
#define DDR0_11_SREFRESH_ENCODE(n)          ((((unsigned long)(n))&0x1)<<24)
#define DDR0_11_SREFRESH_DECODE(n)          ((((unsigned long)(n))>>24)&0x1F)
#define DDR0_11_TXSNR_MASK                0x00FF0000
#define DDR0_11_TXSNR_ENCODE(n)             ((((unsigned long)(n))&0xFF)<<16)
#define DDR0_11_TXSNR_DECODE(n)             ((((unsigned long)(n))>>16)&0xFF)
#define DDR0_11_TXSR_MASK                 0x0000FF00
#define DDR0_11_TXSR_ENCODE(n)              ((((unsigned long)(n))&0xFF)<<8)
#define DDR0_11_TXSR_DECODE(n)              ((((unsigned long)(n))>>8)&0xFF)

#define DDR0_12                         0x0C
#define DDR0_12_TCKE_MASK                 0x0000007
#define DDR0_12_TCKE_ENCODE(n)              ((((unsigned long)(n))&0x7)<<0)
#define DDR0_12_TCKE_DECODE(n)              ((((unsigned long)(n))>>0)&0x7)

#define DDR0_13                         0x0D

#define DDR0_14                         0x0E
#define DDR0_14_DLL_BYPASS_MODE_MASK      0x01000000
#define DDR0_14_DLL_BYPASS_MODE_ENCODE(n)   ((((unsigned long)(n))&0x1)<<24)
#define DDR0_14_DLL_BYPASS_MODE_DECODE(n)   ((((unsigned long)(n))>>24)&0x1)
#define DDR0_14_REDUC_MASK                0x00010000
#define DDR0_14_REDUC_64BITS              0x00000000
#define DDR0_14_REDUC_32BITS              0x00010000
#define DDR0_14_REDUC_ENCODE(n)             ((((unsigned long)(n))&0x1)<<16)
#define DDR0_14_REDUC_DECODE(n)             ((((unsigned long)(n))>>16)&0x1)
#define DDR0_14_REG_DIMM_ENABLE_MASK      0x00000100
#define DDR0_14_REG_DIMM_ENABLE_ENCODE(n)   ((((unsigned long)(n))&0x1)<<8)
#define DDR0_14_REG_DIMM_ENABLE_DECODE(n)   ((((unsigned long)(n))>>8)&0x1)

#define DDR0_15                         0x0F

#define DDR0_16                         0x10

#define DDR0_17                         0x11
#define DDR0_17_DLL_DQS_DELAY_0_MASK      0x7F000000
#define DDR0_17_DLL_DQS_DELAY_0_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_17_DLL_DQS_DELAY_0_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_17_DLLLOCKREG_MASK           0x00010000 /* Read only */
#define DDR0_17_DLLLOCKREG_LOCKED         0x00010000
#define DDR0_17_DLLLOCKREG_UNLOCKED       0x00000000
#define DDR0_17_DLLLOCKREG_ENCODE(n)        ((((unsigned long)(n))&0x1)<<16)
#define DDR0_17_DLLLOCKREG_DECODE(n)        ((((unsigned long)(n))>>16)&0x1)
#define DDR0_17_DLL_LOCK_MASK             0x00007F00 /* Read only */
#define DDR0_17_DLL_LOCK_ENCODE(n)          ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_17_DLL_LOCK_DECODE(n)          ((((unsigned long)(n))>>8)&0x7F)

#define DDR0_18                         0x12
#define DDR0_18_DLL_DQS_DELAY_X_MASK      0x7F7F7F7F
#define DDR0_18_DLL_DQS_DELAY_4_MASK      0x7F000000
#define DDR0_18_DLL_DQS_DELAY_4_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_18_DLL_DQS_DELAY_4_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_18_DLL_DQS_DELAY_3_MASK      0x007F0000
#define DDR0_18_DLL_DQS_DELAY_3_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_18_DLL_DQS_DELAY_3_DECODE(n)   ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_18_DLL_DQS_DELAY_2_MASK      0x00007F00
#define DDR0_18_DLL_DQS_DELAY_2_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_18_DLL_DQS_DELAY_2_DECODE(n)   ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_18_DLL_DQS_DELAY_1_MASK      0x0000007F
#define DDR0_18_DLL_DQS_DELAY_1_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_18_DLL_DQS_DELAY_1_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_19                         0x13
#define DDR0_19_DLL_DQS_DELAY_X_MASK      0x7F7F7F7F
#define DDR0_19_DLL_DQS_DELAY_8_MASK      0x7F000000
#define DDR0_19_DLL_DQS_DELAY_8_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_19_DLL_DQS_DELAY_8_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_19_DLL_DQS_DELAY_7_MASK      0x007F0000
#define DDR0_19_DLL_DQS_DELAY_7_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_19_DLL_DQS_DELAY_7_DECODE(n)   ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_19_DLL_DQS_DELAY_6_MASK      0x00007F00
#define DDR0_19_DLL_DQS_DELAY_6_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_19_DLL_DQS_DELAY_6_DECODE(n)   ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_19_DLL_DQS_DELAY_5_MASK      0x0000007F
#define DDR0_19_DLL_DQS_DELAY_5_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_19_DLL_DQS_DELAY_5_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_20                         0x14
#define DDR0_20_DLL_DQS_BYPASS_3_MASK      0x7F000000
#define DDR0_20_DLL_DQS_BYPASS_3_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_20_DLL_DQS_BYPASS_3_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_20_DLL_DQS_BYPASS_2_MASK      0x007F0000
#define DDR0_20_DLL_DQS_BYPASS_2_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_20_DLL_DQS_BYPASS_2_DECODE(n)   ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_20_DLL_DQS_BYPASS_1_MASK      0x00007F00
#define DDR0_20_DLL_DQS_BYPASS_1_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_20_DLL_DQS_BYPASS_1_DECODE(n)   ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_20_DLL_DQS_BYPASS_0_MASK      0x0000007F
#define DDR0_20_DLL_DQS_BYPASS_0_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_20_DLL_DQS_BYPASS_0_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_21                         0x15
#define DDR0_21_DLL_DQS_BYPASS_7_MASK      0x7F000000
#define DDR0_21_DLL_DQS_BYPASS_7_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<24)
#define DDR0_21_DLL_DQS_BYPASS_7_DECODE(n)   ((((unsigned long)(n))>>24)&0x7F)
#define DDR0_21_DLL_DQS_BYPASS_6_MASK      0x007F0000
#define DDR0_21_DLL_DQS_BYPASS_6_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_21_DLL_DQS_BYPASS_6_DECODE(n)   ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_21_DLL_DQS_BYPASS_5_MASK      0x00007F00
#define DDR0_21_DLL_DQS_BYPASS_5_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_21_DLL_DQS_BYPASS_5_DECODE(n)   ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_21_DLL_DQS_BYPASS_4_MASK      0x0000007F
#define DDR0_21_DLL_DQS_BYPASS_4_ENCODE(n)   ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_21_DLL_DQS_BYPASS_4_DECODE(n)   ((((unsigned long)(n))>>0)&0x7F)

#define DDR0_22                         0x16
/* ECC */
#define DDR0_22_CTRL_RAW_MASK             0x03000000
#define DDR0_22_CTRL_RAW_ECC_DISABLE      0x00000000 /* ECC not being used */
#define DDR0_22_CTRL_RAW_ECC_CHECK_ONLY   0x01000000 /* ECC checking is on, but no attempts to correct*/
#define DDR0_22_CTRL_RAW_NO_ECC_RAM       0x02000000 /* No ECC RAM storage available */
#define DDR0_22_CTRL_RAW_ECC_ENABLE       0x03000000 /* ECC checking and correcting on */
#define DDR0_22_CTRL_RAW_ENCODE(n)          ((((unsigned long)(n))&0x3)<<24)
#define DDR0_22_CTRL_RAW_DECODE(n)          ((((unsigned long)(n))>>24)&0x3)

#define DDR0_22_DQS_OUT_SHIFT_BYPASS_MASK 0x007F0000
#define DDR0_22_DQS_OUT_SHIFT_BYPASS_ENCODE(n) ((((unsigned long)(n))&0x7F)<<16)
#define DDR0_22_DQS_OUT_SHIFT_BYPASS_DECODE(n) ((((unsigned long)(n))>>16)&0x7F)
#define DDR0_22_DQS_OUT_SHIFT_MASK        0x00007F00
#define DDR0_22_DQS_OUT_SHIFT_ENCODE(n)     ((((unsigned long)(n))&0x7F)<<8)
#define DDR0_22_DQS_OUT_SHIFT_DECODE(n)     ((((unsigned long)(n))>>8)&0x7F)
#define DDR0_22_DLL_DQS_BYPASS_8_MASK     0x0000007F
#define DDR0_22_DLL_DQS_BYPASS_8_ENCODE(n)  ((((unsigned long)(n))&0x7F)<<0)
#define DDR0_22_DLL_DQS_BYPASS_8_DECODE(n)  ((((unsigned long)(n))>>0)&0x7F)


#define DDR0_23                         0x17
#define DDR0_23_ODT_RD_MAP_CS0_MASK       0x03000000
#define DDR0_23_ODT_RD_MAP_CS0_ENCODE(n)   ((((unsigned long)(n))&0x3)<<24)
#define DDR0_23_ODT_RD_MAP_CS0_DECODE(n)   ((((unsigned long)(n))>>24)&0x3)
#define DDR0_23_ECC_C_SYND_MASK           0x00FF0000 /* Read only */
#define DDR0_23_ECC_C_SYND_ENCODE(n)        ((((unsigned long)(n))&0xFF)<<16)
#define DDR0_23_ECC_C_SYND_DECODE(n)        ((((unsigned long)(n))>>16)&0xFF)
#define DDR0_23_ECC_U_SYND_MASK           0x0000FF00 /* Read only */
#define DDR0_23_ECC_U_SYND_ENCODE(n)        ((((unsigned long)(n))&0xFF)<<8)
#define DDR0_23_ECC_U_SYND_DECODE(n)        ((((unsigned long)(n))>>8)&0xFF)
#define DDR0_23_FWC_MASK                  0x00000001 /* Write only */
#define DDR0_23_FWC_ENCODE(n)               ((((unsigned long)(n))&0x1)<<0)
#define DDR0_23_FWC_DECODE(n)               ((((unsigned long)(n))>>0)&0x1)

#define DDR0_24                         0x18
#define DDR0_24_RTT_PAD_TERMINATION_MASK  0x03000000
#define DDR0_24_RTT_PAD_TERMINATION_ENCODE(n) ((((unsigned long)(n))&0x3)<<24)
#define DDR0_24_RTT_PAD_TERMINATION_DECODE(n) ((((unsigned long)(n))>>24)&0x3)
#define DDR0_24_ODT_WR_MAP_CS1_MASK       0x00030000
#define DDR0_24_ODT_WR_MAP_CS1_ENCODE(n)    ((((unsigned long)(n))&0x3)<<16)
#define DDR0_24_ODT_WR_MAP_CS1_DECODE(n)    ((((unsigned long)(n))>>16)&0x3)
#define DDR0_24_ODT_RD_MAP_CS1_MASK       0x00000300
#define DDR0_24_ODT_RD_MAP_CS1_ENCODE(n)    ((((unsigned long)(n))&0x3)<<8)
#define DDR0_24_ODT_RD_MAP_CS1_DECODE(n)    ((((unsigned long)(n))>>8)&0x3)
#define DDR0_24_ODT_WR_MAP_CS0_MASK       0x00000003
#define DDR0_24_ODT_WR_MAP_CS0_ENCODE(n)    ((((unsigned long)(n))&0x3)<<0)
#define DDR0_24_ODT_WR_MAP_CS0_DECODE(n)    ((((unsigned long)(n))>>0)&0x3)

#define DDR0_25                         0x19
#define DDR0_25_VERSION_MASK              0xFFFF0000 /* Read only */
#define DDR0_25_VERSION_ENCODE(n)           ((((unsigned long)(n))&0xFFFF)<<16)
#define DDR0_25_VERSION_DECODE(n)           ((((unsigned long)(n))>>16)&0xFFFF)
#define DDR0_25_OUT_OF_RANGE_LENGTH_MASK  0x000003FF /* Read only */
#define DDR0_25_OUT_OF_RANGE_LENGTH_ENCODE(n) ((((unsigned long)(n))&0x3FF)<<0)
#define DDR0_25_OUT_OF_RANGE_LENGTH_DECODE(n) ((((unsigned long)(n))>>0)&0x3FF)

#define DDR0_26                         0x1A
#define DDR0_26_TRAS_MAX_MASK             0xFFFF0000
#define DDR0_26_TRAS_MAX_ENCODE(n)          ((((unsigned long)(n))&0xFFFF)<<16)
#define DDR0_26_TRAS_MAX_DECODE(n)          ((((unsigned long)(n))>>16)&0xFFFF)
#define DDR0_26_TREF_MASK                 0x00003FFF
#define DDR0_26_TREF_ENCODE(n)              ((((unsigned long)(n))&0x3FFF)<<0)
#define DDR0_26_TREF_DECODE(n)              ((((unsigned long)(n))>>0)&0x3FFF)

#define DDR0_27                         0x1B
#define DDR0_27_EMRS_DATA_MASK            0x3FFF0000
#define DDR0_27_EMRS_DATA_ENCODE(n)         ((((unsigned long)(n))&0x3FFF)<<16)
#define DDR0_27_EMRS_DATA_DECODE(n)         ((((unsigned long)(n))>>16)&0x3FFF)
#define DDR0_27_TINIT_MASK                0x0000FFFF
#define DDR0_27_TINIT_ENCODE(n)             ((((unsigned long)(n))&0xFFFF)<<0)
#define DDR0_27_TINIT_DECODE(n)             ((((unsigned long)(n))>>0)&0xFFFF)

#define DDR0_28                         0x1C
#define DDR0_28_EMRS3_DATA_MASK           0x3FFF0000
#define DDR0_28_EMRS3_DATA_ENCODE(n)        ((((unsigned long)(n))&0x3FFF)<<16)
#define DDR0_28_EMRS3_DATA_DECODE(n)        ((((unsigned long)(n))>>16)&0x3FFF)
#define DDR0_28_EMRS2_DATA_MASK           0x00003FFF
#define DDR0_28_EMRS2_DATA_ENCODE(n)        ((((unsigned long)(n))&0x3FFF)<<0)
#define DDR0_28_EMRS2_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0x3FFF)

#define DDR0_29                         0x1D

#define DDR0_30                         0x1E

#define DDR0_31                         0x1F
#define DDR0_31_XOR_CHECK_BITS_MASK       0x0000FFFF
#define DDR0_31_XOR_CHECK_BITS_ENCODE(n)    ((((unsigned long)(n))&0xFFFF)<<0)
#define DDR0_31_XOR_CHECK_BITS_DECODE(n)    ((((unsigned long)(n))>>0)&0xFFFF)

#define DDR0_32                         0x20
#define DDR0_32_OUT_OF_RANGE_ADDR_MASK    0xFFFFFFFF /* Read only */
#define DDR0_32_OUT_OF_RANGE_ADDR_ENCODE(n) ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_32_OUT_OF_RANGE_ADDR_DECODE(n) ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_33                         0x21
#define DDR0_33_OUT_OF_RANGE_ADDR_MASK    0x00000001 /* Read only */
#define DDR0_33_OUT_OF_RANGE_ADDR_ENCODE(n) ((((unsigned long)(n))&0x1)<<0)
#define DDR0_33_OUT_OF_RANGE_ADDR_DECODE(n)               ((((unsigned long)(n))>>0)&0x1)

#define DDR0_34                         0x22
#define DDR0_34_ECC_U_ADDR_MASK           0xFFFFFFFF /* Read only */
#define DDR0_34_ECC_U_ADDR_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_34_ECC_U_ADDR_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_35                         0x23
#define DDR0_35_ECC_U_ADDR_MASK           0x00000001 /* Read only */
#define DDR0_35_ECC_U_ADDR_ENCODE(n)        ((((unsigned long)(n))&0x1)<<0)
#define DDR0_35_ECC_U_ADDR_DECODE(n)        ((((unsigned long)(n))>>0)&0x1)

#define DDR0_36                         0x24
#define DDR0_36_ECC_U_DATA_MASK           0xFFFFFFFF /* Read only */
#define DDR0_36_ECC_U_DATA_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_36_ECC_U_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_37                         0x25
#define DDR0_37_ECC_U_DATA_MASK           0xFFFFFFFF /* Read only */
#define DDR0_37_ECC_U_DATA_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_37_ECC_U_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_38                         0x26
#define DDR0_38_ECC_C_ADDR_MASK           0xFFFFFFFF /* Read only */
#define DDR0_38_ECC_C_ADDR_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_38_ECC_C_ADDR_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_39                         0x27
#define DDR0_39_ECC_C_ADDR_MASK           0x00000001 /* Read only */
#define DDR0_39_ECC_C_ADDR_ENCODE(n)        ((((unsigned long)(n))&0x1)<<0)
#define DDR0_39_ECC_C_ADDR_DECODE(n)        ((((unsigned long)(n))>>0)&0x1)

#define DDR0_40                         0x28
#define DDR0_40_ECC_C_DATA_MASK           0xFFFFFFFF /* Read only */
#define DDR0_40_ECC_C_DATA_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_40_ECC_C_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_41                         0x29
#define DDR0_41_ECC_C_DATA_MASK           0xFFFFFFFF /* Read only */
#define DDR0_41_ECC_C_DATA_ENCODE(n)        ((((unsigned long)(n))&0xFFFFFFFF)<<0)
#define DDR0_41_ECC_C_DATA_DECODE(n)        ((((unsigned long)(n))>>0)&0xFFFFFFFF)

#define DDR0_42                         0x2A
#define DDR0_42_ADDR_PINS_MASK            0x07000000
#define DDR0_42_ADDR_PINS_ENCODE(n)         ((((unsigned long)(n))&0x7)<<24)
#define DDR0_42_ADDR_PINS_DECODE(n)         ((((unsigned long)(n))>>24)&0x7)
#define DDR0_42_CASLAT_LIN_GATE_MASK      0x0000000F
#define DDR0_42_CASLAT_LIN_GATE_ENCODE(n)   ((((unsigned long)(n))&0xF)<<0)
#define DDR0_42_CASLAT_LIN_GATE_DECODE(n)   ((((unsigned long)(n))>>0)&0xF)

#define DDR0_43                         0x2B
#define DDR0_43_TWR_MASK                  0x07000000
#define DDR0_43_TWR_ENCODE(n)               ((((unsigned long)(n))&0x7)<<24)
#define DDR0_43_TWR_DECODE(n)               ((((unsigned long)(n))>>24)&0x7)
#define DDR0_43_APREBIT_MASK              0x000F0000
#define DDR0_43_APREBIT_ENCODE(n)           ((((unsigned long)(n))&0xF)<<16)
#define DDR0_43_APREBIT_DECODE(n)           ((((unsigned long)(n))>>16)&0xF)
#define DDR0_43_COLUMN_SIZE_MASK          0x00000700
#define DDR0_43_COLUMN_SIZE_ENCODE(n)       ((((unsigned long)(n))&0x7)<<8)
#define DDR0_43_COLUMN_SIZE_DECODE(n)       ((((unsigned long)(n))>>8)&0x7)
#define DDR0_43_EIGHT_BANK_MODE_MASK      0x00000001
#define DDR0_43_EIGHT_BANK_MODE_8_BANKS     0x00000001
#define DDR0_43_EIGHT_BANK_MODE_4_BANKS     0x00000000
#define DDR0_43_EIGHT_BANK_MODE_ENCODE(n)   ((((unsigned long)(n))&0x1)<<0)
#define DDR0_43_EIGHT_BANK_MODE_DECODE(n)   ((((unsigned long)(n))>>0)&0x1)

#define DDR0_44                         0x2C
#define DDR0_44_TRCD_MASK                 0x000000FF
#define DDR0_44_TRCD_ENCODE(n)              ((((unsigned long)(n))&0xFF)<<0)
#define DDR0_44_TRCD_DECODE(n)              ((((unsigned long)(n))>>0)&0xFF)

#endif /* _SPD_SDRAM_DENALI_H_ */
