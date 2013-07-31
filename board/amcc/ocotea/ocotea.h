/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Board specific FPGA stuff ... */
#define FPGA_REG0                       (CONFIG_SYS_FPGA_BASE + 0x00)
#define   FPGA_REG0_SSCG_MASK             0x80
#define   FPGA_REG0_SSCG_DISABLE          0x00
#define   FPGA_REG0_SSCG_ENABLE           0x80
#define   FPGA_REG0_BOOT_MASK             0x40
#define   FPGA_REG0_BOOT_LARGE_FLASH      0x00
#define   FPGA_REG0_BOOT_SMALL_FLASH      0x40
#define   FPGA_REG0_ECLS_MASK             0x38  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_0                0x20  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_1                0x10  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_2                0x08  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_VER1             0x00  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_VER3             0x08  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_VER4             0x10  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_VER5             0x18  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_VER2             0x20  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_VER6             0x28  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_VER7             0x30  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ECLS_VER8             0x38  /* New for Ocotea Rev 2 */
#define   FPGA_REG0_ARBITER_MASK          0x04
#define   FPGA_REG0_ARBITER_EXT           0x00
#define   FPGA_REG0_ARBITER_INT           0x04
#define   FPGA_REG0_ONBOARD_FLASH_MASK    0x02
#define   FPGA_REG0_ONBOARD_FLASH_ENABLE  0x00
#define   FPGA_REG0_ONBOARD_FLASH_DISABLE 0x02
#define   FPGA_REG0_FLASH                 0x01
#define FPGA_REG1                       (CONFIG_SYS_FPGA_BASE + 0x01)
#define   FPGA_REG1_9772_FSELFBX_MASK     0x80
#define   FPGA_REG1_9772_FSELFBX_6        0x00
#define   FPGA_REG1_9772_FSELFBX_10       0x80
#define   FPGA_REG1_9531_SX_MASK          0x60
#define   FPGA_REG1_9531_SX_33MHZ         0x00
#define   FPGA_REG1_9531_SX_100MHZ        0x20
#define   FPGA_REG1_9531_SX_66MHZ         0x40
#define   FPGA_REG1_9531_SX_133MHZ        0x60
#define   FPGA_REG1_9772_FSELBX_MASK      0x18
#define   FPGA_REG1_9772_FSELBX_4         0x00
#define   FPGA_REG1_9772_FSELBX_6         0x08
#define   FPGA_REG1_9772_FSELBX_8         0x10
#define   FPGA_REG1_9772_FSELBX_10        0x18
#define   FPGA_REG1_SOURCE_MASK           0x07
#define   FPGA_REG1_SOURCE_TC             0x00
#define   FPGA_REG1_SOURCE_66MHZ          0x01
#define   FPGA_REG1_SOURCE_50MHZ          0x02
#define   FPGA_REG1_SOURCE_33MHZ          0x03
#define   FPGA_REG1_SOURCE_25MHZ          0x04
#define   FPGA_REG1_SOURCE_SSDIV1         0x05
#define   FPGA_REG1_SOURCE_SSDIV2         0x06
#define   FPGA_REG1_SOURCE_SSDIV4         0x07
#define FPGA_REG2                       (CONFIG_SYS_FPGA_BASE + 0x02)
#define   FPGA_REG2_TC0                   0x80
#define   FPGA_REG2_TC1                   0x40
#define   FPGA_REG2_TC2                   0x20
#define   FPGA_REG2_TC3                   0x10
#define   FPGA_REG2_GIGABIT_RESET_DISABLE 0x08   /*Use on Ocotea pass 2 boards*/
#define   FPGA_REG2_EXT_INTFACE_MASK      0x04
#define   FPGA_REG2_EXT_INTFACE_ENABLE    0x00
#define   FPGA_REG2_EXT_INTFACE_DISABLE   0x04
#define   FPGA_REG2_SMII_RESET_DISABLE    0x02   /*Use on Ocotea pass 3 boards*/
#define   FPGA_REG2_DEFAULT_UART1_N       0x01
#define FPGA_REG3                       (CONFIG_SYS_FPGA_BASE + 0x03)
#define   FPGA_REG3_GIGABIT_RESET_DISABLE 0x80   /*Use on Ocotea pass 1 boards*/
#define   FPGA_REG3_ENET_MASK1            0x70   /*Use on Ocotea pass 1 boards*/
#define   FPGA_REG3_ENET_MASK2            0xF0   /*Use on Ocotea pass 2 boards*/
#define   FPGA_REG3_ENET_GROUP0           0x00
#define   FPGA_REG3_ENET_GROUP1           0x10
#define   FPGA_REG3_ENET_GROUP2           0x20
#define   FPGA_REG3_ENET_GROUP3           0x30
#define   FPGA_REG3_ENET_GROUP4           0x40
#define   FPGA_REG3_ENET_GROUP5           0x50
#define   FPGA_REG3_ENET_GROUP6           0x60
#define   FPGA_REG3_ENET_GROUP7           0x70
#define   FPGA_REG3_ENET_GROUP8           0x80   /*Use on Ocotea pass 2 boards*/
#define   FPGA_REG3_ENET_ENCODE1(n) ((((unsigned long)(n))&0x07)<<4) /*pass1*/
#define   FPGA_REG3_ENET_DECODE1(n) ((((unsigned long)(n))>>4)&0x07) /*pass1*/
#define   FPGA_REG3_ENET_ENCODE2(n) ((((unsigned long)(n))&0x0F)<<4) /*pass2*/
#define   FPGA_REG3_ENET_DECODE2(n) ((((unsigned long)(n))>>4)&0x0F) /*pass2*/
#define   FPGA_REG3_STAT_MASK             0x0F
#define   FPGA_REG3_STAT_LED8_ENAB        0x08
#define   FPGA_REG3_STAT_LED4_ENAB        0x04
#define   FPGA_REG3_STAT_LED2_ENAB        0x02
#define   FPGA_REG3_STAT_LED1_ENAB        0x01
#define   FPGA_REG3_STAT_LED8_DISAB       0x00
#define   FPGA_REG3_STAT_LED4_DISAB       0x00
#define   FPGA_REG3_STAT_LED2_DISAB       0x00
#define   FPGA_REG3_STAT_LED1_DISAB       0x00
#define FPGA_REG4                       (CONFIG_SYS_FPGA_BASE + 0x04)
#define   FPGA_REG4_GPHY_MODE10           0x80
#define   FPGA_REG4_GPHY_MODE100          0x40
#define   FPGA_REG4_GPHY_MODE1000         0x20
#define   FPGA_REG4_GPHY_FRC_DPLX         0x10
#define   FPGA_REG4_GPHY_ANEG_DIS         0x08
#define   FPGA_REG4_CONNECT_PHYS          0x04


#define   SDR0_CUST0_ENET3_MASK         0x00000080
#define   SDR0_CUST0_ENET3_COPPER       0x00000000
#define   SDR0_CUST0_ENET3_FIBER        0x00000080
#define   SDR0_CUST0_RGMII3_MASK        0x00000070
#define   SDR0_CUST0_RGMII3_ENCODE(n) ((((unsigned long)(n))&0x7)<<4)
#define   SDR0_CUST0_RGMII3_DECODE(n) ((((unsigned long)(n))>>4)&0x07)
#define   SDR0_CUST0_RGMII3_DISAB       0x00000000
#define   SDR0_CUST0_RGMII3_RTBI        0x00000040
#define   SDR0_CUST0_RGMII3_RGMII       0x00000050
#define   SDR0_CUST0_RGMII3_TBI         0x00000060
#define   SDR0_CUST0_RGMII3_GMII        0x00000070
#define   SDR0_CUST0_ENET2_MASK         0x00000008
#define   SDR0_CUST0_ENET2_COPPER       0x00000000
#define   SDR0_CUST0_ENET2_FIBER        0x00000008
#define   SDR0_CUST0_RGMII2_MASK        0x00000007
#define   SDR0_CUST0_RGMII2_ENCODE(n) ((((unsigned long)(n))&0x7)<<0)
#define   SDR0_CUST0_RGMII2_DECODE(n) ((((unsigned long)(n))>>0)&0x07)
#define   SDR0_CUST0_RGMII2_DISAB       0x00000000
#define   SDR0_CUST0_RGMII2_RTBI        0x00000004
#define   SDR0_CUST0_RGMII2_RGMII       0x00000005
#define   SDR0_CUST0_RGMII2_TBI         0x00000006
#define   SDR0_CUST0_RGMII2_GMII        0x00000007
