/* $Id: xipif_v1_23_b.c,v 1.1 2002/03/18 23:24:52 linnj Exp $ */
/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2002 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* FILENAME:
*
* xipif.c
*
* DESCRIPTION:
*
* This file contains the implementation of the XIpIf component. The
* XIpIf component encapsulates the IPIF, which is the standard interface
* that IP must adhere to when connecting to a bus.  The purpose of this
* component is to encapsulate the IPIF processing such that maintainability
* is increased.  This component does not provide a lot of abstraction from
* from the details of the IPIF as it is considered a building block for
* device drivers.  A device driver designer must be familiar with the
* details of the IPIF hardware to use this component.
*
* The IPIF hardware provides a building block for all hardware devices such
* that each device does not need to reimplement these building blocks. The
* IPIF contains other building blocks, such as FIFOs and DMA channels, which
* are also common to many devices.  These blocks are implemented as separate
* hardware blocks and instantiated within the IPIF.  The primary hardware of
* the IPIF which is implemented by this software component is the interrupt
* architecture.  Since there are many blocks of a device which may generate
* interrupts, all the interrupt processing is contained in the common part
* of the device, the IPIF.  This interrupt processing is for the device level
* only and does not include any processing for the interrupt controller.
*
* A device is a mechanism such as an Ethernet MAC.  The device is made
* up of several parts which include an IPIF and the IP.  The IPIF contains most
* of the device infrastructure which is common to all devices, such as
* interrupt processing, DMA channels, and FIFOs.  The infrastructure may also
* be referred to as IPIF internal blocks since they are part of the IPIF and
* are separate blocks that can be selected based upon the needs of the device.
* The IP of the device is the logic that is unique to the device and interfaces
* to the IPIF of the device.
*
* In general, there are two levels of registers within the IPIF.  The first
* level, referred to as the device level, contains registers which are for the
* entire device.  The second level, referred to as the IP level, contains
* registers which are specific to the IP of the device.  The two levels of
* registers are designed to be hierarchical such that the device level is
* is a more general register set above the more specific registers of the IP.
* The IP level of registers provides functionality which is typically common
* across all devices and allows IP designers to focus on the unique aspects
* of the IP.
*
* The interrupt registers of the IPIF are parameterizable such that the only
* the number of bits necessary for the device are implemented. The functions
* of this component do not attempt to validate that the passed in arguments are
* valid based upon the number of implemented bits.  This is necessary to
* maintain the level of performance required for the common components.  Bits
* of the registers are assigned starting at the least significant bit of the
* registers.
*
* Critical Sections
*
* It is the responsibility of the device driver designer to use critical
* sections as necessary when calling functions of the IPIF.  This component
* does not use critical sections and it does access registers using
* read-modify-write operations.  Calls to IPIF functions from a main thread
* and from an interrupt context could produce unpredictable behavior such that
* the caller must provide the appropriate critical sections.
*
* Mutual Exclusion
*
* The functions of the IPIF are not thread safe such that the caller of all
* functions is responsible for ensuring mutual exclusion for an IPIF.  Mutual
* exclusion across multiple IPIF components is not necessary.
*
* NOTES:
*
* None.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.23b jhl  02/27/01 Repartioned to reduce size
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xipif_v1_23_b.h"
#include "xio.h"

/************************** Constant Definitions *****************************/

/* the following constant is used to generate bit masks for register testing
 * in the self test functions, it defines the starting bit mask that is to be
 * shifted from the LSB to MSB in creating a register test mask
 */
#define XIIF_V123B_FIRST_BIT_MASK     1UL

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

static XStatus IpIntrSelfTest(u32 RegBaseAddress, u32 IpRegistersWidth);

/******************************************************************************
*
* FUNCTION:
*
* XIpIf_SelfTest
*
* DESCRIPTION:
*
* This function performs a self test on the specified IPIF component.  Many
* of the registers in the IPIF are tested to ensure proper operation.  This
* function is destructive because the IPIF is reset at the start of the test
* and at the end of the test to ensure predictable results.  The IPIF reset
* also resets the entire device that uses the IPIF.  This function exits with
* all interrupts for the device disabled.
*
* ARGUMENTS:
*
* InstancePtr points to the XIpIf to operate on.
*
* DeviceRegistersWidth contains the number of bits in the device interrupt
* registers. The hardware is parameterizable such that only the number of bits
* necessary to support a device are implemented.  This value must be between 0
* and 32 with 0 indicating there are no device interrupt registers used.
*
* IpRegistersWidth contains the number of bits in the IP interrupt registers
* of the device.  The hardware is parameterizable such that only the number of
* bits necessary to support a device are implemented.  This value must be
* between 0 and 32 with 0 indicating there are no IP interrupt registers used.
*
* RETURN VALUE:
*
* A value of XST_SUCCESS indicates the test was successful with no errors.
* Any one of the following error values may also be returned.
*
*   XST_IPIF_RESET_REGISTER_ERROR       The value of a register at reset was
*                                       not valid
*   XST_IPIF_IP_STATUS_ERROR            A write to the IP interrupt status
*                                       register did not read back correctly
*   XST_IPIF_IP_ACK_ERROR               One or more bits in the IP interrupt
*                                       status register did not reset when acked
*   XST_IPIF_IP_ENABLE_ERROR            The IP interrupt enable register
*                                       did not read back correctly based upon
*                                       what was written to it
*
* NOTES:
*
* None.
*
******************************************************************************/

/* the following constant defines the maximum number of bits which may be
 * used in the registers at the device and IP levels, this is based upon the
 * number of bits available in the registers
 */
#define XIIF_V123B_MAX_REG_BIT_COUNT 32

XStatus
XIpIfV123b_SelfTest(u32 RegBaseAddress, u8 IpRegistersWidth)
{
	XStatus Status;

	/* assert to verify arguments are valid */

	XASSERT_NONVOID(IpRegistersWidth <= XIIF_V123B_MAX_REG_BIT_COUNT);

	/* reset the IPIF such that it's in a known state before the test
	 * and interrupts are globally disabled
	 */
	XIIF_V123B_RESET(RegBaseAddress);

	/* perform the self test on the IP interrupt registers, if
	 * it is not successful exit with the status
	 */
	Status = IpIntrSelfTest(RegBaseAddress, IpRegistersWidth);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* reset the IPIF such that it's in a known state before exiting test */

	XIIF_V123B_RESET(RegBaseAddress);

	/* reaching this point means there were no errors, return success */

	return XST_SUCCESS;
}

/******************************************************************************
*
* FUNCTION:
*
* IpIntrSelfTest
*
* DESCRIPTION:
*
* Perform a self test on the IP interrupt registers of the IPIF. This
* function modifies registers of the IPIF such that they are not guaranteed
* to be in the same state when it returns.  Any bits in the IP interrupt
* status register which are set are assumed to be set by default after a reset
* and are not tested in the test.
*
* ARGUMENTS:
*
* InstancePtr points to the XIpIf to operate on.
*
* IpRegistersWidth contains the number of bits in the IP interrupt registers
* of the device.  The hardware is parameterizable such that only the number of
* bits necessary to support a device are implemented.  This value must be
* between 0 and 32 with 0 indicating there are no IP interrupt registers used.
*
* RETURN VALUE:
*
* A status indicating XST_SUCCESS if the test was successful.  Otherwise, one
* of the following values is returned.
*
*   XST_IPIF_RESET_REGISTER_ERROR       The value of a register at reset was
*                                       not valid
*   XST_IPIF_IP_STATUS_ERROR            A write to the IP interrupt status
*                                       register did not read back correctly
*   XST_IPIF_IP_ACK_ERROR               One or more bits in the IP status
*                                       register did not reset when acked
*   XST_IPIF_IP_ENABLE_ERROR            The IP interrupt enable register
*                                       did not read back correctly based upon
*                                       what was written to it
* NOTES:
*
* None.
*
******************************************************************************/
static XStatus
IpIntrSelfTest(u32 RegBaseAddress, u32 IpRegistersWidth)
{
	/* ensure that the IP interrupt interrupt enable register is  zero
	 * as it should be at reset, the interrupt status is dependent upon the
	 * IP such that it's reset value is not known
	 */
	if (XIIF_V123B_READ_IIER(RegBaseAddress) != 0) {
		return XST_IPIF_RESET_REGISTER_ERROR;
	}

	/* if there are any used IP interrupts, then test all of the interrupt
	 * bits in all testable registers
	 */
	if (IpRegistersWidth > 0) {
		u32 BitCount;
		u32 IpInterruptMask = XIIF_V123B_FIRST_BIT_MASK;
		u32 Mask = XIIF_V123B_FIRST_BIT_MASK;	/* bits assigned MSB to LSB */
		u32 InterruptStatus;

		/* generate the register masks to be used for IP register tests, the
		 * number of bits supported by the hardware is parameterizable such
		 * that only that number of bits are implemented in the registers, the
		 * bits are allocated starting at the MSB of the registers
		 */
		for (BitCount = 1; BitCount < IpRegistersWidth; BitCount++) {
			Mask = Mask << 1;
			IpInterruptMask |= Mask;
		}

		/* get the current IP interrupt status register contents, any bits
		 * already set must default to 1 at reset in the device and these
		 * bits can't be tested in the following test, remove these bits from
		 * the mask that was generated for the test
		 */
		InterruptStatus = XIIF_V123B_READ_IISR(RegBaseAddress);
		IpInterruptMask &= ~InterruptStatus;

		/* set the bits in the device status register and verify them by reading
		 * the register again, all bits of the register are latched
		 */
		XIIF_V123B_WRITE_IISR(RegBaseAddress, IpInterruptMask);
		InterruptStatus = XIIF_V123B_READ_IISR(RegBaseAddress);
		if ((InterruptStatus & IpInterruptMask) != IpInterruptMask)
		{
			return XST_IPIF_IP_STATUS_ERROR;
		}

		/* test to ensure that the bits set in the IP interrupt status register
		 * can be cleared by acknowledging them in the IP interrupt status
		 * register then read it again and verify it was cleared
		 */
		XIIF_V123B_WRITE_IISR(RegBaseAddress, IpInterruptMask);
		InterruptStatus = XIIF_V123B_READ_IISR(RegBaseAddress);
		if ((InterruptStatus & IpInterruptMask) != 0) {
			return XST_IPIF_IP_ACK_ERROR;
		}

		/* set the IP interrupt enable set register and then read the IP
		 * interrupt enable register and verify the interrupts were enabled
		 */
		XIIF_V123B_WRITE_IIER(RegBaseAddress, IpInterruptMask);
		if (XIIF_V123B_READ_IIER(RegBaseAddress) != IpInterruptMask) {
			return XST_IPIF_IP_ENABLE_ERROR;
		}

		/* clear the IP interrupt enable register and then read the
		 * IP interrupt enable register and verify the interrupts were disabled
		 */
		XIIF_V123B_WRITE_IIER(RegBaseAddress, 0);
		if (XIIF_V123B_READ_IIER(RegBaseAddress) != 0) {
			return XST_IPIF_IP_ENABLE_ERROR;
		}
	}
	return XST_SUCCESS;
}
