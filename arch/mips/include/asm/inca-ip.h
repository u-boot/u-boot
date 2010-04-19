
/******************************************************************************
       Copyright (c) 2002, Infineon Technologies.  All rights reserved.

			       No Warranty
   Because the program is licensed free of charge, there is no warranty for
   the program, to the extent permitted by applicable law.  Except when
   otherwise stated in writing the copyright holders and/or other parties
   provide the program "as is" without warranty of any kind, either
   expressed or implied, including, but not limited to, the implied
   warranties of merchantability and fitness for a particular purpose. The
   entire risk as to the quality and performance of the program is with
   you.  should the program prove defective, you assume the cost of all
   necessary servicing, repair or correction.

   In no event unless required by applicable law or agreed to in writing
   will any copyright holder, or any other party who may modify and/or
   redistribute the program as permitted above, be liable to you for
   damages, including any general, special, incidental or consequential
   damages arising out of the use or inability to use the program
   (including but not limited to loss of data or data being rendered
   inaccurate or losses sustained by you or third parties or a failure of
   the program to operate with any other programs), even if such holder or
   other party has been advised of the possibility of such damages.
******************************************************************************/


/***********************************************************************/
/*  Module      :  WDT register address and bits                       */
/***********************************************************************/

#define INCA_IP_WDT                          (0xB8000000)
/***********************************************************************/


/***Reset Status Register Power On***/
#define INCA_IP_WDT_RST_SR                       ((volatile u32*)(INCA_IP_WDT+ 0x0014))

/***Reset Request Register***/
#define INCA_IP_WDT_RST_REQ                      ((volatile u32*)(INCA_IP_WDT+ 0x0010))
#define INCA_IP_WDT_RST_REQ_SWBOOT                        (1 << 24)
#define INCA_IP_WDT_RST_REQ_SWCFG                          (1 << 16)
#define INCA_IP_WDT_RST_REQ_RRPHY                          (1 << 5)
#define INCA_IP_WDT_RST_REQ_RRHSP                          (1 << 4)
#define INCA_IP_WDT_RST_REQ_RRFPI                          (1 << 3)
#define INCA_IP_WDT_RST_REQ_RREXT                          (1 << 2)
#define INCA_IP_WDT_RST_REQ_RRDSP                          (1 << 1)
#define INCA_IP_WDT_RST_REQ_RRCPU                          (1 << 0)

/***NMI Status Register***/
#define INCA_IP_WDT_NMISR                        ((volatile u32*)(INCA_IP_WDT+ 0x002C))
#define INCA_IP_WDT_NMISR_NMIWDT                        (1 << 2)
#define INCA_IP_WDT_NMISR_NMIPLL                        (1 << 1)
#define INCA_IP_WDT_NMISR_NMIEXT                        (1 << 0)

/***Manufacturer Identification Register***/
#define INCA_IP_WDT_MANID                        ((volatile u32*)(INCA_IP_WDT+ 0x0070))
#define INCA_IP_WDT_MANID_MANUF (value)              (((( 1 << 11) - 1) & (value)) << 5)

/***Chip Identification Register***/
#define INCA_IP_WDT_CHIPID                       ((volatile u32*)(INCA_IP_WDT+ 0x0074))
#define INCA_IP_WDT_CHIPID_VERSION (value)            (((( 1 << 4) - 1) & (value)) << 28)
#define INCA_IP_WDT_CHIPID_PART_NUMBER (value)        (((( 1 << 16) - 1) & (value)) << 12)
#define INCA_IP_WDT_CHIPID_MANID (value)              (((( 1 << 11) - 1) & (value)) << 1)

/***Redesign Tracing Identification Register***/
#define INCA_IP_WDT_RTID                         ((volatile u32*)(INCA_IP_WDT+ 0x0078))
#define INCA_IP_WDT_RTID_LC                              (1 << 15)
#define INCA_IP_WDT_RTID_RIX (value)                (((( 1 << 3) - 1) & (value)) << 0)

/***Watchdog Timer Control Register 0***/
#define INCA_IP_WDT_WDT_CON0                    ((volatile u32*)(INCA_IP_WDT+ 0x0020))

/***Watchdog Timer Control Register 1***/
#define INCA_IP_WDT_WDT_CON1                    ((volatile u32*)(INCA_IP_WDT+ 0x0024))
#define INCA_IP_WDT_WDT_CON1_WDTDR                          (1 << 3)
#define INCA_IP_WDT_WDT_CON1_WDTIR                          (1 << 2)

/***Watchdog Timer Status Register***/
#define INCA_IP_WDT_WDT_SR                       ((volatile u32*)(INCA_IP_WDT+ 0x0028))
#define INCA_IP_WDT_WDT_SR_WDTTIM (value)             (((( 1 << 16) - 1) & (value)) << 16)
#define INCA_IP_WDT_WDT_SR_WDTPR                          (1 << 5)
#define INCA_IP_WDT_WDT_SR_WDTTO                          (1 << 4)
#define INCA_IP_WDT_WDT_SR_WDTDS                          (1 << 3)
#define INCA_IP_WDT_WDT_SR_WDTIS                          (1 << 2)
#define INCA_IP_WDT_WDT_SR_WDTOE                          (1 << 1)
#define INCA_IP_WDT_WDT_SR_WDTAE                          (1 << 0)

/***********************************************************************/
/*  Module      :  CGU register address and bits                       */
/***********************************************************************/

#define INCA_IP_CGU                          (0xBF107000)
/***********************************************************************/


/***CGU PLL1 Control Register***/
#define INCA_IP_CGU_CGU_PLL1CR                   ((volatile u32*)(INCA_IP_CGU+ 0x0008))
#define INCA_IP_CGU_CGU_PLL1CR_SWRST                          (1 << 31)
#define INCA_IP_CGU_CGU_PLL1CR_EN                              (1 << 30)
#define INCA_IP_CGU_CGU_PLL1CR_NDIV (value)               (((( 1 << 6) - 1) & (value)) << 16)
#define INCA_IP_CGU_CGU_PLL1CR_MDIV (value)               (((( 1 << 4) - 1) & (value)) << 0)

/***CGU PLL0 Control Register***/
#define INCA_IP_CGU_CGU_PLL0CR                   ((volatile u32*)(INCA_IP_CGU+ 0x0000))
#define INCA_IP_CGU_CGU_PLL0CR_SWRST                          (1 << 31)
#define INCA_IP_CGU_CGU_PLL0CR_EN                              (1 << 30)
#define INCA_IP_CGU_CGU_PLL0CR_NDIV (value)               (((( 1 << 6) - 1) & (value)) << 16)
#define INCA_IP_CGU_CGU_PLL0CR_MDIV (value)               (((( 1 << 4) - 1) & (value)) << 0)

/***CGU PLL0 Status Register***/
#define INCA_IP_CGU_CGU_PLL0SR                   ((volatile u32*)(INCA_IP_CGU+ 0x0004))
#define INCA_IP_CGU_CGU_PLL0SR_LOCK                            (1 << 31)
#define INCA_IP_CGU_CGU_PLL0SR_RCF                              (1 << 29)
#define INCA_IP_CGU_CGU_PLL0SR_PLLBYP                        (1 << 15)

/***CGU PLL1 Status Register***/
#define INCA_IP_CGU_CGU_PLL1SR                   ((volatile u32*)(INCA_IP_CGU+ 0x000C))
#define INCA_IP_CGU_CGU_PLL1SR_LOCK                            (1 << 31)
#define INCA_IP_CGU_CGU_PLL1SR_RCF                              (1 << 29)
#define INCA_IP_CGU_CGU_PLL1SR_PLLBYP                        (1 << 15)

/***CGU Divider Control Register***/
#define INCA_IP_CGU_CGU_DIVCR                    ((volatile u32*)(INCA_IP_CGU+ 0x0010))

/***CGU Multiplexer Control Register***/
#define INCA_IP_CGU_CGU_MUXCR                    ((volatile u32*)(INCA_IP_CGU+ 0x0014))
#define INCA_IP_CGU_CGU_MUXCR_SWRST                          (1 << 31)
#define INCA_IP_CGU_CGU_MUXCR_MUXII                          (1 << 1)
#define INCA_IP_CGU_CGU_MUXCR_MUXI                            (1 << 0)

/***CGU Fractional Divider Control Register***/
#define INCA_IP_CGU_CGU_FDCR                    ((volatile u32*)(INCA_IP_CGU+ 0x0018))
#define INCA_IP_CGU_CGU_FDCR_FDEN                            (1 << 31)
#define INCA_IP_CGU_CGU_FDCR_INTEGER (value)            (((( 1 << 12) - 1) & (value)) << 16)
#define INCA_IP_CGU_CGU_FDCR_FRACTION (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***********************************************************************/
/*  Module      :  PMU register address and bits                       */
/***********************************************************************/

#define INCA_IP_PMU                          (0xBF102000)
/***********************************************************************/


/***PM Global Enable Register***/
#define INCA_IP_PMU_PM_GEN                       ((volatile u32*)(INCA_IP_PMU+ 0x0000))
#define INCA_IP_PMU_PM_GEN_EN16                            (1 << 16)
#define INCA_IP_PMU_PM_GEN_EN15                            (1 << 15)
#define INCA_IP_PMU_PM_GEN_EN14                            (1 << 14)
#define INCA_IP_PMU_PM_GEN_EN13                            (1 << 13)
#define INCA_IP_PMU_PM_GEN_EN12                            (1 << 12)
#define INCA_IP_PMU_PM_GEN_EN11                            (1 << 11)
#define INCA_IP_PMU_PM_GEN_EN10                            (1 << 10)
#define INCA_IP_PMU_PM_GEN_EN9                              (1 << 9)
#define INCA_IP_PMU_PM_GEN_EN8                              (1 << 8)
#define INCA_IP_PMU_PM_GEN_EN7                              (1 << 7)
#define INCA_IP_PMU_PM_GEN_EN6                              (1 << 6)
#define INCA_IP_PMU_PM_GEN_EN5                              (1 << 5)
#define INCA_IP_PMU_PM_GEN_EN4                              (1 << 4)
#define INCA_IP_PMU_PM_GEN_EN3                              (1 << 3)
#define INCA_IP_PMU_PM_GEN_EN2                              (1 << 2)
#define INCA_IP_PMU_PM_GEN_EN0                              (1 << 0)

/***PM Power Down Enable Register***/
#define INCA_IP_PMU_PM_PDEN                      ((volatile u32*)(INCA_IP_PMU+ 0x0008))
#define INCA_IP_PMU_PM_PDEN_EN16                            (1 << 16)
#define INCA_IP_PMU_PM_PDEN_EN15                            (1 << 15)
#define INCA_IP_PMU_PM_PDEN_EN14                            (1 << 14)
#define INCA_IP_PMU_PM_PDEN_EN13                            (1 << 13)
#define INCA_IP_PMU_PM_PDEN_EN12                            (1 << 12)
#define INCA_IP_PMU_PM_PDEN_EN11                            (1 << 11)
#define INCA_IP_PMU_PM_PDEN_EN10                            (1 << 10)
#define INCA_IP_PMU_PM_PDEN_EN9                              (1 << 9)
#define INCA_IP_PMU_PM_PDEN_EN8                              (1 << 8)
#define INCA_IP_PMU_PM_PDEN_EN7                              (1 << 7)
#define INCA_IP_PMU_PM_PDEN_EN5                              (1 << 5)
#define INCA_IP_PMU_PM_PDEN_EN4                              (1 << 4)
#define INCA_IP_PMU_PM_PDEN_EN3                              (1 << 3)
#define INCA_IP_PMU_PM_PDEN_EN2                              (1 << 2)
#define INCA_IP_PMU_PM_PDEN_EN0                              (1 << 0)

/***PM Wake-Up from Power Down Register***/
#define INCA_IP_PMU_PM_WUP                       ((volatile u32*)(INCA_IP_PMU+ 0x0010))
#define INCA_IP_PMU_PM_WUP_WUP16                          (1 << 16)
#define INCA_IP_PMU_PM_WUP_WUP15                          (1 << 15)
#define INCA_IP_PMU_PM_WUP_WUP14                          (1 << 14)
#define INCA_IP_PMU_PM_WUP_WUP13                          (1 << 13)
#define INCA_IP_PMU_PM_WUP_WUP12                          (1 << 12)
#define INCA_IP_PMU_PM_WUP_WUP11                          (1 << 11)
#define INCA_IP_PMU_PM_WUP_WUP10                          (1 << 10)
#define INCA_IP_PMU_PM_WUP_WUP9                            (1 << 9)
#define INCA_IP_PMU_PM_WUP_WUP8                            (1 << 8)
#define INCA_IP_PMU_PM_WUP_WUP7                            (1 << 7)
#define INCA_IP_PMU_PM_WUP_WUP5                            (1 << 5)
#define INCA_IP_PMU_PM_WUP_WUP4                            (1 << 4)
#define INCA_IP_PMU_PM_WUP_WUP3                            (1 << 3)
#define INCA_IP_PMU_PM_WUP_WUP2                            (1 << 2)
#define INCA_IP_PMU_PM_WUP_WUP0                            (1 << 0)

/***PM Control Register***/
#define INCA_IP_PMU_PM_CR                        ((volatile u32*)(INCA_IP_PMU+ 0x0014))
#define INCA_IP_PMU_PM_CR_AWEN                            (1 << 31)
#define INCA_IP_PMU_PM_CR_SWRST                          (1 << 30)
#define INCA_IP_PMU_PM_CR_SWCR                            (1 << 2)
#define INCA_IP_PMU_PM_CR_CRD (value)                (((( 1 << 2) - 1) & (value)) << 0)

/***********************************************************************/
/*  Module      :  BCU register address and bits                       */
/***********************************************************************/

#define INCA_IP_BCU                          (0xB8000100)
/***********************************************************************/


/***BCU Control Register (0010H)***/
#define INCA_IP_BCU_BCU_CON                      ((volatile u32*)(INCA_IP_BCU+ 0x0010))
#define INCA_IP_BCU_BCU_CON_SPC (value)                (((( 1 << 8) - 1) & (value)) << 24)
#define INCA_IP_BCU_BCU_CON_SPE                              (1 << 19)
#define INCA_IP_BCU_BCU_CON_PSE                              (1 << 18)
#define INCA_IP_BCU_BCU_CON_DBG                              (1 << 16)
#define INCA_IP_BCU_BCU_CON_TOUT (value)               (((( 1 << 16) - 1) & (value)) << 0)

/***BCU Error Control Capture Register (0020H)***/
#define INCA_IP_BCU_BCU_ECON                    ((volatile u32*)(INCA_IP_BCU+ 0x0020))
#define INCA_IP_BCU_BCU_ECON_TAG (value)                (((( 1 << 4) - 1) & (value)) << 24)
#define INCA_IP_BCU_BCU_ECON_RDN                              (1 << 23)
#define INCA_IP_BCU_BCU_ECON_WRN                              (1 << 22)
#define INCA_IP_BCU_BCU_ECON_SVM                              (1 << 21)
#define INCA_IP_BCU_BCU_ECON_ACK (value)                (((( 1 << 2) - 1) & (value)) << 19)
#define INCA_IP_BCU_BCU_ECON_ABT                              (1 << 18)
#define INCA_IP_BCU_BCU_ECON_RDY                              (1 << 17)
#define INCA_IP_BCU_BCU_ECON_TOUT                            (1 << 16)
#define INCA_IP_BCU_BCU_ECON_ERRCNT (value)             (((( 1 << 16) - 1) & (value)) << 0)
#define INCA_IP_BCU_BCU_ECON_OPC (value)                (((( 1 << 4) - 1) & (value)) << 28)

/***BCU Error Address Capture Register (0024 H)***/
#define INCA_IP_BCU_BCU_EADD                    ((volatile u32*)(INCA_IP_BCU+ 0x0024))
#define INCA_IP_BCU_BCU_EADD_FPIADR

/***BCU Error Data Capture Register (0028H)***/
#define INCA_IP_BCU_BCU_EDAT                    ((volatile u32*)(INCA_IP_BCU+ 0x0028))
#define INCA_IP_BCU_BCU_EDAT_FPIDAT

/***********************************************************************/
/*  Module      :  MBC register address and bits                       */
/***********************************************************************/

#define INCA_IP_MBC                          (0xBF103000)
/***********************************************************************/


/***Mailbox CPU Configuration Register***/
#define INCA_IP_MBC_MBC_CFG                      ((volatile u32*)(INCA_IP_MBC+ 0x0080))
#define INCA_IP_MBC_MBC_CFG_SWAP (value)               (((( 1 << 2) - 1) & (value)) << 6)
#define INCA_IP_MBC_MBC_CFG_RES                              (1 << 5)
#define INCA_IP_MBC_MBC_CFG_FWID (value)               (((( 1 << 4) - 1) & (value)) << 1)
#define INCA_IP_MBC_MBC_CFG_SIZE                            (1 << 0)

/***Mailbox CPU Interrupt Status Register***/
#define INCA_IP_MBC_MBC_ISR                      ((volatile u32*)(INCA_IP_MBC+ 0x0084))
#define INCA_IP_MBC_MBC_ISR_B3DA                            (1 << 31)
#define INCA_IP_MBC_MBC_ISR_B2DA                            (1 << 30)
#define INCA_IP_MBC_MBC_ISR_B1E                              (1 << 29)
#define INCA_IP_MBC_MBC_ISR_B0E                              (1 << 28)
#define INCA_IP_MBC_MBC_ISR_WDT                              (1 << 27)
#define INCA_IP_MBC_MBC_ISR_DS260 (value)             (((( 1 << 27) - 1) & (value)) << 0)

/***Mailbox CPU Mask Register***/
#define INCA_IP_MBC_MBC_MSK                      ((volatile u32*)(INCA_IP_MBC+ 0x0088))
#define INCA_IP_MBC_MBC_MSK_B3DA                            (1 << 31)
#define INCA_IP_MBC_MBC_MSK_B2DA                            (1 << 30)
#define INCA_IP_MBC_MBC_MSK_B1E                              (1 << 29)
#define INCA_IP_MBC_MBC_MSK_B0E                              (1 << 28)
#define INCA_IP_MBC_MBC_MSK_WDT                              (1 << 27)
#define INCA_IP_MBC_MBC_MSK_DS260 (value)             (((( 1 << 27) - 1) & (value)) << 0)

/***Mailbox CPU Mask 01 Register***/
#define INCA_IP_MBC_MBC_MSK01                    ((volatile u32*)(INCA_IP_MBC+ 0x008C))
#define INCA_IP_MBC_MBC_MSK01_B3DA                            (1 << 31)
#define INCA_IP_MBC_MBC_MSK01_B2DA                            (1 << 30)
#define INCA_IP_MBC_MBC_MSK01_B1E                              (1 << 29)
#define INCA_IP_MBC_MBC_MSK01_B0E                              (1 << 28)
#define INCA_IP_MBC_MBC_MSK01_WDT                              (1 << 27)
#define INCA_IP_MBC_MBC_MSK01_DS260 (value)             (((( 1 << 27) - 1) & (value)) << 0)

/***Mailbox CPU Mask 10 Register***/
#define INCA_IP_MBC_MBC_MSK10                    ((volatile u32*)(INCA_IP_MBC+ 0x0090))
#define INCA_IP_MBC_MBC_MSK10_B3DA                            (1 << 31)
#define INCA_IP_MBC_MBC_MSK10_B2DA                            (1 << 30)
#define INCA_IP_MBC_MBC_MSK10_B1E                              (1 << 29)
#define INCA_IP_MBC_MBC_MSK10_B0E                              (1 << 28)
#define INCA_IP_MBC_MBC_MSK10_WDT                              (1 << 27)
#define INCA_IP_MBC_MBC_MSK10_DS260 (value)             (((( 1 << 27) - 1) & (value)) << 0)

/***Mailbox CPU Short Command Register***/
#define INCA_IP_MBC_MBC_CMD                      ((volatile u32*)(INCA_IP_MBC+ 0x0094))
#define INCA_IP_MBC_MBC_CMD_CS270 (value)             (((( 1 << 28) - 1) & (value)) << 0)

/***Mailbox CPU Input Data of Buffer 0***/
#define INCA_IP_MBC_MBC_ID0                      ((volatile u32*)(INCA_IP_MBC+ 0x0000))
#define INCA_IP_MBC_MBC_ID0_INDATA

/***Mailbox CPU Input Data of Buffer 1***/
#define INCA_IP_MBC_MBC_ID1                      ((volatile u32*)(INCA_IP_MBC+ 0x0020))
#define INCA_IP_MBC_MBC_ID1_INDATA

/***Mailbox CPU Output Data of Buffer 2***/
#define INCA_IP_MBC_MBC_OD2                      ((volatile u32*)(INCA_IP_MBC+ 0x0040))
#define INCA_IP_MBC_MBC_OD2_OUTDATA

/***Mailbox CPU Output Data of Buffer 3***/
#define INCA_IP_MBC_MBC_OD3                      ((volatile u32*)(INCA_IP_MBC+ 0x0060))
#define INCA_IP_MBC_MBC_OD3_OUTDATA

/***Mailbox CPU Control Register of Buffer 0***/
#define INCA_IP_MBC_MBC_CR0                      ((volatile u32*)(INCA_IP_MBC+ 0x0004))
#define INCA_IP_MBC_MBC_CR0_RDYABTFLS (value)          (((( 1 << 3) - 1) & (value)) << 0)

/***Mailbox CPU Control Register of Buffer 1***/
#define INCA_IP_MBC_MBC_CR1                      ((volatile u32*)(INCA_IP_MBC+ 0x0024))
#define INCA_IP_MBC_MBC_CR1_RDYABTFLS (value)          (((( 1 << 3) - 1) & (value)) << 0)

/***Mailbox CPU Control Register of Buffer 2***/
#define INCA_IP_MBC_MBC_CR2                      ((volatile u32*)(INCA_IP_MBC+ 0x0044))
#define INCA_IP_MBC_MBC_CR2_RDYABTFLS (value)          (((( 1 << 3) - 1) & (value)) << 0)

/***Mailbox CPU Control Register of Buffer 3***/
#define INCA_IP_MBC_MBC_CR3                      ((volatile u32*)(INCA_IP_MBC+ 0x0064))
#define INCA_IP_MBC_MBC_CR3_RDYABTFLS (value)          (((( 1 << 3) - 1) & (value)) << 0)

/***Mailbox CPU Free Space of Buffer 0***/
#define INCA_IP_MBC_MBC_FS0                      ((volatile u32*)(INCA_IP_MBC+ 0x0008))
#define INCA_IP_MBC_MBC_FS0_FS

/***Mailbox CPU Free Space of Buffer 1***/
#define INCA_IP_MBC_MBC_FS1                      ((volatile u32*)(INCA_IP_MBC+ 0x0028))
#define INCA_IP_MBC_MBC_FS1_FS

/***Mailbox CPU Free Space of Buffer 2***/
#define INCA_IP_MBC_MBC_FS2                      ((volatile u32*)(INCA_IP_MBC+ 0x0048))
#define INCA_IP_MBC_MBC_FS2_FS

/***Mailbox CPU Free Space of Buffer 3***/
#define INCA_IP_MBC_MBC_FS3                      ((volatile u32*)(INCA_IP_MBC+ 0x0068))
#define INCA_IP_MBC_MBC_FS3_FS

/***Mailbox CPU Data Available in Buffer 0***/
#define INCA_IP_MBC_MBC_DA0                      ((volatile u32*)(INCA_IP_MBC+ 0x000C))
#define INCA_IP_MBC_MBC_DA0_DA

/***Mailbox CPU Data Available in Buffer 1***/
#define INCA_IP_MBC_MBC_DA1                      ((volatile u32*)(INCA_IP_MBC+ 0x002C))
#define INCA_IP_MBC_MBC_DA1_DA

/***Mailbox CPU Data Available in Buffer 2***/
#define INCA_IP_MBC_MBC_DA2                      ((volatile u32*)(INCA_IP_MBC+ 0x004C))
#define INCA_IP_MBC_MBC_DA2_DA

/***Mailbox CPU Data Available in Buffer 3***/
#define INCA_IP_MBC_MBC_DA3                      ((volatile u32*)(INCA_IP_MBC+ 0x006C))
#define INCA_IP_MBC_MBC_DA3_DA

/***Mailbox CPU Input Absolute Pointer of Buffer 0***/
#define INCA_IP_MBC_MBC_IABS0                    ((volatile u32*)(INCA_IP_MBC+ 0x0010))
#define INCA_IP_MBC_MBC_IABS0_IABS

/***Mailbox CPU Input Absolute Pointer of Buffer 1***/
#define INCA_IP_MBC_MBC_IABS1                    ((volatile u32*)(INCA_IP_MBC+ 0x0030))
#define INCA_IP_MBC_MBC_IABS1_IABS

/***Mailbox CPU Input Absolute Pointer of Buffer 2***/
#define INCA_IP_MBC_MBC_IABS2                    ((volatile u32*)(INCA_IP_MBC+ 0x0050))
#define INCA_IP_MBC_MBC_IABS2_IABS

/***Mailbox CPU Input Absolute Pointer of Buffer 3***/
#define INCA_IP_MBC_MBC_IABS3                    ((volatile u32*)(INCA_IP_MBC+ 0x0070))
#define INCA_IP_MBC_MBC_IABS3_IABS

/***Mailbox CPU Input Temporary Pointer of Buffer 0***/
#define INCA_IP_MBC_MBC_ITMP0                    ((volatile u32*)(INCA_IP_MBC+ 0x0014))
#define INCA_IP_MBC_MBC_ITMP0_ITMP

/***Mailbox CPU Input Temporary Pointer of Buffer 1***/
#define INCA_IP_MBC_MBC_ITMP1                    ((volatile u32*)(INCA_IP_MBC+ 0x0034))
#define INCA_IP_MBC_MBC_ITMP1_ITMP

/***Mailbox CPU Input Temporary Pointer of Buffer 2***/
#define INCA_IP_MBC_MBC_ITMP2                    ((volatile u32*)(INCA_IP_MBC+ 0x0054))
#define INCA_IP_MBC_MBC_ITMP2_ITMP

/***Mailbox CPU Input Temporary Pointer of Buffer 3***/
#define INCA_IP_MBC_MBC_ITMP3                    ((volatile u32*)(INCA_IP_MBC+ 0x0074))
#define INCA_IP_MBC_MBC_ITMP3_ITMP

/***Mailbox CPU Output Absolute Pointer of Buffer 0***/
#define INCA_IP_MBC_MBC_OABS0                    ((volatile u32*)(INCA_IP_MBC+ 0x0018))
#define INCA_IP_MBC_MBC_OABS0_OABS

/***Mailbox CPU Output Absolute Pointer of Buffer 1***/
#define INCA_IP_MBC_MBC_OABS1                    ((volatile u32*)(INCA_IP_MBC+ 0x0038))
#define INCA_IP_MBC_MBC_OABS1_OABS

/***Mailbox CPU Output Absolute Pointer of Buffer 2***/
#define INCA_IP_MBC_MBC_OABS2                    ((volatile u32*)(INCA_IP_MBC+ 0x0058))
#define INCA_IP_MBC_MBC_OABS2_OABS

/***Mailbox CPU Output Absolute Pointer of Buffer 3***/
#define INCA_IP_MBC_MBC_OABS3                    ((volatile u32*)(INCA_IP_MBC+ 0x0078))
#define INCA_IP_MBC_MBC_OABS3_OABS

/***Mailbox CPU Output Temporary Pointer of Buffer 0***/
#define INCA_IP_MBC_MBC_OTMP0                    ((volatile u32*)(INCA_IP_MBC+ 0x001C))
#define INCA_IP_MBC_MBC_OTMP0_OTMP

/***Mailbox CPU Output Temporary Pointer of Buffer 1***/
#define INCA_IP_MBC_MBC_OTMP1                    ((volatile u32*)(INCA_IP_MBC+ 0x003C))
#define INCA_IP_MBC_MBC_OTMP1_OTMP

/***Mailbox CPU Output Temporary Pointer of Buffer 2***/
#define INCA_IP_MBC_MBC_OTMP2                    ((volatile u32*)(INCA_IP_MBC+ 0x005C))
#define INCA_IP_MBC_MBC_OTMP2_OTMP

/***Mailbox CPU Output Temporary Pointer of Buffer 3***/
#define INCA_IP_MBC_MBC_OTMP3                    ((volatile u32*)(INCA_IP_MBC+ 0x007C))
#define INCA_IP_MBC_MBC_OTMP3_OTMP

/***DSP Control Register***/
#define INCA_IP_MBC_DCTRL                        ((volatile u32*)(INCA_IP_MBC+ 0x00A0))
#define INCA_IP_MBC_DCTRL_BA                              (1 << 0)
#define INCA_IP_MBC_DCTRL_BMOD (value)               (((( 1 << 3) - 1) & (value)) << 1)
#define INCA_IP_MBC_DCTRL_IDL                              (1 << 4)
#define INCA_IP_MBC_DCTRL_RES                              (1 << 15)

/***DSP Status Register***/
#define INCA_IP_MBC_DSTA                         ((volatile u32*)(INCA_IP_MBC+ 0x00A4))
#define INCA_IP_MBC_DSTA_IDLE                            (1 << 0)
#define INCA_IP_MBC_DSTA_PD                              (1 << 1)

/***DSP Test 1 Register***/
#define INCA_IP_MBC_DTST1                        ((volatile u32*)(INCA_IP_MBC+ 0x00A8))
#define INCA_IP_MBC_DTST1_ABORT                          (1 << 0)
#define INCA_IP_MBC_DTST1_HWF32                          (1 << 1)
#define INCA_IP_MBC_DTST1_HWF4M                          (1 << 2)
#define INCA_IP_MBC_DTST1_HWFOP                          (1 << 3)

/***********************************************************************/
/*  Module      :  Switch register address and bits                    */
/***********************************************************************/

#define INCA_IP_Switch                       (0xBF104000)
/***********************************************************************/


/***Unknown Destination Register***/
#define INCA_IP_Switch_UN_DEST                      ((volatile u32*)(INCA_IP_Switch+ 0x0000))
#define INCA_IP_Switch_UN_DEST_CB                              (1 << 8)
#define INCA_IP_Switch_UN_DEST_LB                              (1 << 7)
#define INCA_IP_Switch_UN_DEST_PB                              (1 << 6)
#define INCA_IP_Switch_UN_DEST_CM                              (1 << 5)
#define INCA_IP_Switch_UN_DEST_LM                              (1 << 4)
#define INCA_IP_Switch_UN_DEST_PM                              (1 << 3)
#define INCA_IP_Switch_UN_DEST_CU                              (1 << 2)
#define INCA_IP_Switch_UN_DEST_LU                              (1 << 1)
#define INCA_IP_Switch_UN_DEST_PU                              (1 << 0)

/***VLAN Control Register***/
#define INCA_IP_Switch_VLAN_CTRL                    ((volatile u32*)(INCA_IP_Switch+ 0x0004))
#define INCA_IP_Switch_VLAN_CTRL_SC                              (1 << 6)
#define INCA_IP_Switch_VLAN_CTRL_SL                              (1 << 5)
#define INCA_IP_Switch_VLAN_CTRL_SP                              (1 << 4)
#define INCA_IP_Switch_VLAN_CTRL_TC                              (1 << 3)
#define INCA_IP_Switch_VLAN_CTRL_TL                              (1 << 2)
#define INCA_IP_Switch_VLAN_CTRL_TP                              (1 << 1)
#define INCA_IP_Switch_VLAN_CTRL_VA                              (1 << 0)

/***PC VLAN Configuration Register***/
#define INCA_IP_Switch_PC_VLAN                      ((volatile u32*)(INCA_IP_Switch+ 0x0008))
#define INCA_IP_Switch_PC_VLAN_PRI (value)                (((( 1 << 3) - 1) & (value)) << 12)
#define INCA_IP_Switch_PC_VLAN_VLAN_ID (value)            (((( 1 << 12) - 1) & (value)) << 0)

/***LAN VLAN Configuration Register***/
#define INCA_IP_Switch_LAN_VLAN                    ((volatile u32*)(INCA_IP_Switch+ 0x000C))
#define INCA_IP_Switch_LAN_VLAN_PRI (value)                (((( 1 << 3) - 1) & (value)) << 12)
#define INCA_IP_Switch_LAN_VLAN_VLAN_ID (value)            (((( 1 << 12) - 1) & (value)) << 0)

/***CPU VLAN Configuration Register***/
#define INCA_IP_Switch_CPU_VLAN                    ((volatile u32*)(INCA_IP_Switch+ 0x0010))
#define INCA_IP_Switch_CPU_VLAN_PRI (value)                (((( 1 << 3) - 1) & (value)) << 12)
#define INCA_IP_Switch_CPU_VLAN_VLAN_ID (value)            (((( 1 << 12) - 1) & (value)) << 0)

/***Priority CoS Mapping Register***/
#define INCA_IP_Switch_PRI_CoS                      ((volatile u32*)(INCA_IP_Switch+ 0x0014))
#define INCA_IP_Switch_PRI_CoS_P7                              (1 << 7)
#define INCA_IP_Switch_PRI_CoS_P6                              (1 << 6)
#define INCA_IP_Switch_PRI_CoS_P5                              (1 << 5)
#define INCA_IP_Switch_PRI_CoS_P4                              (1 << 4)
#define INCA_IP_Switch_PRI_CoS_P3                              (1 << 3)
#define INCA_IP_Switch_PRI_CoS_P2                              (1 << 2)
#define INCA_IP_Switch_PRI_CoS_P1                              (1 << 1)
#define INCA_IP_Switch_PRI_CoS_P0                              (1 << 0)

/***Spanning Tree Port Status Register***/
#define INCA_IP_Switch_ST_PT                        ((volatile u32*)(INCA_IP_Switch+ 0x0018))
#define INCA_IP_Switch_ST_PT_CPS (value)                (((( 1 << 2) - 1) & (value)) << 4)
#define INCA_IP_Switch_ST_PT_LPS (value)                (((( 1 << 2) - 1) & (value)) << 2)
#define INCA_IP_Switch_ST_PT_PPS (value)                (((( 1 << 2) - 1) & (value)) << 0)

/***ARL Control Register***/
#define INCA_IP_Switch_ARL_CTL                      ((volatile u32*)(INCA_IP_Switch+ 0x001C))
#define INCA_IP_Switch_ARL_CTL_CHCC                            (1 << 15)
#define INCA_IP_Switch_ARL_CTL_CHCL                            (1 << 14)
#define INCA_IP_Switch_ARL_CTL_CHCP                            (1 << 13)
#define INCA_IP_Switch_ARL_CTL_CC                              (1 << 12)
#define INCA_IP_Switch_ARL_CTL_CL                              (1 << 11)
#define INCA_IP_Switch_ARL_CTL_CP                              (1 << 10)
#define INCA_IP_Switch_ARL_CTL_CG                              (1 << 9)
#define INCA_IP_Switch_ARL_CTL_PS                              (1 << 8)
#define INCA_IP_Switch_ARL_CTL_MRO                              (1 << 7)
#define INCA_IP_Switch_ARL_CTL_SRC                              (1 << 6)
#define INCA_IP_Switch_ARL_CTL_ATS                              (1 << 5)
#define INCA_IP_Switch_ARL_CTL_AGE_TICK_SEL (value)       (((( 1 << 3) - 1) & (value)) << 2)
#define INCA_IP_Switch_ARL_CTL_MAF                              (1 << 1)
#define INCA_IP_Switch_ARL_CTL_ENL                              (1 << 0)
#define INCA_IP_Switch_ARL_CTL_Res (value)                (((( 1 << 19) - 1) & (value)) << 13)

/***CPU Access Control Register***/
#define INCA_IP_Switch_CPU_ACTL                    ((volatile u32*)(INCA_IP_Switch+ 0x0020))
#define INCA_IP_Switch_CPU_ACTL_RA                              (1 << 31)
#define INCA_IP_Switch_CPU_ACTL_RW                              (1 << 30)
#define INCA_IP_Switch_CPU_ACTL_Res (value)                (((( 1 << 21) - 1) & (value)) << 9)
#define INCA_IP_Switch_CPU_ACTL_AVA                              (1 << 8)
#define INCA_IP_Switch_CPU_ACTL_IDX (value)                (((( 1 << 8) - 1) & (value)) << 0)

/***CPU Access Data Register 1***/
#define INCA_IP_Switch_DATA1                        ((volatile u32*)(INCA_IP_Switch+ 0x0024))
#define INCA_IP_Switch_DATA1_Data (value)               (((( 1 << 24) - 1) & (value)) << 0)

/***CPU Access Data Register 2***/
#define INCA_IP_Switch_DATA2                        ((volatile u32*)(INCA_IP_Switch+ 0x0028))
#define INCA_IP_Switch_DATA2_Data

/***CPU Port Control Register***/
#define INCA_IP_Switch_CPU_PCTL                    ((volatile u32*)(INCA_IP_Switch+ 0x002C))
#define INCA_IP_Switch_CPU_PCTL_DA_PORTS (value)          (((( 1 << 3) - 1) & (value)) << 11)
#define INCA_IP_Switch_CPU_PCTL_DAC                              (1 << 10)
#define INCA_IP_Switch_CPU_PCTL_MA_STATE (value)          (((( 1 << 3) - 1) & (value)) << 7)
#define INCA_IP_Switch_CPU_PCTL_MAM                              (1 << 6)
#define INCA_IP_Switch_CPU_PCTL_MA_Ports (value)          (((( 1 << 3) - 1) & (value)) << 3)
#define INCA_IP_Switch_CPU_PCTL_MAC                              (1 << 2)
#define INCA_IP_Switch_CPU_PCTL_EML                              (1 << 1)
#define INCA_IP_Switch_CPU_PCTL_EDL                              (1 << 0)
#define INCA_IP_Switch_CPU_PCTL_Res (value)                (((( 1 << 18) - 1) & (value)) << 14)

/***DSCP CoS Mapping Register 1***/
#define INCA_IP_Switch_DSCP_COS1                    ((volatile u32*)(INCA_IP_Switch+ 0x0030))
#define INCA_IP_Switch_DSCP_COS1_DSCP

/***DSCP CoS Mapping Register 1***/
#define INCA_IP_Switch_DSCP_COS2                    ((volatile u32*)(INCA_IP_Switch+ 0x0034))
#define INCA_IP_Switch_DSCP_COS2_DSCP

/***PC WFQ Control Register***/
#define INCA_IP_Switch_PC_WFQ_CTL                   ((volatile u32*)(INCA_IP_Switch+ 0x0080))
#define INCA_IP_Switch_PC_WFQ_CTL_P1                              (1 << 9)
#define INCA_IP_Switch_PC_WFQ_CTL_P0                              (1 << 8)
#define INCA_IP_Switch_PC_WFQ_CTL_WT1 (value)                (((( 1 << 3) - 1) & (value)) << 5)
#define INCA_IP_Switch_PC_WFQ_CTL_WT0 (value)                (((( 1 << 3) - 1) & (value)) << 2)
#define INCA_IP_Switch_PC_WFQ_CTL_SCH_SEL (value)            (((( 1 << 2) - 1) & (value)) << 0)

/***PC TX Control Register***/
#define INCA_IP_Switch_PC_TX_CTL                    ((volatile u32*)(INCA_IP_Switch+ 0x0084))
#define INCA_IP_Switch_PC_TX_CTL_ELR                              (1 << 1)
#define INCA_IP_Switch_PC_TX_CTL_EER                              (1 << 0)

/***LAN WFQ Control Register***/
#define INCA_IP_Switch_LAN_WFQ_CTL                  ((volatile u32*)(INCA_IP_Switch+ 0x0100))
#define INCA_IP_Switch_LAN_WFQ_CTL_P1                              (1 << 9)
#define INCA_IP_Switch_LAN_WFQ_CTL_P0                              (1 << 8)
#define INCA_IP_Switch_LAN_WFQ_CTL_WT1 (value)                (((( 1 << 3) - 1) & (value)) << 5)
#define INCA_IP_Switch_LAN_WFQ_CTL_WT0 (value)                (((( 1 << 3) - 1) & (value)) << 2)
#define INCA_IP_Switch_LAN_WFQ_CTL_SCH_SEL (value)            (((( 1 << 2) - 1) & (value)) << 0)

/***LAN TX Control Register***/
#define INCA_IP_Switch_LAN_TX_CTL                   ((volatile u32*)(INCA_IP_Switch+ 0x0104))
#define INCA_IP_Switch_LAN_TX_CTL_ELR                              (1 << 1)
#define INCA_IP_Switch_LAN_TX_CTL_EER                              (1 << 0)

/***CPU WFQ Control Register***/
#define INCA_IP_Switch_CPU_WFQ_CTL                  ((volatile u32*)(INCA_IP_Switch+ 0x0180))
#define INCA_IP_Switch_CPU_WFQ_CTL_P1                              (1 << 9)
#define INCA_IP_Switch_CPU_WFQ_CTL_P0                              (1 << 8)
#define INCA_IP_Switch_CPU_WFQ_CTL_WT1 (value)                (((( 1 << 3) - 1) & (value)) << 5)
#define INCA_IP_Switch_CPU_WFQ_CTL_WT0 (value)                (((( 1 << 3) - 1) & (value)) << 2)
#define INCA_IP_Switch_CPU_WFQ_CTL_SCH_SEL (value)            (((( 1 << 2) - 1) & (value)) << 0)

/***PM PC RX Watermark Register***/
#define INCA_IP_Switch_PC_WM                        ((volatile u32*)(INCA_IP_Switch+ 0x0200))
#define INCA_IP_Switch_PC_WM_RX_WM1 (value)             (((( 1 << 8) - 1) & (value)) << 24)
#define INCA_IP_Switch_PC_WM_RX_WM2 (value)             (((( 1 << 8) - 1) & (value)) << 16)
#define INCA_IP_Switch_PC_WM_RX_WM3 (value)             (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_Switch_PC_WM_RX_WM4 (value)             (((( 1 << 8) - 1) & (value)) << 0)

/***PM LAN RX Watermark Register***/
#define INCA_IP_Switch_LAN_WM                       ((volatile u32*)(INCA_IP_Switch+ 0x0204))
#define INCA_IP_Switch_LAN_WM_RX_WM1 (value)             (((( 1 << 8) - 1) & (value)) << 24)
#define INCA_IP_Switch_LAN_WM_RX_WM2 (value)             (((( 1 << 8) - 1) & (value)) << 16)
#define INCA_IP_Switch_LAN_WM_RX_WM3 (value)             (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_Switch_LAN_WM_RX_WM4 (value)             (((( 1 << 8) - 1) & (value)) << 0)

/***PM CPU RX Watermark Register***/
#define INCA_IP_Switch_CPU_WM                       ((volatile u32*)(INCA_IP_Switch+ 0x0208))
#define INCA_IP_Switch_CPU_WM_RX_WM1 (value)             (((( 1 << 8) - 1) & (value)) << 24)
#define INCA_IP_Switch_CPU_WM_RX_WM2 (value)             (((( 1 << 8) - 1) & (value)) << 16)
#define INCA_IP_Switch_CPU_WM_RX_WM3 (value)             (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_Switch_CPU_WM_RX_WM4 (value)             (((( 1 << 8) - 1) & (value)) << 0)

/***PM CPU RX Watermark Register***/
#define INCA_IP_Switch_GBL_WM                       ((volatile u32*)(INCA_IP_Switch+ 0x020C))
#define INCA_IP_Switch_GBL_WM_GBL_RX_WM1 (value)         (((( 1 << 8) - 1) & (value)) << 24)
#define INCA_IP_Switch_GBL_WM_GBL_RX_WM2 (value)         (((( 1 << 8) - 1) & (value)) << 16)
#define INCA_IP_Switch_GBL_WM_GBL_RX_WM3 (value)         (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_Switch_GBL_WM_GBL_RX_WM4 (value)         (((( 1 << 8) - 1) & (value)) << 0)

/***PM Control Register***/
#define INCA_IP_Switch_PM_CTL                       ((volatile u32*)(INCA_IP_Switch+ 0x0210))
#define INCA_IP_Switch_PM_CTL_GDN                              (1 << 3)
#define INCA_IP_Switch_PM_CTL_CDN                              (1 << 2)
#define INCA_IP_Switch_PM_CTL_LDN                              (1 << 1)
#define INCA_IP_Switch_PM_CTL_PDN                              (1 << 0)

/***PM Header Control Register***/
#define INCA_IP_Switch_PMAC_HD_CTL                  ((volatile u32*)(INCA_IP_Switch+ 0x0280))
#define INCA_IP_Switch_PMAC_HD_CTL_RL2                              (1 << 21)
#define INCA_IP_Switch_PMAC_HD_CTL_RC                              (1 << 20)
#define INCA_IP_Switch_PMAC_HD_CTL_CM                              (1 << 19)
#define INCA_IP_Switch_PMAC_HD_CTL_CV                              (1 << 18)
#define INCA_IP_Switch_PMAC_HD_CTL_TYPE_LEN (value)          (((( 1 << 16) - 1) & (value)) << 2)
#define INCA_IP_Switch_PMAC_HD_CTL_TAG                              (1 << 1)
#define INCA_IP_Switch_PMAC_HD_CTL_ADD                              (1 << 0)

/***PM Source Address Register 1***/
#define INCA_IP_Switch_PMAC_SA1                    ((volatile u32*)(INCA_IP_Switch+ 0x0284))
#define INCA_IP_Switch_PMAC_SA1_SA_47_32 (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***PM Source Address Register 2***/
#define INCA_IP_Switch_PMAC_SA2                    ((volatile u32*)(INCA_IP_Switch+ 0x0288))
#define INCA_IP_Switch_PMAC_SA2_SA_31_0

/***PM Dest Address Register 1***/
#define INCA_IP_Switch_PMAC_DA1                    ((volatile u32*)(INCA_IP_Switch+ 0x028C))
#define INCA_IP_Switch_PMAC_DA1_DA_47_32 (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***PM Dest Address Register 2***/
#define INCA_IP_Switch_PMAC_DA2                    ((volatile u32*)(INCA_IP_Switch+ 0x0290))
#define INCA_IP_Switch_PMAC_DA2_DA_31_0

/***PM VLAN Register***/
#define INCA_IP_Switch_PMAC_VLAN                    ((volatile u32*)(INCA_IP_Switch+ 0x0294))
#define INCA_IP_Switch_PMAC_VLAN_PRI (value)                (((( 1 << 3) - 1) & (value)) << 13)
#define INCA_IP_Switch_PMAC_VLAN_CFI                              (1 << 12)
#define INCA_IP_Switch_PMAC_VLAN_VLANID (value)             (((( 1 << 12) - 1) & (value)) << 0)

/***PM TX IPG Counter Register***/
#define INCA_IP_Switch_PMAC_TX_IPG                  ((volatile u32*)(INCA_IP_Switch+ 0x0298))
#define INCA_IP_Switch_PMAC_TX_IPG_IPGCNT (value)             (((( 1 << 8) - 1) & (value)) << 0)

/***PM RX IPG Counter Register***/
#define INCA_IP_Switch_PMAC_RX_IPG                  ((volatile u32*)(INCA_IP_Switch+ 0x029C))
#define INCA_IP_Switch_PMAC_RX_IPG_IPGCNT (value)             (((( 1 << 8) - 1) & (value)) << 0)

/***Mirror Register***/
#define INCA_IP_Switch_MRR                          ((volatile u32*)(INCA_IP_Switch+ 0x0300))
#define INCA_IP_Switch_MRR_MRR (value)                (((( 1 << 2) - 1) & (value)) << 6)
#define INCA_IP_Switch_MRR_EC                              (1 << 5)
#define INCA_IP_Switch_MRR_EL                              (1 << 4)
#define INCA_IP_Switch_MRR_EP                              (1 << 3)
#define INCA_IP_Switch_MRR_IC                              (1 << 2)
#define INCA_IP_Switch_MRR_IL                              (1 << 1)
#define INCA_IP_Switch_MRR_IP                              (1 << 0)

/***Packet Length Register***/
#define INCA_IP_Switch_PKT_LEN                      ((volatile u32*)(INCA_IP_Switch+ 0x0304))
#define INCA_IP_Switch_PKT_LEN_ADD                              (1 << 11)
#define INCA_IP_Switch_PKT_LEN_MAX_PKT_LEN (value)        (((( 1 << 11) - 1) & (value)) << 0)

/***MDIO Access Register***/
#define INCA_IP_Switch_MDIO_ACC                    ((volatile u32*)(INCA_IP_Switch+ 0x0480))
#define INCA_IP_Switch_MDIO_ACC_RA                              (1 << 31)
#define INCA_IP_Switch_MDIO_ACC_RW                              (1 << 30)
#define INCA_IP_Switch_MDIO_ACC_PHY_ADDR (value)          (((( 1 << 5) - 1) & (value)) << 21)
#define INCA_IP_Switch_MDIO_ACC_REG_ADDR (value)          (((( 1 << 5) - 1) & (value)) << 16)
#define INCA_IP_Switch_MDIO_ACC_PHY_DATA (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***Ethernet PHY Register***/
#define INCA_IP_Switch_EPHY                         ((volatile u32*)(INCA_IP_Switch+ 0x0484))
#define INCA_IP_Switch_EPHY_SL                              (1 << 7)
#define INCA_IP_Switch_EPHY_SP                              (1 << 6)
#define INCA_IP_Switch_EPHY_LL                              (1 << 5)
#define INCA_IP_Switch_EPHY_LP                              (1 << 4)
#define INCA_IP_Switch_EPHY_DL                              (1 << 3)
#define INCA_IP_Switch_EPHY_DP                              (1 << 2)
#define INCA_IP_Switch_EPHY_PL                              (1 << 1)
#define INCA_IP_Switch_EPHY_PP                              (1 << 0)

/***Pause Write Enable Register***/
#define INCA_IP_Switch_PWR_EN                       ((volatile u32*)(INCA_IP_Switch+ 0x0488))
#define INCA_IP_Switch_PWR_EN_PL                              (1 << 1)
#define INCA_IP_Switch_PWR_EN_PP                              (1 << 0)

/***MDIO Configuration Register***/
#define INCA_IP_Switch_MDIO_CFG                    ((volatile u32*)(INCA_IP_Switch+ 0x048C))
#define INCA_IP_Switch_MDIO_CFG_MDS (value)                (((( 1 << 2) - 1) & (value)) << 14)
#define INCA_IP_Switch_MDIO_CFG_PHY_LAN_ADDR (value)       (((( 1 << 5) - 1) & (value)) << 9)
#define INCA_IP_Switch_MDIO_CFG_PHY_PC_ADDR (value)        (((( 1 << 5) - 1) & (value)) << 4)
#define INCA_IP_Switch_MDIO_CFG_UEP                              (1 << 3)
#define INCA_IP_Switch_MDIO_CFG_PS                              (1 << 2)
#define INCA_IP_Switch_MDIO_CFG_PT                              (1 << 1)
#define INCA_IP_Switch_MDIO_CFG_UMM                              (1 << 0)

/***Clock Configuration Register***/
#define INCA_IP_Switch_CLK_CFG                      ((volatile u32*)(INCA_IP_Switch+ 0x0500))
#define INCA_IP_Switch_CLK_CFG_ARL_ID                        (1 << 9)
#define INCA_IP_Switch_CLK_CFG_CPU_ID                        (1 << 8)
#define INCA_IP_Switch_CLK_CFG_LAN_ID                        (1 << 7)
#define INCA_IP_Switch_CLK_CFG_PC_ID                          (1 << 6)
#define INCA_IP_Switch_CLK_CFG_SE_ID                          (1 << 5)

/***********************************************************************/
/*  Module      :  SSC1 register address and bits                      */
/***********************************************************************/

#define INCA_IP_SSC1                         (0xB8000500)
/***********************************************************************/


/***Control Register (Programming Mode)***/
#define INCA_IP_SSC1_SCC_CON_PRG                  ((volatile u32*)(INCA_IP_SSC1+ 0x0010))
#define INCA_IP_SSC1_SCC_CON_PRG_EN                              (1 << 15)
#define INCA_IP_SSC1_SCC_CON_PRG_MS                              (1 << 14)
#define INCA_IP_SSC1_SCC_CON_PRG_AREN                            (1 << 12)
#define INCA_IP_SSC1_SCC_CON_PRG_BEN                              (1 << 11)
#define INCA_IP_SSC1_SCC_CON_PRG_PEN                              (1 << 10)
#define INCA_IP_SSC1_SCC_CON_PRG_REN                              (1 << 9)
#define INCA_IP_SSC1_SCC_CON_PRG_TEN                              (1 << 8)
#define INCA_IP_SSC1_SCC_CON_PRG_LB                              (1 << 7)
#define INCA_IP_SSC1_SCC_CON_PRG_PO                              (1 << 6)
#define INCA_IP_SSC1_SCC_CON_PRG_PH                              (1 << 5)
#define INCA_IP_SSC1_SCC_CON_PRG_HB                              (1 << 4)
#define INCA_IP_SSC1_SCC_CON_PRG_BM (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***SCC Control Register (Operating Mode)***/
#define INCA_IP_SSC1_SCC_CON_OPR                  ((volatile u32*)(INCA_IP_SSC1+ 0x0010))
#define INCA_IP_SSC1_SCC_CON_OPR_EN                              (1 << 15)
#define INCA_IP_SSC1_SCC_CON_OPR_MS                              (1 << 14)
#define INCA_IP_SSC1_SCC_CON_OPR_BSY                              (1 << 12)
#define INCA_IP_SSC1_SCC_CON_OPR_BE                              (1 << 11)
#define INCA_IP_SSC1_SCC_CON_OPR_PE                              (1 << 10)
#define INCA_IP_SSC1_SCC_CON_OPR_RE                              (1 << 9)
#define INCA_IP_SSC1_SCC_CON_OPR_TE                              (1 << 8)
#define INCA_IP_SSC1_SCC_CON_OPR_BC (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***SSC Write Hardware Modified Control Register***/
#define INCA_IP_SSC1_SSC_WHBCON                   ((volatile u32*)(INCA_IP_SSC1+ 0x0040))
#define INCA_IP_SSC1_SSC_WHBCON_SETBE                          (1 << 15)
#define INCA_IP_SSC1_SSC_WHBCON_SETPE                          (1 << 14)
#define INCA_IP_SSC1_SSC_WHBCON_SETRE                          (1 << 13)
#define INCA_IP_SSC1_SSC_WHBCON_SETTE                          (1 << 12)
#define INCA_IP_SSC1_SSC_WHBCON_CLRBE                          (1 << 11)
#define INCA_IP_SSC1_SSC_WHBCON_CLRPE                          (1 << 10)
#define INCA_IP_SSC1_SSC_WHBCON_CLRRE                          (1 << 9)
#define INCA_IP_SSC1_SSC_WHBCON_CLRTE                          (1 << 8)

/***SSC Baudrate Timer Reload Register***/
#define INCA_IP_SSC1_SSC_BR                       ((volatile u32*)(INCA_IP_SSC1+ 0x0014))
#define INCA_IP_SSC1_SSC_BR_BR_VALUE (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***SSC Transmitter Buffer Register***/
#define INCA_IP_SSC1_SSC_TB                       ((volatile u32*)(INCA_IP_SSC1+ 0x0020))
#define INCA_IP_SSC1_SSC_TB_TB_VALUE (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***SSC Receiver Buffer Register***/
#define INCA_IP_SSC1_SSC_RB                       ((volatile u32*)(INCA_IP_SSC1+ 0x0024))
#define INCA_IP_SSC1_SSC_RB_RB_VALUE (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***SSC Receive FIFO Control Register***/
#define INCA_IP_SSC1_SSC_RXFCON                   ((volatile u32*)(INCA_IP_SSC1+ 0x0030))
#define INCA_IP_SSC1_SSC_RXFCON_RXFITL (value)             (((( 1 << 6) - 1) & (value)) << 8)
#define INCA_IP_SSC1_SSC_RXFCON_RXTMEN                        (1 << 2)
#define INCA_IP_SSC1_SSC_RXFCON_RXFLU                          (1 << 1)
#define INCA_IP_SSC1_SSC_RXFCON_RXFEN                          (1 << 0)

/***SSC Transmit FIFO Control Register***/
#define INCA_IP_SSC1_SSC_TXFCON                   ((volatile u32*)(INCA_IP_SSC1+ 0x0034))
#define INCA_IP_SSC1_SSC_TXFCON_RXFITL (value)             (((( 1 << 6) - 1) & (value)) << 8)
#define INCA_IP_SSC1_SSC_TXFCON_TXTMEN                        (1 << 2)
#define INCA_IP_SSC1_SSC_TXFCON_TXFLU                          (1 << 1)
#define INCA_IP_SSC1_SSC_TXFCON_TXFEN                          (1 << 0)

/***SSC FIFO Status Register***/
#define INCA_IP_SSC1_SSC_FSTAT                    ((volatile u32*)(INCA_IP_SSC1+ 0x0038))
#define INCA_IP_SSC1_SSC_FSTAT_TXFFL (value)              (((( 1 << 6) - 1) & (value)) << 8)
#define INCA_IP_SSC1_SSC_FSTAT_RXFFL (value)              (((( 1 << 6) - 1) & (value)) << 0)

/***SSC Clock Control Register***/
#define INCA_IP_SSC1_SSC_CLC                      ((volatile u32*)(INCA_IP_SSC1+ 0x0000))
#define INCA_IP_SSC1_SSC_CLC_RMC (value)                (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_SSC1_SSC_CLC_DISS                            (1 << 1)
#define INCA_IP_SSC1_SSC_CLC_DISR                            (1 << 0)

/***********************************************************************/
/*  Module      :  SSC2 register address and bits                      */
/***********************************************************************/

#define INCA_IP_SSC2                         (0xB8000600)
/***********************************************************************/


/***Control Register (Programming Mode)***/
#define INCA_IP_SSC2_SCC_CON_PRG                  ((volatile u32*)(INCA_IP_SSC2+ 0x0010))
#define INCA_IP_SSC2_SCC_CON_PRG_EN                              (1 << 15)
#define INCA_IP_SSC2_SCC_CON_PRG_MS                              (1 << 14)
#define INCA_IP_SSC2_SCC_CON_PRG_AREN                            (1 << 12)
#define INCA_IP_SSC2_SCC_CON_PRG_BEN                              (1 << 11)
#define INCA_IP_SSC2_SCC_CON_PRG_PEN                              (1 << 10)
#define INCA_IP_SSC2_SCC_CON_PRG_REN                              (1 << 9)
#define INCA_IP_SSC2_SCC_CON_PRG_TEN                              (1 << 8)
#define INCA_IP_SSC2_SCC_CON_PRG_LB                              (1 << 7)
#define INCA_IP_SSC2_SCC_CON_PRG_PO                              (1 << 6)
#define INCA_IP_SSC2_SCC_CON_PRG_PH                              (1 << 5)
#define INCA_IP_SSC2_SCC_CON_PRG_HB                              (1 << 4)
#define INCA_IP_SSC2_SCC_CON_PRG_BM (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***SCC Control Register (Operating Mode)***/
#define INCA_IP_SSC2_SCC_CON_OPR                  ((volatile u32*)(INCA_IP_SSC2+ 0x0010))
#define INCA_IP_SSC2_SCC_CON_OPR_EN                              (1 << 15)
#define INCA_IP_SSC2_SCC_CON_OPR_MS                              (1 << 14)
#define INCA_IP_SSC2_SCC_CON_OPR_BSY                              (1 << 12)
#define INCA_IP_SSC2_SCC_CON_OPR_BE                              (1 << 11)
#define INCA_IP_SSC2_SCC_CON_OPR_PE                              (1 << 10)
#define INCA_IP_SSC2_SCC_CON_OPR_RE                              (1 << 9)
#define INCA_IP_SSC2_SCC_CON_OPR_TE                              (1 << 8)
#define INCA_IP_SSC2_SCC_CON_OPR_BC (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***SSC Write Hardware Modified Control Register***/
#define INCA_IP_SSC2_SSC_WHBCON                   ((volatile u32*)(INCA_IP_SSC2+ 0x0040))
#define INCA_IP_SSC2_SSC_WHBCON_SETBE                          (1 << 15)
#define INCA_IP_SSC2_SSC_WHBCON_SETPE                          (1 << 14)
#define INCA_IP_SSC2_SSC_WHBCON_SETRE                          (1 << 13)
#define INCA_IP_SSC2_SSC_WHBCON_SETTE                          (1 << 12)
#define INCA_IP_SSC2_SSC_WHBCON_CLRBE                          (1 << 11)
#define INCA_IP_SSC2_SSC_WHBCON_CLRPE                          (1 << 10)
#define INCA_IP_SSC2_SSC_WHBCON_CLRRE                          (1 << 9)
#define INCA_IP_SSC2_SSC_WHBCON_CLRTE                          (1 << 8)

/***SSC Baudrate Timer Reload Register***/
#define INCA_IP_SSC2_SSC_BR                       ((volatile u32*)(INCA_IP_SSC2+ 0x0014))
#define INCA_IP_SSC2_SSC_BR_BR_VALUE (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***SSC Transmitter Buffer Register***/
#define INCA_IP_SSC2_SSC_TB                       ((volatile u32*)(INCA_IP_SSC2+ 0x0020))
#define INCA_IP_SSC2_SSC_TB_TB_VALUE (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***SSC Receiver Buffer Register***/
#define INCA_IP_SSC2_SSC_RB                       ((volatile u32*)(INCA_IP_SSC2+ 0x0024))
#define INCA_IP_SSC2_SSC_RB_RB_VALUE (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***SSC Receive FIFO Control Register***/
#define INCA_IP_SSC2_SSC_RXFCON                   ((volatile u32*)(INCA_IP_SSC2+ 0x0030))
#define INCA_IP_SSC2_SSC_RXFCON_RXFITL (value)             (((( 1 << 6) - 1) & (value)) << 8)
#define INCA_IP_SSC2_SSC_RXFCON_RXTMEN                        (1 << 2)
#define INCA_IP_SSC2_SSC_RXFCON_RXFLU                          (1 << 1)
#define INCA_IP_SSC2_SSC_RXFCON_RXFEN                          (1 << 0)

/***SSC Transmit FIFO Control Register***/
#define INCA_IP_SSC2_SSC_TXFCON                   ((volatile u32*)(INCA_IP_SSC2+ 0x0034))
#define INCA_IP_SSC2_SSC_TXFCON_RXFITL (value)             (((( 1 << 6) - 1) & (value)) << 8)
#define INCA_IP_SSC2_SSC_TXFCON_TXTMEN                        (1 << 2)
#define INCA_IP_SSC2_SSC_TXFCON_TXFLU                          (1 << 1)
#define INCA_IP_SSC2_SSC_TXFCON_TXFEN                          (1 << 0)

/***SSC FIFO Status Register***/
#define INCA_IP_SSC2_SSC_FSTAT                    ((volatile u32*)(INCA_IP_SSC2+ 0x0038))
#define INCA_IP_SSC2_SSC_FSTAT_TXFFL (value)              (((( 1 << 6) - 1) & (value)) << 8)
#define INCA_IP_SSC2_SSC_FSTAT_RXFFL (value)              (((( 1 << 6) - 1) & (value)) << 0)

/***SSC Clock Control Register***/
#define INCA_IP_SSC2_SSC_CLC                      ((volatile u32*)(INCA_IP_SSC2+ 0x0000))
#define INCA_IP_SSC2_SSC_CLC_RMC (value)                (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_SSC2_SSC_CLC_DISS                            (1 << 1)
#define INCA_IP_SSC2_SSC_CLC_DISR                            (1 << 0)

/***********************************************************************/
/*  Module      :  EBU register address and bits                       */
/***********************************************************************/

#if defined(CONFIG_INCA_IP)
#define INCA_IP_EBU                          (0xB8000200)
#elif defined(CONFIG_PURPLE)
#define INCA_IP_EBU                          (0xB800D800)
#endif

/***********************************************************************/


/***EBU Clock Control Register***/
#define INCA_IP_EBU_EBU_CLC                      ((volatile u32*)(INCA_IP_EBU+ 0x0000))
#define INCA_IP_EBU_EBU_CLC_DISS                            (1 << 1)
#define INCA_IP_EBU_EBU_CLC_DISR                            (1 << 0)

/***EBU Global Control Register***/
#define INCA_IP_EBU_EBU_CON                      ((volatile u32*)(INCA_IP_EBU+ 0x0010))
#define INCA_IP_EBU_EBU_CON_DTACS (value)              (((( 1 << 3) - 1) & (value)) << 20)
#define INCA_IP_EBU_EBU_CON_DTARW (value)              (((( 1 << 3) - 1) & (value)) << 16)
#define INCA_IP_EBU_EBU_CON_TOUTC (value)              (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_EBU_EBU_CON_ARBMODE (value)            (((( 1 << 2) - 1) & (value)) << 6)
#define INCA_IP_EBU_EBU_CON_ARBSYNC                      (1 << 5)
#define INCA_IP_EBU_EBU_CON_1                              (1 << 3)

/***EBU Address Select Register 0***/
#define INCA_IP_EBU_EBU_ADDSEL0                  ((volatile u32*)(INCA_IP_EBU+ 0x0020))
#define INCA_IP_EBU_EBU_ADDSEL0_BASE (value)               (((( 1 << 20) - 1) & (value)) << 12)
#define INCA_IP_EBU_EBU_ADDSEL0_MASK (value)               (((( 1 << 4) - 1) & (value)) << 4)
#define INCA_IP_EBU_EBU_ADDSEL0_MIRRORE                      (1 << 1)
#define INCA_IP_EBU_EBU_ADDSEL0_REGEN                          (1 << 0)

/***EBU Address Select Register 1***/
#define INCA_IP_EBU_EBU_ADDSEL1                  ((volatile u32*)(INCA_IP_EBU+ 0x0024))
#define INCA_IP_EBU_EBU_ADDSEL1_BASE (value)               (((( 1 << 20) - 1) & (value)) << 12)
#define INCA_IP_EBU_EBU_ADDSEL1_MASK (value)               (((( 1 << 4) - 1) & (value)) << 4)
#define INCA_IP_EBU_EBU_ADDSEL1_MIRRORE                      (1 << 1)
#define INCA_IP_EBU_EBU_ADDSEL1_REGEN                          (1 << 0)

/***EBU Address Select Register 2***/
#define INCA_IP_EBU_EBU_ADDSEL2                  ((volatile u32*)(INCA_IP_EBU+ 0x0028))
#define INCA_IP_EBU_EBU_ADDSEL2_BASE (value)               (((( 1 << 20) - 1) & (value)) << 12)
#define INCA_IP_EBU_EBU_ADDSEL2_MASK (value)               (((( 1 << 4) - 1) & (value)) << 4)
#define INCA_IP_EBU_EBU_ADDSEL2_MIRRORE                      (1 << 1)
#define INCA_IP_EBU_EBU_ADDSEL2_REGEN                          (1 << 0)

/***EBU Bus Configuration Register 0***/
#define INCA_IP_EBU_EBU_BUSCON0                  ((volatile u32*)(INCA_IP_EBU+ 0x0060))
#define INCA_IP_EBU_EBU_BUSCON0_WRDIS                          (1 << 31)
#define INCA_IP_EBU_EBU_BUSCON0_ALEC (value)               (((( 1 << 2) - 1) & (value)) << 29)
#define INCA_IP_EBU_EBU_BUSCON0_BCGEN (value)              (((( 1 << 2) - 1) & (value)) << 27)
#define INCA_IP_EBU_EBU_BUSCON0_AGEN (value)               (((( 1 << 2) - 1) & (value)) << 24)
#define INCA_IP_EBU_EBU_BUSCON0_CMULTR (value)             (((( 1 << 2) - 1) & (value)) << 22)
#define INCA_IP_EBU_EBU_BUSCON0_WAIT (value)               (((( 1 << 2) - 1) & (value)) << 20)
#define INCA_IP_EBU_EBU_BUSCON0_WAITINV                      (1 << 19)
#define INCA_IP_EBU_EBU_BUSCON0_SETUP                          (1 << 18)
#define INCA_IP_EBU_EBU_BUSCON0_PORTW (value)              (((( 1 << 2) - 1) & (value)) << 16)
#define INCA_IP_EBU_EBU_BUSCON0_WAITRDC (value)            (((( 1 << 7) - 1) & (value)) << 9)
#define INCA_IP_EBU_EBU_BUSCON0_WAITWRC (value)            (((( 1 << 3) - 1) & (value)) << 6)
#define INCA_IP_EBU_EBU_BUSCON0_HOLDC (value)              (((( 1 << 2) - 1) & (value)) << 4)
#define INCA_IP_EBU_EBU_BUSCON0_RECOVC (value)             (((( 1 << 2) - 1) & (value)) << 2)
#define INCA_IP_EBU_EBU_BUSCON0_CMULT (value)              (((( 1 << 2) - 1) & (value)) << 0)

/***EBU Bus Configuration Register 1***/
#define INCA_IP_EBU_EBU_BUSCON1                  ((volatile u32*)(INCA_IP_EBU+ 0x0064))
#define INCA_IP_EBU_EBU_BUSCON1_WRDIS                          (1 << 31)
#define INCA_IP_EBU_EBU_BUSCON1_ALEC (value)               (((( 1 << 2) - 1) & (value)) << 29)
#define INCA_IP_EBU_EBU_BUSCON1_BCGEN (value)              (((( 1 << 2) - 1) & (value)) << 27)
#define INCA_IP_EBU_EBU_BUSCON1_AGEN (value)               (((( 1 << 2) - 1) & (value)) << 24)
#define INCA_IP_EBU_EBU_BUSCON1_CMULTR (value)             (((( 1 << 2) - 1) & (value)) << 22)
#define INCA_IP_EBU_EBU_BUSCON1_WAIT (value)               (((( 1 << 2) - 1) & (value)) << 20)
#define INCA_IP_EBU_EBU_BUSCON1_WAITINV                      (1 << 19)
#define INCA_IP_EBU_EBU_BUSCON1_SETUP                          (1 << 18)
#define INCA_IP_EBU_EBU_BUSCON1_PORTW (value)              (((( 1 << 2) - 1) & (value)) << 16)
#define INCA_IP_EBU_EBU_BUSCON1_WAITRDC (value)            (((( 1 << 7) - 1) & (value)) << 9)
#define INCA_IP_EBU_EBU_BUSCON1_WAITWRC (value)            (((( 1 << 3) - 1) & (value)) << 6)
#define INCA_IP_EBU_EBU_BUSCON1_HOLDC (value)              (((( 1 << 2) - 1) & (value)) << 4)
#define INCA_IP_EBU_EBU_BUSCON1_RECOVC (value)             (((( 1 << 2) - 1) & (value)) << 2)
#define INCA_IP_EBU_EBU_BUSCON1_CMULT (value)              (((( 1 << 2) - 1) & (value)) << 0)

/***EBU Bus Configuration Register 2***/
#define INCA_IP_EBU_EBU_BUSCON2                  ((volatile u32*)(INCA_IP_EBU+ 0x0068))
#define INCA_IP_EBU_EBU_BUSCON2_WRDIS                          (1 << 31)
#define INCA_IP_EBU_EBU_BUSCON2_ALEC (value)               (((( 1 << 2) - 1) & (value)) << 29)
#define INCA_IP_EBU_EBU_BUSCON2_BCGEN (value)              (((( 1 << 2) - 1) & (value)) << 27)
#define INCA_IP_EBU_EBU_BUSCON2_AGEN (value)               (((( 1 << 2) - 1) & (value)) << 24)
#define INCA_IP_EBU_EBU_BUSCON2_CMULTR (value)             (((( 1 << 2) - 1) & (value)) << 22)
#define INCA_IP_EBU_EBU_BUSCON2_WAIT (value)               (((( 1 << 2) - 1) & (value)) << 20)
#define INCA_IP_EBU_EBU_BUSCON2_WAITINV                      (1 << 19)
#define INCA_IP_EBU_EBU_BUSCON2_SETUP                          (1 << 18)
#define INCA_IP_EBU_EBU_BUSCON2_PORTW (value)              (((( 1 << 2) - 1) & (value)) << 16)
#define INCA_IP_EBU_EBU_BUSCON2_WAITRDC (value)            (((( 1 << 7) - 1) & (value)) << 9)
#define INCA_IP_EBU_EBU_BUSCON2_WAITWRC (value)            (((( 1 << 3) - 1) & (value)) << 6)
#define INCA_IP_EBU_EBU_BUSCON2_HOLDC (value)              (((( 1 << 2) - 1) & (value)) << 4)
#define INCA_IP_EBU_EBU_BUSCON2_RECOVC (value)             (((( 1 << 2) - 1) & (value)) << 2)
#define INCA_IP_EBU_EBU_BUSCON2_CMULT (value)              (((( 1 << 2) - 1) & (value)) << 0)

/***********************************************************************/
/*  Module      :  SDRAM register address and bits                     */
/***********************************************************************/

#define INCA_IP_SDRAM                        (0xBF800000)
/***********************************************************************/


/***MC Access Error Cause Register***/
#define INCA_IP_SDRAM_MC_ERRCAUSE                  ((volatile u32*)(INCA_IP_SDRAM+ 0x0100))
#define INCA_IP_SDRAM_MC_ERRCAUSE_ERR                              (1 << 31)
#define INCA_IP_SDRAM_MC_ERRCAUSE_PORT (value)               (((( 1 << 4) - 1) & (value)) << 16)
#define INCA_IP_SDRAM_MC_ERRCAUSE_CAUSE (value)              (((( 1 << 2) - 1) & (value)) << 0)
#define INCA_IP_SDRAM_MC_ERRCAUSE_Res (value)                (((( 1 << NaN) - 1) & (value)) << NaN)

/***MC Access Error Address Register***/
#define INCA_IP_SDRAM_MC_ERRADDR                   ((volatile u32*)(INCA_IP_SDRAM+ 0x0108))
#define INCA_IP_SDRAM_MC_ERRADDR_ADDR

/***MC I/O General Purpose Register***/
#define INCA_IP_SDRAM_MC_IOGP                      ((volatile u32*)(INCA_IP_SDRAM+ 0x0800))
#define INCA_IP_SDRAM_MC_IOGP_GPR6 (value)               (((( 1 << 4) - 1) & (value)) << 28)
#define INCA_IP_SDRAM_MC_IOGP_GPR5 (value)               (((( 1 << 4) - 1) & (value)) << 24)
#define INCA_IP_SDRAM_MC_IOGP_GPR4 (value)               (((( 1 << 4) - 1) & (value)) << 20)
#define INCA_IP_SDRAM_MC_IOGP_GPR3 (value)               (((( 1 << 4) - 1) & (value)) << 16)
#define INCA_IP_SDRAM_MC_IOGP_GPR2 (value)               (((( 1 << 4) - 1) & (value)) << 12)
#define INCA_IP_SDRAM_MC_IOGP_CPS                              (1 << 11)
#define INCA_IP_SDRAM_MC_IOGP_CLKDELAY (value)          (((( 1 << 3) - 1) & (value)) << 8)
#define INCA_IP_SDRAM_MC_IOGP_CLKRAT (value)             (((( 1 << 4) - 1) & (value)) << 4)
#define INCA_IP_SDRAM_MC_IOGP_RDDEL (value)              (((( 1 << 4) - 1) & (value)) << 0)

/***MC Self Refresh Register***/
#define INCA_IP_SDRAM_MC_SELFRFSH                  ((volatile u32*)(INCA_IP_SDRAM+ 0x0A00))
#define INCA_IP_SDRAM_MC_SELFRFSH_PWDS                            (1 << 1)
#define INCA_IP_SDRAM_MC_SELFRFSH_PWD                              (1 << 0)
#define INCA_IP_SDRAM_MC_SELFRFSH_Res (value)                (((( 1 << 30) - 1) & (value)) << 2)

/***MC Enable Register***/
#define INCA_IP_SDRAM_MC_CTRLENA                   ((volatile u32*)(INCA_IP_SDRAM+ 0x1000))
#define INCA_IP_SDRAM_MC_CTRLENA_ENA                              (1 << 0)
#define INCA_IP_SDRAM_MC_CTRLENA_Res (value)                (((( 1 << 31) - 1) & (value)) << 1)

/***MC Mode Register Setup Code***/
#define INCA_IP_SDRAM_MC_MRSCODE                   ((volatile u32*)(INCA_IP_SDRAM+ 0x1008))
#define INCA_IP_SDRAM_MC_MRSCODE_UMC (value)                (((( 1 << 5) - 1) & (value)) << 7)
#define INCA_IP_SDRAM_MC_MRSCODE_CL (value)                (((( 1 << 3) - 1) & (value)) << 4)
#define INCA_IP_SDRAM_MC_MRSCODE_WT                              (1 << 3)
#define INCA_IP_SDRAM_MC_MRSCODE_BL (value)                (((( 1 << 3) - 1) & (value)) << 0)

/***MC Configuration Data-word Width Register***/
#define INCA_IP_SDRAM_MC_CFGDW                    ((volatile u32*)(INCA_IP_SDRAM+ 0x1010))
#define INCA_IP_SDRAM_MC_CFGDW_DW (value)                (((( 1 << 4) - 1) & (value)) << 0)
#define INCA_IP_SDRAM_MC_CFGDW_Res (value)                (((( 1 << 28) - 1) & (value)) << 4)

/***MC Configuration Physical Bank 0 Register***/
#define INCA_IP_SDRAM_MC_CFGPB0                    ((volatile u32*)(INCA_IP_SDRAM+ 0x1018))
#define INCA_IP_SDRAM_MC_CFGPB0_MCSEN0 (value)             (((( 1 << 4) - 1) & (value)) << 12)
#define INCA_IP_SDRAM_MC_CFGPB0_BANKN0 (value)             (((( 1 << 4) - 1) & (value)) << 8)
#define INCA_IP_SDRAM_MC_CFGPB0_ROWW0 (value)              (((( 1 << 4) - 1) & (value)) << 4)
#define INCA_IP_SDRAM_MC_CFGPB0_COLW0 (value)              (((( 1 << 4) - 1) & (value)) << 0)
#define INCA_IP_SDRAM_MC_CFGPB0_Res (value)                (((( 1 << 16) - 1) & (value)) << 16)

/***MC Latency Register***/
#define INCA_IP_SDRAM_MC_LATENCY                   ((volatile u32*)(INCA_IP_SDRAM+ 0x1038))
#define INCA_IP_SDRAM_MC_LATENCY_TRP (value)                (((( 1 << 4) - 1) & (value)) << 16)
#define INCA_IP_SDRAM_MC_LATENCY_TRAS (value)               (((( 1 << 4) - 1) & (value)) << 12)
#define INCA_IP_SDRAM_MC_LATENCY_TRCD (value)               (((( 1 << 4) - 1) & (value)) << 8)
#define INCA_IP_SDRAM_MC_LATENCY_TDPL (value)               (((( 1 << 4) - 1) & (value)) << 4)
#define INCA_IP_SDRAM_MC_LATENCY_TDAL (value)               (((( 1 << 4) - 1) & (value)) << 0)
#define INCA_IP_SDRAM_MC_LATENCY_Res (value)                (((( 1 << 12) - 1) & (value)) << 20)

/***MC Refresh Cycle Time Register***/
#define INCA_IP_SDRAM_MC_TREFRESH                  ((volatile u32*)(INCA_IP_SDRAM+ 0x1040))
#define INCA_IP_SDRAM_MC_TREFRESH_TREF (value)               (((( 1 << 13) - 1) & (value)) << 0)
#define INCA_IP_SDRAM_MC_TREFRESH_Res (value)                (((( 1 << 19) - 1) & (value)) << 13)

/***********************************************************************/
/*  Module      :  GPTU register address and bits                      */
/***********************************************************************/

#define INCA_IP_GPTU                         (0xB8000300)
/***********************************************************************/


/***GPT Clock Control Register***/
#define INCA_IP_GPTU_GPT_CLC                      ((volatile u32*)(INCA_IP_GPTU+ 0x0000))
#define INCA_IP_GPTU_GPT_CLC_RMC (value)                (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_GPTU_GPT_CLC_DISS                            (1 << 1)
#define INCA_IP_GPTU_GPT_CLC_DISR                            (1 << 0)

/***GPT Timer 3 Control Register***/
#define INCA_IP_GPTU_GPT_T3CON                    ((volatile u32*)(INCA_IP_GPTU+ 0x0014))
#define INCA_IP_GPTU_GPT_T3CON_T3RDIR                        (1 << 15)
#define INCA_IP_GPTU_GPT_T3CON_T3CHDIR                      (1 << 14)
#define INCA_IP_GPTU_GPT_T3CON_T3EDGE                        (1 << 13)
#define INCA_IP_GPTU_GPT_T3CON_BPS1 (value)               (((( 1 << 2) - 1) & (value)) << 11)
#define INCA_IP_GPTU_GPT_T3CON_T3OTL                          (1 << 10)
#define INCA_IP_GPTU_GPT_T3CON_T3UD                            (1 << 7)
#define INCA_IP_GPTU_GPT_T3CON_T3R                              (1 << 6)
#define INCA_IP_GPTU_GPT_T3CON_T3M (value)                (((( 1 << 3) - 1) & (value)) << 3)
#define INCA_IP_GPTU_GPT_T3CON_T3I (value)                (((( 1 << 3) - 1) & (value)) << 0)

/***GPT Write Hardware Modified Timer 3 Control Register
If set and clear bit are written concurrently with 1, the associated bit is not changed.***/
#define INCA_IP_GPTU_GPT_WHBT3CON                 ((volatile u32*)(INCA_IP_GPTU+ 0x004C))
#define INCA_IP_GPTU_GPT_WHBT3CON_SETT3CHDIR                (1 << 15)
#define INCA_IP_GPTU_GPT_WHBT3CON_CLRT3CHDIR                (1 << 14)
#define INCA_IP_GPTU_GPT_WHBT3CON_SETT3EDGE                  (1 << 13)
#define INCA_IP_GPTU_GPT_WHBT3CON_CLRT3EDGE                  (1 << 12)
#define INCA_IP_GPTU_GPT_WHBT3CON_SETT3OTL                  (1 << 11)
#define INCA_IP_GPTU_GPT_WHBT3CON_CLRT3OTL                  (1 << 10)

/***GPT Timer 2 Control Register***/
#define INCA_IP_GPTU_GPT_T2CON                    ((volatile u32*)(INCA_IP_GPTU+ 0x0010))
#define INCA_IP_GPTU_GPT_T2CON_TxRDIR                        (1 << 15)
#define INCA_IP_GPTU_GPT_T2CON_TxCHDIR                      (1 << 14)
#define INCA_IP_GPTU_GPT_T2CON_TxEDGE                        (1 << 13)
#define INCA_IP_GPTU_GPT_T2CON_TxIRDIS                      (1 << 12)
#define INCA_IP_GPTU_GPT_T2CON_TxRC                            (1 << 9)
#define INCA_IP_GPTU_GPT_T2CON_TxUD                            (1 << 7)
#define INCA_IP_GPTU_GPT_T2CON_TxR                              (1 << 6)
#define INCA_IP_GPTU_GPT_T2CON_TxM (value)                (((( 1 << 3) - 1) & (value)) << 3)
#define INCA_IP_GPTU_GPT_T2CON_TxI (value)                (((( 1 << 3) - 1) & (value)) << 0)

/***GPT Timer 4 Control Register***/
#define INCA_IP_GPTU_GPT_T4CON                    ((volatile u32*)(INCA_IP_GPTU+ 0x0018))
#define INCA_IP_GPTU_GPT_T4CON_TxRDIR                        (1 << 15)
#define INCA_IP_GPTU_GPT_T4CON_TxCHDIR                      (1 << 14)
#define INCA_IP_GPTU_GPT_T4CON_TxEDGE                        (1 << 13)
#define INCA_IP_GPTU_GPT_T4CON_TxIRDIS                      (1 << 12)
#define INCA_IP_GPTU_GPT_T4CON_TxRC                            (1 << 9)
#define INCA_IP_GPTU_GPT_T4CON_TxUD                            (1 << 7)
#define INCA_IP_GPTU_GPT_T4CON_TxR                              (1 << 6)
#define INCA_IP_GPTU_GPT_T4CON_TxM (value)                (((( 1 << 3) - 1) & (value)) << 3)
#define INCA_IP_GPTU_GPT_T4CON_TxI (value)                (((( 1 << 3) - 1) & (value)) << 0)

/***GPT Write HW Modified Timer 2 Control Register If set
 and clear bit are written concurrently with 1, the associated bit is not changed.***/
#define INCA_IP_GPTU_GPT_WHBT2CON                 ((volatile u32*)(INCA_IP_GPTU+ 0x0048))
#define INCA_IP_GPTU_GPT_WHBT2CON_SETTxCHDIR                (1 << 15)
#define INCA_IP_GPTU_GPT_WHBT2CON_CLRTxCHDIR                (1 << 14)
#define INCA_IP_GPTU_GPT_WHBT2CON_SETTxEDGE                  (1 << 13)
#define INCA_IP_GPTU_GPT_WHBT2CON_CLRTxEDGE                  (1 << 12)

/***GPT Write HW Modified Timer 4 Control Register If set
 and clear bit are written concurrently with 1, the associated bit is not changed.***/
#define INCA_IP_GPTU_GPT_WHBT4CON                 ((volatile u32*)(INCA_IP_GPTU+ 0x0050))
#define INCA_IP_GPTU_GPT_WHBT4CON_SETTxCHDIR                (1 << 15)
#define INCA_IP_GPTU_GPT_WHBT4CON_CLRTxCHDIR                (1 << 14)
#define INCA_IP_GPTU_GPT_WHBT4CON_SETTxEDGE                  (1 << 13)
#define INCA_IP_GPTU_GPT_WHBT4CON_CLRTxEDGE                  (1 << 12)

/***GPT Capture Reload Register***/
#define INCA_IP_GPTU_GPT_CAPREL                   ((volatile u32*)(INCA_IP_GPTU+ 0x0030))
#define INCA_IP_GPTU_GPT_CAPREL_CAPREL (value)             (((( 1 << 16) - 1) & (value)) << 0)

/***GPT Timer 2 Register***/
#define INCA_IP_GPTU_GPT_T2                       ((volatile u32*)(INCA_IP_GPTU+ 0x0034))
#define INCA_IP_GPTU_GPT_T2_TVAL (value)               (((( 1 << 16) - 1) & (value)) << 0)

/***GPT Timer 3 Register***/
#define INCA_IP_GPTU_GPT_T3                       ((volatile u32*)(INCA_IP_GPTU+ 0x0038))
#define INCA_IP_GPTU_GPT_T3_TVAL (value)               (((( 1 << 16) - 1) & (value)) << 0)

/***GPT Timer 4 Register***/
#define INCA_IP_GPTU_GPT_T4                       ((volatile u32*)(INCA_IP_GPTU+ 0x003C))
#define INCA_IP_GPTU_GPT_T4_TVAL (value)               (((( 1 << 16) - 1) & (value)) << 0)

/***GPT Timer 5 Register***/
#define INCA_IP_GPTU_GPT_T5                       ((volatile u32*)(INCA_IP_GPTU+ 0x0040))
#define INCA_IP_GPTU_GPT_T5_TVAL (value)               (((( 1 << 16) - 1) & (value)) << 0)

/***GPT Timer 6 Register***/
#define INCA_IP_GPTU_GPT_T6                       ((volatile u32*)(INCA_IP_GPTU+ 0x0044))
#define INCA_IP_GPTU_GPT_T6_TVAL (value)               (((( 1 << 16) - 1) & (value)) << 0)

/***GPT Timer 6 Control Register***/
#define INCA_IP_GPTU_GPT_T6CON                    ((volatile u32*)(INCA_IP_GPTU+ 0x0020))
#define INCA_IP_GPTU_GPT_T6CON_T6SR                            (1 << 15)
#define INCA_IP_GPTU_GPT_T6CON_T6CLR                          (1 << 14)
#define INCA_IP_GPTU_GPT_T6CON_BPS2 (value)               (((( 1 << 2) - 1) & (value)) << 11)
#define INCA_IP_GPTU_GPT_T6CON_T6OTL                          (1 << 10)
#define INCA_IP_GPTU_GPT_T6CON_T6UD                            (1 << 7)
#define INCA_IP_GPTU_GPT_T6CON_T6R                              (1 << 6)
#define INCA_IP_GPTU_GPT_T6CON_T6M (value)                (((( 1 << 3) - 1) & (value)) << 3)
#define INCA_IP_GPTU_GPT_T6CON_T6I (value)                (((( 1 << 3) - 1) & (value)) << 0)

/***GPT Write HW Modified Timer 6 Control Register If set
 and clear bit are written concurrently with 1, the associated bit is not changed.***/
#define INCA_IP_GPTU_GPT_WHBT6CON                 ((volatile u32*)(INCA_IP_GPTU+ 0x0054))
#define INCA_IP_GPTU_GPT_WHBT6CON_SETT6OTL                  (1 << 11)
#define INCA_IP_GPTU_GPT_WHBT6CON_CLRT6OTL                  (1 << 10)

/***GPT Timer 5 Control Register***/
#define INCA_IP_GPTU_GPT_T5CON                    ((volatile u32*)(INCA_IP_GPTU+ 0x001C))
#define INCA_IP_GPTU_GPT_T5CON_T5SC                            (1 << 15)
#define INCA_IP_GPTU_GPT_T5CON_T5CLR                          (1 << 14)
#define INCA_IP_GPTU_GPT_T5CON_CI (value)                (((( 1 << 2) - 1) & (value)) << 12)
#define INCA_IP_GPTU_GPT_T5CON_T5CC                            (1 << 11)
#define INCA_IP_GPTU_GPT_T5CON_CT3                              (1 << 10)
#define INCA_IP_GPTU_GPT_T5CON_T5RC                            (1 << 9)
#define INCA_IP_GPTU_GPT_T5CON_T5UDE                          (1 << 8)
#define INCA_IP_GPTU_GPT_T5CON_T5UD                            (1 << 7)
#define INCA_IP_GPTU_GPT_T5CON_T5R                              (1 << 6)
#define INCA_IP_GPTU_GPT_T5CON_T5M (value)                (((( 1 << 3) - 1) & (value)) << 3)
#define INCA_IP_GPTU_GPT_T5CON_T5I (value)                (((( 1 << 3) - 1) & (value)) << 0)

/***********************************************************************/
/*  Module      :  IOM register address and bits                       */
/***********************************************************************/

#define INCA_IP_IOM                          (0xBF105000)
/***********************************************************************/


/***Receive FIFO***/
#define INCA_IP_IOM_RFIFO                        ((volatile u32*)(INCA_IP_IOM+ 0x0000))
#define INCA_IP_IOM_RFIFO_RXD (value)                (((( 1 << 8) - 1) & (value)) << 0)

/***Transmit FIFO***/
#define INCA_IP_IOM_XFIFO                        ((volatile u32*)(INCA_IP_IOM+ 0x0000))
#define INCA_IP_IOM_XFIFO_TXD (value)                (((( 1 << 8) - 1) & (value)) << 0)

/***Interrupt Status Register HDLC***/
#define INCA_IP_IOM_ISTAH                        ((volatile u32*)(INCA_IP_IOM+ 0x0080))
#define INCA_IP_IOM_ISTAH_RME                              (1 << 7)
#define INCA_IP_IOM_ISTAH_RPF                              (1 << 6)
#define INCA_IP_IOM_ISTAH_RFO                              (1 << 5)
#define INCA_IP_IOM_ISTAH_XPR                              (1 << 4)
#define INCA_IP_IOM_ISTAH_XMR                              (1 << 3)
#define INCA_IP_IOM_ISTAH_XDU                              (1 << 2)

/***Interrupt Mask Register HDLC***/
#define INCA_IP_IOM_MASKH                        ((volatile u32*)(INCA_IP_IOM+ 0x0080))
#define INCA_IP_IOM_MASKH_RME                              (1 << 7)
#define INCA_IP_IOM_MASKH_RPF                              (1 << 6)
#define INCA_IP_IOM_MASKH_RFO                              (1 << 5)
#define INCA_IP_IOM_MASKH_XPR                              (1 << 4)
#define INCA_IP_IOM_MASKH_XMR                              (1 << 3)
#define INCA_IP_IOM_MASKH_XDU                              (1 << 2)

/***Status Register***/
#define INCA_IP_IOM_STAR                         ((volatile u32*)(INCA_IP_IOM+ 0x0084))
#define INCA_IP_IOM_STAR_XDOV                            (1 << 7)
#define INCA_IP_IOM_STAR_XFW                              (1 << 6)
#define INCA_IP_IOM_STAR_RACI                            (1 << 3)
#define INCA_IP_IOM_STAR_XACI                            (1 << 1)

/***Command Register***/
#define INCA_IP_IOM_CMDR                         ((volatile u32*)(INCA_IP_IOM+ 0x0084))
#define INCA_IP_IOM_CMDR_RMC                              (1 << 7)
#define INCA_IP_IOM_CMDR_RRES                            (1 << 6)
#define INCA_IP_IOM_CMDR_XTF                              (1 << 3)
#define INCA_IP_IOM_CMDR_XME                              (1 << 1)
#define INCA_IP_IOM_CMDR_XRES                            (1 << 0)

/***Mode Register***/
#define INCA_IP_IOM_MODEH                        ((volatile u32*)(INCA_IP_IOM+ 0x0088))
#define INCA_IP_IOM_MODEH_MDS2                            (1 << 7)
#define INCA_IP_IOM_MODEH_MDS1                            (1 << 6)
#define INCA_IP_IOM_MODEH_MDS0                            (1 << 5)
#define INCA_IP_IOM_MODEH_RAC                              (1 << 3)
#define INCA_IP_IOM_MODEH_DIM2                            (1 << 2)
#define INCA_IP_IOM_MODEH_DIM1                            (1 << 1)
#define INCA_IP_IOM_MODEH_DIM0                            (1 << 0)

/***Extended Mode Register***/
#define INCA_IP_IOM_EXMR                         ((volatile u32*)(INCA_IP_IOM+ 0x008C))
#define INCA_IP_IOM_EXMR_XFBS                            (1 << 7)
#define INCA_IP_IOM_EXMR_RFBS (value)               (((( 1 << 2) - 1) & (value)) << 5)
#define INCA_IP_IOM_EXMR_SRA                              (1 << 4)
#define INCA_IP_IOM_EXMR_XCRC                            (1 << 3)
#define INCA_IP_IOM_EXMR_RCRC                            (1 << 2)
#define INCA_IP_IOM_EXMR_ITF                              (1 << 0)

/***SAPI1 Register***/
#define INCA_IP_IOM_SAP1                         ((volatile u32*)(INCA_IP_IOM+ 0x0094))
#define INCA_IP_IOM_SAP1_SAPI1 (value)              (((( 1 << 6) - 1) & (value)) << 2)
#define INCA_IP_IOM_SAP1_MHA                              (1 << 0)

/***Receive Frame Byte Count Low***/
#define INCA_IP_IOM_RBCL                         ((volatile u32*)(INCA_IP_IOM+ 0x0098))
#define INCA_IP_IOM_RBCL_RBC(value)              (1 << value)


/***SAPI2 Register***/
#define INCA_IP_IOM_SAP2                         ((volatile u32*)(INCA_IP_IOM+ 0x0098))
#define INCA_IP_IOM_SAP2_SAPI2 (value)              (((( 1 << 6) - 1) & (value)) << 2)
#define INCA_IP_IOM_SAP2_MLA                              (1 << 0)

/***Receive Frame Byte Count High***/
#define INCA_IP_IOM_RBCH                         ((volatile u32*)(INCA_IP_IOM+ 0x009C))
#define INCA_IP_IOM_RBCH_OV                              (1 << 4)
#define INCA_IP_IOM_RBCH_RBC11                          (1 << 3)
#define INCA_IP_IOM_RBCH_RBC10                          (1 << 2)
#define INCA_IP_IOM_RBCH_RBC9                            (1 << 1)
#define INCA_IP_IOM_RBCH_RBC8                            (1 << 0)

/***TEI1 Register 1***/
#define INCA_IP_IOM_TEI1                         ((volatile u32*)(INCA_IP_IOM+ 0x009C))
#define INCA_IP_IOM_TEI1_TEI1 (value)               (((( 1 << 7) - 1) & (value)) << 1)
#define INCA_IP_IOM_TEI1_EA                              (1 << 0)

/***Receive Status Register***/
#define INCA_IP_IOM_RSTA                         ((volatile u32*)(INCA_IP_IOM+ 0x00A0))
#define INCA_IP_IOM_RSTA_VFR                              (1 << 7)
#define INCA_IP_IOM_RSTA_RDO                              (1 << 6)
#define INCA_IP_IOM_RSTA_CRC                              (1 << 5)
#define INCA_IP_IOM_RSTA_RAB                              (1 << 4)
#define INCA_IP_IOM_RSTA_SA1                              (1 << 3)
#define INCA_IP_IOM_RSTA_SA0                              (1 << 2)
#define INCA_IP_IOM_RSTA_TA                              (1 << 0)
#define INCA_IP_IOM_RSTA_CR                              (1 << 1)

/***TEI2 Register***/
#define INCA_IP_IOM_TEI2                         ((volatile u32*)(INCA_IP_IOM+ 0x00A0))
#define INCA_IP_IOM_TEI2_TEI2 (value)               (((( 1 << 7) - 1) & (value)) << 1)
#define INCA_IP_IOM_TEI2_EA                              (1 << 0)

/***Test Mode Register HDLC***/
#define INCA_IP_IOM_TMH                          ((volatile u32*)(INCA_IP_IOM+ 0x00A4))
#define INCA_IP_IOM_TMH_TLP                              (1 << 0)

/***Command/Indication Receive 0***/
#define INCA_IP_IOM_CIR0                         ((volatile u32*)(INCA_IP_IOM+ 0x00B8))
#define INCA_IP_IOM_CIR0_CODR0 (value)              (((( 1 << 4) - 1) & (value)) << 4)
#define INCA_IP_IOM_CIR0_CIC0                            (1 << 3)
#define INCA_IP_IOM_CIR0_CIC1                            (1 << 2)
#define INCA_IP_IOM_CIR0_SG                              (1 << 1)
#define INCA_IP_IOM_CIR0_BAS                              (1 << 0)

/***Command/Indication Transmit 0***/
#define INCA_IP_IOM_CIX0                         ((volatile u32*)(INCA_IP_IOM+ 0x00B8))
#define INCA_IP_IOM_CIX0_CODX0 (value)              (((( 1 << 4) - 1) & (value)) << 4)
#define INCA_IP_IOM_CIX0_TBA2                            (1 << 3)
#define INCA_IP_IOM_CIX0_TBA1                            (1 << 2)
#define INCA_IP_IOM_CIX0_TBA0                            (1 << 1)
#define INCA_IP_IOM_CIX0_BAC                              (1 << 0)

/***Command/Indication Receive 1***/
#define INCA_IP_IOM_CIR1                         ((volatile u32*)(INCA_IP_IOM+ 0x00BC))
#define INCA_IP_IOM_CIR1_CODR1 (value)              (((( 1 << 6) - 1) & (value)) << 2)

/***Command/Indication Transmit 1***/
#define INCA_IP_IOM_CIX1                         ((volatile u32*)(INCA_IP_IOM+ 0x00BC))
#define INCA_IP_IOM_CIX1_CODX1 (value)              (((( 1 << 6) - 1) & (value)) << 2)
#define INCA_IP_IOM_CIX1_CICW                            (1 << 1)
#define INCA_IP_IOM_CIX1_CI1E                            (1 << 0)

/***Controller Data Access Reg. (CH10)***/
#define INCA_IP_IOM_CDA10                        ((volatile u32*)(INCA_IP_IOM+ 0x0100))
#define INCA_IP_IOM_CDA10_CDA (value)                (((( 1 << 8) - 1) & (value)) << 0)

/***Controller Data Access Reg. (CH11)***/
#define INCA_IP_IOM_CDA11                        ((volatile u32*)(INCA_IP_IOM+ 0x0104))
#define INCA_IP_IOM_CDA11_CDA (value)                (((( 1 << 8) - 1) & (value)) << 0)

/***Controller Data Access Reg. (CH20)***/
#define INCA_IP_IOM_CDA20                        ((volatile u32*)(INCA_IP_IOM+ 0x0108))
#define INCA_IP_IOM_CDA20_CDA (value)                (((( 1 << 8) - 1) & (value)) << 0)

/***Controller Data Access Reg. (CH21)***/
#define INCA_IP_IOM_CDA21                        ((volatile u32*)(INCA_IP_IOM+ 0x010C))
#define INCA_IP_IOM_CDA21_CDA (value)                (((( 1 << 8) - 1) & (value)) << 0)

/***Time Slot and Data Port Sel. (CH10)***/
#define INCA_IP_IOM_CDA_TSDP10                   ((volatile u32*)(INCA_IP_IOM+ 0x0110))
#define INCA_IP_IOM_CDA_TSDP10_DPS                              (1 << 7)
#define INCA_IP_IOM_CDA_TSDP10_TSS (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Time Slot and Data Port Sel. (CH11)***/
#define INCA_IP_IOM_CDA_TSDP11                   ((volatile u32*)(INCA_IP_IOM+ 0x0114))
#define INCA_IP_IOM_CDA_TSDP11_DPS                              (1 << 7)
#define INCA_IP_IOM_CDA_TSDP11_TSS (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Time Slot and Data Port Sel. (CH20)***/
#define INCA_IP_IOM_CDA_TSDP20                   ((volatile u32*)(INCA_IP_IOM+ 0x0118))
#define INCA_IP_IOM_CDA_TSDP20_DPS                              (1 << 7)
#define INCA_IP_IOM_CDA_TSDP20_TSS (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Time Slot and Data Port Sel. (CH21)***/
#define INCA_IP_IOM_CDA_TSDP21                   ((volatile u32*)(INCA_IP_IOM+ 0x011C))
#define INCA_IP_IOM_CDA_TSDP21_DPS                              (1 << 7)
#define INCA_IP_IOM_CDA_TSDP21_TSS (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Time Slot and Data Port Sel. (CH10)***/
#define INCA_IP_IOM_CO_TSDP10                    ((volatile u32*)(INCA_IP_IOM+ 0x0120))
#define INCA_IP_IOM_CO_TSDP10_DPS                              (1 << 7)
#define INCA_IP_IOM_CO_TSDP10_TSS (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Time Slot and Data Port Sel. (CH11)***/
#define INCA_IP_IOM_CO_TSDP11                    ((volatile u32*)(INCA_IP_IOM+ 0x0124))
#define INCA_IP_IOM_CO_TSDP11_DPS                              (1 << 7)
#define INCA_IP_IOM_CO_TSDP11_TSS (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Time Slot and Data Port Sel. (CH20)***/
#define INCA_IP_IOM_CO_TSDP20                    ((volatile u32*)(INCA_IP_IOM+ 0x0128))
#define INCA_IP_IOM_CO_TSDP20_DPS                              (1 << 7)
#define INCA_IP_IOM_CO_TSDP20_TSS (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Time Slot and Data Port Sel. (CH21)***/
#define INCA_IP_IOM_CO_TSDP21                    ((volatile u32*)(INCA_IP_IOM+ 0x012C))
#define INCA_IP_IOM_CO_TSDP21_DPS                              (1 << 7)
#define INCA_IP_IOM_CO_TSDP21_TSS (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Ctrl. Reg. Contr. Data Access CH1x***/
#define INCA_IP_IOM_CDA1_CR                      ((volatile u32*)(INCA_IP_IOM+ 0x0138))
#define INCA_IP_IOM_CDA1_CR_EN_TBM                        (1 << 5)
#define INCA_IP_IOM_CDA1_CR_EN_I1                          (1 << 4)
#define INCA_IP_IOM_CDA1_CR_EN_I0                          (1 << 3)
#define INCA_IP_IOM_CDA1_CR_EN_O1                          (1 << 2)
#define INCA_IP_IOM_CDA1_CR_EN_O0                          (1 << 1)
#define INCA_IP_IOM_CDA1_CR_SWAP                            (1 << 0)

/***Ctrl. Reg. Contr. Data Access CH1x***/
#define INCA_IP_IOM_CDA2_CR                      ((volatile u32*)(INCA_IP_IOM+ 0x013C))
#define INCA_IP_IOM_CDA2_CR_EN_TBM                        (1 << 5)
#define INCA_IP_IOM_CDA2_CR_EN_I1                          (1 << 4)
#define INCA_IP_IOM_CDA2_CR_EN_I0                          (1 << 3)
#define INCA_IP_IOM_CDA2_CR_EN_O1                          (1 << 2)
#define INCA_IP_IOM_CDA2_CR_EN_O0                          (1 << 1)
#define INCA_IP_IOM_CDA2_CR_SWAP                            (1 << 0)

/***Control Register B-Channel Data***/
#define INCA_IP_IOM_BCHA_CR                      ((volatile u32*)(INCA_IP_IOM+ 0x0144))
#define INCA_IP_IOM_BCHA_CR_EN_BC2                        (1 << 4)
#define INCA_IP_IOM_BCHA_CR_EN_BC1                        (1 << 3)

/***Control Register B-Channel Data***/
#define INCA_IP_IOM_BCHB_CR                      ((volatile u32*)(INCA_IP_IOM+ 0x0148))
#define INCA_IP_IOM_BCHB_CR_EN_BC2                        (1 << 4)
#define INCA_IP_IOM_BCHB_CR_EN_BC1                        (1 << 3)

/***Control Reg. for HDLC and CI1 Data***/
#define INCA_IP_IOM_DCI_CR                       ((volatile u32*)(INCA_IP_IOM+ 0x014C))
#define INCA_IP_IOM_DCI_CR_DPS_CI1                      (1 << 7)
#define INCA_IP_IOM_DCI_CR_EN_CI1                        (1 << 6)
#define INCA_IP_IOM_DCI_CR_EN_D                            (1 << 5)

/***Control Reg. for HDLC and CI1 Data***/
#define INCA_IP_IOM_DCIC_CR                      ((volatile u32*)(INCA_IP_IOM+ 0x014C))
#define INCA_IP_IOM_DCIC_CR_DPS_CI0                      (1 << 7)
#define INCA_IP_IOM_DCIC_CR_EN_CI0                        (1 << 6)
#define INCA_IP_IOM_DCIC_CR_DPS_D                          (1 << 5)

/***Control Reg. Serial Data Strobe x***/
#define INCA_IP_IOM_SDS_CR                       ((volatile u32*)(INCA_IP_IOM+ 0x0154))
#define INCA_IP_IOM_SDS_CR_ENS_TSS                      (1 << 7)
#define INCA_IP_IOM_SDS_CR_ENS_TSS_1                  (1 << 6)
#define INCA_IP_IOM_SDS_CR_ENS_TSS_3                  (1 << 5)
#define INCA_IP_IOM_SDS_CR_TSS (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Control Register IOM Data***/
#define INCA_IP_IOM_IOM_CR                       ((volatile u32*)(INCA_IP_IOM+ 0x015C))
#define INCA_IP_IOM_IOM_CR_SPU                              (1 << 7)
#define INCA_IP_IOM_IOM_CR_CI_CS                          (1 << 5)
#define INCA_IP_IOM_IOM_CR_TIC_DIS                      (1 << 4)
#define INCA_IP_IOM_IOM_CR_EN_BCL                        (1 << 3)
#define INCA_IP_IOM_IOM_CR_CLKM                            (1 << 2)
#define INCA_IP_IOM_IOM_CR_Res                              (1 << 1)
#define INCA_IP_IOM_IOM_CR_DIS_IOM                      (1 << 0)

/***Synchronous Transfer Interrupt***/
#define INCA_IP_IOM_STI                          ((volatile u32*)(INCA_IP_IOM+ 0x0160))
#define INCA_IP_IOM_STI_STOV21                        (1 << 7)
#define INCA_IP_IOM_STI_STOV20                        (1 << 6)
#define INCA_IP_IOM_STI_STOV11                        (1 << 5)
#define INCA_IP_IOM_STI_STOV10                        (1 << 4)
#define INCA_IP_IOM_STI_STI21                          (1 << 3)
#define INCA_IP_IOM_STI_STI20                          (1 << 2)
#define INCA_IP_IOM_STI_STI11                          (1 << 1)
#define INCA_IP_IOM_STI_STI10                          (1 << 0)

/***Acknowledge Synchronous Transfer Interrupt***/
#define INCA_IP_IOM_ASTI                         ((volatile u32*)(INCA_IP_IOM+ 0x0160))
#define INCA_IP_IOM_ASTI_ACK21                          (1 << 3)
#define INCA_IP_IOM_ASTI_ACK20                          (1 << 2)
#define INCA_IP_IOM_ASTI_ACK11                          (1 << 1)
#define INCA_IP_IOM_ASTI_ACK10                          (1 << 0)

/***Mask Synchronous Transfer Interrupt***/
#define INCA_IP_IOM_MSTI                         ((volatile u32*)(INCA_IP_IOM+ 0x0164))
#define INCA_IP_IOM_MSTI_STOV21                        (1 << 7)
#define INCA_IP_IOM_MSTI_STOV20                        (1 << 6)
#define INCA_IP_IOM_MSTI_STOV11                        (1 << 5)
#define INCA_IP_IOM_MSTI_STOV10                        (1 << 4)
#define INCA_IP_IOM_MSTI_STI21                          (1 << 3)
#define INCA_IP_IOM_MSTI_STI20                          (1 << 2)
#define INCA_IP_IOM_MSTI_STI11                          (1 << 1)
#define INCA_IP_IOM_MSTI_STI10                          (1 << 0)

/***Configuration Register for Serial Data Strobes***/
#define INCA_IP_IOM_SDS_CONF                    ((volatile u32*)(INCA_IP_IOM+ 0x0168))
#define INCA_IP_IOM_SDS_CONF_SDS_BCL                      (1 << 0)

/***Monitoring CDA Bits***/
#define INCA_IP_IOM_MCDA                         ((volatile u32*)(INCA_IP_IOM+ 0x016C))
#define INCA_IP_IOM_MCDA_MCDA21 (value)             (((( 1 << 2) - 1) & (value)) << 6)
#define INCA_IP_IOM_MCDA_MCDA20 (value)             (((( 1 << 2) - 1) & (value)) << 4)
#define INCA_IP_IOM_MCDA_MCDA11 (value)             (((( 1 << 2) - 1) & (value)) << 2)
#define INCA_IP_IOM_MCDA_MCDA10 (value)             (((( 1 << 2) - 1) & (value)) << 0)

/***********************************************************************/
/*  Module      :  ASC register address and bits                       */
/***********************************************************************/

#if defined(CONFIG_INCA_IP)
#define INCA_IP_ASC                          (0xB8000400)
#elif defined(CONFIG_PURPLE)
#define INCA_IP_ASC                          (0xBE500000)
#endif

/***********************************************************************/


/***ASC Port Input Select Register***/
#define INCA_IP_ASC_ASC_PISEL                    ((volatile u32*)(INCA_IP_ASC+ 0x0004))
#define INCA_IP_ASC_ASC_PISEL_RIS                              (1 << 0)

/***ASC Control Register***/
#define INCA_IP_ASC_ASC_CON                      ((volatile u32*)(INCA_IP_ASC+ 0x0010))
#define INCA_IP_ASC_ASC_CON_R                              (1 << 15)
#define INCA_IP_ASC_ASC_CON_LB                              (1 << 14)
#define INCA_IP_ASC_ASC_CON_BRS                              (1 << 13)
#define INCA_IP_ASC_ASC_CON_ODD                              (1 << 12)
#define INCA_IP_ASC_ASC_CON_FDE                              (1 << 11)
#define INCA_IP_ASC_ASC_CON_OE                              (1 << 10)
#define INCA_IP_ASC_ASC_CON_FE                              (1 << 9)
#define INCA_IP_ASC_ASC_CON_PE                              (1 << 8)
#define INCA_IP_ASC_ASC_CON_OEN                              (1 << 7)
#define INCA_IP_ASC_ASC_CON_FEN                              (1 << 6)
#define INCA_IP_ASC_ASC_CON_PENRXDI                  (1 << 5)
#define INCA_IP_ASC_ASC_CON_REN                              (1 << 4)
#define INCA_IP_ASC_ASC_CON_STP                              (1 << 3)
#define INCA_IP_ASC_ASC_CON_M (value)                (((( 1 << 3) - 1) & (value)) << 0)

/***ASC Write Hardware Modified Control Register***/
#define INCA_IP_ASC_ASC_WHBCON                   ((volatile u32*)(INCA_IP_ASC+ 0x0050))
#define INCA_IP_ASC_ASC_WHBCON_SETOE                          (1 << 13)
#define INCA_IP_ASC_ASC_WHBCON_SETFE                          (1 << 12)
#define INCA_IP_ASC_ASC_WHBCON_SETPE                          (1 << 11)
#define INCA_IP_ASC_ASC_WHBCON_CLROE                          (1 << 10)
#define INCA_IP_ASC_ASC_WHBCON_CLRFE                          (1 << 9)
#define INCA_IP_ASC_ASC_WHBCON_CLRPE                          (1 << 8)
#define INCA_IP_ASC_ASC_WHBCON_SETREN                        (1 << 5)
#define INCA_IP_ASC_ASC_WHBCON_CLRREN                        (1 << 4)

/***ASC Baudrate Timer/Reload Register***/
#define INCA_IP_ASC_ASC_BTR                      ((volatile u32*)(INCA_IP_ASC+ 0x0014))
#define INCA_IP_ASC_ASC_BTR_BR_VALUE (value)          (((( 1 << 13) - 1) & (value)) << 0)

/***ASC Fractional Divider Register***/
#define INCA_IP_ASC_ASC_FDV                      ((volatile u32*)(INCA_IP_ASC+ 0x0018))
#define INCA_IP_ASC_ASC_FDV_FD_VALUE (value)          (((( 1 << 9) - 1) & (value)) << 0)

/***ASC IrDA Pulse Mode/Width Register***/
#define INCA_IP_ASC_ASC_PMW                      ((volatile u32*)(INCA_IP_ASC+ 0x001C))
#define INCA_IP_ASC_ASC_PMW_IRPW                            (1 << 8)
#define INCA_IP_ASC_ASC_PMW_PW_VALUE (value)          (((( 1 << 8) - 1) & (value)) << 0)

/***ASC Transmit Buffer Register***/
#define INCA_IP_ASC_ASC_TBUF                    ((volatile u32*)(INCA_IP_ASC+ 0x0020))
#define INCA_IP_ASC_ASC_TBUF_TD_VALUE (value)          (((( 1 << 9) - 1) & (value)) << 0)

/***ASC Receive Buffer Register***/
#define INCA_IP_ASC_ASC_RBUF                    ((volatile u32*)(INCA_IP_ASC+ 0x0024))
#define INCA_IP_ASC_ASC_RBUF_RD_VALUE (value)          (((( 1 << 9) - 1) & (value)) << 0)

/***ASC Autobaud Control Register***/
#define INCA_IP_ASC_ASC_ABCON                    ((volatile u32*)(INCA_IP_ASC+ 0x0030))
#define INCA_IP_ASC_ASC_ABCON_RXINV                          (1 << 11)
#define INCA_IP_ASC_ASC_ABCON_TXINV                          (1 << 10)
#define INCA_IP_ASC_ASC_ABCON_ABEM (value)               (((( 1 << 2) - 1) & (value)) << 8)
#define INCA_IP_ASC_ASC_ABCON_FCDETEN                      (1 << 4)
#define INCA_IP_ASC_ASC_ABCON_ABDETEN                      (1 << 3)
#define INCA_IP_ASC_ASC_ABCON_ABSTEN                        (1 << 2)
#define INCA_IP_ASC_ASC_ABCON_AUREN                          (1 << 1)
#define INCA_IP_ASC_ASC_ABCON_ABEN                            (1 << 0)

/***Receive FIFO Control Register***/
#define INCA_IP_ASC_RXFCON                       ((volatile u32*)(INCA_IP_ASC+ 0x0040))
#define INCA_IP_ASC_RXFCON_RXFITL (value)             (((( 1 << 6) - 1) & (value)) << 8)
#define INCA_IP_ASC_RXFCON_RXTMEN                        (1 << 2)
#define INCA_IP_ASC_RXFCON_RXFFLU                        (1 << 1)
#define INCA_IP_ASC_RXFCON_RXFEN                          (1 << 0)

/***Transmit FIFO Control Register***/
#define INCA_IP_ASC_TXFCON                       ((volatile u32*)(INCA_IP_ASC+ 0x0044))
#define INCA_IP_ASC_TXFCON_TXFITL (value)             (((( 1 << 6) - 1) & (value)) << 8)
#define INCA_IP_ASC_TXFCON_TXTMEN                        (1 << 2)
#define INCA_IP_ASC_TXFCON_TXFFLU                        (1 << 1)
#define INCA_IP_ASC_TXFCON_TXFEN                          (1 << 0)

/***FIFO Status Register***/
#define INCA_IP_ASC_FSTAT                        ((volatile u32*)(INCA_IP_ASC+ 0x0048))
#define INCA_IP_ASC_FSTAT_TXFFL (value)              (((( 1 << 6) - 1) & (value)) << 8)
#define INCA_IP_ASC_FSTAT_RXFFL (value)              (((( 1 << 6) - 1) & (value)) << 0)

/***ASC Write HW Modified Autobaud Control Register***/
#define INCA_IP_ASC_ASC_WHBABCON                 ((volatile u32*)(INCA_IP_ASC+ 0x0054))
#define INCA_IP_ASC_ASC_WHBABCON_SETABEN                      (1 << 1)
#define INCA_IP_ASC_ASC_WHBABCON_CLRABEN                      (1 << 0)

/***ASC Autobaud Status Register***/
#define INCA_IP_ASC_ASC_ABSTAT                   ((volatile u32*)(INCA_IP_ASC+ 0x0034))
#define INCA_IP_ASC_ASC_ABSTAT_DETWAIT                      (1 << 4)
#define INCA_IP_ASC_ASC_ABSTAT_SCCDET                        (1 << 3)
#define INCA_IP_ASC_ASC_ABSTAT_SCSDET                        (1 << 2)
#define INCA_IP_ASC_ASC_ABSTAT_FCCDET                        (1 << 1)
#define INCA_IP_ASC_ASC_ABSTAT_FCSDET                        (1 << 0)

/***ASC Write HW Modified Autobaud Status Register***/
#define INCA_IP_ASC_ASC_WHBABSTAT                 ((volatile u32*)(INCA_IP_ASC+ 0x0058))
#define INCA_IP_ASC_ASC_WHBABSTAT_SETDETWAIT                (1 << 9)
#define INCA_IP_ASC_ASC_WHBABSTAT_CLRDETWAIT                (1 << 8)
#define INCA_IP_ASC_ASC_WHBABSTAT_SETSCCDET                  (1 << 7)
#define INCA_IP_ASC_ASC_WHBABSTAT_CLRSCCDET                  (1 << 6)
#define INCA_IP_ASC_ASC_WHBABSTAT_SETSCSDET                  (1 << 5)
#define INCA_IP_ASC_ASC_WHBABSTAT_CLRSCSDET                  (1 << 4)
#define INCA_IP_ASC_ASC_WHBABSTAT_SETFCCDET                  (1 << 3)
#define INCA_IP_ASC_ASC_WHBABSTAT_CLRFCCDET                  (1 << 2)
#define INCA_IP_ASC_ASC_WHBABSTAT_SETFCSDET                  (1 << 1)
#define INCA_IP_ASC_ASC_WHBABSTAT_CLRFCSDET                  (1 << 0)

/***ASC Clock Control Register***/
#define INCA_IP_ASC_ASC_CLC                      ((volatile u32*)(INCA_IP_ASC+ 0x0000))
#define INCA_IP_ASC_ASC_CLC_RMC (value)                (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_ASC_ASC_CLC_DISS                            (1 << 1)
#define INCA_IP_ASC_ASC_CLC_DISR                            (1 << 0)

/***********************************************************************/
/*  Module      :  DMA register address and bits                       */
/***********************************************************************/

#define INCA_IP_DMA                          (0xBF108000)
/***********************************************************************/


/***DMA RX Channel 0 Command Register***/
#define INCA_IP_DMA_DMA_RXCCR0                   ((volatile u32*)(INCA_IP_DMA+ 0x0800))
#define INCA_IP_DMA_DMA_RXCCR0_LBE                              (1 << 31)
#define INCA_IP_DMA_DMA_RXCCR0_HPEN                            (1 << 30)
#define INCA_IP_DMA_DMA_RXCCR0_INIT                            (1 << 2)
#define INCA_IP_DMA_DMA_RXCCR0_OFF                              (1 << 1)
#define INCA_IP_DMA_DMA_RXCCR0_HR                              (1 << 0)

/***DMA RX Channel 1 Command Register***/
#define INCA_IP_DMA_DMA_RXCCR1                   ((volatile u32*)(INCA_IP_DMA+ 0x0804))
#define INCA_IP_DMA_DMA_RXCCR1_LBE                              (1 << 31)
#define INCA_IP_DMA_DMA_RXCCR1_HPEN                            (1 << 30)
#define INCA_IP_DMA_DMA_RXCCR1_INIT                            (1 << 2)
#define INCA_IP_DMA_DMA_RXCCR1_OFF                              (1 << 1)
#define INCA_IP_DMA_DMA_RXCCR1_HR                              (1 << 0)

/***DMA Receive Interrupt Status Register***/
#define INCA_IP_DMA_DMA_RXISR                    ((volatile u32*)(INCA_IP_DMA+ 0x0808))
#define INCA_IP_DMA_DMA_RXISR_RDERRx (value)             (((( 1 << 2) - 1) & (value)) << 8)
#define INCA_IP_DMA_DMA_RXISR_CMDCPTx (value)            (((( 1 << 2) - 1) & (value)) << 6)
#define INCA_IP_DMA_DMA_RXISR_EOPx (value)               (((( 1 << 2) - 1) & (value)) << 4)
#define INCA_IP_DMA_DMA_RXISR_CPTx (value)               (((( 1 << 2) - 1) & (value)) << 2)
#define INCA_IP_DMA_DMA_RXISR_HLDx (value)               (((( 1 << 2) - 1) & (value)) << 0)

/***DMA Receive Interrupt Mask Register***/
#define INCA_IP_DMA_DMA_RXIMR                    ((volatile u32*)(INCA_IP_DMA+ 0x080C))
#define INCA_IP_DMA_DMA_RXIMR_RDERRx (value)             (((( 1 << 2) - 1) & (value)) << 8)
#define INCA_IP_DMA_DMA_RXIMR_CMDCPTx (value)            (((( 1 << 2) - 1) & (value)) << 6)
#define INCA_IP_DMA_DMA_RXIMR_EOPx (value)               (((( 1 << 2) - 1) & (value)) << 4)
#define INCA_IP_DMA_DMA_RXIMR_CPTx (value)               (((( 1 << 2) - 1) & (value)) << 2)
#define INCA_IP_DMA_DMA_RXIMR_HLDx (value)               (((( 1 << 2) - 1) & (value)) << 0)

/***DMA First Receive Descriptor Addr. for Rx Channel 0
***/
#define INCA_IP_DMA_DMA_RXFRDA0                  ((volatile u32*)(INCA_IP_DMA+ 0x0810))
#define INCA_IP_DMA_DMA_RXFRDA0_RXFRDA (value)             (((( 1 << 28) - 1) & (value)) << 0)

/***DMA First Receive Descriptor Addr. for Rx Channel 1
***/
#define INCA_IP_DMA_DMA_RXFRDA1                  ((volatile u32*)(INCA_IP_DMA+ 0x0814))
#define INCA_IP_DMA_DMA_RXFRDA1_RXFRDA (value)             (((( 1 << 28) - 1) & (value)) << 0)

/***DMA Receive Channel Polling Time***/
#define INCA_IP_DMA_DMA_RXPOLL                   ((volatile u32*)(INCA_IP_DMA+ 0x0818))
#define INCA_IP_DMA_DMA_RXPOLL_BSZ1 (value)               (((( 1 << 2) - 1) & (value)) << 30)
#define INCA_IP_DMA_DMA_RXPOLL_BSZ0 (value)               (((( 1 << 2) - 1) & (value)) << 28)
#define INCA_IP_DMA_DMA_RXPOLL_RXPOLLTIME (value)         (((( 1 << 8) - 1) & (value)) << 0)

/***DMA TX Channel 0 Command Register (Voice Port)***/
#define INCA_IP_DMA_DMA_TXCCR0                   ((volatile u32*)(INCA_IP_DMA+ 0x0880))
#define INCA_IP_DMA_DMA_TXCCR0_LBE                              (1 << 31)
#define INCA_IP_DMA_DMA_TXCCR0_HPEN                            (1 << 30)
#define INCA_IP_DMA_DMA_TXCCR0_HR                              (1 << 2)
#define INCA_IP_DMA_DMA_TXCCR0_OFF                              (1 << 1)
#define INCA_IP_DMA_DMA_TXCCR0_INIT                            (1 << 0)

/***DMA TX Channel 1 Command Register (Mangmt Port)***/
#define INCA_IP_DMA_DMA_TXCCR1                   ((volatile u32*)(INCA_IP_DMA+ 0x0884))
#define INCA_IP_DMA_DMA_TXCCR1_LBE                              (1 << 31)
#define INCA_IP_DMA_DMA_TXCCR1_HPEN                            (1 << 30)
#define INCA_IP_DMA_DMA_TXCCR1_HR                              (1 << 2)
#define INCA_IP_DMA_DMA_TXCCR1_OFF                              (1 << 1)
#define INCA_IP_DMA_DMA_TXCCR1_INIT                            (1 << 0)

/***DMA TX Channel 2 Command Register (SSC Port)***/
#define INCA_IP_DMA_DMA_TXCCR2                   ((volatile u32*)(INCA_IP_DMA+ 0x0888))
#define INCA_IP_DMA_DMA_TXCCR2_LBE                              (1 << 31)
#define INCA_IP_DMA_DMA_TXCCR2_HPEN                            (1 << 30)
#define INCA_IP_DMA_DMA_TXCCR2_HBF                              (1 << 29)
#define INCA_IP_DMA_DMA_TXCCR2_HR                              (1 << 2)
#define INCA_IP_DMA_DMA_TXCCR2_OFF                              (1 << 1)
#define INCA_IP_DMA_DMA_TXCCR2_INIT                            (1 << 0)

/***DMA First Receive Descriptor Addr. for Tx Channel 0
***/
#define INCA_IP_DMA_DMA_TXFRDA0                  ((volatile u32*)(INCA_IP_DMA+ 0x08A0))
#define INCA_IP_DMA_DMA_TXFRDA0_TXFRDA (value)             (((( 1 << 28) - 1) & (value)) << 0)

/***DMA First Receive Descriptor Addr. for Tx Channel 1
***/
#define INCA_IP_DMA_DMA_TXFRDA1                  ((volatile u32*)(INCA_IP_DMA+ 0x08A4))
#define INCA_IP_DMA_DMA_TXFRDA1_TXFRDA (value)             (((( 1 << 28) - 1) & (value)) << 0)

/***DMA First Receive Descriptor Addr. for Tx Channel 2
***/
#define INCA_IP_DMA_DMA_TXFRDA2                  ((volatile u32*)(INCA_IP_DMA+ 0x08A8))
#define INCA_IP_DMA_DMA_TXFRDA2_TXFRDA (value)             (((( 1 << 28) - 1) & (value)) << 0)

/***DMA Transmit Channel Arbitration Register***/
#define INCA_IP_DMA_DMA_TXWGT                    ((volatile u32*)(INCA_IP_DMA+ 0x08C0))
#define INCA_IP_DMA_DMA_TXWGT_TX2PR (value)              (((( 1 << 2) - 1) & (value)) << 4)
#define INCA_IP_DMA_DMA_TXWGT_TX1PRI (value)             (((( 1 << 2) - 1) & (value)) << 2)
#define INCA_IP_DMA_DMA_TXWGT_TX0PRI (value)             (((( 1 << 2) - 1) & (value)) << 0)

/***DMA Transmit Channel Polling Time***/
#define INCA_IP_DMA_DMA_TXPOLL                   ((volatile u32*)(INCA_IP_DMA+ 0x08C4))
#define INCA_IP_DMA_DMA_TXPOLL_BSZ2 (value)               (((( 1 << 2) - 1) & (value)) << 30)
#define INCA_IP_DMA_DMA_TXPOLL_BSZ1 (value)               (((( 1 << 2) - 1) & (value)) << 28)
#define INCA_IP_DMA_DMA_TXPOLL_BSZ0 (value)               (((( 1 << 2) - 1) & (value)) << 26)
#define INCA_IP_DMA_DMA_TXPOLL_TXPOLLTIME (value)         (((( 1 << 8) - 1) & (value)) << 0)

/***DMA Transmit Interrupt Status Register***/
#define INCA_IP_DMA_DMA_TXISR                    ((volatile u32*)(INCA_IP_DMA+ 0x08C8))
#define INCA_IP_DMA_DMA_TXISR_RDERRx (value)             (((( 1 << 3) - 1) & (value)) << 12)
#define INCA_IP_DMA_DMA_TXISR_HLDx (value)               (((( 1 << 3) - 1) & (value)) << 9)
#define INCA_IP_DMA_DMA_TXISR_CPTx (value)               (((( 1 << 3) - 1) & (value)) << 6)
#define INCA_IP_DMA_DMA_TXISR_EOPx (value)               (((( 1 << 3) - 1) & (value)) << 3)
#define INCA_IP_DMA_DMA_TXISR_CMDCPTx (value)            (((( 1 << 3) - 1) & (value)) << 0)

/***DMA Transmit Interrupt Mask Register***/
#define INCA_IP_DMA_DMA_TXIMR                    ((volatile u32*)(INCA_IP_DMA+ 0x08CC))
#define INCA_IP_DMA_DMA_TXIMR_RDERRx (value)             (((( 1 << 3) - 1) & (value)) << 12)
#define INCA_IP_DMA_DMA_TXIMR_HLDx (value)               (((( 1 << 3) - 1) & (value)) << 9)
#define INCA_IP_DMA_DMA_TXIMR_CPTx (value)               (((( 1 << 3) - 1) & (value)) << 6)
#define INCA_IP_DMA_DMA_TXIMR_EOPx (value)               (((( 1 << 3) - 1) & (value)) << 3)
#define INCA_IP_DMA_DMA_TXIMR_CMDCPTx (value)            (((( 1 << 3) - 1) & (value)) << 0)

/***********************************************************************/
/*  Module      :  Debug register address and bits                     */
/***********************************************************************/

#define INCA_IP_Debug                        (0xBF106000)
/***********************************************************************/


/***MCD Break Bus Switch Register***/
#define INCA_IP_Debug_MCD_BBS                      ((volatile u32*)(INCA_IP_Debug+ 0x0000))
#define INCA_IP_Debug_MCD_BBS_BTP1                            (1 << 19)
#define INCA_IP_Debug_MCD_BBS_BTP0                            (1 << 18)
#define INCA_IP_Debug_MCD_BBS_BSP1                            (1 << 17)
#define INCA_IP_Debug_MCD_BBS_BSP0                            (1 << 16)
#define INCA_IP_Debug_MCD_BBS_BT5EN                          (1 << 15)
#define INCA_IP_Debug_MCD_BBS_BT4EN                          (1 << 14)
#define INCA_IP_Debug_MCD_BBS_BT5                              (1 << 13)
#define INCA_IP_Debug_MCD_BBS_BT4                              (1 << 12)
#define INCA_IP_Debug_MCD_BBS_BS5EN                          (1 << 7)
#define INCA_IP_Debug_MCD_BBS_BS4EN                          (1 << 6)
#define INCA_IP_Debug_MCD_BBS_BS5                              (1 << 5)
#define INCA_IP_Debug_MCD_BBS_BS4                              (1 << 4)

/***MCD Multiplexer Control Register***/
#define INCA_IP_Debug_MCD_MCR                      ((volatile u32*)(INCA_IP_Debug+ 0x0008))
#define INCA_IP_Debug_MCD_MCR_MUX5                            (1 << 4)
#define INCA_IP_Debug_MCD_MCR_MUX4                            (1 << 3)
#define INCA_IP_Debug_MCD_MCR_MUX1                            (1 << 0)

/***********************************************************************/
/*  Module      :  TSF register address and bits                       */
/***********************************************************************/

#define INCA_IP_TSF                          (0xB8000900)
/***********************************************************************/


/***TSF Configuration Register (0000H)***/
#define INCA_IP_TSF_TSF_CONF                    ((volatile u32*)(INCA_IP_TSF+ 0x0000))
#define INCA_IP_TSF_TSF_CONF_PWMEN                          (1 << 2)
#define INCA_IP_TSF_TSF_CONF_LEDEN                          (1 << 1)
#define INCA_IP_TSF_TSF_CONF_KEYEN                          (1 << 0)

/***Key scan Configuration Register (0004H)***/
#define INCA_IP_TSF_KEY_CONF                    ((volatile u32*)(INCA_IP_TSF+ 0x0004))
#define INCA_IP_TSF_KEY_CONF_SL (value)                (((( 1 << 4) - 1) & (value)) << 0)

/***Scan Register Line 0 and 1 (0008H)***/
#define INCA_IP_TSF_SREG01                       ((volatile u32*)(INCA_IP_TSF+ 0x0008))
#define INCA_IP_TSF_SREG01_RES1x (value)              (((( 1 << 12) - 1) & (value)) << 16)
#define INCA_IP_TSF_SREG01_RES0x (value)              (((( 1 << 13) - 1) & (value)) << 0)

/***Scan Register Line 2 and 3 (000CH)***/
#define INCA_IP_TSF_SREG23                       ((volatile u32*)(INCA_IP_TSF+ 0x000C))
#define INCA_IP_TSF_SREG23_RES3x (value)              (((( 1 << 10) - 1) & (value)) << 16)
#define INCA_IP_TSF_SREG23_RES2x (value)              (((( 1 << 11) - 1) & (value)) << 0)

/***Scan Register Line 4, 5 and 6 (0010H)***/
#define INCA_IP_TSF_SREG456                      ((volatile u32*)(INCA_IP_TSF+ 0x0010))
#define INCA_IP_TSF_SREG456_RES6x (value)              (((( 1 << 7) - 1) & (value)) << 24)
#define INCA_IP_TSF_SREG456_RES5x (value)              (((( 1 << 8) - 1) & (value)) << 16)
#define INCA_IP_TSF_SREG456_RES4x (value)              (((( 1 << 9) - 1) & (value)) << 0)

/***Scan Register Line 7 to 12 (0014H)***/
#define INCA_IP_TSF_SREG7to12                    ((volatile u32*)(INCA_IP_TSF+ 0x0014))
#define INCA_IP_TSF_SREG7to12_RES12x                        (1 << 28)
#define INCA_IP_TSF_SREG7to12_RES11x (value)             (((( 1 << 2) - 1) & (value)) << 24)
#define INCA_IP_TSF_SREG7to12_RES10x (value)             (((( 1 << 3) - 1) & (value)) << 20)
#define INCA_IP_TSF_SREG7to12_RES9x (value)              (((( 1 << 4) - 1) & (value)) << 16)
#define INCA_IP_TSF_SREG7to12_RES8x (value)              (((( 1 << 5) - 1) & (value)) << 8)
#define INCA_IP_TSF_SREG7to12_RES7x (value)              (((( 1 << 6) - 1) & (value)) << 0)

/***LEDMUX Configuration Register (0018H)***/
#define INCA_IP_TSF_LEDMUX_CONF                  ((volatile u32*)(INCA_IP_TSF+ 0x0018))
#define INCA_IP_TSF_LEDMUX_CONF_ETL1                            (1 << 25)
#define INCA_IP_TSF_LEDMUX_CONF_ESTA1                          (1 << 24)
#define INCA_IP_TSF_LEDMUX_CONF_EDPX1                          (1 << 23)
#define INCA_IP_TSF_LEDMUX_CONF_EACT1                          (1 << 22)
#define INCA_IP_TSF_LEDMUX_CONF_ESPD1                          (1 << 21)
#define INCA_IP_TSF_LEDMUX_CONF_ETL0                            (1 << 20)
#define INCA_IP_TSF_LEDMUX_CONF_ESTA0                          (1 << 19)
#define INCA_IP_TSF_LEDMUX_CONF_EDPX0                          (1 << 18)
#define INCA_IP_TSF_LEDMUX_CONF_EACT0                          (1 << 17)
#define INCA_IP_TSF_LEDMUX_CONF_ESPD0                          (1 << 16)
#define INCA_IP_TSF_LEDMUX_CONF_INV                              (1 << 1)
#define INCA_IP_TSF_LEDMUX_CONF_NCOL                            (1 << 0)

/***LED Register (001CH)***/
#define INCA_IP_TSF_LED_REG                      ((volatile u32*)(INCA_IP_TSF+ 0x001C))
#define INCA_IP_TSF_LED_REG_Lxy (value)                (((( 1 << 24) - 1) & (value)) << 0)

/***Pulse Width Modulator 1 and 2 Register (0020H)***/
#define INCA_IP_TSF_PWM12                        ((volatile u32*)(INCA_IP_TSF+ 0x0020))
#define INCA_IP_TSF_PWM12_PW2PW1 (value)             (((( 1 << NaN) - 1) & (value)) << NaN)

/***********************************************************************/
/*  Module      :  Ports register address and bits                     */
/***********************************************************************/

#define INCA_IP_Ports                        (0xB8000A00)
/***********************************************************************/


/***Port 1 Data Output Register (0020H)***/
#define INCA_IP_Ports_P1_OUT                       ((volatile u32*)(INCA_IP_Ports+ 0x0020))
#define INCA_IP_Ports_P1_OUT_P(value)               (1 << value)


/***Port 2 Data Output Register (0040H)***/
#define INCA_IP_Ports_P2_OUT                       ((volatile u32*)(INCA_IP_Ports+ 0x0040))
#define INCA_IP_Ports_P2_OUT_P(value)               (1 << value)


/***Port 1 Data Input Register (0024H)***/
#define INCA_IP_Ports_P1_IN                        ((volatile u32*)(INCA_IP_Ports+ 0x0024))
#define INCA_IP_Ports_P1_IN_P(value)               (1 << value)


/***Port 2 Data Input Register (0044H)***/
#define INCA_IP_Ports_P2_IN                        ((volatile u32*)(INCA_IP_Ports+ 0x0044))
#define INCA_IP_Ports_P2_IN_P(value)               (1 << value)


/***Port 1 Direction Register (0028H)***/
#define INCA_IP_Ports_P1_DIR                       ((volatile u32*)(INCA_IP_Ports+ 0x0028))
#define INCA_IP_Ports_P1_DIR_Port1P(value)         (1 << value)

#define INCA_IP_Ports_P1_DIR_Port2Pn (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***Port 2 Direction Register (0048H)***/
#define INCA_IP_Ports_P2_DIR                       ((volatile u32*)(INCA_IP_Ports+ 0x0048))
#define INCA_IP_Ports_P2_DIR_Port1P(value)         (1 << value)

#define INCA_IP_Ports_P2_DIR_Port2Pn (value)          (((( 1 << 16) - 1) & (value)) << 0)

/***Port 0 Alternate Function Select Register 0 (000C H)
***/
#define INCA_IP_Ports_P0_ALTSEL                    ((volatile u32*)(INCA_IP_Ports+ 0x000C))
#define INCA_IP_Ports_P0_ALTSEL_Port0P(value)         (1 << value)


/***Port 1 Alternate Function Select Register 0 (002C H)
***/
#define INCA_IP_Ports_P1_ALTSEL                    ((volatile u32*)(INCA_IP_Ports+ 0x002C))
#define INCA_IP_Ports_P1_ALTSEL_Port1P(value)         (1 << value)

#define INCA_IP_Ports_P1_ALTSEL_Port2P(value)         (1 << value)


/***Port 2 Alternate Function Select Register 0 (004C H)
***/
#define INCA_IP_Ports_P2_ALTSEL                    ((volatile u32*)(INCA_IP_Ports+ 0x004C))
#define INCA_IP_Ports_P2_ALTSEL_Port1P(value)         (1 << value)

#define INCA_IP_Ports_P2_ALTSEL_Port2P(value)         (1 << value)


/***Port 0 Input Schmitt-Trigger Off Register (0010 H)
***/
#define INCA_IP_Ports_P0_STOFF                    ((volatile u32*)(INCA_IP_Ports+ 0x0010))
#define INCA_IP_Ports_P0_STOFF_Port0P(value)         (1 << value)


/***Port 1 Input Schmitt-Trigger Off Register (0030 H)
***/
#define INCA_IP_Ports_P1_STOFF                    ((volatile u32*)(INCA_IP_Ports+ 0x0030))
#define INCA_IP_Ports_P1_STOFF_Port1P(value)         (1 << value)

#define INCA_IP_Ports_P1_STOFF_Port2P(value)         (1 << value)


/***Port 2 Input Schmitt-Trigger Off Register (0050 H)
***/
#define INCA_IP_Ports_P2_STOFF                    ((volatile u32*)(INCA_IP_Ports+ 0x0050))
#define INCA_IP_Ports_P2_STOFF_Port1P(value)         (1 << value)

#define INCA_IP_Ports_P2_STOFF_Port2P(value)         (1 << value)


/***Port 2 Open Drain Control Register (0054H)***/
#define INCA_IP_Ports_P2_OD                        ((volatile u32*)(INCA_IP_Ports+ 0x0054))
#define INCA_IP_Ports_P2_OD_Port2P(value)         (1 << value)


/***Port 0 Pull Up Device Enable Register (0018 H)***/
#define INCA_IP_Ports_P0_PUDEN                    ((volatile u32*)(INCA_IP_Ports+ 0x0018))
#define INCA_IP_Ports_P0_PUDEN_Port0P(value)         (1 << value)


/***Port 2 Pull Up Device Enable Register (0058 H)***/
#define INCA_IP_Ports_P2_PUDEN                    ((volatile u32*)(INCA_IP_Ports+ 0x0058))
#define INCA_IP_Ports_P2_PUDEN_Port2P(value)         (1 << value)

#define INCA_IP_Ports_P2_PUDEN_Port2P(value)         (1 << value)


/***Port 0 Pull Up/Pull Down Select Register (001C H)***/
#define INCA_IP_Ports_P0_PUDSEL                    ((volatile u32*)(INCA_IP_Ports+ 0x001C))
#define INCA_IP_Ports_P0_PUDSEL_Port0P(value)         (1 << value)


/***Port 2 Pull Up/Pull Down Select Register (005C H)***/
#define INCA_IP_Ports_P2_PUDSEL                    ((volatile u32*)(INCA_IP_Ports+ 0x005C))
#define INCA_IP_Ports_P2_PUDSEL_Port2P(value)         (1 << value)

#define INCA_IP_Ports_P2_PUDSEL_Port2P(value)         (1 << value)


/***********************************************************************/
/*  Module      :  DES/3DES register address and bits                 */
/***********************************************************************/

#define INCA_IP_DES_3DES                    (0xB8000800)
/***********************************************************************/


/***DES Input Data High Register***/
#define INCA_IP_DES_3DES_DES_IHR                      ((volatile u32*)(INCA_IP_DES_3DES+ 0x0000))
#define INCA_IP_DES_3DES_DES_IHR_IH(value)               (1 << value)


/***DES Input Data Low Register***/
#define INCA_IP_DES_3DES_DES_ILR                      ((volatile u32*)(INCA_IP_DES_3DES+ 0x0004))
#define INCA_IP_DES_3DES_DES_ILR_IL(value)               (1 << value)


/***DES Key #1 High Register***/
#define INCA_IP_DES_3DES_DES_K1HR                    ((volatile u32*)(INCA_IP_DES_3DES+ 0x0008))
#define INCA_IP_DES_3DES_DES_K1HR_K1H(value)              (1 << value)


/***DES Key #1 Low Register***/
#define INCA_IP_DES_3DES_DES_K1LR                    ((volatile u32*)(INCA_IP_DES_3DES+ 0x000C))
#define INCA_IP_DES_3DES_DES_K1LR_K1L(value)              (1 << value)


/***DES Key #2 High Register***/
#define INCA_IP_DES_3DES_DES_K2HR                    ((volatile u32*)(INCA_IP_DES_3DES+ 0x0010))
#define INCA_IP_DES_3DES_DES_K2HR_K2H(value)              (1 << value)


/***DES Key #2 Low Register***/
#define INCA_IP_DES_3DES_DES_K2LR                    ((volatile u32*)(INCA_IP_DES_3DES+ 0x0014))
#define INCA_IP_DES_3DES_DES_K2LR_K2L(value)              (1 << value)


/***DES Key #3 High Register***/
#define INCA_IP_DES_3DES_DES_K3HR                    ((volatile u32*)(INCA_IP_DES_3DES+ 0x0018))
#define INCA_IP_DES_3DES_DES_K3HR_K3H(value)              (1 << value)


/***DES Key #3 Low Register***/
#define INCA_IP_DES_3DES_DES_K3LR                    ((volatile u32*)(INCA_IP_DES_3DES+ 0x001C))
#define INCA_IP_DES_3DES_DES_K3LR_K3L(value)              (1 << value)


/***DES Initialization Vector High Register***/
#define INCA_IP_DES_3DES_DES_IVHR                    ((volatile u32*)(INCA_IP_DES_3DES+ 0x0020))
#define INCA_IP_DES_3DES_DES_IVHR_IVH(value)              (1 << value)


/***DES Initialization Vector Low Register***/
#define INCA_IP_DES_3DES_DES_IVLR                    ((volatile u32*)(INCA_IP_DES_3DES+ 0x0024))
#define INCA_IP_DES_3DES_DES_IVLR_IVL(value)              (1 << value)


/***DES Control Register***/
#define INCA_IP_DES_3DES_DES_CONTROLR                 ((volatile u32*)(INCA_IP_DES_3DES+ 0x0028))
#define INCA_IP_DES_3DES_DES_CONTROLR_KRE                              (1 << 31)
#define INCA_IP_DES_3DES_DES_CONTROLR_DAU                              (1 << 16)
#define INCA_IP_DES_3DES_DES_CONTROLR_F(value)               (1 << value)

#define INCA_IP_DES_3DES_DES_CONTROLR_O(value)               (1 << value)

#define INCA_IP_DES_3DES_DES_CONTROLR_GO                              (1 << 8)
#define INCA_IP_DES_3DES_DES_CONTROLR_STP                              (1 << 7)
#define INCA_IP_DES_3DES_DES_CONTROLR_IEN                              (1 << 6)
#define INCA_IP_DES_3DES_DES_CONTROLR_BUS                              (1 << 5)
#define INCA_IP_DES_3DES_DES_CONTROLR_SM                              (1 << 4)
#define INCA_IP_DES_3DES_DES_CONTROLR_E_D                              (1 << 3)
#define INCA_IP_DES_3DES_DES_CONTROLR_M(value)               (1 << value)


/***DES Output Data High Register***/
#define INCA_IP_DES_3DES_DES_OHR                      ((volatile u32*)(INCA_IP_DES_3DES+ 0x002C))
#define INCA_IP_DES_3DES_DES_OHR_OH(value)               (1 << value)


/***DES Output Data Low Register***/
#define INCA_IP_DES_3DES_DES_OLR                      ((volatile u32*)(INCA_IP_DES_3DES+ 0x0030))
#define INCA_IP_DES_3DES_DES_OLR_OL(value)               (1 << value)


/***********************************************************************/
/*  Module      :  AES register address and bits                       */
/***********************************************************************/

#define INCA_IP_AES                          (0xB8000880)
/***********************************************************************/


/***AES Input Data 3 Register***/
#define INCA_IP_AES_AES_ID3R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_ID3R_I(value)               (1 << value)


/***AES Input Data 2 Register***/
#define INCA_IP_AES_AES_ID2R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_ID2R_I(value)               (1 << value)


/***AES Input Data 1 Register***/
#define INCA_IP_AES_AES_ID1R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_ID1R_I(value)               (1 << value)


/***AES Input Data 0 Register***/
#define INCA_IP_AES_AES_ID0R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_ID0R_I(value)               (1 << value)


/***AES Output Data 3 Register***/
#define INCA_IP_AES_AES_OD3R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_OD3R_O(value)               (1 << value)


/***AES Output Data 2 Register***/
#define INCA_IP_AES_AES_OD2R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_OD2R_O(value)               (1 << value)


/***AES Output Data 1 Register***/
#define INCA_IP_AES_AES_OD1R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_OD1R_O(value)               (1 << value)


/***AES Output Data 0 Register***/
#define INCA_IP_AES_AES_OD0R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_OD0R_O(value)               (1 << value)


/***AES Key 7 Register***/
#define INCA_IP_AES_AES_K7R                      ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_K7R_K(value)               (1 << value)


/***AES Key 6 Register***/
#define INCA_IP_AES_AES_K6R                      ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_K6R_K(value)               (1 << value)


/***AES Key 5 Register***/
#define INCA_IP_AES_AES_K5R                      ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_K5R_K(value)               (1 << value)


/***AES Key 4 Register***/
#define INCA_IP_AES_AES_K4R                      ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_K4R_K(value)               (1 << value)


/***AES Key 3 Register***/
#define INCA_IP_AES_AES_K3R                      ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_K3R_K(value)               (1 << value)


/***AES Key 2 Register***/
#define INCA_IP_AES_AES_K2R                      ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_K2R_K(value)               (1 << value)


/***AES Key 1 Register***/
#define INCA_IP_AES_AES_K1R                      ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_K1R_K(value)               (1 << value)


/***AES Key 0 Register***/
#define INCA_IP_AES_AES_K0R                      ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_K0R_K(value)               (1 << value)


/***AES Initialization Vector 3 Register***/
#define INCA_IP_AES_AES_IV3R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_IV3R_IV(value)               (1 << value)


/***AES Initialization Vector 2 Register***/
#define INCA_IP_AES_AES_IV2R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_IV2R_IV(value)               (1 << value)


/***AES Initialization Vector 1 Register***/
#define INCA_IP_AES_AES_IV1R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_IV1R_IV(value)               (1 << value)


/***AES Initialization Vector 0 Register***/
#define INCA_IP_AES_AES_IV0R                    ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_IV0R_IV (value)                (((( 1 << 32) - 1) &(value)) << 0)

/***AES Control Register***/
#define INCA_IP_AES_AES_CONTROLR                 ((volatile u32*)(INCA_IP_AES+ 0x0000))
#define INCA_IP_AES_AES_CONTROLR_KRE                              (1 << 31)
#define INCA_IP_AES_AES_CONTROLR_DAU                              (1 << 16)
#define INCA_IP_AES_AES_CONTROLR_PNK                              (1 << 15)
#define INCA_IP_AES_AES_CONTROLR_F(value)               (1 << value)

#define INCA_IP_AES_AES_CONTROLR_O(value)               (1 << value)

#define INCA_IP_AES_AES_CONTROLR_GO                              (1 << 8)
#define INCA_IP_AES_AES_CONTROLR_STP                              (1 << 7)
#define INCA_IP_AES_AES_CONTROLR_IEN                              (1 << 6)
#define INCA_IP_AES_AES_CONTROLR_BUS                              (1 << 5)
#define INCA_IP_AES_AES_CONTROLR_SM                              (1 << 4)
#define INCA_IP_AES_AES_CONTROLR_E_D                              (1 << 3)
#define INCA_IP_AES_AES_CONTROLR_KV                              (1 << 2)
#define INCA_IP_AES_AES_CONTROLR_K(value)               (1 << value)


/***********************************************************************/
/*  Module      :  I²C register address and bits                       */
/***********************************************************************/

#define INCA_IP_IIC                          (0xB8000700)
/***********************************************************************/


/***I²C Port Input Select Register***/
#define INCA_IP_IIC_IIC_PISEL                    ((volatile u32*)(INCA_IP_IIC+ 0x0004))
#define INCA_IP_IIC_IIC_PISEL_SDAIS(value)            (1 << value)

#define INCA_IP_IIC_IIC_PISEL_SCLIS(value)            (1 << value)


/***I²C Clock Control Register***/
#define INCA_IP_IIC_IIC_CLC                      ((volatile u32*)(INCA_IP_IIC+ 0x0000))
#define INCA_IP_IIC_IIC_CLC_RMC (value)                (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_IIC_IIC_CLC_DISS                            (1 << 1)
#define INCA_IP_IIC_IIC_CLC_DISR                            (1 << 0)

/***I²C System Control Register***/
#define INCA_IP_IIC_IIC_SYSCON_0                 ((volatile u32*)(INCA_IP_IIC+ 0x0010))
#define INCA_IP_IIC_IIC_SYSCON_0_WMEN                            (1 << 31)
#define INCA_IP_IIC_IIC_SYSCON_0_CI (value)                (((( 1 << 2) - 1) & (value)) << 26)
#define INCA_IP_IIC_IIC_SYSCON_0_STP                              (1 << 25)
#define INCA_IP_IIC_IIC_SYSCON_0_IGE                              (1 << 24)
#define INCA_IP_IIC_IIC_SYSCON_0_TRX                              (1 << 23)
#define INCA_IP_IIC_IIC_SYSCON_0_INT                              (1 << 22)
#define INCA_IP_IIC_IIC_SYSCON_0_ACKDIS                        (1 << 21)
#define INCA_IP_IIC_IIC_SYSCON_0_BUM                              (1 << 20)
#define INCA_IP_IIC_IIC_SYSCON_0_MOD (value)                (((( 1 << 2) - 1) & (value)) << 18)
#define INCA_IP_IIC_IIC_SYSCON_0_RSC                              (1 << 17)
#define INCA_IP_IIC_IIC_SYSCON_0_M10                              (1 << 16)
#define INCA_IP_IIC_IIC_SYSCON_0_RMEN                            (1 << 15)
#define INCA_IP_IIC_IIC_SYSCON_0_CO (value)                (((( 1 << 3) - 1) & (value)) << 8)
#define INCA_IP_IIC_IIC_SYSCON_0_IRQE                            (1 << 7)
#define INCA_IP_IIC_IIC_SYSCON_0_IRQP                            (1 << 6)
#define INCA_IP_IIC_IIC_SYSCON_0_IRQD                            (1 << 5)
#define INCA_IP_IIC_IIC_SYSCON_0_BB                              (1 << 4)
#define INCA_IP_IIC_IIC_SYSCON_0_LRB                              (1 << 3)
#define INCA_IP_IIC_IIC_SYSCON_0_SLA                              (1 << 2)
#define INCA_IP_IIC_IIC_SYSCON_0_AL                              (1 << 1)
#define INCA_IP_IIC_IIC_SYSCON_0_ADR                              (1 << 0)

/***I²C System Control Register***/
#define INCA_IP_IIC_IIC_SYSCON_1                 ((volatile u32*)(INCA_IP_IIC+ 0x0010))
#define INCA_IP_IIC_IIC_SYSCON_1_RM (value)                (((( 1 << 8) - 1) & (value)) << 24)
#define INCA_IP_IIC_IIC_SYSCON_1_TRX                              (1 << 23)
#define INCA_IP_IIC_IIC_SYSCON_1_INT                              (1 << 22)
#define INCA_IP_IIC_IIC_SYSCON_1_ACKDIS                        (1 << 21)
#define INCA_IP_IIC_IIC_SYSCON_1_BUM                              (1 << 20)
#define INCA_IP_IIC_IIC_SYSCON_1_MOD (value)                (((( 1 << 2) - 1) & (value)) << 18)
#define INCA_IP_IIC_IIC_SYSCON_1_RSC                              (1 << 17)
#define INCA_IP_IIC_IIC_SYSCON_1_M10                              (1 << 16)
#define INCA_IP_IIC_IIC_SYSCON_1_RMEN                            (1 << 15)
#define INCA_IP_IIC_IIC_SYSCON_1_CO (value)                (((( 1 << 3) - 1) & (value)) << 8)
#define INCA_IP_IIC_IIC_SYSCON_1_IRQE                            (1 << 7)
#define INCA_IP_IIC_IIC_SYSCON_1_IRQP                            (1 << 6)
#define INCA_IP_IIC_IIC_SYSCON_1_IRQD                            (1 << 5)
#define INCA_IP_IIC_IIC_SYSCON_1_BB                              (1 << 4)
#define INCA_IP_IIC_IIC_SYSCON_1_LRB                              (1 << 3)
#define INCA_IP_IIC_IIC_SYSCON_1_SLA                              (1 << 2)
#define INCA_IP_IIC_IIC_SYSCON_1_AL                              (1 << 1)
#define INCA_IP_IIC_IIC_SYSCON_1_ADR                              (1 << 0)

/***I²C System Control Register***/
#define INCA_IP_IIC_IIC_SYSCON_2                 ((volatile u32*)(INCA_IP_IIC+ 0x0010))
#define INCA_IP_IIC_IIC_SYSCON_2_WMEN                            (1 << 31)
#define INCA_IP_IIC_IIC_SYSCON_2_CI (value)                (((( 1 << 2) - 1) & (value)) << 26)
#define INCA_IP_IIC_IIC_SYSCON_2_STP                              (1 << 25)
#define INCA_IP_IIC_IIC_SYSCON_2_IGE                              (1 << 24)
#define INCA_IP_IIC_IIC_SYSCON_2_TRX                              (1 << 23)
#define INCA_IP_IIC_IIC_SYSCON_2_INT                              (1 << 22)
#define INCA_IP_IIC_IIC_SYSCON_2_ACKDIS                        (1 << 21)
#define INCA_IP_IIC_IIC_SYSCON_2_BUM                              (1 << 20)
#define INCA_IP_IIC_IIC_SYSCON_2_MOD (value)                (((( 1 << 2) - 1) & (value)) << 18)
#define INCA_IP_IIC_IIC_SYSCON_2_RSC                              (1 << 17)
#define INCA_IP_IIC_IIC_SYSCON_2_M10                              (1 << 16)
#define INCA_IP_IIC_IIC_SYSCON_2_WM (value)                (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_IIC_IIC_SYSCON_2_IRQE                            (1 << 7)
#define INCA_IP_IIC_IIC_SYSCON_2_IRQP                            (1 << 6)
#define INCA_IP_IIC_IIC_SYSCON_2_IRQD                            (1 << 5)
#define INCA_IP_IIC_IIC_SYSCON_2_BB                              (1 << 4)
#define INCA_IP_IIC_IIC_SYSCON_2_LRB                              (1 << 3)
#define INCA_IP_IIC_IIC_SYSCON_2_SLA                              (1 << 2)
#define INCA_IP_IIC_IIC_SYSCON_2_AL                              (1 << 1)
#define INCA_IP_IIC_IIC_SYSCON_2_ADR                              (1 << 0)

/***I²C Write Hardware Modified System Control Register
***/
#define INCA_IP_IIC_IIC_WHBSYSCON                 ((volatile u32*)(INCA_IP_IIC+ 0x0020))
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRWMEN                      (1 << 31)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETWMEN                      (1 << 30)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETSTP                        (1 << 26)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRSTP                        (1 << 25)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETTRX                        (1 << 24)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRTRX                        (1 << 23)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETACKDIS                  (1 << 22)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRACKDIS                  (1 << 21)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETBUM                        (1 << 20)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRBUM                        (1 << 19)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETRSC                        (1 << 17)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRRSC                        (1 << 16)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETRMEN                      (1 << 15)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRRMEN                      (1 << 14)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETIRQE                      (1 << 10)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETIRQP                      (1 << 9)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETIRQD                      (1 << 8)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRIRQE                      (1 << 7)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRIRQP                      (1 << 6)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRIRQD                      (1 << 5)
#define INCA_IP_IIC_IIC_WHBSYSCON_SETAL                          (1 << 2)
#define INCA_IP_IIC_IIC_WHBSYSCON_CLRAL                          (1 << 1)

/***I²C Bus Control Register***/
#define INCA_IP_IIC_IIC_BUSCON_0                 ((volatile u32*)(INCA_IP_IIC+ 0x0014))
#define INCA_IP_IIC_IIC_BUSCON_0_BRPMOD                        (1 << 31)
#define INCA_IP_IIC_IIC_BUSCON_0_PREDIV (value)             (((( 1 << 2) - 1) & (value)) << 29)
#define INCA_IP_IIC_IIC_BUSCON_0_ICA9_0 (value)             (((( 1 << 10) - 1) & (value)) << 16)
#define INCA_IP_IIC_IIC_BUSCON_0_BRP (value)                (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_IIC_IIC_BUSCON_0_SCLEN(value)            (1 << value)

#define INCA_IP_IIC_IIC_BUSCON_0_SDAEN(value)            (1 << value)


/***I²C Bus Control Register***/
#define INCA_IP_IIC_IIC_BUSCON_1                 ((volatile u32*)(INCA_IP_IIC+ 0x0014))
#define INCA_IP_IIC_IIC_BUSCON_1_BRPMOD                        (1 << 31)
#define INCA_IP_IIC_IIC_BUSCON_1_PREDIV (value)             (((( 1 << 2) - 1) & (value)) << 29)
#define INCA_IP_IIC_IIC_BUSCON_1_ICA7_1 (value)             (((( 1 << 7) - 1) & (value)) << 17)
#define INCA_IP_IIC_IIC_BUSCON_1_BRP (value)                (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_IIC_IIC_BUSCON_1_SCLEN(value)            (1 << value)

#define INCA_IP_IIC_IIC_BUSCON_1_SDAEN(value)            (1 << value)


/***I²C Receive Transmit Buffer***/
#define INCA_IP_IIC_IIC_RTB                      ((volatile u32*)(INCA_IP_IIC+ 0x0018))
#define INCA_IP_IIC_IIC_RTB_RTB(value)              (1 << value)


/***********************************************************************/
/*  Module      :  FB register address and bits                       */
/***********************************************************************/

#define INCA_IP_FB                          (0xBF880000)
/***********************************************************************/


/***FB Access Error Cause Register***/
#define INCA_IP_FB_FB_ERRCAUSE                  ((volatile u32*)(INCA_IP_FB+ 0x0100))
#define INCA_IP_FB_FB_ERRCAUSE_ERR                              (1 << 31)
#define INCA_IP_FB_FB_ERRCAUSE_PORT (value)               (((( 1 << 4) - 1) & (value)) << 16)
#define INCA_IP_FB_FB_ERRCAUSE_CAUSE (value)              (((( 1 << 2) - 1) & (value)) << 0)

/***FB Access Error Address Register***/
#define INCA_IP_FB_FB_ERRADDR                   ((volatile u32*)(INCA_IP_FB+ 0x0108))
#define INCA_IP_FB_FB_ERRADDR_ADDR

/***FB Configuration Register***/
#define INCA_IP_FB_FB_CFG                       ((volatile u32*)(INCA_IP_FB+ 0x0800))
#define INCA_IP_FB_FB_CFG_SVM                              (1 << 0)

/***********************************************************************/
/*  Module      :  SRAM register address and bits                      */
/***********************************************************************/

#define INCA_IP_SRAM                         (0xBF980000)
/***********************************************************************/


/***SRAM Size Register***/
#define INCA_IP_SRAM_SRAM_SIZE                    ((volatile u32*)(INCA_IP_SRAM+ 0x0800))
#define INCA_IP_SRAM_SRAM_SIZE_SIZE (value)               (((( 1 << 23) - 1) & (value)) << 0)

/***********************************************************************/
/*  Module      :  BIU register address and bits                       */
/***********************************************************************/

#define INCA_IP_BIU                          (0xBFA80000)
/***********************************************************************/


/***BIU Identification Register***/
#define INCA_IP_BIU_BIU_ID                       ((volatile u32*)(INCA_IP_BIU+ 0x0000))
#define INCA_IP_BIU_BIU_ID_ARCH                            (1 << 16)
#define INCA_IP_BIU_BIU_ID_ID (value)                (((( 1 << 8) - 1) & (value)) << 8)
#define INCA_IP_BIU_BIU_ID_REV (value)                (((( 1 << 8) - 1) & (value)) << 0)

/***BIU Access Error Cause Register***/
#define INCA_IP_BIU_BIU_ERRCAUSE                 ((volatile u32*)(INCA_IP_BIU+ 0x0100))
#define INCA_IP_BIU_BIU_ERRCAUSE_ERR                              (1 << 31)
#define INCA_IP_BIU_BIU_ERRCAUSE_PORT (value)               (((( 1 << 4) - 1) & (value)) << 16)
#define INCA_IP_BIU_BIU_ERRCAUSE_CAUSE (value)              (((( 1 << 2) - 1) & (value)) << 0)

/***BIU Access Error Address Register***/
#define INCA_IP_BIU_BIU_ERRADDR                  ((volatile u32*)(INCA_IP_BIU+ 0x0108))
#define INCA_IP_BIU_BIU_ERRADDR_ADDR

/***********************************************************************/
/*  Module      :  ICU register address and bits                       */
/***********************************************************************/

#define INCA_IP_ICU                          (0xBF101000)
/***********************************************************************/


/***IM0 Interrupt Status Register***/
#define INCA_IP_ICU_IM0_ISR                      ((volatile u32*)(INCA_IP_ICU+ 0x0000))
#define INCA_IP_ICU_IM0_ISR_IR(value)               (1 << value)


/***IM1 Interrupt Status Register***/
#define INCA_IP_ICU_IM1_ISR                      ((volatile u32*)(INCA_IP_ICU+ 0x0200))
#define INCA_IP_ICU_IM1_ISR_IR(value)               (1 << value)


/***IM2 Interrupt Status Register***/
#define INCA_IP_ICU_IM2_ISR                      ((volatile u32*)(INCA_IP_ICU+ 0x0400))
#define INCA_IP_ICU_IM2_ISR_IR(value)               (1 << value)


/***IM0 Interrupt Enable Register***/
#define INCA_IP_ICU_IM0_IER                      ((volatile u32*)(INCA_IP_ICU+ 0x0008))
#define INCA_IP_ICU_IM0_IER_IR(value)               (1 << value)


/***IM1 Interrupt Enable Register***/
#define INCA_IP_ICU_IM1_IER                      ((volatile u32*)(INCA_IP_ICU+ 0x0208))
#define INCA_IP_ICU_IM1_IER_IR(value)               (1 << value)


/***IM2 Interrupt Enable Register***/
#define INCA_IP_ICU_IM2_IER                      ((volatile u32*)(INCA_IP_ICU+ 0x0408))
#define INCA_IP_ICU_IM2_IER_IR(value)               (1 << value)


/***IM0 Interrupt Output Status Register***/
#define INCA_IP_ICU_IM0_IOSR                    ((volatile u32*)(INCA_IP_ICU+ 0x0010))
#define INCA_IP_ICU_IM0_IOSR_IR(value)               (1 << value)


/***IM1 Interrupt Output Status Register***/
#define INCA_IP_ICU_IM1_IOSR                    ((volatile u32*)(INCA_IP_ICU+ 0x0210))
#define INCA_IP_ICU_IM1_IOSR_IR(value)               (1 << value)


/***IM2 Interrupt Output Status Register***/
#define INCA_IP_ICU_IM2_IOSR                    ((volatile u32*)(INCA_IP_ICU+ 0x0410))
#define INCA_IP_ICU_IM2_IOSR_IR(value)               (1 << value)


/***IM0 Interrupt Request Set Register***/
#define INCA_IP_ICU_IM0_IRSR                    ((volatile u32*)(INCA_IP_ICU+ 0x0018))
#define INCA_IP_ICU_IM0_IRSR_IR(value)               (1 << value)


/***IM1 Interrupt Request Set Register***/
#define INCA_IP_ICU_IM1_IRSR                    ((volatile u32*)(INCA_IP_ICU+ 0x0218))
#define INCA_IP_ICU_IM1_IRSR_IR(value)               (1 << value)


/***IM2 Interrupt Request Set Register***/
#define INCA_IP_ICU_IM2_IRSR                    ((volatile u32*)(INCA_IP_ICU+ 0x0418))
#define INCA_IP_ICU_IM2_IRSR_IR(value)               (1 << value)


/***External Interrupt Control Register***/
#define INCA_IP_ICU_ICU_EICR                    ((volatile u32*)(INCA_IP_ICU+ 0x0B00))
#define INCA_IP_ICU_ICU_EICR_EII5 (value)               (((( 1 << 3) - 1) & (value)) << 20)
#define INCA_IP_ICU_ICU_EICR_EII4 (value)               (((( 1 << 3) - 1) & (value)) << 16)
#define INCA_IP_ICU_ICU_EICR_EII3 (value)               (((( 1 << 3) - 1) & (value)) << 12)
#define INCA_IP_ICU_ICU_EICR_EII2 (value)               (((( 1 << 3) - 1) & (value)) << 8)
#define INCA_IP_ICU_ICU_EICR_EII1 (value)               (((( 1 << 3) - 1) & (value)) << 4)
#define INCA_IP_ICU_ICU_EICR_EII0 (value)               (((( 1 << 3) - 1) & (value)) << 0)
