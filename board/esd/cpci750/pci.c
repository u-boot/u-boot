/*
 * (C) Copyright 2000
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
/* PCI.c - PCI functions */


#include <common.h>
#ifdef CONFIG_PCI
#include <pci.h>

#ifdef CONFIG_PCI_PNP
void pciauto_config_init(struct pci_controller *hose);
int  pciauto_region_allocate(struct pci_region* res, unsigned int size, unsigned int *bar);
#endif

#include "../../Marvell/include/pci.h"

#undef DEBUG
#undef IDE_SET_NATIVE_MODE
static unsigned int local_buses[] = { 0, 0 };

static const unsigned char pci_irq_swizzle[2][PCI_MAX_DEVICES] = {
	{0, 0, 0, 0, 0, 0, 0, 27, 27, [9 ... PCI_MAX_DEVICES - 1] = 0 },
	{0, 0, 0, 0, 0, 0, 0, 29, 29, [9 ... PCI_MAX_DEVICES - 1] = 0 },
};

#ifdef CONFIG_USE_CPCIDVI
typedef struct {
	unsigned int base;
	unsigned int init;
} GT_CPCIDVI_ROM_T;

static GT_CPCIDVI_ROM_T gt_cpcidvi_rom = {0, 0};
#endif

#ifdef DEBUG
static const unsigned int pci_bus_list[] = { PCI_0_MODE, PCI_1_MODE };
static void gt_pci_bus_mode_display (PCI_HOST host)
{
	unsigned int mode;


	mode = (GTREGREAD (pci_bus_list[host]) & (BIT4 | BIT5)) >> 4;
	switch (mode) {
	case 0:
		printf ("PCI %d bus mode: Conventional PCI\n", host);
		break;
	case 1:
		printf ("PCI %d bus mode: 66 MHz PCIX\n", host);
		break;
	case 2:
		printf ("PCI %d bus mode: 100 MHz PCIX\n", host);
		break;
	case 3:
		printf ("PCI %d bus mode: 133 MHz PCIX\n", host);
		break;
	default:
		printf ("Unknown BUS %d\n", mode);
	}
}
#endif

static const unsigned int pci_p2p_configuration_reg[] = {
	PCI_0P2P_CONFIGURATION, PCI_1P2P_CONFIGURATION
};

static const unsigned int pci_configuration_address[] = {
	PCI_0CONFIGURATION_ADDRESS, PCI_1CONFIGURATION_ADDRESS
};

static const unsigned int pci_configuration_data[] = {
	PCI_0CONFIGURATION_DATA_VIRTUAL_REGISTER,
	PCI_1CONFIGURATION_DATA_VIRTUAL_REGISTER
};

static const unsigned int pci_error_cause_reg[] = {
	PCI_0ERROR_CAUSE, PCI_1ERROR_CAUSE
};

static const unsigned int pci_arbiter_control[] = {
	PCI_0ARBITER_CONTROL, PCI_1ARBITER_CONTROL
};

static const unsigned int pci_address_space_en[] = {
	PCI_0_BASE_ADDR_REG_ENABLE, PCI_1_BASE_ADDR_REG_ENABLE
};

static const unsigned int pci_snoop_control_base_0_low[] = {
	PCI_0SNOOP_CONTROL_BASE_0_LOW, PCI_1SNOOP_CONTROL_BASE_0_LOW
};
static const unsigned int pci_snoop_control_top_0[] = {
	PCI_0SNOOP_CONTROL_TOP_0, PCI_1SNOOP_CONTROL_TOP_0
};

static const unsigned int pci_access_control_base_0_low[] = {
	PCI_0ACCESS_CONTROL_BASE_0_LOW, PCI_1ACCESS_CONTROL_BASE_0_LOW
};
static const unsigned int pci_access_control_top_0[] = {
	PCI_0ACCESS_CONTROL_TOP_0, PCI_1ACCESS_CONTROL_TOP_0
};

static const unsigned int pci_scs_bank_size[2][4] = {
	{PCI_0SCS_0_BANK_SIZE, PCI_0SCS_1_BANK_SIZE,
	 PCI_0SCS_2_BANK_SIZE, PCI_0SCS_3_BANK_SIZE},
	{PCI_1SCS_0_BANK_SIZE, PCI_1SCS_1_BANK_SIZE,
	 PCI_1SCS_2_BANK_SIZE, PCI_1SCS_3_BANK_SIZE}
};

static const unsigned int pci_p2p_configuration[] = {
	PCI_0P2P_CONFIGURATION, PCI_1P2P_CONFIGURATION
};


/********************************************************************
* pciWriteConfigReg - Write to a PCI configuration register
*		    - Make sure the GT is configured as a master before writing
*		      to another device on the PCI.
*		    - The function takes care of Big/Little endian conversion.
*
*
* Inputs:   unsigned int regOffset: The register offset as it apears in the GT spec
*		   (or any other PCI device spec)
*	    pciDevNum: The device number needs to be addressed.
*
*  Configuration Address 0xCF8:
*
*	31 30	 24 23	16 15  11 10	 8 7	  2  0	   <=bit Number
*  |congif|Reserved|  Bus |Device|Function|Register|00|
*  |Enable|	   |Number|Number| Number | Number |  |	   <=field Name
*
*********************************************************************/
void pciWriteConfigReg (PCI_HOST host, unsigned int regOffset,
			unsigned int pciDevNum, unsigned int data)
{
	volatile unsigned int DataForAddrReg;
	unsigned int functionNum;
	unsigned int busNum = 0;
	unsigned int addr;

	if (pciDevNum > 32)	/* illegal device Number */
		return;
	if (pciDevNum == SELF) {	/* configure our configuration space. */
		pciDevNum =
			(GTREGREAD (pci_p2p_configuration_reg[host]) >> 24) &
			0x1f;
		busNum = GTREGREAD (pci_p2p_configuration_reg[host]) &
			0xff0000;
	}
	functionNum = regOffset & 0x00000700;
	pciDevNum = pciDevNum << 11;
	regOffset = regOffset & 0xfc;
	DataForAddrReg =
		(regOffset | pciDevNum | functionNum | busNum) | BIT31;
	GT_REG_WRITE (pci_configuration_address[host], DataForAddrReg);
	GT_REG_READ (pci_configuration_address[host], &addr);
	if (addr != DataForAddrReg)
		return;
	GT_REG_WRITE (pci_configuration_data[host], data);
}

/********************************************************************
* pciReadConfigReg  - Read from a PCI0 configuration register
*		    - Make sure the GT is configured as a master before reading
*		      from another device on the PCI.
*		    - The function takes care of Big/Little endian conversion.
* INPUTS:   regOffset: The register offset as it apears in the GT spec (or PCI
*			spec)
*	    pciDevNum: The device number needs to be addressed.
* RETURNS: data , if the data == 0xffffffff check the master abort bit in the
*		  cause register to make sure the data is valid
*
*  Configuration Address 0xCF8:
*
*	31 30	 24 23	16 15  11 10	 8 7	  2  0	   <=bit Number
*  |congif|Reserved|  Bus |Device|Function|Register|00|
*  |Enable|	   |Number|Number| Number | Number |  |	   <=field Name
*
*********************************************************************/
unsigned int pciReadConfigReg (PCI_HOST host, unsigned int regOffset,
			       unsigned int pciDevNum)
{
	volatile unsigned int DataForAddrReg;
	unsigned int data;
	unsigned int functionNum;
	unsigned int busNum = 0;

	if (pciDevNum > 32)	/* illegal device Number */
		return 0xffffffff;
	if (pciDevNum == SELF) {	/* configure our configuration space. */
		pciDevNum =
			(GTREGREAD (pci_p2p_configuration_reg[host]) >> 24) &
			0x1f;
		busNum = GTREGREAD (pci_p2p_configuration_reg[host]) &
			0xff0000;
	}
	functionNum = regOffset & 0x00000700;
	pciDevNum = pciDevNum << 11;
	regOffset = regOffset & 0xfc;
	DataForAddrReg =
		(regOffset | pciDevNum | functionNum | busNum) | BIT31;
	GT_REG_WRITE (pci_configuration_address[host], DataForAddrReg);
	GT_REG_READ (pci_configuration_address[host], &data);
	if (data != DataForAddrReg)
		return 0xffffffff;
	GT_REG_READ (pci_configuration_data[host], &data);
	return data;
}

/********************************************************************
* pciOverBridgeWriteConfigReg - Write to a PCI configuration register where
*				the agent is placed on another Bus. For more
*				information read P2P in the PCI spec.
*
* Inputs:   unsigned int regOffset - The register offset as it apears in the
*	    GT spec (or any other PCI device spec).
*	    unsigned int pciDevNum - The device number needs to be addressed.
*	    unsigned int busNum - On which bus does the Target agent connect
*				  to.
*	    unsigned int data - data to be written.
*
*  Configuration Address 0xCF8:
*
*	31 30	 24 23	16 15  11 10	 8 7	  2  0	   <=bit Number
*  |congif|Reserved|  Bus |Device|Function|Register|01|
*  |Enable|	   |Number|Number| Number | Number |  |	   <=field Name
*
*  The configuration Address is configure as type-I (bits[1:0] = '01') due to
*   PCI spec referring to P2P.
*
*********************************************************************/
void pciOverBridgeWriteConfigReg (PCI_HOST host,
				  unsigned int regOffset,
				  unsigned int pciDevNum,
				  unsigned int busNum, unsigned int data)
{
	unsigned int DataForReg;
	unsigned int functionNum;

	functionNum = regOffset & 0x00000700;
	pciDevNum = pciDevNum << 11;
	regOffset = regOffset & 0xff;
	busNum = busNum << 16;
	if (pciDevNum == SELF) {	/* This board */
		DataForReg = (regOffset | pciDevNum | functionNum) | BIT0;
	} else {
		DataForReg = (regOffset | pciDevNum | functionNum | busNum) |
			BIT31 | BIT0;
	}
	GT_REG_WRITE (pci_configuration_address[host], DataForReg);
	GT_REG_WRITE (pci_configuration_data[host], data);
}


/********************************************************************
* pciOverBridgeReadConfigReg  - Read from a PCIn configuration register where
*				the agent target locate on another PCI bus.
*			      - Make sure the GT is configured as a master
*				before reading from another device on the PCI.
*			      - The function takes care of Big/Little endian
*				conversion.
* INPUTS:   regOffset: The register offset as it apears in the GT spec (or PCI
*			 spec). (configuration register offset.)
*	    pciDevNum: The device number needs to be addressed.
*	    busNum: the Bus number where the agent is place.
* RETURNS: data , if the data == 0xffffffff check the master abort bit in the
*		  cause register to make sure the data is valid
*
*  Configuration Address 0xCF8:
*
*	31 30	 24 23	16 15  11 10	 8 7	  2  0	   <=bit Number
*  |congif|Reserved|  Bus |Device|Function|Register|01|
*  |Enable|	   |Number|Number| Number | Number |  |	   <=field Name
*
*********************************************************************/
unsigned int pciOverBridgeReadConfigReg (PCI_HOST host,
					 unsigned int regOffset,
					 unsigned int pciDevNum,
					 unsigned int busNum)
{
	unsigned int DataForReg;
	unsigned int data;
	unsigned int functionNum;

	functionNum = regOffset & 0x00000700;
	pciDevNum = pciDevNum << 11;
	regOffset = regOffset & 0xff;
	busNum = busNum << 16;
	if (pciDevNum == SELF) {	/* This board */
		DataForReg = (regOffset | pciDevNum | functionNum) | BIT31;
	} else {		/* agent on another bus */

		DataForReg = (regOffset | pciDevNum | functionNum | busNum) |
			BIT0 | BIT31;
	}
	GT_REG_WRITE (pci_configuration_address[host], DataForReg);
	GT_REG_READ (pci_configuration_data[host], &data);
	return data;
}


/********************************************************************
* pciGetRegOffset - Gets the register offset for this region config.
*
* INPUT:   Bus, Region - The bus and region we ask for its base address.
* OUTPUT:   N/A
* RETURNS: PCI register base address
*********************************************************************/
static unsigned int pciGetRegOffset (PCI_HOST host, PCI_REGION region)
{
	switch (host) {
	case PCI_HOST0:
		switch (region) {
		case PCI_IO:
			return PCI_0I_O_LOW_DECODE_ADDRESS;
		case PCI_REGION0:
			return PCI_0MEMORY0_LOW_DECODE_ADDRESS;
		case PCI_REGION1:
			return PCI_0MEMORY1_LOW_DECODE_ADDRESS;
		case PCI_REGION2:
			return PCI_0MEMORY2_LOW_DECODE_ADDRESS;
		case PCI_REGION3:
			return PCI_0MEMORY3_LOW_DECODE_ADDRESS;
		}
	case PCI_HOST1:
		switch (region) {
		case PCI_IO:
			return PCI_1I_O_LOW_DECODE_ADDRESS;
		case PCI_REGION0:
			return PCI_1MEMORY0_LOW_DECODE_ADDRESS;
		case PCI_REGION1:
			return PCI_1MEMORY1_LOW_DECODE_ADDRESS;
		case PCI_REGION2:
			return PCI_1MEMORY2_LOW_DECODE_ADDRESS;
		case PCI_REGION3:
			return PCI_1MEMORY3_LOW_DECODE_ADDRESS;
		}
	}
	return PCI_0MEMORY0_LOW_DECODE_ADDRESS;
}

static unsigned int pciGetRemapOffset (PCI_HOST host, PCI_REGION region)
{
	switch (host) {
	case PCI_HOST0:
		switch (region) {
		case PCI_IO:
			return PCI_0I_O_ADDRESS_REMAP;
		case PCI_REGION0:
			return PCI_0MEMORY0_ADDRESS_REMAP;
		case PCI_REGION1:
			return PCI_0MEMORY1_ADDRESS_REMAP;
		case PCI_REGION2:
			return PCI_0MEMORY2_ADDRESS_REMAP;
		case PCI_REGION3:
			return PCI_0MEMORY3_ADDRESS_REMAP;
		}
	case PCI_HOST1:
		switch (region) {
		case PCI_IO:
			return PCI_1I_O_ADDRESS_REMAP;
		case PCI_REGION0:
			return PCI_1MEMORY0_ADDRESS_REMAP;
		case PCI_REGION1:
			return PCI_1MEMORY1_ADDRESS_REMAP;
		case PCI_REGION2:
			return PCI_1MEMORY2_ADDRESS_REMAP;
		case PCI_REGION3:
			return PCI_1MEMORY3_ADDRESS_REMAP;
		}
	}
	return PCI_0MEMORY0_ADDRESS_REMAP;
}

/********************************************************************
* pciGetBaseAddress - Gets the base address of a PCI.
*	    - If the PCI size is 0 then this base address has no meaning!!!
*
*
* INPUT:   Bus, Region - The bus and region we ask for its base address.
* OUTPUT:   N/A
* RETURNS: PCI base address.
*********************************************************************/
unsigned int pciGetBaseAddress (PCI_HOST host, PCI_REGION region)
{
	unsigned int regBase;
	unsigned int regEnd;
	unsigned int regOffset = pciGetRegOffset (host, region);

	GT_REG_READ (regOffset, &regBase);
	GT_REG_READ (regOffset + 8, &regEnd);

	if (regEnd <= regBase)
		return 0xffffffff;	/* ERROR !!! */

	regBase = regBase << 16;
	return regBase;
}

bool pciMapSpace (PCI_HOST host, PCI_REGION region, unsigned int remapBase,
		  unsigned int bankBase, unsigned int bankLength)
{
	unsigned int low = 0xfff;
	unsigned int high = 0x0;
	unsigned int regOffset = pciGetRegOffset (host, region);
	unsigned int remapOffset = pciGetRemapOffset (host, region);

	if (bankLength != 0) {
		low = (bankBase >> 16) & 0xffff;
		high = ((bankBase + bankLength) >> 16) - 1;
	}

	GT_REG_WRITE (regOffset, low | (1 << 24));	/* no swapping */
	GT_REG_WRITE (regOffset + 8, high);

	if (bankLength != 0) {	/* must do AFTER writing maps */
		GT_REG_WRITE (remapOffset, remapBase >> 16);	/* sorry, 32 bits only.
								   dont support upper 32
								   in this driver */
	}
	return true;
}

unsigned int pciGetSpaceBase (PCI_HOST host, PCI_REGION region)
{
	unsigned int low;
	unsigned int regOffset = pciGetRegOffset (host, region);

	GT_REG_READ (regOffset, &low);
	return (low & 0xffff) << 16;
}

unsigned int pciGetSpaceSize (PCI_HOST host, PCI_REGION region)
{
	unsigned int low, high;
	unsigned int regOffset = pciGetRegOffset (host, region);

	GT_REG_READ (regOffset, &low);
	GT_REG_READ (regOffset + 8, &high);
	return ((high & 0xffff) + 1) << 16;
}


/* ronen - 7/Dec/03*/
/********************************************************************
* gtPciDisable/EnableInternalBAR - This function enable/disable PCI BARS.
* Inputs: one of the PCI BAR
*********************************************************************/
void gtPciEnableInternalBAR (PCI_HOST host, PCI_INTERNAL_BAR pciBAR)
{
	RESET_REG_BITS (pci_address_space_en[host], BIT0 << pciBAR);
}

void gtPciDisableInternalBAR (PCI_HOST host, PCI_INTERNAL_BAR pciBAR)
{
	SET_REG_BITS (pci_address_space_en[host], BIT0 << pciBAR);
}

/********************************************************************
* pciMapMemoryBank - Maps PCI_host memory bank "bank" for the slave.
*
* Inputs: base and size of PCI SCS
*********************************************************************/
void pciMapMemoryBank (PCI_HOST host, MEMORY_BANK bank,
		       unsigned int pciDramBase, unsigned int pciDramSize)
{
	/*ronen different function for 3rd bank. */
	unsigned int offset = (bank < 2) ? bank * 8 : 0x100 + (bank - 2) * 8;

	pciDramBase = pciDramBase & 0xfffff000;
	pciDramBase = pciDramBase | (pciReadConfigReg (host,
						       PCI_SCS_0_BASE_ADDRESS
						       + offset,
						       SELF) & 0x00000fff);
	pciWriteConfigReg (host, PCI_SCS_0_BASE_ADDRESS + offset, SELF,
			   pciDramBase);
	if (pciDramSize == 0)
		pciDramSize++;
	GT_REG_WRITE (pci_scs_bank_size[host][bank], pciDramSize - 1);
	gtPciEnableInternalBAR (host, bank);
}

/********************************************************************
* pciSetRegionFeatures - This function modifys one of the 8 regions with
*			  feature bits given as an input.
*			- Be advised to check the spec before modifying them.
* Inputs: PCI_PROTECT_REGION region - one of the eight regions.
*	  unsigned int features - See file: pci.h there are defintion for those
*				  region features.
*	  unsigned int baseAddress - The region base Address.
*	  unsigned int topAddress - The region top Address.
* Returns: false if one of the parameters is erroneous true otherwise.
*********************************************************************/
bool pciSetRegionFeatures (PCI_HOST host, PCI_ACCESS_REGIONS region,
			   unsigned int features, unsigned int baseAddress,
			   unsigned int regionLength)
{
	unsigned int accessLow;
	unsigned int accessHigh;
	unsigned int accessTop = baseAddress + regionLength;

	if (regionLength == 0) {	/* close the region. */
		pciDisableAccessRegion (host, region);
		return true;
	}
	/* base Address is store is bits [11:0] */
	accessLow = (baseAddress & 0xfff00000) >> 20;
	/* All the features are update according to the defines in pci.h (to be on
	   the safe side we disable bits: [11:0] */
	accessLow = accessLow | (features & 0xfffff000);
	/* write to the Low Access Region register */
	GT_REG_WRITE (pci_access_control_base_0_low[host] + 0x10 * region,
		      accessLow);

	accessHigh = (accessTop & 0xfff00000) >> 20;

	/* write to the High Access Region register */
	GT_REG_WRITE (pci_access_control_top_0[host] + 0x10 * region,
		      accessHigh - 1);
	return true;
}

/********************************************************************
* pciDisableAccessRegion - Disable The given Region by writing MAX size
*			    to its low Address and MIN size to its high Address.
*
* Inputs:   PCI_ACCESS_REGIONS region - The region we to be Disabled.
* Returns:  N/A.
*********************************************************************/
void pciDisableAccessRegion (PCI_HOST host, PCI_ACCESS_REGIONS region)
{
	/* writing back the registers default values. */
	GT_REG_WRITE (pci_access_control_base_0_low[host] + 0x10 * region,
		      0x01001fff);
	GT_REG_WRITE (pci_access_control_top_0[host] + 0x10 * region, 0);
}

/********************************************************************
* pciArbiterEnable - Enables PCI-0`s Arbitration mechanism.
*
* Inputs:   N/A
* Returns:  true.
*********************************************************************/
bool pciArbiterEnable (PCI_HOST host)
{
	unsigned int regData;

	GT_REG_READ (pci_arbiter_control[host], &regData);
	GT_REG_WRITE (pci_arbiter_control[host], regData | BIT31);
	return true;
}

/********************************************************************
* pciArbiterDisable - Disable PCI-0`s Arbitration mechanism.
*
* Inputs:   N/A
* Returns:  true
*********************************************************************/
bool pciArbiterDisable (PCI_HOST host)
{
	unsigned int regData;

	GT_REG_READ (pci_arbiter_control[host], &regData);
	GT_REG_WRITE (pci_arbiter_control[host], regData & 0x7fffffff);
	return true;
}

/********************************************************************
* pciSetArbiterAgentsPriority - Priority setup for the PCI agents (Hi or Low)
*
* Inputs:   PCI_AGENT_PRIO internalAgent - priotity for internal agent.
*	    PCI_AGENT_PRIO externalAgent0 - priotity for external#0 agent.
*	    PCI_AGENT_PRIO externalAgent1 - priotity for external#1 agent.
*	    PCI_AGENT_PRIO externalAgent2 - priotity for external#2 agent.
*	    PCI_AGENT_PRIO externalAgent3 - priotity for external#3 agent.
*	    PCI_AGENT_PRIO externalAgent4 - priotity for external#4 agent.
*	    PCI_AGENT_PRIO externalAgent5 - priotity for external#5 agent.
* Returns:  true
*********************************************************************/
bool pciSetArbiterAgentsPriority (PCI_HOST host, PCI_AGENT_PRIO internalAgent,
				  PCI_AGENT_PRIO externalAgent0,
				  PCI_AGENT_PRIO externalAgent1,
				  PCI_AGENT_PRIO externalAgent2,
				  PCI_AGENT_PRIO externalAgent3,
				  PCI_AGENT_PRIO externalAgent4,
				  PCI_AGENT_PRIO externalAgent5)
{
	unsigned int regData;
	unsigned int writeData;

	GT_REG_READ (pci_arbiter_control[host], &regData);
	writeData = (internalAgent << 7) + (externalAgent0 << 8) +
		(externalAgent1 << 9) + (externalAgent2 << 10) +
		(externalAgent3 << 11) + (externalAgent4 << 12) +
		(externalAgent5 << 13);
	regData = (regData & 0xffffc07f) | writeData;
	GT_REG_WRITE (pci_arbiter_control[host], regData & regData);
	return true;
}

/********************************************************************
* pciParkingDisable - Park on last option disable, with this function you can
*		       disable the park on last mechanism for each agent.
*		       disabling this option for all agents results parking
*		       on the internal master.
*
* Inputs: PCI_AGENT_PARK internalAgent -  parking Disable for internal agent.
*	  PCI_AGENT_PARK externalAgent0 - parking Disable for external#0 agent.
*	  PCI_AGENT_PARK externalAgent1 - parking Disable for external#1 agent.
*	  PCI_AGENT_PARK externalAgent2 - parking Disable for external#2 agent.
*	  PCI_AGENT_PARK externalAgent3 - parking Disable for external#3 agent.
*	  PCI_AGENT_PARK externalAgent4 - parking Disable for external#4 agent.
*	  PCI_AGENT_PARK externalAgent5 - parking Disable for external#5 agent.
* Returns:  true
*********************************************************************/
bool pciParkingDisable (PCI_HOST host, PCI_AGENT_PARK internalAgent,
			PCI_AGENT_PARK externalAgent0,
			PCI_AGENT_PARK externalAgent1,
			PCI_AGENT_PARK externalAgent2,
			PCI_AGENT_PARK externalAgent3,
			PCI_AGENT_PARK externalAgent4,
			PCI_AGENT_PARK externalAgent5)
{
	unsigned int regData;
	unsigned int writeData;

	GT_REG_READ (pci_arbiter_control[host], &regData);
	writeData = (internalAgent << 14) + (externalAgent0 << 15) +
		(externalAgent1 << 16) + (externalAgent2 << 17) +
		(externalAgent3 << 18) + (externalAgent4 << 19) +
		(externalAgent5 << 20);
	regData = (regData & ~(0x7f << 14)) | writeData;
	GT_REG_WRITE (pci_arbiter_control[host], regData);
	return true;
}

/********************************************************************
* pciEnableBrokenAgentDetection - A master is said to be broken if it fails to
*			respond to grant assertion within a window specified in
*			the input value: 'brokenValue'.
*
* Inputs: unsigned char brokenValue -  A value which limits the Master to hold the
*			grant without asserting frame.
* Returns:  Error for illegal broken value otherwise true.
*********************************************************************/
bool pciEnableBrokenAgentDetection (PCI_HOST host, unsigned char brokenValue)
{
	unsigned int data;
	unsigned int regData;

	if (brokenValue > 0xf)
		return false;	/* brokenValue must be 4 bit */
	data = brokenValue << 3;
	GT_REG_READ (pci_arbiter_control[host], &regData);
	regData = (regData & 0xffffff87) | data;
	GT_REG_WRITE (pci_arbiter_control[host], regData | BIT1);
	return true;
}

/********************************************************************
* pciDisableBrokenAgentDetection - This function disable the Broken agent
*			    Detection mechanism.
*			    NOTE: This operation may cause a dead lock on the
*			    pci0 arbitration.
*
* Inputs:   N/A
* Returns:  true.
*********************************************************************/
bool pciDisableBrokenAgentDetection (PCI_HOST host)
{
	unsigned int regData;

	GT_REG_READ (pci_arbiter_control[host], &regData);
	regData = regData & 0xfffffffd;
	GT_REG_WRITE (pci_arbiter_control[host], regData);
	return true;
}

/********************************************************************
* pciP2PConfig - This function set the PCI_n P2P configurate.
*		  For more information on the P2P read PCI spec.
*
* Inputs:  unsigned int SecondBusLow - Secondery PCI interface Bus Range Lower
*				       Boundry.
*	   unsigned int SecondBusHigh - Secondry PCI interface Bus Range upper
*				       Boundry.
*	   unsigned int busNum - The CPI bus number to which the PCI interface
*				       is connected.
*	   unsigned int devNum - The PCI interface's device number.
*
* Returns:  true.
*********************************************************************/
bool pciP2PConfig (PCI_HOST host, unsigned int SecondBusLow,
		   unsigned int SecondBusHigh,
		   unsigned int busNum, unsigned int devNum)
{
	unsigned int regData;

	regData = (SecondBusLow & 0xff) | ((SecondBusHigh & 0xff) << 8) |
		((busNum & 0xff) << 16) | ((devNum & 0x1f) << 24);
	GT_REG_WRITE (pci_p2p_configuration[host], regData);
	return true;
}

/********************************************************************
* pciSetRegionSnoopMode - This function modifys one of the 4 regions which
*			   supports Cache Coherency in the PCI_n interface.
* Inputs: region - One of the four regions.
*	  snoopType - There is four optional Types:
*			 1. No Snoop.
*			 2. Snoop to WT region.
*			 3. Snoop to WB region.
*			 4. Snoop & Invalidate to WB region.
*	  baseAddress - Base Address of this region.
*	  regionLength - Region length.
* Returns: false if one of the parameters is wrong otherwise return true.
*********************************************************************/
bool pciSetRegionSnoopMode (PCI_HOST host, PCI_SNOOP_REGION region,
			    PCI_SNOOP_TYPE snoopType,
			    unsigned int baseAddress,
			    unsigned int regionLength)
{
	unsigned int snoopXbaseAddress;
	unsigned int snoopXtopAddress;
	unsigned int data;
	unsigned int snoopHigh = baseAddress + regionLength;

	if ((region > PCI_SNOOP_REGION3) || (snoopType > PCI_SNOOP_WB))
		return false;
	snoopXbaseAddress =
		pci_snoop_control_base_0_low[host] + 0x10 * region;
	snoopXtopAddress = pci_snoop_control_top_0[host] + 0x10 * region;
	if (regionLength == 0) {	/* closing the region */
		GT_REG_WRITE (snoopXbaseAddress, 0x0000ffff);
		GT_REG_WRITE (snoopXtopAddress, 0);
		return true;
	}
	baseAddress = baseAddress & 0xfff00000; /* Granularity of 1MByte */
	data = (baseAddress >> 20) | snoopType << 12;
	GT_REG_WRITE (snoopXbaseAddress, data);
	snoopHigh = (snoopHigh & 0xfff00000) >> 20;
	GT_REG_WRITE (snoopXtopAddress, snoopHigh - 1);
	return true;
}

static int gt_read_config_dword (struct pci_controller *hose,
				 pci_dev_t dev, int offset, u32 * value)
{
	int bus = PCI_BUS (dev);

	if ((bus == local_buses[0]) || (bus == local_buses[1])) {
	        *value = pciReadConfigReg ((PCI_HOST) hose->cfg_addr,
					   offset | (PCI_FUNC(dev) << 8),
					   PCI_DEV (dev));
	} else {
		*value = pciOverBridgeReadConfigReg ((PCI_HOST) hose->cfg_addr,
						     offset | (PCI_FUNC(dev) << 8),
						     PCI_DEV (dev), bus);
	}

	return 0;
}

static int gt_write_config_dword (struct pci_controller *hose,
				  pci_dev_t dev, int offset, u32 value)
{
	int bus = PCI_BUS (dev);

	if ((bus == local_buses[0]) || (bus == local_buses[1])) {
		pciWriteConfigReg ((PCI_HOST) hose->cfg_addr,
				   offset | (PCI_FUNC(dev) << 8),
				   PCI_DEV (dev), value);
	} else {
		pciOverBridgeWriteConfigReg ((PCI_HOST) hose->cfg_addr,
					     offset | (PCI_FUNC(dev) << 8),
					     PCI_DEV (dev), bus,
					     value);
	}
	return 0;
}


static void gt_setup_ide (struct pci_controller *hose,
			  pci_dev_t dev, struct pci_config_table *entry)
{
	static const int ide_bar[] = { 8, 4, 8, 4, 0, 0 };
	u32 bar_response, bar_value;
	int bar;

	if (CPCI750_SLAVE_TEST != 0)
		return;

	for (bar = 0; bar < 6; bar++) {
		/*ronen different function for 3rd bank. */
		unsigned int offset =
			(bar < 2) ? bar * 8 : 0x100 + (bar - 2) * 8;

		pci_hose_write_config_dword (hose, dev, PCI_BASE_ADDRESS_0 + offset,
					     0x0);
		pci_hose_read_config_dword (hose, dev, PCI_BASE_ADDRESS_0 + offset,
					    &bar_response);

		pciauto_region_allocate (bar_response &
					 PCI_BASE_ADDRESS_SPACE_IO ? hose->
					 pci_io : hose->pci_mem, ide_bar[bar],
					 &bar_value);

		pci_hose_write_config_dword (hose, dev, PCI_BASE_ADDRESS_0 + bar * 4,
					     bar_value);
	}
}

#ifdef CONFIG_USE_CPCIDVI
static void gt_setup_cpcidvi (struct pci_controller *hose,
			      pci_dev_t dev, struct pci_config_table *entry)
{
	u32		  bar_value, pci_response;

	if (CPCI750_SLAVE_TEST != 0)
		return;

	pci_hose_read_config_dword (hose, dev, PCI_COMMAND, &pci_response);
	pci_hose_write_config_dword (hose, dev, PCI_BASE_ADDRESS_0, 0xffffffff);
	pci_hose_read_config_dword (hose, dev, PCI_BASE_ADDRESS_0, &pci_response);
	pciauto_region_allocate (hose->pci_mem, 0x01000000, &bar_value);
	pci_hose_write_config_dword (hose, dev, PCI_BASE_ADDRESS_0, (bar_value & 0xffffff00));
	pci_hose_write_config_dword (hose, dev, PCI_ROM_ADDRESS, 0x0);
	pciauto_region_allocate (hose->pci_mem, 0x40000, &bar_value);
	pci_hose_write_config_dword (hose, dev, PCI_ROM_ADDRESS, (bar_value & 0xffffff00) | 0x01);
	gt_cpcidvi_rom.base = bar_value & 0xffffff00;
	gt_cpcidvi_rom.init = 1;
}

unsigned char gt_cpcidvi_in8(unsigned int offset)
{
	unsigned char	  data;

	if (gt_cpcidvi_rom.init == 0) {
		return(0);
		}
	data = in8((offset & 0x04) + 0x3f000 + gt_cpcidvi_rom.base);
	return(data);
}

void gt_cpcidvi_out8(unsigned int offset, unsigned char data)
{
	unsigned int	  off;

	if (gt_cpcidvi_rom.init == 0) {
		return;
	}
	off = data;
	off = ((off << 3) & 0x7f8) + (offset & 0x4) + 0x3e000 + gt_cpcidvi_rom.base;
	in8(off);
	return;
}
#endif

/* TODO BJW: Change this for DB64360. This was pulled from the EV64260	*/
/* and is curently not called *. */
#if 0
static void gt_fixup_irq (struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char pin, irq;

	pci_read_config_byte (dev, PCI_INTERRUPT_PIN, &pin);

	if (pin == 1) {		/* only allow INT A */
		irq = pci_irq_swizzle[(PCI_HOST) hose->
				      cfg_addr][PCI_DEV (dev)];
		if (irq)
			pci_write_config_byte (dev, PCI_INTERRUPT_LINE, irq);
	}
}
#endif

struct pci_config_table gt_config_table[] = {
#ifdef CONFIG_USE_CPCIDVI
	{PCI_VENDOR_ID_CT, PCI_DEVICE_ID_CT_69030, PCI_CLASS_DISPLAY_VGA,
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, gt_setup_cpcidvi},
#endif
	{PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE,
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, gt_setup_ide},
	{}
};

struct pci_controller pci0_hose = {
/*    fixup_irq: gt_fixup_irq, */
	config_table:gt_config_table,
};

struct pci_controller pci1_hose = {
/*    fixup_irq: gt_fixup_irq, */
	config_table:gt_config_table,
};

void pci_init_board (void)
{
	unsigned int command;
	unsigned int slave;
#ifdef CONFIG_PCI_PNP
	unsigned int bar;
#endif
#ifdef DEBUG
	gt_pci_bus_mode_display (PCI_HOST0);
#endif
#ifdef CONFIG_USE_CPCIDVI
	gt_cpcidvi_rom.init = 0;
	gt_cpcidvi_rom.base = 0;
#endif

	slave = CPCI750_SLAVE_TEST;

	pci0_hose.config_table = gt_config_table;
	pci1_hose.config_table = gt_config_table;

#ifdef CONFIG_USE_CPCIDVI
	gt_config_table[0].config_device =  gt_setup_cpcidvi;
#endif
	gt_config_table[1].config_device =  gt_setup_ide;

	pci0_hose.first_busno = 0;
	pci0_hose.last_busno = 0xff;
	local_buses[0] = pci0_hose.first_busno;

	/* PCI memory space */
	pci_set_region (pci0_hose.regions + 0,
			CONFIG_SYS_PCI0_0_MEM_SPACE,
			CONFIG_SYS_PCI0_0_MEM_SPACE,
			CONFIG_SYS_PCI0_MEM_SIZE, PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region (pci0_hose.regions + 1,
			CONFIG_SYS_PCI0_IO_SPACE_PCI,
			CONFIG_SYS_PCI0_IO_SPACE, CONFIG_SYS_PCI0_IO_SIZE, PCI_REGION_IO);

	pci_set_ops (&pci0_hose,
		     pci_hose_read_config_byte_via_dword,
		     pci_hose_read_config_word_via_dword,
		     gt_read_config_dword,
		     pci_hose_write_config_byte_via_dword,
		     pci_hose_write_config_word_via_dword,
		     gt_write_config_dword);
	pci0_hose.region_count = 2;

	pci0_hose.cfg_addr = (unsigned int *) PCI_HOST0;

	pci_register_hose (&pci0_hose);
	if (slave == 0) {
		pciArbiterEnable (PCI_HOST0);
		pciParkingDisable (PCI_HOST0, 1, 1, 1, 1, 1, 1, 1);
		command = pciReadConfigReg (PCI_HOST0, PCI_COMMAND, SELF);
		command |= PCI_COMMAND_MASTER;
		pciWriteConfigReg (PCI_HOST0, PCI_COMMAND, SELF, command);
		command = pciReadConfigReg (PCI_HOST0, PCI_COMMAND, SELF);
		command |= PCI_COMMAND_MEMORY;
		pciWriteConfigReg (PCI_HOST0, PCI_COMMAND, SELF, command);

#ifdef CONFIG_PCI_PNP
		pciauto_config_init(&pci0_hose);
		pciauto_region_allocate(pci0_hose.pci_io, 0x400, &bar);
#endif
#ifdef CONFIG_PCI_SCAN_SHOW
		printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif
		pci0_hose.last_busno = pci_hose_scan_bus (&pci0_hose,
							  pci0_hose.first_busno);

#ifdef DEBUG
		gt_pci_bus_mode_display (PCI_HOST1);
#endif
	} else {
		pciArbiterDisable (PCI_HOST0);
		pciParkingDisable (PCI_HOST0, 1, 1, 1, 1, 1, 1, 1);
		command = pciReadConfigReg (PCI_HOST0, PCI_COMMAND, SELF);
		command |= PCI_COMMAND_MASTER;
		pciWriteConfigReg (PCI_HOST0, PCI_COMMAND, SELF, command);
		command = pciReadConfigReg (PCI_HOST0, PCI_COMMAND, SELF);
		command |= PCI_COMMAND_MEMORY;
		pciWriteConfigReg (PCI_HOST0, PCI_COMMAND, SELF, command);
		pci0_hose.last_busno = pci0_hose.first_busno;
	}
	pci1_hose.first_busno = pci0_hose.last_busno + 1;
	pci1_hose.last_busno = 0xff;
	pci1_hose.current_busno = pci1_hose.first_busno;
	local_buses[1] = pci1_hose.first_busno;

	/* PCI memory space */
	pci_set_region (pci1_hose.regions + 0,
			CONFIG_SYS_PCI1_0_MEM_SPACE,
			CONFIG_SYS_PCI1_0_MEM_SPACE,
			CONFIG_SYS_PCI1_MEM_SIZE, PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region (pci1_hose.regions + 1,
			CONFIG_SYS_PCI1_IO_SPACE_PCI,
			CONFIG_SYS_PCI1_IO_SPACE, CONFIG_SYS_PCI1_IO_SIZE, PCI_REGION_IO);

	pci_set_ops (&pci1_hose,
		     pci_hose_read_config_byte_via_dword,
		     pci_hose_read_config_word_via_dword,
		     gt_read_config_dword,
		     pci_hose_write_config_byte_via_dword,
		     pci_hose_write_config_word_via_dword,
		     gt_write_config_dword);

	pci1_hose.region_count = 2;

	pci1_hose.cfg_addr = (unsigned int *) PCI_HOST1;

	pci_register_hose (&pci1_hose);

	pciArbiterEnable (PCI_HOST1);
	pciParkingDisable (PCI_HOST1, 1, 1, 1, 1, 1, 1, 1);

	command = pciReadConfigReg (PCI_HOST1, PCI_COMMAND, SELF);
	command |= PCI_COMMAND_MASTER;
	pciWriteConfigReg (PCI_HOST1, PCI_COMMAND, SELF, command);

#ifdef CONFIG_PCI_PNP
	pciauto_config_init(&pci1_hose);
	pciauto_region_allocate(pci1_hose.pci_io, 0x400, &bar);
#endif
	pci1_hose.last_busno = pci_hose_scan_bus (&pci1_hose, pci1_hose.first_busno);

	command = pciReadConfigReg (PCI_HOST1, PCI_COMMAND, SELF);
	command |= PCI_COMMAND_MEMORY;
	pciWriteConfigReg (PCI_HOST1, PCI_COMMAND, SELF, command);

}
#endif /* of CONFIG_PCI */
