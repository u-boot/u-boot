/****************************************************************************
*
*                   SciTech OS Portability Manager Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Any
*
* Description:  Header file for interface routines to the PCI bus.
*
****************************************************************************/

#ifndef __PCILIB_H
#define __PCILIB_H

#include "scitech.h"

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(1)

/* Defines for PCIDeviceInfo.HeaderType */

typedef enum {
    PCI_deviceType                  = 0x00,
    PCI_bridgeType                  = 0x01,
    PCI_cardBusBridgeType           = 0x02,
    PCI_multiFunctionType           = 0x80
    } PCIHeaderTypeFlags;

/* Defines for PCIDeviceInfo.Command */

typedef enum {
    PCI_enableIOSpace               = 0x0001,
    PCI_enableMemorySpace           = 0x0002,
    PCI_enableBusMaster             = 0x0004,
    PCI_enableSpecialCylces         = 0x0008,
    PCI_enableWriteAndInvalidate    = 0x0010,
    PCI_enableVGACompatiblePalette  = 0x0020,
    PCI_enableParity                = 0x0040,
    PCI_enableWaitCycle             = 0x0080,
    PCI_enableSerr                  = 0x0100,
    PCI_enableFastBackToBack        = 0x0200
    } PCICommandFlags;

/* Defines for PCIDeviceInfo.Status */

typedef enum {
    PCI_statusCapabilitiesList      = 0x0010,
    PCI_status66MhzCapable          = 0x0020,
    PCI_statusUDFSupported          = 0x0040,
    PCI_statusFastBackToBack        = 0x0080,
    PCI_statusDataParityDetected    = 0x0100,
    PCI_statusDevSel                = 0x0600,
    PCI_statusSignaledTargetAbort   = 0x0800,
    PCI_statusRecievedTargetAbort   = 0x1000,
    PCI_statusRecievedMasterAbort   = 0x2000,
    PCI_statusSignaledSystemError   = 0x4000,
    PCI_statusDetectedParityError   = 0x8000
    } PCIStatusFlags;

/* PCI capability IDs */

typedef enum {
    PCI_capsPowerManagement         = 0x01,
    PCI_capsAGP                     = 0x02,
    PCI_capsMSI                     = 0x05
    } PCICapsType;

/* PCI AGP rate definitions */

typedef enum {
    PCI_AGPRate1X                   = 0x1,
    PCI_AGPRate2X                   = 0x2,
    PCI_AGPRate4X                   = 0x4
    } PCIAGPRateType;

/* NOTE: We define all bitfield's as uint's, specifically so that the IBM
 *       Visual Age C++ compiler does not complain. We need them to be
 *       32-bits wide, and this is the width of an unsigned integer, but
 *       we can't use a ulong to make this explicit or we get errors.
 */

/* Structure defining a PCI slot identifier */

typedef union {
    struct {
	uint    Zero:2;
	uint    Register:6;
	uint    Function:3;
	uint    Device:5;
	uint    Bus:8;
	uint    Reserved:7;
	uint    Enable:1;
	} p;
    ulong   i;
    } PCIslot;

/* Structure defining the regular (type 0) PCI configuration register
 * layout. We use this in a union below so we can describe all types of
 * PCI configuration spaces with a single structure.
 */

typedef struct {
    ulong   BaseAddress10;
    ulong   BaseAddress14;
    ulong   BaseAddress18;
    ulong   BaseAddress1C;
    ulong   BaseAddress20;
    ulong   BaseAddress24;
    ulong   CardbusCISPointer;
    ushort  SubSystemVendorID;
    ushort  SubSystemID;
    ulong   ROMBaseAddress;
    uchar   CapabilitiesPointer;
    uchar   reserved1;
    uchar   reserved2;
    uchar   reserved3;
    ulong   reserved4;
    uchar   InterruptLine;
    uchar   InterruptPin;
    uchar   MinimumGrant;
    uchar   MaximumLatency;

    /* These are not in the actual config space, but we enumerate them */
    ulong   BaseAddress10Len;
    ulong   BaseAddress14Len;
    ulong   BaseAddress18Len;
    ulong   BaseAddress1CLen;
    ulong   BaseAddress20Len;
    ulong   BaseAddress24Len;
    ulong   ROMBaseAddressLen;
    } PCIType0Info;

/* Structure defining PCI to PCI bridge (type 1) PCI configuration register
 * layout. We use this in a union below so we can describe all types of
 * PCI configuration spaces with a single structure.
 */

typedef struct {
    ulong   BaseAddress10;
    ulong   BaseAddress14;
    uchar   PrimaryBusNumber;
    uchar   SecondayBusNumber;
    uchar   SubordinateBusNumber;
    uchar   SecondaryLatencyTimer;
    uchar   IOBase;
    uchar   IOLimit;
    ushort  SecondaryStatus;
    ushort  MemoryBase;
    ushort  MemoryLimit;
    ushort  PrefetchableMemoryBase;
    ushort  PrefetchableMemoryLimit;
    ulong   PrefetchableBaseHi;
    ulong   PrefetchableLimitHi;
    ushort  IOBaseHi;
    ushort  IOLimitHi;
    uchar   CapabilitiesPointer;
    uchar   reserved1;
    uchar   reserved2;
    uchar   reserved3;
    ulong   ROMBaseAddress;
    uchar   InterruptLine;
    uchar   InterruptPin;
    ushort  BridgeControl;
    } PCIType1Info;

/* PCI to CardBus bridge (type 2) configuration information */
typedef struct {
    ulong   SocketRegistersBaseAddress;
    uchar   CapabilitiesPointer;
    uchar   reserved1;
    ushort  SecondaryStatus;
    uchar   PrimaryBus;
    uchar   SecondaryBus;
    uchar   SubordinateBus;
    uchar   SecondaryLatency;
    struct  {
	ulong   Base;
	ulong   Limit;
	} Range[4];
    uchar   InterruptLine;
    uchar   InterruptPin;
    ushort  BridgeControl;
    } PCIType2Info;

/* Structure defining the PCI configuration space information for a
 * single PCI device on the PCI bus. We enumerate all this information
 * for all PCI devices on the bus.
 */

typedef struct {
    ulong               dwSize;
    PCIslot             slot;
    ulong               mech1;
    ushort              VendorID;
    ushort              DeviceID;
    ushort              Command;
    ushort              Status;
    uchar               RevID;
    uchar               Interface;
    uchar               SubClass;
    uchar               BaseClass;
    uchar               CacheLineSize;
    uchar               LatencyTimer;
    uchar               HeaderType;
    uchar               BIST;
    union {
	PCIType0Info    type0;
	PCIType1Info    type1;
	PCIType2Info    type2;
	} u;
    } PCIDeviceInfo;

/* PCI Capability header structure. All PCI capabilities have the
 * following header.
 *
 * capsID is used to identify the type of the structure as define above.
 *
 * next is the offset in PCI configuration space (0x40-0xFC) of the
 * next capability structure in the list, or 0x00 if there are no more
 * entries.
 */

typedef struct {
    uchar   capsID;
    uchar   next;
    } PCICapsHeader;

/* Structure defining the PCI AGP status register contents */

typedef struct {
    uint    rate:3;
    uint    rsvd1:1;
    uint    fastWrite:1;
    uint    fourGB:1;
    uint    rsvd2:3;
    uint    sideBandAddressing:1;
    uint    rsvd3:14;
    uint    requestQueueDepthMaximum:8;
    } PCIAGPStatus;

/* Structure defining the PCI AGP command register contents */

typedef struct {
    uint    rate:3;
    uint    rsvd1:1;
    uint    fastWriteEnable:1;
    uint    fourGBEnable:1;
    uint    rsvd2:2;
    uint    AGPEnable:1;
    uint    SBAEnable:1;
    uint    rsvd3:14;
    uint    requestQueueDepth:8;
    } PCIAGPCommand;

/* AGP Capability structure */

typedef struct {
    PCICapsHeader   h;
    ushort          majMin;
    PCIAGPStatus    AGPStatus;
    PCIAGPCommand   AGPCommand;
    } PCIAGPCapability;

/* Structure for obtaining the PCI IRQ routing information */

typedef struct {
    uchar   bus;
    uchar   device;
    uchar   linkA;
    ushort  mapA;
    uchar   linkB;
    ushort  mapB;
    uchar   linkC;
    ushort  mapC;
    uchar   linkD;
    ushort  mapD;
    uchar   slot;
    uchar   reserved;
    } PCIRouteInfo;

typedef struct {
    ushort          BufferSize;
    PCIRouteInfo    *DataBuffer;
    } PCIRoutingOptionsBuffer;

#define NUM_PCI_REG                 (sizeof(PCIDeviceInfo) / 4) - 10
#define PCI_BRIDGE_CLASS            0x06
#define PCI_HOST_BRIDGE_SUBCLASS    0x00
#define PCI_EARLY_VGA_CLASS         0x00
#define PCI_EARLY_VGA_SUBCLASS      0x01
#define PCI_DISPLAY_CLASS           0x03
#define PCI_DISPLAY_VGA_SUBCLASS    0x00
#define PCI_DISPLAY_XGA_SUBCLASS    0x01
#define PCI_DISPLAY_OTHER_SUBCLASS  0x80
#define PCI_MM_CLASS                0x04
#define PCI_AUDIO_SUBCLASS          0x01

/* Macros to detect specific classes of devices */

#define PCI_IS_3DLABS_NONVGA_CLASS(pci) \
   (((pci)->BaseClass == PCI_DISPLAY_CLASS && (pci)->SubClass == PCI_DISPLAY_OTHER_SUBCLASS) \
 && ((pci)->VendorID == 0x3D3D || (pci)->VendorID == 0x104C))

#define PCI_IS_DISPLAY_CLASS(pci) \
   (((pci)->BaseClass == PCI_DISPLAY_CLASS && (pci)->SubClass == PCI_DISPLAY_VGA_SUBCLASS) \
 || ((pci)->BaseClass == PCI_DISPLAY_CLASS && (pci)->SubClass == PCI_DISPLAY_XGA_SUBCLASS) \
 || ((pci)->BaseClass == PCI_EARLY_VGA_CLASS && (pci)->SubClass == PCI_EARLY_VGA_SUBCLASS) \
 || PCI_IS_3DLABS_NONVGA_CLASS(pci))

/* Function codes to pass to PCI_accessReg */

#define PCI_READ_BYTE               0
#define PCI_READ_WORD               1
#define PCI_READ_DWORD              2
#define PCI_WRITE_BYTE              3
#define PCI_WRITE_WORD              4
#define PCI_WRITE_DWORD             5

/* Macros to read/write PCI registers. These assume a global PCI array
 * of device information.
 */

#define PCI_readPCIRegB(index,device)   \
    PCI_accessReg(index,0,0,&PCI[DeviceIndex[device]])

#define PCI_readPCIRegW(index,device)   \
    PCI_accessReg(index,0,1,&PCI[DeviceIndex[device]])

#define PCI_readPCIRegL(index,device)   \
    PCI_accessReg(index,0,2,&PCI[DeviceIndex[device]])

#define PCI_writePCIRegB(index,value,device)    \
    PCI_accessReg(index,value,3,&PCI[DeviceIndex[device]])

#define PCI_writePCIRegW(index,value,device)    \
    PCI_accessReg(index,value,4,&PCI[DeviceIndex[device]])

#define PCI_writePCIRegL(index,value,device)    \
    PCI_accessReg(index,value,5,&PCI[DeviceIndex[device]])

#pragma pack()

/*-------------------------- Function Prototypes --------------------------*/

#ifdef  __cplusplus
extern "C" {                        /* Use "C" linkage when in C++ mode */
#endif

/* Function to determine the number of PCI devices in the system */

int     _ASMAPI PCI_getNumDevices(void);

/* Function to enumerate all device on the PCI bus */

int     _ASMAPI PCI_enumerate(PCIDeviceInfo info[]);

/* Function to access PCI configuration registers */

ulong   _ASMAPI PCI_accessReg(int index,ulong value,int func,PCIDeviceInfo *info);

/* Function to get PCI IRQ routing options for a card */

int     _ASMAPI PCI_getIRQRoutingOptions(int numDevices,PCIRouteInfo *buffer);

/* Function to re-route the PCI IRQ setting for a device */

ibool   _ASMAPI PCI_setHardwareIRQ(PCIDeviceInfo *info,uint intPin,uint IRQ);

/* Function to generate a special cyle on the specified PCI bus */

void    _ASMAPI PCI_generateSpecialCyle(uint bus,ulong specialCycleData);

/* Function to determine the size of a PCI base address register */

ulong   _ASMAPI PCI_findBARSize(int bar,PCIDeviceInfo *pci);

/* Function to read a block of PCI configuration space registers */

void    _ASMAPI PCI_readRegBlock(PCIDeviceInfo *info,int index,void *dst,int count);

/* Function to write a block of PCI configuration space registers */

void    _ASMAPI PCI_writeRegBlock(PCIDeviceInfo *info,int index,void *src,int count);

/* Function to return the 32-bit PCI BIOS entry point */

ulong   _ASMAPI PCIBIOS_getEntry(void);

#ifdef  __cplusplus
}                                   /* End of "C" linkage for C++       */
#endif

#endif  /* __PCILIB_H */
