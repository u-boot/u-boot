/*
 * K2HK: SoC definitions
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef __ASM_ARCH_HARDWARE_K2HK_H
#define __ASM_ARCH_HARDWARE_K2HK_H

#define K2HK_PLL_CNTRL_BASE             0x02310000
#define CLOCK_BASE                      K2HK_PLL_CNTRL_BASE
#define KS2_RSTCTRL                     (K2HK_PLL_CNTRL_BASE + 0xe8)
#define KS2_RSTCTRL_KEY                 0x5a69
#define KS2_RSTCTRL_MASK                0xffff0000
#define KS2_RSTCTRL_SWRST               0xfffe0000

#define K2HK_PSC_BASE                   0x02350000
#define KS2_DEVICE_STATE_CTRL_BASE      0x02620000
#define JTAG_ID_REG                     (KS2_DEVICE_STATE_CTRL_BASE + 0x18)
#define K2HK_DEVSTAT                    (KS2_DEVICE_STATE_CTRL_BASE + 0x20)

#define K2HK_MISC_CTRL                  (KS2_DEVICE_STATE_CTRL_BASE + 0xc7c)

#define ARM_PLL_EN                      BIT(13)

#define K2HK_SPI0_BASE                  0x21000400
#define K2HK_SPI1_BASE                  0x21000600
#define K2HK_SPI2_BASE                  0x21000800
#define K2HK_SPI_BASE                   K2HK_SPI0_BASE

/* Chip configuration unlock codes and registers */
#define KEYSTONE_KICK0                 (KS2_DEVICE_STATE_CTRL_BASE + 0x38)
#define KEYSTONE_KICK1                 (KS2_DEVICE_STATE_CTRL_BASE + 0x3c)
#define KEYSTONE_KICK0_MAGIC           0x83e70b13
#define KEYSTONE_KICK1_MAGIC           0x95a4f1e0

/* PA SS Registers */
#define KS2_PASS_BASE                  0x02000000

/* PLL control registers */
#define K2HK_MAINPLLCTL0               (KS2_DEVICE_STATE_CTRL_BASE + 0x350)
#define K2HK_MAINPLLCTL1               (KS2_DEVICE_STATE_CTRL_BASE + 0x354)
#define K2HK_PASSPLLCTL0               (KS2_DEVICE_STATE_CTRL_BASE + 0x358)
#define K2HK_PASSPLLCTL1               (KS2_DEVICE_STATE_CTRL_BASE + 0x35C)
#define K2HK_DDR3APLLCTL0              (KS2_DEVICE_STATE_CTRL_BASE + 0x360)
#define K2HK_DDR3APLLCTL1              (KS2_DEVICE_STATE_CTRL_BASE + 0x364)
#define K2HK_DDR3BPLLCTL0              (KS2_DEVICE_STATE_CTRL_BASE + 0x368)
#define K2HK_DDR3BPLLCTL1              (KS2_DEVICE_STATE_CTRL_BASE + 0x36C)
#define K2HK_ARMPLLCTL0	               (KS2_DEVICE_STATE_CTRL_BASE + 0x370)
#define K2HK_ARMPLLCTL1                (KS2_DEVICE_STATE_CTRL_BASE + 0x374)

/* Power and Sleep Controller (PSC) Domains */
#define K2HK_LPSC_MOD                  0
#define K2HK_LPSC_DUMMY1               1
#define K2HK_LPSC_USB                  2
#define K2HK_LPSC_EMIF25_SPI           3
#define K2HK_LPSC_TSIP                 4
#define K2HK_LPSC_DEBUGSS_TRC          5
#define K2HK_LPSC_TETB_TRC             6
#define K2HK_LPSC_PKTPROC              7
#define KS2_LPSC_PA                    K2HK_LPSC_PKTPROC
#define K2HK_LPSC_SGMII                8
#define KS2_LPSC_CPGMAC                K2HK_LPSC_SGMII
#define K2HK_LPSC_CRYPTO               9
#define K2HK_LPSC_PCIE                 10
#define K2HK_LPSC_SRIO                 11
#define K2HK_LPSC_VUSR0                12
#define K2HK_LPSC_CHIP_SRSS            13
#define K2HK_LPSC_MSMC                 14
#define K2HK_LPSC_GEM_0                15
#define K2HK_LPSC_GEM_1                16
#define K2HK_LPSC_GEM_2                17
#define K2HK_LPSC_GEM_3                18
#define K2HK_LPSC_GEM_4                19
#define K2HK_LPSC_GEM_5                20
#define K2HK_LPSC_GEM_6                21
#define K2HK_LPSC_GEM_7                22
#define K2HK_LPSC_EMIF4F_DDR3A         23
#define K2HK_LPSC_EMIF4F_DDR3B         24
#define K2HK_LPSC_TAC                  25
#define K2HK_LPSC_RAC                  26
#define K2HK_LPSC_RAC_1                27
#define K2HK_LPSC_FFTC_A               28
#define K2HK_LPSC_FFTC_B               29
#define K2HK_LPSC_FFTC_C               30
#define K2HK_LPSC_FFTC_D               31
#define K2HK_LPSC_FFTC_E               32
#define K2HK_LPSC_FFTC_F               33
#define K2HK_LPSC_AI2                  34
#define K2HK_LPSC_TCP3D_0              35
#define K2HK_LPSC_TCP3D_1              36
#define K2HK_LPSC_TCP3D_2              37
#define K2HK_LPSC_TCP3D_3              38
#define K2HK_LPSC_VCP2X4_A             39
#define K2HK_LPSC_CP2X4_B              40
#define K2HK_LPSC_VCP2X4_C             41
#define K2HK_LPSC_VCP2X4_D             42
#define K2HK_LPSC_VCP2X4_E             43
#define K2HK_LPSC_VCP2X4_F             44
#define K2HK_LPSC_VCP2X4_G             45
#define K2HK_LPSC_VCP2X4_H             46
#define K2HK_LPSC_BCP                  47
#define K2HK_LPSC_DXB                  48
#define K2HK_LPSC_VUSR1                49
#define K2HK_LPSC_XGE                  50
#define K2HK_LPSC_ARM_SREFLEX          51
#define K2HK_LPSC_TETRIS               52

/* DDR3A definitions */
#define K2HK_DDR3A_EMIF_CTRL_BASE      0x21010000
#define K2HK_DDR3A_EMIF_DATA_BASE      0x80000000
#define K2HK_DDR3A_DDRPHYC             0x02329000
/* DDR3B definitions */
#define K2HK_DDR3B_EMIF_CTRL_BASE      0x21020000
#define K2HK_DDR3B_EMIF_DATA_BASE      0x60000000
#define K2HK_DDR3B_DDRPHYC             0x02328000

/* Queue manager */
#define DEVICE_QM_MANAGER_BASE         0x02a02000
#define DEVICE_QM_DESC_SETUP_BASE      0x02a03000
#define DEVICE_QM_MANAGER_QUEUES_BASE  0x02a80000
#define DEVICE_QM_MANAGER_Q_PROXY_BASE 0x02ac0000
#define DEVICE_QM_QUEUE_STATUS_BASE    0x02a40000
#define DEVICE_QM_NUM_LINKRAMS         2
#define DEVICE_QM_NUM_MEMREGIONS       20

#define DEVICE_PA_CDMA_GLOBAL_CFG_BASE  0x02004000
#define DEVICE_PA_CDMA_TX_CHAN_CFG_BASE 0x02004400
#define DEVICE_PA_CDMA_RX_CHAN_CFG_BASE	0x02004800
#define DEVICE_PA_CDMA_RX_FLOW_CFG_BASE	0x02005000

#define DEVICE_PA_CDMA_RX_NUM_CHANNELS  24
#define DEVICE_PA_CDMA_RX_NUM_FLOWS     32
#define DEVICE_PA_CDMA_TX_NUM_CHANNELS  9

/* MSMC control */
#define K2HK_MSMC_CTRL_BASE             0x0bc00000

#endif /* __ASM_ARCH_HARDWARE_H */
