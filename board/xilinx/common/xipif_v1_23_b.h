/* $Id: xipif_v1_23_b.h,v 1.1 2002/03/18 23:24:52 linnj Exp $ */
/******************************************************************************
*
*	XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*	AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*	SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*	OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*	APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*	THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*	AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*	FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*	WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*	IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*	REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*	INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*	FOR A PARTICULAR PURPOSE.
*
*	(c) Copyright 2002 Xilinx Inc.
*	All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* FILENAME:
*
* xipif.h
*
* DESCRIPTION:
*
* The XIpIf component encapsulates the IPIF, which is the standard interface
* that IP must adhere to when connecting to a bus.  The purpose of this
* component is to encapsulate the IPIF processing such that maintainability
* is increased.	 This component does not provide a lot of abstraction from
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
* architecture.	 Since there are many blocks of a device which may generate
* interrupts, all the interrupt processing is contained in the common part
* of the device, the IPIF.  This interrupt processing is for the device level
* only and does not include any processing for the interrupt controller.
*
* A device is a mechanism such as an Ethernet MAC.  The device is made
* up of several parts which include an IPIF and the IP.	 The IPIF contains most
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
* registers which are specific to the IP of the device.	 The two levels of
* registers are designed to be hierarchical such that the device level is
* is a more general register set above the more specific registers of the IP.
* The IP level of registers provides functionality which is typically common
* across all devices and allows IP designers to focus on the unique aspects
* of the IP.
*
* Critical Sections
*
* It is the responsibility of the device driver designer to use critical
* sections as necessary when calling functions of the IPIF.  This component
* does not use critical sections and it does access registers using
* read-modify-write operations.	 Calls to IPIF functions from a main thread
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
* Ver	Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.23b jhl  02/27/01 Repartioned to minimize size
*
******************************************************************************/

#ifndef XIPIF_H			/* prevent circular inclusions */
#define XIPIF_H			/* by using protection macros */

/***************************** Include Files *********************************/
#include "xbasic_types.h"
#include "xstatus.h"
#include "xversion.h"

/************************** Constant Definitions *****************************/

/* the following constants define the register offsets for the registers of the
 * IPIF, there are some holes in the memory map for reserved addresses to allow
 * other registers to be added and still match the memory map of the interrupt
 * controller registers
 */
#define XIIF_V123B_DISR_OFFSET	   0UL	/* device interrupt status register */
#define XIIF_V123B_DIPR_OFFSET	   4UL	/* device interrupt pending register */
#define XIIF_V123B_DIER_OFFSET	   8UL	/* device interrupt enable register */
#define XIIF_V123B_DIIR_OFFSET	   24UL /* device interrupt ID register */
#define XIIF_V123B_DGIER_OFFSET	   28UL /* device global interrupt enable reg */
#define XIIF_V123B_IISR_OFFSET	   32UL /* IP interrupt status register */
#define XIIF_V123B_IIER_OFFSET	   40UL /* IP interrupt enable register */
#define XIIF_V123B_RESETR_OFFSET   64UL /* reset register */

#define XIIF_V123B_RESET_MASK		  0xAUL

/* the following constant is used for the device global interrupt enable
 * register, to enable all interrupts for the device, this is the only bit
 * in the register
 */
#define XIIF_V123B_GINTR_ENABLE_MASK	  0x80000000UL

/* the following constants contain the masks to identify each internal IPIF
 * condition in the device registers of the IPIF, interrupts are assigned
 * in the register from LSB to the MSB
 */
#define XIIF_V123B_ERROR_MASK		  1UL	/* LSB of the register */

/* The following constants contain interrupt IDs which identify each internal
 * IPIF condition, this value must correlate with the mask constant for the
 * error
 */
#define XIIF_V123B_ERROR_INTERRUPT_ID	  0	/* interrupt bit #, (LSB = 0) */
#define XIIF_V123B_NO_INTERRUPT_ID	  128	/* no interrupts are pending */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_RESET
*
* DESCRIPTION:
*
* Reset the IPIF component and hardware.  This is a destructive operation that
* could cause the loss of data since resetting the IPIF of a device also
* resets the device using the IPIF and any blocks, such as FIFOs or DMA
* channels, within the IPIF.  All registers of the IPIF will contain their
* reset value when this function returns.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* None.
*
* NOTES:
*
* None.
*
******************************************************************************/

/* the following constant is used in the reset register to cause the IPIF to
 * reset
 */
#define XIIF_V123B_RESET(RegBaseAddress) \
    XIo_Out32(RegBaseAddress + XIIF_V123B_RESETR_OFFSET, XIIF_V123B_RESET_MASK)

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_WRITE_DISR
*
* DESCRIPTION:
*
* This function sets the device interrupt status register to the value.
* This register indicates the status of interrupt sources for a device
* which contains the IPIF.  The status is independent of whether interrupts
* are enabled and could be used for polling a device at a higher level rather
* than a more detailed level.
*
* Each bit of the register correlates to a specific interrupt source within the
* device which contains the IPIF.  With the exception of some internal IPIF
* conditions, the contents of this register are not latched but indicate
* the live status of the interrupt sources within the device.  Writing any of
* the non-latched bits of the register will have no effect on the register.
*
* For the latched bits of this register only, setting a bit which is zero
* within this register causes an interrupt to generated.  The device global
* interrupt enable register and the device interrupt enable register must be set
* appropriately to allow an interrupt to be passed out of the device. The
* interrupt is cleared by writing to this register with the bits to be
* cleared set to a one and all others to zero.	This register implements a
* toggle on write functionality meaning any bits which are set in the value
* written cause the bits in the register to change to the opposite state.
*
* This function writes the specified value to the register such that
* some bits may be set and others cleared.  It is the caller's responsibility
* to get the value of the register prior to setting the value to prevent a
* destructive behavior.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* Status contains the value to be written to the interrupt status register of
* the device.  The only bits which can be written are the latched bits which
* contain the internal IPIF conditions.	 The following values may be used to
* set the status register or clear an interrupt condition.
*
*   XIIF_V123B_ERROR_MASK     Indicates a device error in the IPIF
*
* RETURN VALUE:
*
* None.
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_WRITE_DISR(RegBaseAddress, Status) \
    XIo_Out32((RegBaseAddress) + XIIF_V123B_DISR_OFFSET, (Status))

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_READ_DISR
*
* DESCRIPTION:
*
* This function gets the device interrupt status register contents.
* This register indicates the status of interrupt sources for a device
* which contains the IPIF.  The status is independent of whether interrupts
* are enabled and could be used for polling a device at a higher level.
*
* Each bit of the register correlates to a specific interrupt source within the
* device which contains the IPIF.  With the exception of some internal IPIF
* conditions, the contents of this register are not latched but indicate
* the live status of the interrupt sources within the device.
*
* For only the latched bits of this register, the interrupt may be cleared by
* writing to these bits in the status register.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* A status which contains the value read from the interrupt status register of
* the device. The bit definitions are specific to the device with
* the exception of the latched internal IPIF condition bits. The following
* values may be used to detect internal IPIF conditions in the status.
*
*   XIIF_V123B_ERROR_MASK     Indicates a device error in the IPIF
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_READ_DISR(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XIIF_V123B_DISR_OFFSET)

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_WRITE_DIER
*
* DESCRIPTION:
*
* This function sets the device interrupt enable register contents.
* This register controls which interrupt sources of the device are allowed to
* generate an interrupt.  The device global interrupt enable register must also
* be set appropriately for an interrupt to be passed out of the device.
*
* Each bit of the register correlates to a specific interrupt source within the
* device which contains the IPIF.  Setting a bit in this register enables that
* interrupt source to generate an interrupt.  Clearing a bit in this register
* disables interrupt generation for that interrupt source.
*
* This function writes only the specified value to the register such that
* some interrupts source may be enabled and others disabled.  It is the
* caller's responsibility to get the value of the interrupt enable register
* prior to setting the value to prevent an destructive behavior.
*
* An interrupt source may not be enabled to generate an interrupt, but can
* still be polled in the interrupt status register.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* Enable contains the value to be written to the interrupt enable register
* of the device.  The bit definitions are specific to the device with
* the exception of the internal IPIF conditions. The following
* values may be used to enable the internal IPIF conditions interrupts.
*
*   XIIF_V123B_ERROR_MASK     Indicates a device error in the IPIF
*
* RETURN VALUE:
*
* None.
*
* NOTES:
*
* Signature: u32 XIIF_V123B_WRITE_DIER(u32 RegBaseAddress,
*					  u32 Enable)
*
******************************************************************************/
#define XIIF_V123B_WRITE_DIER(RegBaseAddress, Enable) \
    XIo_Out32((RegBaseAddress) + XIIF_V123B_DIER_OFFSET, (Enable))

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_READ_DIER
*
* DESCRIPTION:
*
* This function gets the device interrupt enable register contents.
* This register controls which interrupt sources of the device
* are allowed to generate an interrupt.	 The device global interrupt enable
* register and the device interrupt enable register must also be set
* appropriately for an interrupt to be passed out of the device.
*
* Each bit of the register correlates to a specific interrupt source within the
* device which contains the IPIF.  Setting a bit in this register enables that
* interrupt source to generate an interrupt if the global enable is set
* appropriately.  Clearing a bit in this register disables interrupt generation
* for that interrupt source regardless of the global interrupt enable.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* The value read from the interrupt enable register of the device.  The bit
* definitions are specific to the device with the exception of the internal
* IPIF conditions. The following values may be used to determine from the
* value if the internal IPIF conditions interrupts are enabled.
*
*   XIIF_V123B_ERROR_MASK     Indicates a device error in the IPIF
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_READ_DIER(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XIIF_V123B_DIER_OFFSET)

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_READ_DIPR
*
* DESCRIPTION:
*
* This function gets the device interrupt pending register contents.
* This register indicates the pending interrupt sources, those that are waiting
* to be serviced by the software, for a device which contains the IPIF.
* An interrupt must be enabled in the interrupt enable register of the IPIF to
* be pending.
*
* Each bit of the register correlates to a specific interrupt source within the
* the device which contains the IPIF.  With the exception of some internal IPIF
* conditions, the contents of this register are not latched since the condition
* is latched in the IP interrupt status register, by an internal block of the
* IPIF such as a FIFO or DMA channel, or by the IP of the device.  This register
* is read only and is not latched, but it is necessary to acknowledge (clear)
* the interrupt condition by performing the appropriate processing for the IP
* or block within the IPIF.
*
* This register can be thought of as the contents of the interrupt status
* register ANDed with the contents of the interrupt enable register.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* The value read from the interrupt pending register of the device.  The bit
* definitions are specific to the device with the exception of the latched
* internal IPIF condition bits. The following values may be used to detect
* internal IPIF conditions in the value.
*
*   XIIF_V123B_ERROR_MASK     Indicates a device error in the IPIF
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_READ_DIPR(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XIIF_V123B_DIPR_OFFSET)

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_READ_DIIR
*
* DESCRIPTION:
*
* This function gets the device interrupt ID for the highest priority interrupt
* which is pending from the interrupt ID register. This function provides
* priority resolution such that faster interrupt processing is possible.
* Without priority resolution, it is necessary for the software to read the
* interrupt pending register and then check each interrupt source to determine
* if an interrupt is pending.  Priority resolution becomes more important as the
* number of interrupt sources becomes larger.
*
* Interrupt priorities are based upon the bit position of the interrupt in the
* interrupt pending register with bit 0 being the highest priority. The
* interrupt ID is the priority of the interrupt, 0 - 31, with 0 being the
* highest priority. The interrupt ID register is live rather than latched such
* that multiple calls to this function may not yield the same results.	A
* special value, outside of the interrupt priority range of 0 - 31, is
* contained in the register which indicates that no interrupt is pending.  This
* may be useful for allowing software to continue processing interrupts in a
* loop until there are no longer any interrupts pending.
*
* The interrupt ID is designed to allow a function pointer table to be used
* in the software such that the interrupt ID is used as an index into that
* table.  The function pointer table could contain an instance pointer, such
* as to DMA channel, and a function pointer to the function which handles
* that interrupt.  This design requires the interrupt processing of the device
* driver to be partitioned into smaller more granular pieces based upon
* hardware used by the device, such as DMA channels and FIFOs.
*
* It is not mandatory that this function be used by the device driver software.
* It may choose to read the pending register and resolve the pending interrupt
* priorities on it's own.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* An interrupt ID, 0 - 31, which identifies the highest priority interrupt
* which is pending.  A value of XIIF_NO_INTERRUPT_ID indicates that there is
* no interrupt pending. The following values may be used to identify the
* interrupt ID for the internal IPIF interrupts.
*
*   XIIF_V123B_ERROR_INTERRUPT_ID     Indicates a device error in the IPIF
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_READ_DIIR(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XIIF_V123B_DIIR_OFFSET)

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_GLOBAL_INTR_DISABLE
*
* DESCRIPTION:
*
* This function disables all interrupts for the device by writing to the global
* interrupt enable register.  This register provides the ability to disable
* interrupts without any modifications to the interrupt enable register such
* that it is minimal effort to restore the interrupts to the previous enabled
* state.  The corresponding function, XIpIf_GlobalIntrEnable, is provided to
* restore the interrupts to the previous enabled state.	 This function is
* designed to be used in critical sections of device drivers such that it is
* not necessary to disable other device interrupts.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* None.
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_GINTR_DISABLE(RegBaseAddress) \
    XIo_Out32((RegBaseAddress) + XIIF_V123B_DGIER_OFFSET, 0)

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_GINTR_ENABLE
*
* DESCRIPTION:
*
* This function writes to the global interrupt enable register to enable
* interrupts from the device.  This register provides the ability to enable
* interrupts without any modifications to the interrupt enable register such
* that it is minimal effort to restore the interrupts to the previous enabled
* state.  This function does not enable individual interrupts as the interrupt
* enable register must be set appropriately.  This function is designed to be
* used in critical sections of device drivers such that it is not necessary to
* disable other device interrupts.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* None.
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_GINTR_ENABLE(RegBaseAddress)		  \
    XIo_Out32((RegBaseAddress) + XIIF_V123B_DGIER_OFFSET, \
	       XIIF_V123B_GINTR_ENABLE_MASK)

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_IS_GINTR_ENABLED
*
* DESCRIPTION:
*
* This function determines if interrupts are enabled at the global level by
* reading the gloabl interrupt register. This register provides the ability to
* disable interrupts without any modifications to the interrupt enable register
* such that it is minimal effort to restore the interrupts to the previous
* enabled state.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* XTRUE if interrupts are enabled for the IPIF, XFALSE otherwise.
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_IS_GINTR_ENABLED(RegBaseAddress)		\
    (XIo_In32((RegBaseAddress) + XIIF_V123B_DGIER_OFFSET) ==	\
	      XIIF_V123B_GINTR_ENABLE_MASK)

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_WRITE_IISR
*
* DESCRIPTION:
*
* This function sets the IP interrupt status register to the specified value.
* This register indicates the status of interrupt sources for the IP of the
* device.  The IP is defined as the part of the device that connects to the
* IPIF.	 The status is independent of whether interrupts are enabled such that
* the status register may also be polled when interrupts are not enabled.
*
* Each bit of the register correlates to a specific interrupt source within the
* IP.  All bits of this register are latched. Setting a bit which is zero
* within this register causes an interrupt to be generated.  The device global
* interrupt enable register and the device interrupt enable register must be set
* appropriately to allow an interrupt to be passed out of the device. The
* interrupt is cleared by writing to this register with the bits to be
* cleared set to a one and all others to zero.	This register implements a
* toggle on write functionality meaning any bits which are set in the value
* written cause the bits in the register to change to the opposite state.
*
* This function writes only the specified value to the register such that
* some status bits may be set and others cleared.  It is the caller's
* responsibility to get the value of the register prior to setting the value
* to prevent an destructive behavior.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* Status contains the value to be written to the IP interrupt status
* register.  The bit definitions are specific to the device IP.
*
* RETURN VALUE:
*
* None.
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_WRITE_IISR(RegBaseAddress, Status) \
    XIo_Out32((RegBaseAddress) + XIIF_V123B_IISR_OFFSET, (Status))

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_READ_IISR
*
* DESCRIPTION:
*
* This function gets the contents of the IP interrupt status register.
* This register indicates the status of interrupt sources for the IP of the
* device.  The IP is defined as the part of the device that connects to the
* IPIF. The status is independent of whether interrupts are enabled such
* that the status register may also be polled when interrupts are not enabled.
*
* Each bit of the register correlates to a specific interrupt source within the
* device.  All bits of this register are latched.  Writing a 1 to a bit within
* this register causes an interrupt to be generated if enabled in the interrupt
* enable register and the global interrupt enable is set.  Since the status is
* latched, each status bit must be acknowledged in order for the bit in the
* status register to be updated.  Each bit can be acknowledged by writing a
* 0 to the bit in the status register.

* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* A status which contains the value read from the IP interrupt status register.
* The bit definitions are specific to the device IP.
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_READ_IISR(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XIIF_V123B_IISR_OFFSET)

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_WRITE_IIER
*
* DESCRIPTION:
*
* This function sets the IP interrupt enable register contents.	 This register
* controls which interrupt sources of the IP are allowed to generate an
* interrupt.  The global interrupt enable register and the device interrupt
* enable register must also be set appropriately for an interrupt to be
* passed out of the device containing the IPIF and the IP.
*
* Each bit of the register correlates to a specific interrupt source within the
* IP.  Setting a bit in this register enables the interrupt source to generate
* an interrupt.	 Clearing a bit in this register disables interrupt generation
* for that interrupt source.
*
* This function writes only the specified value to the register such that
* some interrupt sources may be enabled and others disabled.  It is the
* caller's responsibility to get the value of the interrupt enable register
* prior to setting the value to prevent an destructive behavior.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* Enable contains the value to be written to the IP interrupt enable register.
* The bit definitions are specific to the device IP.
*
* RETURN VALUE:
*
* None.
*
* NOTES:
*
* None.
*
******************************************************************************/
#define XIIF_V123B_WRITE_IIER(RegBaseAddress, Enable) \
    XIo_Out32((RegBaseAddress) + XIIF_V123B_IIER_OFFSET, (Enable))

/******************************************************************************
*
* MACRO:
*
* XIIF_V123B_READ_IIER
*
* DESCRIPTION:
*
*
* This function gets the IP interrupt enable register contents.	 This register
* controls which interrupt sources of the IP are allowed to generate an
* interrupt.  The global interrupt enable register and the device interrupt
* enable register must also be set appropriately for an interrupt to be
* passed out of the device containing the IPIF and the IP.
*
* Each bit of the register correlates to a specific interrupt source within the
* IP.  Setting a bit in this register enables the interrupt source to generate
* an interrupt.	 Clearing a bit in this register disables interrupt generation
* for that interrupt source.
*
* ARGUMENTS:
*
* RegBaseAddress contains the base address of the IPIF registers.
*
* RETURN VALUE:
*
* The contents read from the IP interrupt enable register.  The bit definitions
* are specific to the device IP.
*
* NOTES:
*
* Signature: u32 XIIF_V123B_READ_IIER(u32 RegBaseAddress)
*
******************************************************************************/
#define XIIF_V123B_READ_IIER(RegBaseAddress) \
    XIo_In32((RegBaseAddress) + XIIF_V123B_IIER_OFFSET)

/************************** Function Prototypes ******************************/

/*
 * Initialization Functions
 */
XStatus XIpIfV123b_SelfTest(u32 RegBaseAddress, u8 IpRegistersWidth);

#endif				/* end of protection macro */
