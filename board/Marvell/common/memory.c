/*
 * Copyright - Galileo technology.
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

/*
 *
 * written or collected and sometimes rewritten by
 * Ingo Assmus <ingo.assmus@keymile.com>
 *
 */


#include <common.h>
#include "../include/core.h"
#include "../include/memory.h"

/*******************************************************************************
* memoryGetBankBaseAddress - Returns the base address of a memory bank.
* DESCRIPTION:
*       This function returns the base address of one of the SDRAM's memory
*       banks. There are 4 memory banks and each one represents one DIMM side.
* INPUT:
*       MEMORY_BANK bank - Selects one of the four banks as defined in Memory.h.
* OUTPUT:
*       None.
* RETURN:
*       32 bit Memory bank base address.
*******************************************************************************/
static unsigned long memoryGetBankRegOffset (MEMORY_BANK bank)
{
	switch (bank) {
	case BANK0:
		return SCS_0_LOW_DECODE_ADDRESS;
	case BANK1:
		return SCS_1_LOW_DECODE_ADDRESS;
	case BANK2:
		return SCS_2_LOW_DECODE_ADDRESS;
	case BANK3:
		return SCS_3_LOW_DECODE_ADDRESS;

	}
	return SCS_0_LOW_DECODE_ADDRESS;	/* default value */
}

unsigned int memoryGetBankBaseAddress (MEMORY_BANK bank)
{
	unsigned int base;
	unsigned int regOffset = memoryGetBankRegOffset (bank);

	GT_REG_READ (regOffset, &base);
	base = base << 16;	/* MV6436x */
	return base;
}

/*******************************************************************************
* memoryGetDeviceBaseAddress - Returns the base address of a device.
* DESCRIPTION:
*       This function returns the base address of a device on the system. There
*       are 5 possible devices (0 - 4 and one boot device) as defined in
*       gtMemory.h. Each of the device parameters is maped to one of the CS
*       (Devices chip selects) base address register.
* INPUT:
*       device - Selects one of the five devices as defined in Memory.h.
* OUTPUT:
*       None.
* RETURN:
*       32 bit Device base address.
*
*******************************************************************************/
static unsigned int memoryGetDeviceRegOffset (DEVICE device)
{
	switch (device) {
	case DEVICE0:
		return CS_0_LOW_DECODE_ADDRESS;
	case DEVICE1:
		return CS_1_LOW_DECODE_ADDRESS;
	case DEVICE2:
		return CS_2_LOW_DECODE_ADDRESS;
	case DEVICE3:
		return CS_3_LOW_DECODE_ADDRESS;
	case BOOT_DEVICE:
		return BOOTCS_LOW_DECODE_ADDRESS;
	}
	return CS_0_LOW_DECODE_ADDRESS;	/* default value */
}

unsigned int memoryGetDeviceBaseAddress (DEVICE device)
{
	unsigned int regBase;
	unsigned int regOffset = memoryGetDeviceRegOffset (device);

	GT_REG_READ (regOffset, &regBase);

	regBase = regBase << 16;	/* MV6436x */
	return regBase;
}

/*******************************************************************************
* MemoryGetPciBaseAddr - Returns the base address of a PCI window.
* DESCRIPTION:
*       This function returns the base address of a PCI window. There are 5
*       possible PCI windows (memory 0 - 3 and one for I/O) for each PCI
*       interface as defined in gtMemory.h, used by the CPU's address decoding
*       mechanism.
*	New in MV6436x
* INPUT:
*       pciWindow - Selects one of the PCI windows as defined in Memory.h.
* OUTPUT:
*       None.
* RETURN:
*       32 bit PCI window base address.
*******************************************************************************/
unsigned int MemoryGetPciBaseAddr (PCI_MEM_WINDOW pciWindow)
{
	unsigned int baseAddrReg, base;

	switch (pciWindow) {
	case PCI_0_IO:
		baseAddrReg = PCI_0I_O_LOW_DECODE_ADDRESS;	/*PCI_0_IO_BASE_ADDR;  */
		break;
	case PCI_0_MEM0:
		baseAddrReg = PCI_0MEMORY0_LOW_DECODE_ADDRESS;	/*PCI_0_MEMORY0_BASE_ADDR; */
		break;
	case PCI_0_MEM1:
		baseAddrReg = PCI_0MEMORY1_LOW_DECODE_ADDRESS;	/*PCI_0_MEMORY1_BASE_ADDR; */
		break;
	case PCI_0_MEM2:
		baseAddrReg = PCI_0MEMORY2_LOW_DECODE_ADDRESS;	/*PCI_0_MEMORY2_BASE_ADDR;  */
		break;
	case PCI_0_MEM3:
		baseAddrReg = PCI_0MEMORY3_LOW_DECODE_ADDRESS;	/*PCI_0_MEMORY3_BASE_ADDR;  */
		break;
#ifdef INCLUDE_PCI_1
	case PCI_1_IO:
		baseAddrReg = PCI_1I_O_LOW_DECODE_ADDRESS;	/*PCI_1_IO_BASE_ADDR;  */
		break;
	case PCI_1_MEM0:
		baseAddrReg = PCI_1MEMORY0_LOW_DECODE_ADDRESS;	/*PCI_1_MEMORY0_BASE_ADDR; */
		break;
	case PCI_1_MEM1:
		baseAddrReg = PCI_1MEMORY1_LOW_DECODE_ADDRESS;	/*PCI_1_MEMORY1_BASE_ADDR;  */
		break;
	case PCI_1_MEM2:
		baseAddrReg = PCI_1MEMORY2_LOW_DECODE_ADDRESS;	/*PCI_1_MEMORY2_BASE_ADDR;  */
		break;
	case PCI_1_MEM3:
		baseAddrReg = PCI_1MEMORY3_LOW_DECODE_ADDRESS;	/*PCI_1_MEMORY3_BASE_ADDR; */
		break;
#endif /* INCLUDE_PCI_1 */
	default:
		return 0xffffffff;
	}
	GT_REG_READ (baseAddrReg, &base);
	return (base << 16);
}

/*******************************************************************************
* memoryGetBankSize - Returns the size of a memory bank.
* DESCRIPTION:
*       This function returns the size of memory bank as described in
*       'gtMemoryGetBankBaseAddress' function.
* INPUT:
*       bank - Selects one of the four banks as defined in Memory.h.
* OUTPUT:
*       None.
* RETURN:
*       32 bit size memory bank size or 0 for a closed or non populated bank.
*
*******************************************************************************/
unsigned int memoryGetBankSize (MEMORY_BANK bank)
{
	unsigned int sizeReg, size;
	MEMORY_WINDOW window;

	switch (bank) {
	case BANK0:
		sizeReg = SCS_0_HIGH_DECODE_ADDRESS;	/* CS_0_SIZE; */
		window = CS_0_WINDOW;
		break;
	case BANK1:
		sizeReg = SCS_1_HIGH_DECODE_ADDRESS;	/* CS_1_SIZE; */
		window = CS_1_WINDOW;
		break;
	case BANK2:
		sizeReg = SCS_2_HIGH_DECODE_ADDRESS;	/* CS_2_SIZE; */
		window = CS_2_WINDOW;
		break;
	case BANK3:
		sizeReg = SCS_3_HIGH_DECODE_ADDRESS;	/* CS_3_SIZE; */
		window = CS_3_WINDOW;
		break;
	default:
		return 0;
		break;
	}
	/* If the window is closed, a size of 0 is returned */
	if (MemoryGetMemWindowStatus (window) != MEM_WINDOW_ENABLED)
		return 0;
	GT_REG_READ (sizeReg, &size);
	size = ((size << 16) | 0xffff) + 1;
	return size;
}

/*******************************************************************************
* memoryGetDeviceSize - Returns the size of a device memory space.
* DESCRIPTION:
*       This function returns the memory space size of a given device.
* INPUT:
*       device - Selects one of the five devices as defined in Memory.h.
* OUTPUT:
*       None.
* RETURN:
*       32 bit size of a device memory space.
*******************************************************************************/
unsigned int memoryGetDeviceSize (DEVICE device)
{
	unsigned int sizeReg, size;
	MEMORY_WINDOW window;

	switch (device) {
	case DEVICE0:
		sizeReg = CS_0_HIGH_DECODE_ADDRESS;	/*DEV_CS0_SIZE; */
		window = DEVCS_0_WINDOW;
		break;
	case DEVICE1:
		sizeReg = CS_1_HIGH_DECODE_ADDRESS;	/*DEV_CS1_SIZE; */
		window = DEVCS_1_WINDOW;
		break;
	case DEVICE2:
		sizeReg = CS_2_HIGH_DECODE_ADDRESS;	/*DEV_CS2_SIZE; */
		window = DEVCS_2_WINDOW;
		break;
	case DEVICE3:
		sizeReg = CS_3_HIGH_DECODE_ADDRESS;	/*DEV_CS3_SIZE; */
		window = DEVCS_3_WINDOW;
		break;
	case BOOT_DEVICE:
		sizeReg = BOOTCS_HIGH_DECODE_ADDRESS;	/*BOOTCS_SIZE; */
		window = BOOT_CS_WINDOW;
		break;
	default:
		return 0;
		break;
	}
	/* If the window is closed, a size of 0 is returned */
	if (MemoryGetMemWindowStatus (window) != MEM_WINDOW_ENABLED)
		return 0;
	GT_REG_READ (sizeReg, &size);
	size = ((size << 16) | 0xffff) + 1;
	return size;
}

/*******************************************************************************
* MemoryGetPciWindowSize - Returns the size of a PCI memory window.
* DESCRIPTION:
*       This function returns the size of a PCI window.
* INPUT:
*       pciWindow - Selects one of the PCI memory windows as defined in
*       Memory.h.
* OUTPUT:
*       None.
* RETURN:
*       32 bit size of a PCI memory window.
*******************************************************************************/
unsigned int MemoryGetPciWindowSize (PCI_MEM_WINDOW pciWindow)
{
	unsigned int sizeReg, size;

	switch (pciWindow) {
	case PCI_0_IO:
		sizeReg = PCI_0I_O_HIGH_DECODE_ADDRESS;	/*PCI_0_IO_SIZE; */
		break;
	case PCI_0_MEM0:
		sizeReg = PCI_0MEMORY0_HIGH_DECODE_ADDRESS;	/*PCI_0_MEMORY0_SIZE; */
		break;
	case PCI_0_MEM1:
		sizeReg = PCI_0MEMORY1_HIGH_DECODE_ADDRESS;	/*PCI_0_MEMORY1_SIZE; */
		break;
	case PCI_0_MEM2:
		sizeReg = PCI_0MEMORY2_HIGH_DECODE_ADDRESS;	/*PCI_0_MEMORY2_SIZE; */
		break;
	case PCI_0_MEM3:
		sizeReg = PCI_0MEMORY3_HIGH_DECODE_ADDRESS;	/*PCI_0_MEMORY3_SIZE; */
		break;
#ifdef INCLUDE_PCI_1
	case PCI_1_IO:
		sizeReg = PCI_1I_O_HIGH_DECODE_ADDRESS;	/*PCI_1_IO_SIZE; */
		break;
	case PCI_1_MEM0:
		sizeReg = PCI_1MEMORY0_HIGH_DECODE_ADDRESS;	/*PCI_1_MEMORY0_SIZE; */
		break;
	case PCI_1_MEM1:
		sizeReg = PCI_1MEMORY1_HIGH_DECODE_ADDRESS;	/*PCI_1_MEMORY1_SIZE;  */
		break;
	case PCI_1_MEM2:
		sizeReg = PCI_1MEMORY2_HIGH_DECODE_ADDRESS;	/*PCI_1_MEMORY2_SIZE;  */
		break;
	case PCI_1_MEM3:
		sizeReg = PCI_1MEMORY3_HIGH_DECODE_ADDRESS;	/*PCI_1_MEMORY3_SIZE; */
		break;
#endif /* INCLUDE_PCI_1 */
	default:
		return 0x0;
	}
	/* If the memory window is disabled, retrun size = 0 */
	if (MemoryGetMemWindowStatus (PCI_0_IO_WINDOW << pciWindow)
	    == MEM_WINDOW_DISABLED)
		return 0;
	GT_REG_READ (sizeReg, &size);
	size = ((size << 16) | 0xffff) + 1;
	return size;
}

/*******************************************************************************
* memoryGetDeviceWidth - Returns the width of a given device.
* DESCRIPTION:
*       The MV's device interface supports up to 32 Bit wide devices. A device
*       can have a  1, 2, 4 or 8 Bytes data width. This function returns the
*       width of a device as defined by the user or the operating system.
* INPUT:
*       device - Selects one of the five devices as defined in Memory.h.
* OUTPUT:
*       None.
* RETURN:
*       Device width in Bytes (1,2,4 or 8), 0 if error had occurred.
*******************************************************************************/
unsigned int memoryGetDeviceWidth (DEVICE device)
{
	unsigned int width;
	unsigned int regValue;

	GT_REG_READ (DEVICE_BANK0PARAMETERS + device * 4, &regValue);
	width = (regValue & (BIT20 | BIT21)) >> 20;
	return (BIT0 << width);
}

/*******************************************************************************
* memoryMapBank - Set new base address and size for one of the memory
*                         banks.
*
* DESCRIPTION:
*       The CPU interface address decoding map consists of 21 address windows
*       for the different devices (e.g. CS[3:0] ,PCI0 Mem 0/1/2/3...). Each
*       window can have a minimum of 1Mbytes of address space, and up to 4Gbyte
*       space. Each address window is defined by two registers - base and size.
*       The CPU address is compared with the values in the various CPU windows
*       until a match is found and the address is than targeted to that window.
*       This function sets new base and size for one the memory banks
*       (CS0 - CS3). It is the programmer`s responsibility to make sure that
*       there are no conflicts with other memory spaces. When two memory spaces
*       overlap, the MV's behavior is not defined .If a bank needs to be closed,
*       set the 'bankLength' parameter size to 0x0.
*
* INPUT:
*       bank      - One of the memory banks (CS0-CS3) as defined in gtMemory.h.
*       bankBase  - The memory bank base address.
*       bankLength  - The memory bank size. This function will decrement the
*                   'bankLength' parameter by one and then check if the size is
*                   valid. A valid size must be programed from LSB to MSB as
*                   sequence of '1's followed by sequence of '0's.
*                   To close a memory window simply set the size to 0.
*      NOTE!!!
*       The size must be in 64Kbyte granularity.
*       The base address must be aligned to the size.
* OUTPUT:
*       None.
* RETURN:
*       False for invalid size, true otherwise.
*
* CAUTION: PCI_functions must be implemented later To_do !!!!!!!!!!!!!!!!!
*
*******************************************************************************/

bool memoryMapBank (MEMORY_BANK bank, unsigned int bankBase,
		    unsigned int bankLength)
{
	unsigned int newBase, newSize, baseReg, sizeReg, temp, rShift;

/*    PCI_INTERNAL_BAR pciBAR; */

	switch (bank) {
	case BANK0:
		baseReg = SCS_0_LOW_DECODE_ADDRESS;	/*CS_0_BASE_ADDR; */
		sizeReg = SCS_0_HIGH_DECODE_ADDRESS;	/*CS_0_SIZE; */
/*        pciBAR = PCI_CS0_BAR; */
		break;
	case BANK1:
		baseReg = SCS_1_LOW_DECODE_ADDRESS;	/*CS_1_BASE_ADDR; */
		sizeReg = SCS_1_HIGH_DECODE_ADDRESS;	/*CS_1_SIZE; */
		/*        pciBAR = SCS_0_HIGH_DECODE_ADDRESS; */ /*PCI_CS1_BAR; */
		break;
	case BANK2:
		baseReg = SCS_2_LOW_DECODE_ADDRESS;	/*CS_2_BASE_ADDR; */
		sizeReg = SCS_2_HIGH_DECODE_ADDRESS;	/*CS_2_SIZE; */
/*        pciBAR = PCI_CS2_BAR;*/
		break;
	case BANK3:
		baseReg = SCS_3_LOW_DECODE_ADDRESS;	/*CS_3_BASE_ADDR; */
		sizeReg = SCS_3_HIGH_DECODE_ADDRESS;	/*CS_3_SIZE; */
/*        pciBAR = PCI_CS3_BAR; */
		break;
	default:
		return false;
	}
	/* If the size is 0, the window will be disabled */
	if (bankLength == 0) {
		MemoryDisableWindow (CS_0_WINDOW << bank);
		/* Disable the BAR from the PCI slave side */
/*        gtPci0DisableInternalBAR(pciBAR); */
/*        gtPci1DisableInternalBAR(pciBAR); */
		return true;
	}
	/* The base address must be aligned to the size */
	if ((bankBase % bankLength) != 0) {
		return false;
	}
	if (bankLength >= MINIMUM_MEM_BANK_SIZE) {
		newBase = bankBase >> 16;
		newSize = bankLength >> 16;
		/* Checking that the size is a sequence of '1' followed by a
		   sequence of '0' starting from LSB to MSB. */
		temp = newSize - 1;
		for (rShift = 0; rShift < 16; rShift++) {
			temp = temp >> rShift;
			if ((temp & 0x1) == 0) {	/* Either we got to the last '1' */
							/* or the size is not valid	 */
				if (temp > 0x0)
					return false;
				else
					break;
			}
		}
#ifdef DEBUG
		{
			unsigned int oldBase, oldSize;

			GT_REG_READ (baseReg, &oldBase);
			GT_REG_READ (sizeReg + 8, &oldSize);

			printf ("b%d Base:%x Size:%x -> Base:%x Size:%x\n",
				bank, oldBase, oldSize, newBase, newSize);
		}
#endif
		/* writing the new values */
		GT_REG_WRITE (baseReg, newBase);
		GT_REG_WRITE (sizeReg, newSize - 1);
		/* Enable back the window */
		MemoryEnableWindow (CS_0_WINDOW << bank);
		/* Enable the BAR from the PCI slave side */
/*        gtPci0EnableInternalBAR(pciBAR); */
/*        gtPci1EnableInternalBAR(pciBAR); */
		return true;
	}
	return false;
}


/*******************************************************************************
* memoryMapDeviceSpace - Set new base address and size for one of the device
*                           windows.
*
* DESCRIPTION:
*       The CPU interface address decoding map consists of 21 address windows
*       for the different devices (e.g. CS[3:0] ,PCI0 Mem 0/1/2/3...). Each
*       window can have a minimum of 1Mbytes of address space, and up to 4Gbyte
*       space. Each address window is defined by two registers - base and size.
*       The CPU address is compared with the values in the various CPU windows
*       until a match is found and the address is than targeted to that window.
*       This function sets new base and size for one the device windows
*       (DEV_CS0 - DEV_CS3). It is the programmer`s responsibility to make sure
*       that there are no conflicts with other memory spaces. When two memory
*       spaces overlap, the MV's behavior is not defined .If a device window
*       needs to be closed, set the 'deviceLength' parameter size to 0x0.
*
* INPUT:
*       device           - One of the device windows (DEV_CS0-DEV_CS3) as
*                          defined in gtMemory.h.
*       deviceBase - The device window base address.
*       deviceLength - The device window size. This function will decrement
*                          the 'deviceLength' parameter by one and then
*                          check if the size is valid. A valid size must be
*                          programed from LSB to MSB as sequence of '1's
*                          followed by sequence of '0's.
*                          To close a memory window simply set the size to 0.
*
*      NOTE!!!
*       The size must be in 64Kbyte granularity.
*       The base address must be aligned to the size.
*
* OUTPUT:
*       None.
*
* RETURN:
*       False for invalid size, true otherwise.
*
* CAUTION: PCI_functions must be implemented later To_do !!!!!!!!!!!!!!!!!
*
*******************************************************************************/

bool memoryMapDeviceSpace (DEVICE device, unsigned int deviceBase,
			   unsigned int deviceLength)
{
	unsigned int newBase, newSize, baseReg, sizeReg, temp, rShift;

/*    PCI_INTERNAL_BAR pciBAR;*/

	switch (device) {
	case DEVICE0:
		baseReg = CS_0_LOW_DECODE_ADDRESS;	/*DEV_CS0_BASE_ADDR; */
		sizeReg = CS_0_HIGH_DECODE_ADDRESS;	/*DEV_CS0_SIZE; */
/*        pciBAR = PCI_DEV_CS0_BAR; */
		break;
	case DEVICE1:
		baseReg = CS_1_LOW_DECODE_ADDRESS;	/*DEV_CS1_BASE_ADDR; */
		sizeReg = CS_1_HIGH_DECODE_ADDRESS;	/*DEV_CS1_SIZE; */
/*        pciBAR = PCI_DEV_CS1_BAR; */
		break;
	case DEVICE2:
		baseReg = CS_2_LOW_DECODE_ADDRESS;	/*DEV_CS2_BASE_ADDR; */
		sizeReg = CS_2_HIGH_DECODE_ADDRESS;	/*DEV_CS2_SIZE; */
/*        pciBAR = PCI_DEV_CS2_BAR; */
		break;
	case DEVICE3:
		baseReg = CS_3_LOW_DECODE_ADDRESS;	/*DEV_CS3_BASE_ADDR; */
		sizeReg = CS_3_HIGH_DECODE_ADDRESS;	/*DEV_CS3_SIZE; */
/*        pciBAR = PCI_DEV_CS3_BAR; */
		break;
	case BOOT_DEVICE:
		baseReg = BOOTCS_LOW_DECODE_ADDRESS;	/*BOOTCS_BASE_ADDR; */
		sizeReg = BOOTCS_HIGH_DECODE_ADDRESS;	/*BOOTCS_SIZE; */
/*        pciBAR = PCI_BOOT_CS_BAR; */
		break;
	default:
		return false;
	}
	if (deviceLength == 0) {
		MemoryDisableWindow (DEVCS_0_WINDOW << device);
		/* Disable the BAR from the PCI slave side */
/*        gtPci0DisableInternalBAR(pciBAR); */
/*        gtPci1DisableInternalBAR(pciBAR); */
		return true;
	}
	/* The base address must be aligned to the size */
	if ((deviceBase % deviceLength) != 0) {
		return false;
	}
	if (deviceLength >= MINIMUM_DEVICE_WINDOW_SIZE) {
		newBase = deviceBase >> 16;
		newSize = deviceLength >> 16;
		/* Checking that the size is a sequence of '1' followed by a
		   sequence of '0' starting from LSB to MSB. */
		temp = newSize - 1;
		for (rShift = 0; rShift < 16; rShift++) {
			temp = temp >> rShift;
			if ((temp & 0x1) == 0) {	/* Either we got to the last '1' */
							/* or the size is not valid       */
				if (temp > 0x0)
					return false;
				else
					break;
			}
		}
		/* writing the new values */
		GT_REG_WRITE (baseReg, newBase);
		GT_REG_WRITE (sizeReg, newSize - 1);
		MemoryEnableWindow (DEVCS_0_WINDOW << device);
		/* Enable the BAR from the PCI slave side */
/*        gtPci0EnableInternalBAR(pciBAR); */
/*        gtPci1EnableInternalBAR(pciBAR); */
		return true;
	}
	return false;
}

/*******************************************************************************
* MemorySetPciWindow - Set new base address and size for one of the PCI
*                        windows.
*
* DESCRIPTION:
*       The CPU interface address decoding map consists of 21 address windows
*       for the different devices (e.g. CS[3:0] ,PCI0 Mem 0/1/2/3...). Each
*       window can have a minimum of 1Mbytes of address space, and up to 4Gbyte
*       space. Each address window is defined by two registers - base and size.
*       The CPU address is compared with the values in the various CPU windows
*       until a match is found and the address is than targeted to that window.
*       This function sets new base and size for one the PCI windows
*       (PCI memory0/1/2..). It is the programmer`s responsibility to make sure
*       that there are no conflicts with other memory spaces. When two memory
*       spaces overlap, the MV's behavior is not defined. If a PCI window
*       needs to be closed, set the 'pciWindowSize' parameter size to 0x0.
*
* INPUT:
*       pciWindow     - One of the PCI windows as defined in gtMemory.h.
*       pciWindowBase - The PCI window base address.
*       pciWindowSize - The PCI window size. This function will decrement the
*                       'pciWindowSize' parameter by one and then check if the
*                       size is valid. A valid size must be programed from LSB
*                       to MSB as sequence of '1's followed by sequence of '0's.
*                       To close a memory window simply set the size to 0.
*
*      NOTE!!!
*       The size must be in 64Kbyte granularity.
*       The base address must be aligned to the size.
*
* OUTPUT:
*       None.
*
* RETURN:
*       False for invalid size, true otherwise.
*
*******************************************************************************/
bool memorySetPciWindow (PCI_MEM_WINDOW pciWindow, unsigned int pciWindowBase,
			 unsigned int pciWindowSize)
{
	unsigned int currentLow, baseAddrReg, sizeReg, temp, rShift;

	switch (pciWindow) {
	case PCI_0_IO:
		baseAddrReg = PCI_1I_O_LOW_DECODE_ADDRESS;	/*PCI_0_IO_BASE_ADDR; */
		sizeReg = PCI_0I_O_HIGH_DECODE_ADDRESS;	/*PCI_0_IO_SIZE; */
		break;
	case PCI_0_MEM0:
		baseAddrReg = PCI_0MEMORY0_LOW_DECODE_ADDRESS;	/*PCI_0_MEMORY0_BASE_ADDR; */
		sizeReg = PCI_0MEMORY0_HIGH_DECODE_ADDRESS;	/*PCI_0_MEMORY0_SIZE; */
		break;
	case PCI_0_MEM1:
		baseAddrReg = PCI_0MEMORY1_LOW_DECODE_ADDRESS;	/*PCI_0_MEMORY1_BASE_ADDR; */
		sizeReg = PCI_0MEMORY1_HIGH_DECODE_ADDRESS;	/*PCI_0_MEMORY1_SIZE; */
		break;
	case PCI_0_MEM2:
		baseAddrReg = PCI_0MEMORY2_LOW_DECODE_ADDRESS;	/*PCI_0_MEMORY2_BASE_ADDR; */
		sizeReg = PCI_0MEMORY2_HIGH_DECODE_ADDRESS;	/*PCI_0_MEMORY2_SIZE; */
		break;
	case PCI_0_MEM3:
		baseAddrReg = PCI_0MEMORY3_LOW_DECODE_ADDRESS;	/*PCI_0_MEMORY3_BASE_ADDR; */
		sizeReg = PCI_0MEMORY3_HIGH_DECODE_ADDRESS;	/*PCI_0_MEMORY3_SIZE; */
		break;
#ifdef INCLUDE_PCI_1
	case PCI_1_IO:
		baseAddrReg = PCI_1I_O_LOW_DECODE_ADDRESS;	/*PCI_1_IO_BASE_ADDR; */
		sizeReg = PCI_1I_O_HIGH_DECODE_ADDRESS;	/*PCI_1_IO_SIZE; */
		break;
	case PCI_1_MEM0:
		baseAddrReg = PCI_1MEMORY0_LOW_DECODE_ADDRESS;	/*PCI_1_MEMORY0_BASE_ADDR; */
		sizeReg = PCI_1MEMORY0_HIGH_DECODE_ADDRESS;	/*PCI_1_MEMORY0_SIZE; */
		break;
	case PCI_1_MEM1:
		baseAddrReg = PCI_1MEMORY1_LOW_DECODE_ADDRESS;	/*PCI_1_MEMORY1_BASE_ADDR;  */
		sizeReg = PCI_1MEMORY1_HIGH_DECODE_ADDRESS;	/*PCI_1_MEMORY1_SIZE; */
		break;
	case PCI_1_MEM2:
		baseAddrReg = PCI_1MEMORY2_LOW_DECODE_ADDRESS;	/*PCI_1_MEMORY2_BASE_ADDR;  */
		sizeReg = PCI_1MEMORY2_HIGH_DECODE_ADDRESS;	/*PCI_1_MEMORY2_SIZE; */
		break;
	case PCI_1_MEM3:
		baseAddrReg = PCI_1MEMORY3_LOW_DECODE_ADDRESS;	/*PCI_1_MEMORY3_BASE_ADDR; */
		sizeReg = PCI_1MEMORY3_HIGH_DECODE_ADDRESS;	/*PCI_1_MEMORY3_SIZE; */
		break;
#endif /* INCLUDE_PCI_1 */
	default:
		return false;
	}
	if (pciWindowSize == 0) {
		MemoryDisableWindow (PCI_0_IO_WINDOW << pciWindow);
		return true;
	}
	/* The base address must be aligned to the size */
	if ((pciWindowBase % pciWindowSize) != 0) {
		return false;
	}
	if (pciWindowSize >= MINIMUM_PCI_WINDOW_SIZE) {
		pciWindowBase >>= 16;
		pciWindowSize >>= 16;
		/* Checking that the size is a sequence of '1' followed by a
		   sequence of '0' starting from LSB to MSB. */
		temp = pciWindowSize - 1;
		for (rShift = 0; rShift < 16; rShift++) {
			temp = temp >> rShift;
			if ((temp & 0x1) == 0) {	/* Either we got to the last '1' */
							/* or the size is not valid       */
				if (temp > 0x0)
					return false;
				else
					break;
			}
		}
		GT_REG_WRITE (sizeReg, pciWindowSize - 1);
		GT_REG_READ (baseAddrReg, &currentLow);
		pciWindowBase =
			(pciWindowBase & 0xfffff) | (currentLow & 0xfff00000);
		GT_REG_WRITE (baseAddrReg, pciWindowBase);
		MemoryEnableWindow (PCI_0_IO_WINDOW << pciWindow);
		return true;
	}
	return false;
}

/*******************************************************************************
* memoryMapInternalRegistersSpace - Sets new base address for the internal
*                                  registers memory space.
*
* DESCRIPTION:
*       This function set new base address for the internal registers memory
*       space (the size is fixed and cannot be modified). The function does not
*       handle overlapping with other memory spaces, it is the programer's
*       responsibility to ensure that overlapping does not occur.
*       When two memory spaces overlap, the MV's behavior is not defined.
*
* INPUT:
*       internalRegBase - new base address for the internal registers memory
*                         space.
*
* OUTPUT:
*       None.
*
* RETURN:
*        true on success, false on failure
*
*******************************************************************************/
/********************************************************************
* memoryMapInternalRegistersSpace - Sets new base address for the internals
*                                   registers.
*
* INPUTS:  unsigned int internalRegBase - The new base address.
* RETURNS: true on success, false on failure
*********************************************************************/
bool memoryMapInternalRegistersSpace (unsigned int internalRegBase)
{
	unsigned int currentValue;
	unsigned int internalValue = internalRegBase;

	internalRegBase = (internalRegBase >> 16);
	GT_REG_READ (INTERNAL_SPACE_DECODE, &currentValue);
	internalRegBase = (currentValue & 0xff000000) | internalRegBase;
	GT_REG_WRITE (INTERNAL_SPACE_DECODE, internalRegBase);
	/* initializing also the global variable 'internalRegBaseAddr' */
/*    gtInternalRegBaseAddr = internalValue; */
	INTERNAL_REG_BASE_ADDR = internalValue;
	return true;
}

/*******************************************************************************
* memoryGetInternalRegistersSpace - Returns the internal registers Base
*                                     address.
*
* DESCRIPTION:
*       This function returns the base address of  the internal registers
*       memory space .
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32 bit base address of the internal registers memory space.
*
*******************************************************************************/
unsigned int memoryGetInternalRegistersSpace (void)
{
	unsigned int currentValue = 0;

	GT_REG_READ (INTERNAL_SPACE_DECODE, &currentValue);
	return ((currentValue & 0x000fffff) << 16);
}

/*******************************************************************************
* gtMemoryGetInternalSramBaseAddr - Returns the integrated SRAM base address.
*
* DESCRIPTION:
*       The Atlantis incorporate integrated 2Mbit SRAM for general use. This
*       funcnion return the SRAM's base address.
* INPUT:
*       None.
* OUTPUT:
*       None.
* RETURN:
*       32 bit SRAM's base address.
*
*******************************************************************************/
unsigned int memoryGetInternalSramBaseAddr (void)
{
	return ((GTREGREAD (INTEGRATED_SRAM_BASE_ADDR) & 0xfffff) << 16);
}

/*******************************************************************************
* gtMemorySetInternalSramBaseAddr - Set the integrated SRAM base address.
*
* DESCRIPTION:
*       The Atlantis incorporate integrated 2Mbit SRAM for general use. This
*       function sets a new base address to the SRAM .
* INPUT:
*       sramBaseAddress - The SRAM's base address.
* OUTPUT:
*       None.
* RETURN:
*       None.
*
*******************************************************************************/
void gtMemorySetInternalSramBaseAddr (unsigned int sramBaseAddress)
{
	GT_REG_WRITE (INTEGRATED_SRAM_BASE_ADDR, sramBaseAddress >> 16);
}

/*******************************************************************************
* memorySetProtectRegion - Set protection mode for one of the 8 regions.
*
* DESCRIPTION:
*       The CPU interface supports configurable access protection. This includes
*       up to eight address ranges defined to a different protection type :
*       whether the address range is cacheable or not, whether it is writable or
*       not , and whether it is accessible or not. A Low and High registers
*       define each window while the minimum address range of each window is
*       1Mbyte. An address driven by the CPU, in addition to the address
*       decoding and remapping process, is compared against the eight Access
*       Protection Low/High registers , if an address matches one of the windows
*       , the MV device checks the transaction type against the protection bits
*       defined in CPU Access Protection register, to determine if the access is
*       allowed. This function set a protection mode to one of the 8 possible
*       regions.
*      NOTE:
*       The CPU address windows are restricted to a size of  2 power n and the
*       start address must be aligned to the window size. For example, if using
*       a 16 MB window, the start address bits [23:0] must be 0.The MV's
*       internal registers space is not protected, even if the access protection
*       windows contain this space.
*
* INPUT:
*       region - selects which region to be configured. The values defined in
*                gtMemory.h:
*
*                 - MEM_REGION0
*                 - MEM_REGION1
*                 - etc.
*
*       memAccess - Allows or forbids access (read or write ) to the region. The
*                   values defined in gtMemory.h:
*
*                    - MEM_ACCESS_ALLOWED
*                    - MEM_ACCESS_FORBIDEN
*
*       memWrite - CPU write protection to the region. The values defined in
*                  gtMemory.h:
*
*                   - MEM_WRITE_ALLOWED
*                   - MEM_WRITE_FORBIDEN
*
*       cacheProtection - Defines whether caching the region is allowed or not.
*                         The values defined in gtMemory.h:
*
*                          - MEM_CACHE_ALLOWED
*                          - MEM_CACHE_FORBIDEN
*
*       baseAddress - the region's base Address.
*       regionSize  - The region's size. This function will decrement the
*                     'regionSize' parameter by one and then check if the size
*                     is valid. A valid size must be programed from LSB to MSB
*                     as sequence of '1's followed by sequence of '0's.
*                     To close a memory window simply set the size to 0.
*
*      NOTE!!!
*       The size must be in 64Kbyte granularity.
*       The base address must be aligned to the size.
*
* OUTPUT:
*       None.
*
* RETURN:
*       False for invalid size, true otherwise.
*
*******************************************************************************/
bool memorySetProtectRegion (MEMORY_PROTECT_WINDOW window,
			     MEMORY_ACCESS memAccess,
			     MEMORY_ACCESS_WRITE memWrite,
			     MEMORY_CACHE_PROTECT cacheProtection,
			     unsigned int baseAddress, unsigned int size)
{
	unsigned int dataForReg, temp, rShift;

	if (size == 0) {
		GT_REG_WRITE ((CPU_PROTECT_WINDOW_0_SIZE + 0x10 * window),
			      0x0);
		return true;
	}
	/* The base address must be aligned to the size.  */
	if (baseAddress % size != 0) {
		return false;
	}
	if (size >= MINIMUM_ACCESS_WIN_SIZE) {
		baseAddress = ((baseAddress >> 16) & 0xfffff);
		dataForReg = baseAddress | ((memAccess << 20) & BIT20) |
			((memWrite << 21) & BIT21) | ((cacheProtection << 22)
						      & BIT22) | BIT31;
		GT_REG_WRITE (CPU_PROTECT_WINDOW_0_BASE_ADDR + 0x10 * window,
			      dataForReg);
		size >>= 16;
		/* Checking that the size is a sequence of '1' followed by a
		   sequence of '0' starting from LSB to MSB. */
		temp = size - 1;
		for (rShift = 0; rShift < 16; rShift++) {
			temp = temp >> rShift;
			if ((temp & 0x1) == 0) {	/* Either we got to the last '1' */
							/* or the size is not valid       */
				if (temp > 0x0)
					return false;
				else
					break;
			}
		}
		GT_REG_WRITE ((CPU_PROTECT_WINDOW_0_SIZE + 0x10 * window),
			      size - 1);
		return true;
	}
	return false;
}

/*******************************************************************************
* gtMemoryDisableProtectRegion - Disable a protected window.
*
* DESCRIPTION:
*       This function disable a protected window set by
*       'gtMemorySetProtectRegion' function.
*
* INPUT:
*       window - one of the 4 windows ( defined in gtMemory.h ).
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
void memoryDisableProtectRegion (MEMORY_PROTECT_WINDOW window)
{
	RESET_REG_BITS (((CPU_PROTECT_WINDOW_0_BASE_ADDR) + (0x10 * window)),
			BIT31);
}

/*******************************************************************************
* memorySetPciRemapValue - Set a remap value to a PCI memory space target.
*
* DESCRIPTION:
*       In addition to the address decoding mechanism, the CPU has an address
*       remapping mechanism to be used by every PCI decoding window. Each PCI
*       window can be remaped to a desired address target according to the remap
*       value within the remap register. The address remapping is useful when a
*       CPU address range must be reallocated to a different location on the
*       PCI bus. Also, it enables CPU access to a PCI agent located above the
*       4Gbyte space. On system boot, each of the PCI memory spaces is maped to
*       a defualt value (see CPU interface section in the MV spec for the
*       default values). The remap mechanism does not always produce the desired
*       address on the PCI bus because of the remap mechanism way of working
*       (to fully understand why, please see the 'Address Remapping' section in
*       the MV's spec). Therefor, this function sets a desired remap value to
*       one of the PCI memory windows and return the effective address that
*       should be used when exiting the PCI memory window. You should ALWAYS use
*       the returned value by this function when remapping a PCI window and
*       exiting it. If for example the base address of PCI0 memory 0 is
*       0x90000000, the size is 0x03ffffff and the remap value is 0x11000000,
*       the function will return the value of 0x91000000 that MUST
*       be used to exit this memory window in order to achive the deisred
*       remapping.
*
* INPUT:
*       memoryWindow   - One of the PCI memory windows as defined in Memory.h
*       remapValueLow  - The low remap value.
*       remapValueHigh - The high remap value.
* OUTPUT:
*       None.
*
* RETURN:
*       The effective base address to exit the PCI, or 0xffffffff if one of the
*       parameters is erroneous or the effective base address is higher the top
*       decode value.
*
*******************************************************************************/
unsigned int memorySetPciRemapValue (PCI_MEM_WINDOW memoryWindow,
				     unsigned int remapValueHigh,
				     unsigned int remapValueLow)
{
	unsigned int pciMemWindowBaseAddrReg = 0, baseAddrValue = 0;
	unsigned int pciMemWindowSizeReg = 0, windowSizeValue = 0;
	unsigned int effectiveBaseAddress, remapRegLow, remapRegHigh;

	/* Initializing the base and size variables of the PCI
	   memory windows */
	switch (memoryWindow) {
	case PCI_0_IO:
		pciMemWindowBaseAddrReg = PCI_0_IO_BASE_ADDR;
		pciMemWindowSizeReg = PCI_0_IO_SIZE;
		remapRegLow = PCI_0_IO_ADDR_REMAP;
		remapRegHigh = PCI_0_IO_ADDR_REMAP;
		break;
	case PCI_0_MEM0:
		pciMemWindowBaseAddrReg = PCI_0_MEMORY0_BASE_ADDR;
		pciMemWindowSizeReg = PCI_0_MEMORY0_SIZE;
		remapRegLow = PCI_0_MEMORY0_LOW_ADDR_REMAP;
		remapRegHigh = PCI_0_MEMORY0_HIGH_ADDR_REMAP;
		break;
	case PCI_0_MEM1:
		pciMemWindowBaseAddrReg = PCI_0_MEMORY1_BASE_ADDR;
		pciMemWindowSizeReg = PCI_0_MEMORY1_SIZE;
		remapRegLow = PCI_0_MEMORY1_LOW_ADDR_REMAP;
		remapRegHigh = PCI_0_MEMORY1_HIGH_ADDR_REMAP;
		break;
	case PCI_0_MEM2:
		pciMemWindowBaseAddrReg = PCI_0_MEMORY2_BASE_ADDR;
		pciMemWindowSizeReg = PCI_0_MEMORY2_SIZE;
		remapRegLow = PCI_0_MEMORY2_LOW_ADDR_REMAP;
		remapRegHigh = PCI_0_MEMORY2_HIGH_ADDR_REMAP;
		break;
	case PCI_0_MEM3:
		pciMemWindowBaseAddrReg = PCI_0_MEMORY3_BASE_ADDR;
		pciMemWindowSizeReg = PCI_0_MEMORY3_SIZE;
		remapRegLow = PCI_0_MEMORY3_LOW_ADDR_REMAP;
		remapRegHigh = PCI_0_MEMORY3_HIGH_ADDR_REMAP;
		break;
#ifdef INCLUDE_PCI_1
	case PCI_1_IO:
		pciMemWindowBaseAddrReg = PCI_1_IO_BASE_ADDR;
		pciMemWindowSizeReg = PCI_1_IO_SIZE;
		remapRegLow = PCI_1_IO_ADDR_REMAP;
		remapRegHigh = PCI_1_IO_ADDR_REMAP;
		break;
	case PCI_1_MEM0:
		pciMemWindowBaseAddrReg = PCI_1_MEMORY0_BASE_ADDR;
		pciMemWindowSizeReg = PCI_1_MEMORY0_SIZE;
		remapRegLow = PCI_1_MEMORY0_LOW_ADDR_REMAP;
		remapRegHigh = PCI_1_MEMORY0_HIGH_ADDR_REMAP;
		break;
	case PCI_1_MEM1:
		pciMemWindowBaseAddrReg = PCI_1_MEMORY1_BASE_ADDR;
		pciMemWindowSizeReg = PCI_1_MEMORY1_SIZE;
		remapRegLow = PCI_1_MEMORY1_LOW_ADDR_REMAP;
		remapRegHigh = PCI_1_MEMORY1_HIGH_ADDR_REMAP;
		break;
	case PCI_1_MEM2:
		pciMemWindowBaseAddrReg = PCI_1_MEMORY1_BASE_ADDR;
		pciMemWindowSizeReg = PCI_1_MEMORY1_SIZE;
		remapRegLow = PCI_1_MEMORY1_LOW_ADDR_REMAP;
		remapRegHigh = PCI_1_MEMORY1_HIGH_ADDR_REMAP;
		break;
	case PCI_1_MEM3:
		pciMemWindowBaseAddrReg = PCI_1_MEMORY3_BASE_ADDR;
		pciMemWindowSizeReg = PCI_1_MEMORY3_SIZE;
		remapRegLow = PCI_1_MEMORY3_LOW_ADDR_REMAP;
		remapRegHigh = PCI_1_MEMORY3_HIGH_ADDR_REMAP;
		break;
#endif /* INCLUDE_PCI_1 */
	default:
		/* Retrun an invalid effective base address */
		return 0xffffffff;
	}
	/* Writing the remap value to the remap regisers */
	GT_REG_WRITE (remapRegHigh, remapValueHigh);
	GT_REG_WRITE (remapRegLow, remapValueLow >> 16);
	/* Reading the values from the base address and size registers */
	baseAddrValue = GTREGREAD (pciMemWindowBaseAddrReg) & 0xfffff;
	windowSizeValue = GTREGREAD (pciMemWindowSizeReg) & 0xffff;
	/* Start calculating the effective Base Address */
	effectiveBaseAddress = baseAddrValue << 16;
	/* The effective base address will be combined from the chopped (if any)
	   remap value (according to the size value and remap mechanism) and the
	   window's base address */
	effectiveBaseAddress |=
		(((windowSizeValue << 16) | 0xffff) & remapValueLow);
	/* If the effectiveBaseAddress exceed the window boundaries return an
	   invalid value. */
	if (effectiveBaseAddress >
	    ((baseAddrValue << 16) + ((windowSizeValue << 16) | 0xffff)))
		return 0xffffffff;
	return effectiveBaseAddress;
}

/********************************************************************
* memorySetRegionSnoopMode - This function modifys one of the 4 regions which
*                            supports Cache Coherency.
*
*
* Inputs: SNOOP_REGION region - One of the four regions.
*         SNOOP_TYPE snoopType - There is four optional Types:
*                               1. No Snoop.
*                               2. Snoop to WT region.
*                               3. Snoop to WB region.
*                               4. Snoop & Invalidate to WB region.
*         unsigned int baseAddress - Base Address of this region.
*         unsigned int topAddress - Top Address of this region.
* Returns: false if one of the parameters is wrong and true else
*********************************************************************/
/* evb6260 code */
#if 0
bool memorySetRegionSnoopMode(MEMORY_SNOOP_REGION region,
			      MEMORY_SNOOP_TYPE snoopType,
			      unsigned int baseAddress,
			      unsigned int regionLength)
{
    unsigned int snoopXbaseAddress;
    unsigned int snoopXtopAddress;
    unsigned int data;
    unsigned int snoopHigh = baseAddress + regionLength;

    if( (region > MEM_SNOOP_REGION3) || (snoopType > MEM_SNOOP_WB) )
	return false;
    snoopXbaseAddress = SNOOP_BASE_ADDRESS_0 + 0x10 * region;
    snoopXtopAddress = SNOOP_TOP_ADDRESS_0 + 0x10 * region;
				 if(regionLength == 0) /* closing the region */
    {
	GT_REG_WRITE(snoopXbaseAddress,0x0000ffff);
	GT_REG_WRITE(snoopXtopAddress,0);
	return true;
    }
    baseAddress = baseAddress & 0xffff0000;
    data = (baseAddress >> 16) | snoopType << 16;
    GT_REG_WRITE(snoopXbaseAddress,data);
    snoopHigh = (snoopHigh & 0xfff00000) >> 20;
    GT_REG_WRITE(snoopXtopAddress,snoopHigh - 1);
    return true;
}
#endif

/********************************************************************
* memoryRemapAddress - This fubction used for address remapping.
*
*
* Inputs: regOffset: remap register
*         remapValue :
* Returns: false if one of the parameters is erroneous,true otherwise.
*
* Not needed function To_do !!!!
*********************************************************************/
bool memoryRemapAddress (unsigned int remapReg, unsigned int remapValue)
{
	unsigned int valueForReg;

	valueForReg = (remapValue & 0xfff00000) >> 20;
	GT_REG_WRITE (remapReg, valueForReg);
	return true;
}

/*******************************************************************************
* memoryGetDeviceParam - Extract the device parameters from the device bank
*                          parameters register.
*
* DESCRIPTION:
*       To allow interfacing with very slow devices and fast synchronous SRAMs,
*       each device can be programed to different timing parameters. Each bank
*       has its own parameters register. Bank width can be programmed to 8, 16,
*       or 32-bits. Bank timing parameters can be programmed to support
*       different device types (e.g. Sync Burst SRAM, Flash , ROM, I/O
*       Controllers). The MV allows you to set timing parameters and width for
*       each device through parameters register .
*       This function extracts the parameters described from the Device Bank
*       parameters register and fills the given 'deviceParam' (defined in
*       gtMemory.h) structure with the read data.
*
* INPUT:
*       deviceParam -  pointer to a structure DEVICE_PARAM (defined in
*                      Memory.h).For details about each structure field please
*                      see the device timing parameter section in the MV
*                      datasheet.
*       deviceNum  -   Select on of the five device banks (defined in
*                      Memory.h) :
*
*                       - DEVICE0
*                       - DEVICE1
*                       - DEVICE2
*                       - etc.
*
* OUTPUT:
*       None.
*
* RETURN:
*       false if one of the parameters is erroneous,true otherwise.
*
*******************************************************************************/
/********************************************************************
* memoryGetDeviceParam - This function used for getting device parameters from
*                        DEVICE BANK PARAMETERS REGISTER
*
*
* Inputs:        - deviceParam: STRUCT with paramiters for DEVICE BANK
*                  PARAMETERS REGISTER
*                - deviceNum : number of device
* Returns: false if one of the parameters is erroneous,true otherwise.
*********************************************************************/

bool memoryGetDeviceParam (DEVICE_PARAM * deviceParam, DEVICE deviceNum)
{
	unsigned int valueOfReg;
	unsigned int calcData;

	if (deviceNum > 4)
		return false;
	GT_REG_READ (DEVICE_BANK0PARAMETERS + 4 * deviceNum, &valueOfReg);
	calcData = (0x7 & valueOfReg) + ((BIT22 & valueOfReg) >> 19);
	deviceParam->turnOff = calcData;	/* Turn Off */

	calcData = ((0x78 & valueOfReg) >> 3) + ((BIT23 & valueOfReg) >> 19);
	deviceParam->acc2First = calcData;	/* Access To First */

	calcData = ((0x780 & valueOfReg) >> 7) + ((BIT24 & valueOfReg) >> 20);
	deviceParam->acc2Next = calcData;	/* Access To Next */

	calcData =
		((0x3800 & valueOfReg) >> 11) + ((BIT25 & valueOfReg) >> 22);
	deviceParam->ale2Wr = calcData;	/* Ale To Write */

	calcData = ((0x1c000 & valueOfReg) >> 14) +
		((BIT26 & valueOfReg) >> 23);
	deviceParam->wrLow = calcData;	/* Write Active */

	calcData = ((0xe0000 & valueOfReg) >> 17) +
		((BIT27 & valueOfReg) >> 24);
	deviceParam->wrHigh = calcData;	/* Write High */

	calcData = ((0x300000 & valueOfReg) >> 20);
	deviceParam->deviceWidth = (BIT0 << calcData);	/* In bytes */
	calcData = ((0x30000000 & valueOfReg) >> 28);
	deviceParam->badrSkew = calcData;	/* Cycles gap between BAdr
						   toggle to read data sample. */
	calcData = ((0x40000000 & valueOfReg) >> 30);
	deviceParam->DPEn = calcData;	/*  Data Parity enable  */
	return true;
}

/*******************************************************************************
* memorySetDeviceParam - Set new parameters for a device.
*
*
* DESCRIPTION:
*       To allow interfacing with very slow devices and fast synchronous SRAMs,
*       each device can be programed to different timing parameters. Each bank
*       has its own parameters register. Bank width can be programmed to 8, 16,
*       or 32-bits. Bank timing parameters can be programmed to support
*       different device types (e.g. Sync Burst SRAM, Flash , ROM, I/O
*       Controllers). The MV allows you to set timing parameters and width for
*       each device through parameters register. This function set new
*       parameters to a device Bank from the delivered structure 'deviceParam'
*       (defined in gtMemory.h). The structure must be initialized with data
*       prior to the use of these function.
*
* INPUT:
*       deviceParam -  pointer to a structure DEVICE_PARAM (defined in
*                      Memory.h).For details about each structure field please
*                      see the device timing parameter section in the MV
*                      datasheet.
*       deviceNum  -   Select on of the five device banks (defined in
*                      Memory.h) :
*
*                       - DEVICE0
*                       - DEVICE1
*                       - DEVICE2
*                       - etc.
*
* OUTPUT:
*       None.
*
* RETURN:
*       false if one of the parameters is erroneous,true otherwise.
*
*******************************************************************************/
/********************************************************************
* memorySetDeviceParam - This function used for setting device parameters to
*                        DEVICE BANK PARAMETERS REGISTER
*
*
* Inputs:        - deviceParam: STRUCT for store paramiters from DEVICE BANK
*                  PARAMETERS REGISTER
*                - deviceNum : number of device
* Returns: false if one of the parameters is erroneous,true otherwise.
*********************************************************************/
bool memorySetDeviceParam (DEVICE_PARAM * deviceParam, DEVICE deviceNum)
{
	unsigned int valueForReg;

	if ((deviceParam->turnOff > 0x7) || (deviceParam->acc2First > 0xf) ||
	    (deviceParam->acc2Next > 0xf) || (deviceParam->ale2Wr > 0x7) ||
	    (deviceParam->wrLow > 0x7) || (deviceParam->wrHigh > 0x7) ||
	    (deviceParam->badrSkew > 0x2) || (deviceParam->DPEn > 0x1)) {
		return false;
	}
	valueForReg = (((deviceParam->turnOff) & 0x7) |
		       (((deviceParam->turnOff) & 0x8) << 19) |
		       (((deviceParam->acc2First) & 0xf) << 3) |
		       (((deviceParam->acc2First) & 0x10) << 19) |
		       (((deviceParam->acc2Next) & 0xf) << 7) |
		       (((deviceParam->acc2Next) & 0x10) << 20) |
		       (((deviceParam->ale2Wr) & 0x7) << 11) |
		       (((deviceParam->ale2Wr) & 0xf) << 22) |
		       (((deviceParam->wrLow) & 0x7) << 14) |
		       (((deviceParam->wrLow) & 0xf) << 23) |
		       (((deviceParam->wrHigh) & 0x7) << 17) |
		       (((deviceParam->wrHigh) & 0xf) << 24) |
		       (((deviceParam->badrSkew) & 0x3) << 28) |
		       (((deviceParam->DPEn) & 0x1) << 30));

	/* insert the device width: */
	switch (deviceParam->deviceWidth) {
	case 1:
		valueForReg = valueForReg | _8BIT;
		break;
	case 2:
		valueForReg = valueForReg | _16BIT;
		break;
	case 4:
		valueForReg = valueForReg | _32BIT;
		break;
	default:
		valueForReg = valueForReg | _8BIT;
		break;
	}
	GT_REG_WRITE (DEVICE_BANK0PARAMETERS + 4 * deviceNum, valueForReg);
	return true;
}

/*******************************************************************************
* MemoryDisableWindow - Disable a memory space by the disable bit.
* DESCRIPTION:
*       This function disables one of the 21 availiable windows dedicated for
*       the CPU decoding mechanism. Its possible to combine several windows with
*       the OR command.
* INPUT:
*       window - One or more of the memory windows (defined in gtMemory.h).
* OUTPUT:
*       None.
* RETURN:
*       None.
*******************************************************************************/
void MemoryDisableWindow (MEMORY_WINDOW window)
{
	SET_REG_BITS (BASE_ADDR_ENABLE, window);
}

/*******************************************************************************
* MemoryEnableWindow - Enable a memory space that was disabled by
*                       'MemoryDisableWindow'.
* DESCRIPTION:
*       This function enables one of the 21 availiable windows dedicated for the
*       CPU decoding mechanism. Its possible to combine several windows with the
*       OR command.
* INPUT:
*       window - One or more of the memory windows (defined in gtMemory.h).
* OUTPUT:
*       None.
* RETURN:
*       None.
*******************************************************************************/
void MemoryEnableWindow (MEMORY_WINDOW window)
{
	RESET_REG_BITS (BASE_ADDR_ENABLE, window);
}

/*******************************************************************************
* MemoryGetMemWindowStatus - This function check whether the memory window is
*                              disabled or not.
* DESCRIPTION:
*       This function checks if the given memory window is closed .
* INPUT:
*       window - One or more of the memory windows (defined in gtMemory.h).
* OUTPUT:
*       None.
* RETURN:
*       True for a closed window, false otherwise .
*******************************************************************************/
MEMORY_WINDOW_STATUS MemoryGetMemWindowStatus (MEMORY_WINDOW window)
{
	if (GTREGREAD (BASE_ADDR_ENABLE) & window)
		return MEM_WINDOW_DISABLED;
	return MEM_WINDOW_ENABLED;
}
