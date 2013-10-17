/**
 * @file IxOsalOsIxp400.h
 *
 * @brief OS and platform specific definitions
 *
 * Design Notes:
 *
 * @par
 * IXP400 SW Release version 2.0
 *
 * -- Copyright Notice --
 *
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 *
 * @par
 * SPDX-License-Identifier:	BSD-3-Clause
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxOsalOsIxp400_H
#define IxOsalOsIxp400_H

#define BIT(x) (1<<(x))

#define IXP425_EthA_BASE	0xc8009000
#define IXP425_EthB_BASE	0xc800a000

#define IXP425_PSMA_BASE	0xc8006000
#define IXP425_PSMB_BASE	0xc8007000
#define IXP425_PSMC_BASE	0xc8008000

#define IXP425_PERIPHERAL_BASE	0xc8000000

#define IXP425_QMGR_BASE	0x60000000
#define IXP425_OSTS		0xC8005000

#define IXP425_INT_LVL_NPEA	0
#define IXP425_INT_LVL_NPEB	1
#define IXP425_INT_LVL_NPEC	2

#define IXP425_INT_LVL_QM1	3
#define IXP425_INT_LVL_QM2	4

#define IXP425_EXPANSION_BUS_BASE1	0x50000000
#define IXP425_EXPANSION_BUS_BASE2	0x50000000
#define IXP425_EXPANSION_BUS_CS1_BASE	0x51000000

#define IXP425_EXP_CONFIG_BASE		0xC4000000

/* physical addresses to be used when requesting memory with IX_OSAL_MEM_MAP */
#define IX_OSAL_IXP400_INTC_PHYS_BASE          IXP425_INTC_BASE
#define IX_OSAL_IXP400_GPIO_PHYS_BASE          IXP425_GPIO_BASE
#define IX_OSAL_IXP400_UART1_PHYS_BASE         IXP425_UART1_BASE
#define IX_OSAL_IXP400_UART2_PHYS_BASE         IXP425_UART2_BASE
#define IX_OSAL_IXP400_ETHA_PHYS_BASE          IXP425_EthA_BASE
#define IX_OSAL_IXP400_ETHB_PHYS_BASE          IXP425_EthB_BASE
#define IX_OSAL_IXP400_NPEA_PHYS_BASE          IXP425_NPEA_BASE
#define IX_OSAL_IXP400_NPEB_PHYS_BASE          IXP425_NPEB_BASE
#define IX_OSAL_IXP400_NPEC_PHYS_BASE          IXP425_NPEC_BASE
#define IX_OSAL_IXP400_PERIPHERAL_PHYS_BASE    IXP425_PERIPHERAL_BASE
#define IX_OSAL_IXP400_QMGR_PHYS_BASE          IXP425_QMGR_BASE
#define IX_OSAL_IXP400_OSTS_PHYS_BASE          IXP425_TIMER_BASE
#define IX_OSAL_IXP400_USB_PHYS_BASE           IXP425_USB_BASE
#define IX_OSAL_IXP400_EXP_CFG_PHYS_BASE       IXP425_EXP_CFG_BASE
#define IX_OSAL_IXP400_EXP_BUS_PHYS_BASE       IXP425_EXP_BUS_BASE2
#define IX_OSAL_IXP400_EXP_BUS_BOOT_PHYS_BASE  IXP425_EXP_BUS_BASE1
#define IX_OSAL_IXP400_EXP_BUS_CS0_PHYS_BASE   IXP425_EXP_BUS_CS0_BASE
#define IX_OSAL_IXP400_EXP_BUS_CS1_PHYS_BASE   IXP425_EXP_BUS_CS1_BASE
#define IX_OSAL_IXP400_EXP_BUS_CS4_PHYS_BASE   IXP425_EXP_BUS_CS4_BASE
#define IX_OSAL_IXP400_EXP_BUS_REGS_PHYS_BASE  IXP425_EXP_CFG_BASE
#define IX_OSAL_IXP400_PCI_CFG_PHYS_BASE       IXP425_PCI_CFG_BASE

/* map sizes to be used when requesting memory with IX_OSAL_MEM_MAP */
#define IX_OSAL_IXP400_QMGR_MAP_SIZE        (0x4000)	 /**< Queue Manager map size */
#define IX_OSAL_IXP400_PERIPHERAL_MAP_SIZE  (0xC000)	 /**< Peripheral space map size */
#define IX_OSAL_IXP400_UART1_MAP_SIZE       (0x1000)	 /**< UART1 map size */
#define IX_OSAL_IXP400_UART2_MAP_SIZE       (0x1000)	 /**< UART2 map size */
#define IX_OSAL_IXP400_PMU_MAP_SIZE         (0x1000)	 /**< PMU map size */
#define IX_OSAL_IXP400_OSTS_MAP_SIZE        (0x1000)	 /**< OS Timers map size */
#define IX_OSAL_IXP400_NPEA_MAP_SIZE        (0x1000)	 /**< NPE A map size */
#define IX_OSAL_IXP400_NPEB_MAP_SIZE        (0x1000)	 /**< NPE B map size */
#define IX_OSAL_IXP400_NPEC_MAP_SIZE        (0x1000)	 /**< NPE C map size */
#define IX_OSAL_IXP400_ETHA_MAP_SIZE        (0x1000)	 /**< Eth A map size */
#define IX_OSAL_IXP400_ETHB_MAP_SIZE        (0x1000)	 /**< Eth B map size */
#define IX_OSAL_IXP400_USB_MAP_SIZE         (0x1000)	 /**< USB map size */
#define IX_OSAL_IXP400_GPIO_MAP_SIZE        (0x1000)	 /**< GPIO map size */
#define IX_OSAL_IXP400_EXP_REG_MAP_SIZE     (0x1000)	 /**< Exp Bus Config Registers map size */
#define IX_OSAL_IXP400_EXP_BUS_MAP_SIZE     (0x08000000) /**< Expansion bus map size */
#define IX_OSAL_IXP400_EXP_BUS_CS0_MAP_SIZE (0x01000000) /**< CS0 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS1_MAP_SIZE (0x01000000) /**< CS1 map size */
#define IX_OSAL_IXP400_EXP_BUS_CS4_MAP_SIZE (0x01000000) /**< CS4 map size */
#define IX_OSAL_IXP400_PCI_CFG_MAP_SIZE     (0x1000)	 /**< PCI Bus Config Registers map size */

#define IX_OSAL_IXP400_EXP_FUSE             (IXP425_EXP_CONFIG_BASE + 0x28)
#define IX_OSAL_IXP400_ETH_NPEA_PHYS_BASE   0xC800C000
#define IX_OSAL_IXP400_ETH_NPEA_MAP_SIZE    0x1000

/*
 * Interrupt Levels
 */
#define IX_OSAL_IXP400_NPEA_IRQ_LVL		(0)
#define IX_OSAL_IXP400_NPEB_IRQ_LVL		(1)
#define IX_OSAL_IXP400_NPEC_IRQ_LVL		(2)
#define IX_OSAL_IXP400_QM1_IRQ_LVL		(3)
#define IX_OSAL_IXP400_QM2_IRQ_LVL		(4)
#define IX_OSAL_IXP400_TIMER1_IRQ_LVL		(5)
#define IX_OSAL_IXP400_GPIO0_IRQ_LVL		(6)
#define IX_OSAL_IXP400_GPIO1_IRQ_LVL		(7)
#define IX_OSAL_IXP400_PCI_INT_IRQ_LVL		(8)
#define IX_OSAL_IXP400_PCI_DMA1_IRQ_LVL		(9)
#define IX_OSAL_IXP400_PCI_DMA2_IRQ_LVL		(10)
#define IX_OSAL_IXP400_TIMER2_IRQ_LVL		(11)
#define IX_OSAL_IXP400_USB_IRQ_LVL		(12)
#define IX_OSAL_IXP400_UART2_IRQ_LVL		(13)
#define IX_OSAL_IXP400_TIMESTAMP_IRQ_LVL	(14)
#define IX_OSAL_IXP400_UART1_IRQ_LVL		(15)
#define IX_OSAL_IXP400_WDOG_IRQ_LVL		(16)
#define IX_OSAL_IXP400_AHB_PMU_IRQ_LVL		(17)
#define IX_OSAL_IXP400_XSCALE_PMU_IRQ_LVL	(18)
#define IX_OSAL_IXP400_GPIO2_IRQ_LVL		(19)
#define IX_OSAL_IXP400_GPIO3_IRQ_LVL		(20)
#define IX_OSAL_IXP400_GPIO4_IRQ_LVL		(21)
#define IX_OSAL_IXP400_GPIO5_IRQ_LVL		(22)
#define IX_OSAL_IXP400_GPIO6_IRQ_LVL		(23)
#define IX_OSAL_IXP400_GPIO7_IRQ_LVL		(24)
#define IX_OSAL_IXP400_GPIO8_IRQ_LVL		(25)
#define IX_OSAL_IXP400_GPIO9_IRQ_LVL		(26)
#define IX_OSAL_IXP400_GPIO10_IRQ_LVL		(27)
#define IX_OSAL_IXP400_GPIO11_IRQ_LVL		(28)
#define IX_OSAL_IXP400_GPIO12_IRQ_LVL		(29)
#define IX_OSAL_IXP400_SW_INT1_IRQ_LVL		(30)
#define IX_OSAL_IXP400_SW_INT2_IRQ_LVL		(31)

/* USB interrupt level mask */
#define IX_OSAL_IXP400_INT_LVL_USB             IRQ_IXP425_USB

/* USB IRQ */
#define IX_OSAL_IXP400_USB_IRQ                 IRQ_IXP425_USB

/*
 * OS name retrieval
 */
#define IX_OSAL_OEM_OS_NAME_GET(name, limit) \
ixOsalOsIxp400NameGet((INT8*)(name), (INT32) (limit))

/*
 * OS version retrieval
 */
#define IX_OSAL_OEM_OS_VERSION_GET(version, limit) \
ixOsalOsIxp400VersionGet((INT8*)(version), (INT32) (limit))

/*
 * Function to retrieve the OS name
 */
PUBLIC IX_STATUS ixOsalOsIxp400NameGet(INT8* osName, INT32 maxSize);

/*
 * Function to retrieve the OS version
 */
PUBLIC IX_STATUS ixOsalOsIxp400VersionGet(INT8* osVersion, INT32 maxSize);

/*
 * TimestampGet
 */
PUBLIC UINT32 ixOsalOsIxp400TimestampGet (void);

/*
 * Timestamp
 */
#define IX_OSAL_OEM_TIMESTAMP_GET ixOsalOsIxp400TimestampGet


/*
 * Timestamp resolution
 */
PUBLIC UINT32 ixOsalOsIxp400TimestampResolutionGet (void);

#define IX_OSAL_OEM_TIMESTAMP_RESOLUTION_GET ixOsalOsIxp400TimestampResolutionGet

/*
 * Retrieves the system clock rate
 */
PUBLIC UINT32 ixOsalOsIxp400SysClockRateGet (void);

#define IX_OSAL_OEM_SYS_CLOCK_RATE_GET ixOsalOsIxp400SysClockRateGet

/*
 * required by FS but is not really platform-specific.
 */
#define IX_OSAL_OEM_TIME_GET(pTv) ixOsalTimeGet(pTv)



/* linux map/unmap functions */
PUBLIC void ixOsalLinuxMemMap (IxOsalMemoryMap * map);

PUBLIC void ixOsalLinuxMemUnmap (IxOsalMemoryMap * map);


/*********************
 *	Memory map
 ********************/

/* Global memmap only visible to IO MEM module */

#ifdef IxOsalIoMem_C

IxOsalMemoryMap ixOsalGlobalMemoryMap[] = {
     {
     /* Global BE and LE_AC map */
     IX_OSAL_STATIC_MAP,	/* type            */
     0x00000000,		/* physicalAddress */
     0x30000000,		/* size            */
     0x00000000,		/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,/* endianType      */
     "global_low"		/* name            */
     },

    /* SDRAM LE_DC alias */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     0x00000000,		/* physicalAddress */
     0x10000000,		/* size            */
     0x30000000,		/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_LE_DC,		/* endianType      */
     "sdram_dc"			/* name            */
     },

    /* QMGR LE_DC alias */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     0x60000000,		/* physicalAddress */
     0x00100000,		/* size            */
     0x60000000,		/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_LE_DC,		/* endianType      */
     "qmgr_dc"			/* name            */
     },

    /* QMGR BE alias */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     0x60000000,		/* physicalAddress */
     0x00100000,		/* size            */
     0x60000000,		/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,/* endianType      */
     "qmgr_be"			/* name            */
     },

    /* Global BE and LE_AC map */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     0x40000000,		/* physicalAddress */
     0x20000000,		/* size            */
     0x40000000,		/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,/* endianType      */
     "Misc Cfg"			/* name            */
     },

    /* Global BE and LE_AC map */
    {
     IX_OSAL_STATIC_MAP,	/* type            */
     0x70000000,		/* physicalAddress */
     0x8FFFFFFF,		/* size            */
     0x70000000,		/* virtualAddress  */
     NULL,			/* mapFunction     */
     NULL,			/* unmapFunction   */
     0,				/* refCount        */
     IX_OSAL_BE | IX_OSAL_LE_AC,/* endianType      */
     "Exp Cfg"			/* name            */
     },
};

#endif /* IxOsalIoMem_C */
#endif /* #define IxOsalOsIxp400_H */
