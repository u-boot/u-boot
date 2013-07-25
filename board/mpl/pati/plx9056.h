/*
 * (C) Copyright 2003
 * Denis Peter, d.peter@mpl.ch
 * SPDX-License-Identifier:	GPL-2.0+
 */
/* PLX9096 register definitions
*/
#ifndef __PLX9056_H_
#define __PLX9056_H_	1

#include <pci.h>

#ifdef PLX9056_LOC
#define LOCAL_OFFSET					0x080
/* PCI Config regs */
#else
#define LOCAL_OFFSET					0x000
#endif

#define PCI9056_VENDOR_ID            PCI_VENDOR_ID
/*#define PCI9656_DEVICE_ID            PCI_DEVICE_ID */
#define PCI9056_COMMAND              PCI_COMMAND
/*#define PCI9656_STATUS               PCI_STATUS */
#define PCI9056_REVISION             PCI_REVISION_ID

#define PCI9056_CACHE_SIZE           PCI_CACHE_LINE_SIZE
#define PCI9056_RTR_BASE             PCI_BASE_ADDRESS_0
#define PCI9056_RTR_IO_BASE          PCI_BASE_ADDRESS_1
#define PCI9056_LOCAL_BASE0          PCI_BASE_ADDRESS_2
#define PCI9056_LOCAL_BASE1          PCI_BASE_ADDRESS_3
#define PCI9056_UNUSED_BASE1         PCI_BASE_ADDRESS_4
#define PCI9056_UNUSED_BASE2         PCI_BASE_ADDRESS_5
#define PCI9056_CIS_PTR              PCI_CARDBUS_CIS
#define PCI9056_SUB_ID               PCI_SUBSYSTEM_VENDOR_ID
#define PCI9056_EXP_ROM_BASE         PCI_ROM_ADDRESS
#define PCI9056_CAP_PTR              PCI_CAPABILITY_LIST
#define PCI9056_INT_LINE             PCI_INTERRUPT_LINE

#if defined(PLX9056_LOC)
    #define PCI9056_PM_CAP_ID            0x180
    #define PCI9056_PM_CSR               0x184
    #define PCI9056_HS_CAP_ID            0x188
    #define PCI9056_VPD_CAP_ID           0x18C
    #define PCI9056_VPD_DATA             0x190
#endif


#define PCI_DEVICE_ID_PLX9056		0x9056

/* Local Configuration Registers Accessible via the PCI Base address + Variable */
#define PCI9056_SPACE0_RANGE         (0x000 + LOCAL_OFFSET)
#define PCI9056_SPACE0_REMAP         (0x004 + LOCAL_OFFSET)
#define PCI9056_LOCAL_DMA_ARBIT      (0x008 + LOCAL_OFFSET)
#define PCI9056_ENDIAN_DESC          (0x00c + LOCAL_OFFSET)
#define PCI9056_EXP_ROM_RANGE        (0x010 + LOCAL_OFFSET)
#define PCI9056_EXP_ROM_REMAP        (0x014 + LOCAL_OFFSET)
#define PCI9056_SPACE0_ROM_DESC      (0x018 + LOCAL_OFFSET)
#define PCI9056_DM_RANGE             (0x01c + LOCAL_OFFSET)
#define PCI9056_DM_MEM_BASE          (0x020 + LOCAL_OFFSET)
#define PCI9056_DM_IO_BASE           (0x024 + LOCAL_OFFSET)
#define PCI9056_DM_PCI_MEM_REMAP     (0x028 + LOCAL_OFFSET)
#define PCI9056_DM_PCI_IO_CONFIG     (0x02c + LOCAL_OFFSET)
#define PCI9056_SPACE1_RANGE         (0x0f0 + LOCAL_OFFSET)
#define PCI9056_SPACE1_REMAP         (0x0f4 + LOCAL_OFFSET)
#define PCI9056_SPACE1_DESC          (0x0f8 + LOCAL_OFFSET)
#define PCI9056_DM_DAC               (0x0fc + LOCAL_OFFSET)

#ifdef PLX9056_LOC
#define PCI9056_ARBITER_CTRL         0x1A0
#define PCI9056_ABORT_ADDRESS        0x1A4
#endif

/* Runtime registers  PCI Address + LOCAL_OFFSET */
#ifdef PLX9056_LOC
#define PCI9056_MAILBOX0				0x0C0
#define PCI9056_MAILBOX1				0x0C4
#else
#define PCI9056_MAILBOX0				0x078
#define PCI9056_MAILBOX1				0x07c
#endif

#define PCI9056_MAILBOX2				(0x048 + LOCAL_OFFSET)
#define PCI9056_MAILBOX3				(0x04c + LOCAL_OFFSET)
#define PCI9056_MAILBOX4				(0x050 + LOCAL_OFFSET)
#define PCI9056_MAILBOX5				(0x054 + LOCAL_OFFSET)
#define PCI9056_MAILBOX6				(0x058 + LOCAL_OFFSET)
#define PCI9056_MAILBOX7				(0x05c + LOCAL_OFFSET)
#define PCI9056_PCI_TO_LOC_DBELL		(0x060 + LOCAL_OFFSET)
#define PCI9056_LOC_TO_PCI_DBELL		(0x064 + LOCAL_OFFSET)
#define PCI9056_INT_CTRL_STAT			(0x068 + LOCAL_OFFSET)
#define PCI9056_EEPROM_CTRL_STAT		(0x06c + LOCAL_OFFSET)
#define PCI9056_PERM_VENDOR_ID		(0x070 + LOCAL_OFFSET)
#define PCI9056_REVISION_ID			(0x074 + LOCAL_OFFSET)

#endif /* #ifndef __PLX9056_H_ */
