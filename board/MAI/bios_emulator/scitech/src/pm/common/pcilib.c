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
* Description:  Module for interfacing to the PCI bus and configuration
*               space registers.
*
****************************************************************************/

#include "pmapi.h"
#include "pcilib.h"
#if !defined(__WIN32_VXD__) && !defined(__NT_DRIVER__)
#include <string.h>
#endif

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(1)

/* Length of the memory mapping for the PCI BIOS */

#define BIOS_LIMIT          (128 * 1024L - 1)

/* Macros for accessing the PCI BIOS functions from 32-bit protected mode */

#define BIOS32_SIGNATURE    (((ulong)'_' << 0) + ((ulong)'3' << 8) + ((ulong)'2' << 16) + ((ulong)'_' << 24))
#define PCI_SIGNATURE       (((ulong)'P' << 0) + ((ulong)'C' << 8) + ((ulong)'I' << 16) + ((ulong)' ' << 24))
#define PCI_SERVICE         (((ulong)'$' << 0) + ((ulong)'P' << 8) + ((ulong)'C' << 16) + ((ulong)'I' << 24))
#define PCI_BIOS_PRESENT    0xB101
#define FIND_PCI_DEVICE     0xB102
#define FIND_PCI_CLASS      0xB103
#define GENERATE_SPECIAL    0xB106
#define READ_CONFIG_BYTE    0xB108
#define READ_CONFIG_WORD    0xB109
#define READ_CONFIG_DWORD   0xB10A
#define WRITE_CONFIG_BYTE   0xB10B
#define WRITE_CONFIG_WORD   0xB10C
#define WRITE_CONFIG_DWORD  0xB10D
#define GET_IRQ_ROUTING_OPT 0xB10E
#define SET_PCI_IRQ         0xB10F

/* This is the standard structure used to identify the entry point to the
 * BIOS32 Service Directory, as documented in PCI 2.1 BIOS Specicition.
 */

typedef union {
    struct {
	ulong signature;        /* _32_                                 */
	ulong entry;            /* 32 bit physical address              */
	uchar revision;         /* Revision level, 0                    */
	uchar length;           /* Length in paragraphs should be 01    */
	uchar checksum;         /* All bytes must add up to zero        */
	uchar reserved[5];      /* Must be zero                         */
	} fields;
    char chars[16];
    } PCI_bios32;

/* Structure for a far pointer to call the PCI BIOS services with */

typedef struct {
    ulong   address;
    ushort  segment;
    } PCIBIOS_entry;

/* Macros to copy a structure that includes dwSize members */

#define COPY_STRUCTURE(d,s) memcpy(d,s,MIN((s)->dwSize,(d)->dwSize))

#pragma pack()

/*--------------------------- Global variables ----------------------------*/

static uchar            *BIOSImage = NULL;  /* BIOS image mapping       */
static int              PCIBIOSVersion = -1;/* PCI BIOS version         */
static PCIBIOS_entry    PCIEntry;           /* PCI services entry point */
static ulong            PCIPhysEntry = 0;   /* Physical address         */

/*----------------------------- Implementation ----------------------------*/

/* External assembler helper functions */

uchar   _ASMAPI _BIOS32_service(ulong service,ulong function,ulong *physBase,ulong *length,ulong *serviceOffset,PCIBIOS_entry entry);
ushort  _ASMAPI _PCIBIOS_isPresent(ulong i_eax,ulong *o_edx,ushort *o_ax,uchar *o_cl,PCIBIOS_entry entry);
ulong   _ASMAPI _PCIBIOS_service(ulong r_eax,ulong r_ebx,ulong r_edi,ulong r_ecx,PCIBIOS_entry entry);
int     _ASMAPI _PCIBIOS_getRouting(PCIRoutingOptionsBuffer *buf,PCIBIOS_entry entry);
ibool   _ASMAPI _PCIBIOS_setIRQ(int busDev,int intPin,int IRQ,PCIBIOS_entry entry);
ulong   _ASMAPI _PCIBIOS_specialCycle(int bus,ulong data,PCIBIOS_entry entry);
ushort  _ASMAPI _PCI_getCS(void);

/****************************************************************************
REMARKS:
This functions returns the physical address of the PCI BIOS entry point.
****************************************************************************/
ulong _ASMAPI PCIBIOS_getEntry(void)
{ return PCIPhysEntry; }

/****************************************************************************
PARAMETERS:
hwType  - Place to store the PCI hardware access mechanism flags
lastBus - Place to store the index of the last PCI bus in the system

RETURNS:
Version number of the PCI BIOS found.

REMARKS:
This function determines if the PCI BIOS is present in the system, and if
so returns the information returned by the PCI BIOS detect function.
****************************************************************************/
static int PCIBIOS_detect(
    uchar *hwType,
    uchar *lastBus)
{
    ulong   signature;
    ushort  stat,version;

#ifndef __16BIT__
    PCIBIOS_entry   BIOSEntry = {0};
    uchar           *BIOSEnd;
    PCI_bios32      *BIOSDir;
    ulong           physBase,length,offset;

    /* Bail if we have already detected no BIOS is present */
    if (PCIBIOSVersion == 0)
	return 0;

    /* First scan the memory from 0xE0000 to 0xFFFFF looking for the
     * BIOS32 service directory, so we can determine if we can call it
     * from 32-bit protected mode.
     */
    if (PCIBIOSVersion == -1) {
	PCIBIOSVersion = 0;
	BIOSImage = PM_mapPhysicalAddr(0xE0000,BIOS_LIMIT,false);
	if (!BIOSImage)
	    return 0;
	BIOSEnd = BIOSImage + 0x20000;
	for (BIOSDir = (PCI_bios32*)BIOSImage; BIOSDir < (PCI_bios32*)BIOSEnd; BIOSDir++) {
	    uchar   sum;
	    int     i,length;

	    if (BIOSDir->fields.signature != BIOS32_SIGNATURE)
		continue;
	    length = BIOSDir->fields.length * 16;
	    if (!length)
		continue;
	    for (sum = i = 0; i < length ; i++)
		sum += BIOSDir->chars[i];
	    if (sum != 0)
		continue;
	    BIOSEntry.address = (ulong)BIOSImage + (BIOSDir->fields.entry - 0xE0000);
	    BIOSEntry.segment = _PCI_getCS();
	    break;
	    }

	/* If we found the BIOS32 directory, call it to get the address of the
	 * PCI services.
	 */
	if (BIOSEntry.address == 0)
	    return 0;
	if (_BIOS32_service(PCI_SERVICE,0,&physBase,&length,&offset,BIOSEntry) != 0)
	    return 0;
	PCIPhysEntry = physBase + offset;
	PCIEntry.address = (ulong)BIOSImage + (PCIPhysEntry - 0xE0000);
	PCIEntry.segment = _PCI_getCS();
	}
#endif
    /* We found the BIOS entry, so now do the version check */
    version = _PCIBIOS_isPresent(PCI_BIOS_PRESENT,&signature,&stat,lastBus,PCIEntry);
    if (version > 0 && ((stat >> 8) == 0) && signature == PCI_SIGNATURE) {
	*hwType = stat & 0xFF;
	return PCIBIOSVersion = version;
	}
    return 0;
}

/****************************************************************************
PARAMETERS:
info    - Array of PCIDeviceInfo structures to check against
index   - Index of the current device to check

RETURNS:
True if the device is a duplicate, false if not.

REMARKS:
This function goes through the list of all devices preceeding the newly
found device in the info structure, and checks that the device is not a
duplicate of a previous device. Some devices incorrectly enumerate
themselves at different function addresses so we check here to exclude
those cases.
****************************************************************************/
static ibool CheckDuplicate(
    PCIDeviceInfo *info,
    PCIDeviceInfo *prev)
{
    /* Ignore devices with a vendor ID of 0 */
    if (info->VendorID == 0)
	return true;

    /* NOTE: We only check against the current device on
     *       the bus to ensure that we do not exclude
     *       multiple controllers of the same device ID.
     */
    if (info->slot.p.Bus == prev->slot.p.Bus &&
	info->slot.p.Device == prev->slot.p.Device &&
	info->DeviceID == prev->DeviceID)
	return true;
    return false;
}

/****************************************************************************
PARAMETERS:
info        - Array of PCIDeviceInfo structures to fill in
maxDevices  - Maximum number of of devices to enumerate into array

RETURNS:
Number of PCI devices found and enumerated on the PCI bus, 0 if not PCI.

REMARKS:
Function to enumerate all available devices on the PCI bus into an array
of configuration information blocks.
****************************************************************************/
static int PCI_enumerateMech1(
    PCIDeviceInfo info[])
{
    int             bus,device,function,i,numFound = 0;
    ulong           *lp,tmp;
    PCIslot         slot = {{0,0,0,0,0,0,1}};
    PCIDeviceInfo   pci,prev = {0};

    /* Try PCI access mechanism 1 */
    PM_outpb(0xCFB,0x01);
    tmp = PM_inpd(0xCF8);
    PM_outpd(0xCF8,slot.i);
    if ((PM_inpd(0xCF8) == slot.i) && (PM_inpd(0xCFC) != 0xFFFFFFFFUL)) {
	/* PCI access mechanism 1 - the preferred mechanism */
	for (bus = 0; bus < 8; bus++) {
	    slot.p.Bus = bus;
	    for (device = 0; device < 32; device++) {
		slot.p.Device = device;
		for (function = 0; function < 8; function++) {
		    slot.p.Function = function;
		    slot.p.Register = 0;
		    PM_outpd(0xCF8,slot.i);
		    if (PM_inpd(0xCFC) != 0xFFFFFFFFUL) {
			memset(&pci,0,sizeof(pci));
			pci.dwSize = sizeof(pci);
			pci.mech1 = 1;
			pci.slot = slot;
			lp = (ulong*)&(pci.VendorID);
			for (i = 0; i < NUM_PCI_REG; i++, lp++) {
			    slot.p.Register = i;
			    PM_outpd(0xCF8,slot.i);
			    *lp = PM_inpd(0xCFC);
			    }
			if (!CheckDuplicate(&pci,&prev)) {
			    if (info)
				COPY_STRUCTURE(&info[numFound],&pci);
			    ++numFound;
			    }
			prev = pci;
			}
		    }
		}
	    }

	/* Disable PCI config cycle on exit */
	PM_outpd(0xCF8,0);
	return numFound;
	}
    PM_outpd(0xCF8,tmp);

    /* No hardware access mechanism 1 found */
    return 0;
}

/****************************************************************************
PARAMETERS:
info        - Array of PCIDeviceInfo structures to fill in
maxDevices  - Maximum number of of devices to enumerate into array

RETURNS:
Number of PCI devices found and enumerated on the PCI bus, 0 if not PCI.

REMARKS:
Function to enumerate all available devices on the PCI bus into an array
of configuration information blocks.
****************************************************************************/
static int PCI_enumerateMech2(
    PCIDeviceInfo info[])
{
    int             bus,device,function,i,numFound = 0;
    ushort          deviceIO;
    ulong           *lp;
    PCIslot         slot = {{0,0,0,0,0,0,1}};
    PCIDeviceInfo   pci,prev = {0};

    /* Try PCI access mechanism 2 */
    PM_outpb(0xCFB,0x00);
    PM_outpb(0xCF8,0x00);
    PM_outpb(0xCFA,0x00);
    if (PM_inpb(0xCF8) == 0x00 && PM_inpb(0xCFB) == 0x00) {
	/* PCI access mechanism 2 - the older mechanism for legacy busses */
	for (bus = 0; bus < 2; bus++) {
	    slot.p.Bus = bus;
	    PM_outpb(0xCFA,(uchar)bus);
	    for (device = 0; device < 16; device++) {
		slot.p.Device = device;
		deviceIO = 0xC000 + (device << 8);
		for (function = 0; function < 8; function++) {
		    slot.p.Function = function;
		    slot.p.Register = 0;
		    PM_outpb(0xCF8,(uchar)((function << 1) | 0x10));
		    if (PM_inpd(deviceIO) != 0xFFFFFFFFUL) {
			memset(&pci,0,sizeof(pci));
			pci.dwSize = sizeof(pci);
			pci.mech1 = 0;
			pci.slot = slot;
			lp = (ulong*)&(pci.VendorID);
			for (i = 0; i < NUM_PCI_REG; i++, lp++) {
			    slot.p.Register = i;
			    *lp = PM_inpd(deviceIO + (i << 2));
			    }
			if (!CheckDuplicate(&pci,&prev)) {
			    if (info)
				COPY_STRUCTURE(&info[numFound],&pci);
			    ++numFound;
			    }
			prev = pci;
			}
		    }
		}
	    }

	/* Disable PCI config cycle on exit */
	PM_outpb(0xCF8,0);
	return numFound;
	}

    /* No hardware access mechanism 2 found */
    return 0;
}

/****************************************************************************
REMARKS:
This functions reads a configuration dword via the PCI BIOS.
****************************************************************************/
static ulong PCIBIOS_readDWORD(
    int index,
    ulong slot)
{
    return (ulong)_PCIBIOS_service(READ_CONFIG_DWORD,slot >> 8,index,0,PCIEntry);
}

/****************************************************************************
PARAMETERS:
info        - Array of PCIDeviceInfo structures to fill in
maxDevices  - Maximum number of of devices to enumerate into array

RETURNS:
Number of PCI devices found and enumerated on the PCI bus, 0 if not PCI.

REMARKS:
Function to enumerate all available devices on the PCI bus into an array
of configuration information blocks.
****************************************************************************/
static int PCI_enumerateBIOS(
    PCIDeviceInfo info[])
{
    uchar           hwType,lastBus;
    int             bus,device,function,i,numFound = 0;
    ulong           *lp;
    PCIslot         slot = {{0,0,0,0,0,0,1}};
    PCIDeviceInfo   pci,prev = {0};

    if (PCIBIOS_detect(&hwType,&lastBus)) {
	/* PCI BIOS access - the ultimate fallback */
	for (bus = 0; bus <= lastBus; bus++) {
	    slot.p.Bus = bus;
	    for (device = 0; device < 32; device++) {
		slot.p.Device = device;
		for (function = 0; function < 8; function++) {
		    slot.p.Function = function;
		    if (PCIBIOS_readDWORD(0,slot.i) != 0xFFFFFFFFUL) {
			memset(&pci,0,sizeof(pci));
			pci.dwSize = sizeof(pci);
			pci.mech1 = 2;
			pci.slot = slot;
			lp = (ulong*)&(pci.VendorID);
			for (i = 0; i < NUM_PCI_REG; i++, lp++)
			    *lp = PCIBIOS_readDWORD(i << 2,slot.i);
			if (!CheckDuplicate(&pci,&prev)) {
			    if (info)
				COPY_STRUCTURE(&info[numFound],&pci);
			    ++numFound;
			    }
			prev = pci;
			}
		    }
		}
	    }
	}

    /* Return number of devices found */
    return numFound;
}

/****************************************************************************
PARAMETERS:
info        - Array of PCIDeviceInfo structures to fill in
maxDevices  - Maximum number of of devices to enumerate into array

RETURNS:
Number of PCI devices found and enumerated on the PCI bus, 0 if not PCI.

REMARKS:
Function to enumerate all available devices on the PCI bus into an array
of configuration information blocks.
****************************************************************************/
int _ASMAPI PCI_enumerate(
    PCIDeviceInfo info[])
{
    int numFound;

    /* First try via the direct access mechanisms which are faster if we
     * have them (nearly always). The BIOS is used as a fallback, and for
     * stuff we can't do directly.
     */
    if ((numFound = PCI_enumerateMech1(info)) == 0) {
	if ((numFound = PCI_enumerateMech2(info)) == 0) {
	    if ((numFound = PCI_enumerateBIOS(info)) == 0)
		return 0;
	    }
	}
    return numFound;
}

/****************************************************************************
PARAMETERS:
info        - Array of PCIDeviceInfo structures to fill in
maxDevices  - Maximum number of of devices to enumerate into array

RETURNS:
Number of PCI devices found and enumerated on the PCI bus, 0 if not PCI.

REMARKS:
Function to enumerate all available devices on the PCI bus into an array
of configuration information blocks.
****************************************************************************/
int _ASMAPI PCI_getNumDevices(void)
{
    return PCI_enumerate(NULL);
}

/****************************************************************************
PARAMETERS:
bar - Base address to measure
pci - PCI device to access

RETURNS:
Size of the PCI base address in bytes

REMARKS:
This function measures the size of the PCI base address register in bytes,
by writing all F's to the register, and reading the value back. The size
of the base address is determines by the bits that are hardwired to zero's.
****************************************************************************/
ulong _ASMAPI PCI_findBARSize(
    int bar,
    PCIDeviceInfo *pci)
{
    ulong   base,size = 0;

    base = PCI_accessReg(bar,0,PCI_READ_DWORD,pci);
    if (base && !(base & 0x1)) {
	/* For some strange reason some devices don't properly decode
	 * their base address registers (Intel PCI/PCI bridges!), and
	 * we read completely bogus values. We check for that here
	 * and clear out those BAR's.
	 *
	 * We check for that here because at least the low 12 bits
	 * of the address range must be zeros, since the page size
	 * on IA32 processors is always 4Kb.
	 */
	if ((base & 0xFFF) == 0) {
	    PCI_accessReg(bar,0xFFFFFFFF,PCI_WRITE_DWORD,pci);
	    size = PCI_accessReg(bar,0,PCI_READ_DWORD,pci) & ~0xFF;
	    size = ~size+1;
	    PCI_accessReg(bar,base,PCI_WRITE_DWORD,pci);
	    }
	}
    pci->slot.p.Register = 0;
    return size;
}

/****************************************************************************
PARAMETERS:
index   - DWORD index of the register to access
value   - Value to write to the register for write access
func    - Function to implement

RETURNS:
The value read from the register for read operations

REMARKS:
The function code are defined as follows

code    - function
0       - Read BYTE
1       - Read WORD
2       - Read DWORD
3       - Write BYTE
4       - Write WORD
5       - Write DWORD
****************************************************************************/
ulong _ASMAPI PCI_accessReg(
    int index,
    ulong value,
    int func,
    PCIDeviceInfo *info)
{
    int iobase;

    if (info->mech1 == 2) {
	/* Use PCI BIOS access since we dont have direct hardware access */
	switch (func) {
	    case PCI_READ_BYTE:
		return (uchar)_PCIBIOS_service(READ_CONFIG_BYTE,info->slot.i >> 8,index,0,PCIEntry);
	    case PCI_READ_WORD:
		return (ushort)_PCIBIOS_service(READ_CONFIG_WORD,info->slot.i >> 8,index,0,PCIEntry);
	    case PCI_READ_DWORD:
		return (ulong)_PCIBIOS_service(READ_CONFIG_DWORD,info->slot.i >> 8,index,0,PCIEntry);
	    case PCI_WRITE_BYTE:
		_PCIBIOS_service(WRITE_CONFIG_BYTE,info->slot.i >> 8,index,value,PCIEntry);
		break;
	    case PCI_WRITE_WORD:
		_PCIBIOS_service(WRITE_CONFIG_WORD,info->slot.i >> 8,index,value,PCIEntry);
		break;
	    case PCI_WRITE_DWORD:
		_PCIBIOS_service(WRITE_CONFIG_DWORD,info->slot.i >> 8,index,value,PCIEntry);
		break;
	    }
	}
    else {
	/* Use direct hardware access mechanisms */
	if (info->mech1) {
	    /* PCI access mechanism 1 */
	    iobase = 0xCFC + (index & 3);
	    info->slot.p.Register = index >> 2;
	    PM_outpd(0xCF8,info->slot.i);
	    }
	else {
	    /* PCI access mechanism 2 */
	    PM_outpb(0xCF8,(uchar)((info->slot.p.Function << 1) | 0x10));
	    PM_outpb(0xCFA,(uchar)info->slot.p.Bus);
	    iobase = 0xC000 + (info->slot.p.Device << 8) + index;
	    }
	switch (func) {
	    case PCI_READ_BYTE:
	    case PCI_READ_WORD:
	    case PCI_READ_DWORD:    value = PM_inpd(iobase);        break;
	    case PCI_WRITE_BYTE:    PM_outpb(iobase,(uchar)value);  break;
	    case PCI_WRITE_WORD:    PM_outpw(iobase,(ushort)value); break;
	    case PCI_WRITE_DWORD:   PM_outpd(iobase,(ulong)value);  break;
	    }
	PM_outpd(0xCF8,0);
	}
    return value;
}

/****************************************************************************
PARAMETERS:
numDevices  - Number of devices to query info for

RETURNS:
0 on success, -1 on error, number of devices to enumerate if numDevices = 0

REMARKS:
This function reads the PCI routing information. If you pass a value of
0 for numDevices, this function will return with the number of devices
needed in the routing buffer that will be filled in by the BIOS.
****************************************************************************/
ibool _ASMAPI PCI_getIRQRoutingOptions(
    int numDevices,
    PCIRouteInfo *buffer)
{
    PCIRoutingOptionsBuffer buf;
    int                     ret;

    if (PCIPhysEntry) {
	buf.BufferSize = numDevices * sizeof(PCIRouteInfo);
	buf.DataBuffer = buffer;
	if ((ret = _PCIBIOS_getRouting(&buf,PCIEntry)) == 0x89)
	    return buf.BufferSize / sizeof(PCIRouteInfo);
	if (ret != 0)
	    return -1;
	return 0;
	}

    /* We currently only support this via the PCI BIOS functions */
    return -1;
}

/****************************************************************************
PARAMETERS:
info    - PCI device information for the specified device
intPin  - Value to store in the PCI InterruptPin register
IRQ     - New ISA IRQ to map the PCI interrupt to (0-15)

RETURNS:
True on success, or false if this function failed.

REMARKS:
This function changes the PCI IRQ routing for the specified device to the
desired PCI interrupt and the desired ISA bus compatible IRQ. This function
may not be supported by the PCI BIOS, in which case this function will
fail.
****************************************************************************/
ibool _ASMAPI PCI_setHardwareIRQ(
    PCIDeviceInfo *info,
    uint intPin,
    uint IRQ)
{
    if (PCIPhysEntry) {
	if (_PCIBIOS_setIRQ(info->slot.i >> 8,intPin,IRQ,PCIEntry)) {
	    info->u.type0.InterruptPin = intPin;
	    info->u.type0.InterruptLine = IRQ;
	    return true;
	    }
	return false;
	}

    /* We currently only support this via the PCI BIOS functions */
    return false;
}

/****************************************************************************
PARAMETERS:
bus                 - Bus number to generate the special cycle for
specialCycleData    - Data to send for the special cyle

REMARKS:
This function generates a special cycle on the specified bus using with
the specified data.
****************************************************************************/
void _ASMAPI PCI_generateSpecialCyle(
    uint bus,
    ulong specialCycleData)
{
    if (PCIPhysEntry)
	_PCIBIOS_specialCycle(bus,specialCycleData,PCIEntry);
    /* We currently only support this via the PCI BIOS functions */
}

/****************************************************************************
PARAMETERS:
info    - PCI device information block for device to access
index   - Index of register to start reading from
dst     - Place to store the values read from configuration space
count   - Count of bytes to read from configuration space

REMARKS:
This function is used to read a block of PCI configuration space registers
from the configuration space into the passed in data block. This function
will properly handle reading non-DWORD aligned data from the configuration
space correctly.
****************************************************************************/
void _ASMAPI PCI_readRegBlock(
    PCIDeviceInfo *info,
    int index,
    void *dst,
    int count)
{
    uchar   *pb;
    ulong   *pd;
    int     i;
    int     startCount = (index & 3);
    int     middleCount = (count - startCount) >> 2;
    int     endCount = count - middleCount * 4 - startCount;

    for (i = 0,pb = dst; i < startCount; i++, index++) {
	*pb++ = (uchar)PCI_accessReg(index,0,PCI_READ_BYTE,info);
	}
    for (i = 0,pd = (ulong*)pb; i < middleCount; i++, index += 4) {
	*pd++ = (ulong)PCI_accessReg(index,0,PCI_READ_DWORD,info);
	}
    for (i = 0,pb = (uchar*)pd; i < endCount; i++, index++) {
	*pb++ = (uchar)PCI_accessReg(index,0,PCI_READ_BYTE,info);
	}
}

/****************************************************************************
PARAMETERS:
info    - PCI device information block for device to access
index   - Index of register to start reading from
dst     - Place to store the values read from configuration space
count   - Count of bytes to read from configuration space

REMARKS:
This function is used to write a block of PCI configuration space registers
to the configuration space from the passed in data block. This function
will properly handle writing non-DWORD aligned data to the configuration
space correctly.
****************************************************************************/
void _ASMAPI PCI_writeRegBlock(
    PCIDeviceInfo *info,
    int index,
    void *src,
    int count)
{
    uchar   *pb;
    ulong   *pd;
    int     i;
    int     startCount = (index & 3);
    int     middleCount = (count - startCount) >> 2;
    int     endCount = count - middleCount * 4 - startCount;

    for (i = 0,pb = src; i < startCount; i++, index++) {
	PCI_accessReg(index,*pb++,PCI_WRITE_BYTE,info);
	}
    for (i = 0,pd = (ulong*)pb; i < middleCount; i++, index += 4) {
	PCI_accessReg(index,*pd++,PCI_WRITE_DWORD,info);
	}
    for (i = 0,pb = (uchar*)pd; i < endCount; i++, index++) {
	PCI_accessReg(index,*pb++,PCI_WRITE_BYTE,info);
	}
}
