/* $Id: xiic_l.c,v 1.2 2002/12/05 19:32:40 meinelte Exp $ */
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
/*****************************************************************************/
/**
*
* @file xiic_l.c
*
* This file contains low-level driver functions that can be used to access the
* device.  The user should refer to the hardware device specification for more
* details of the device operation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date     Changes
* ----- --- -------  -----------------------------------------------
* 1.01b jhl 5/13/02  First release
* 1.01b jhl 10/14/02 Corrected bug in the receive function, the setup of the
*		     interrupt status mask was not being done in the loop such
*		     that a read would sometimes fail on the last byte because
*		     the transmit error which should have been ignored was
*		     being used.  This would leave an extra byte in the FIFO
*		     and the bus throttled such that the next operation would
*		     also fail.	 Also updated the receive function to not
*		     disable the device after the last byte until after the
*		     bus transitions to not busy which is more consistent
*		     with the expected behavior.
* 1.01c ecm 12/05/02 new rev
* </pre>
*
****************************************************************************/

/***************************** Include Files *******************************/

#include "xbasic_types.h"
#include "xio.h"
#include "xipif_v1_23_b.h"
#include "xiic_l.h"

/************************** Constant Definitions ***************************/

/**************************** Type Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *******************/


/******************************************************************************
*
* This macro clears the specified interrupt in the IPIF interrupt status
* register.  It is non-destructive in that the register is read and only the
* interrupt specified is cleared.  Clearing an interrupt acknowledges it.
*
* @param    BaseAddress contains the IPIF registers base address.
*
* @param    InterruptMask contains the interrupts to be disabled
*
* @return
*
* None.
*
* @note
*
* Signature: void XIic_mClearIisr(u32 BaseAddress,
*				  u32 InterruptMask);
*
******************************************************************************/
#define XIic_mClearIisr(BaseAddress, InterruptMask)		    \
    XIIF_V123B_WRITE_IISR((BaseAddress),			    \
	XIIF_V123B_READ_IISR(BaseAddress) & (InterruptMask))

/******************************************************************************
*
* This macro sends the address for a 7 bit address during both read and write
* operations. It takes care of the details to format the address correctly.
* This macro is designed to be called internally to the drivers.
*
* @param    SlaveAddress contains the address of the slave to send to.
*
* @param    Operation indicates XIIC_READ_OPERATION or XIIC_WRITE_OPERATION
*
* @return
*
* None.
*
* @note
*
* Signature: void XIic_mSend7BitAddr(u16 SlaveAddress, u8 Operation);
*
******************************************************************************/
#define XIic_mSend7BitAddress(BaseAddress, SlaveAddress, Operation)	    \
{									    \
    u8 LocalAddr = (u8)(SlaveAddress << 1);			    \
    LocalAddr = (LocalAddr & 0xFE) | (Operation);			    \
    XIo_Out8(BaseAddress + XIIC_DTR_REG_OFFSET, LocalAddr);		    \
}

/************************** Function Prototypes ****************************/

static unsigned RecvData (u32 BaseAddress, u8 * BufferPtr,
			  unsigned ByteCount);
static unsigned SendData (u32 BaseAddress, u8 * BufferPtr,
			  unsigned ByteCount);

/************************** Variable Definitions **************************/


/****************************************************************************/
/**
* Receive data as a master on the IIC bus.  This function receives the data
* using polled I/O and blocks until the data has been received.	 It only
* supports 7 bit addressing and non-repeated start modes of operation.	The
* user is responsible for ensuring the bus is not busy if multiple masters
* are present on the bus.
*
* @param    BaseAddress contains the base address of the IIC device.
* @param    Address contains the 7 bit IIC address of the device to send the
*	    specified data to.
* @param    BufferPtr points to the data to be sent.
* @param    ByteCount is the number of bytes to be sent.
*
* @return
*
* The number of bytes received.
*
* @note
*
* None
*
******************************************************************************/
unsigned XIic_Recv (u32 BaseAddress, u8 Address,
		    u8 * BufferPtr, unsigned ByteCount)
{
	u8 CntlReg;
	unsigned RemainingByteCount;

	/* Tx error is enabled incase the address (7 or 10) has no device to answer
	 * with Ack. When only one byte of data, must set NO ACK before address goes
	 * out therefore Tx error must not be enabled as it will go off immediately
	 * and the Rx full interrupt will be checked.  If full, then the one byte
	 * was received and the Tx error will be disabled without sending an error
	 * callback msg.
	 */
	XIic_mClearIisr (BaseAddress,
			 XIIC_INTR_RX_FULL_MASK | XIIC_INTR_TX_ERROR_MASK |
			 XIIC_INTR_ARB_LOST_MASK);

	/* Set receive FIFO occupancy depth for 1 byte (zero based)
	 */
	XIo_Out8 (BaseAddress + XIIC_RFD_REG_OFFSET, 0);

	/* 7 bit slave address, send the address for a read operation
	 * and set the state to indicate the address has been sent
	 */
	XIic_mSend7BitAddress (BaseAddress, Address, XIIC_READ_OPERATION);

	/* MSMS gets set after putting data in FIFO. Start the master receive
	 * operation by setting CR Bits MSMS to Master, if the buffer is only one
	 * byte, then it should not be acknowledged to indicate the end of data
	 */
	CntlReg = XIIC_CR_MSMS_MASK | XIIC_CR_ENABLE_DEVICE_MASK;
	if (ByteCount == 1) {
		CntlReg |= XIIC_CR_NO_ACK_MASK;
	}

	/* Write out the control register to start receiving data and call the
	 * function to receive each byte into the buffer
	 */
	XIo_Out8 (BaseAddress + XIIC_CR_REG_OFFSET, CntlReg);

	/* Clear the latched interrupt status for the bus not busy bit which must
	 * be done while the bus is busy
	 */
	XIic_mClearIisr (BaseAddress, XIIC_INTR_BNB_MASK);

	/* Try to receive the data from the IIC bus */

	RemainingByteCount = RecvData (BaseAddress, BufferPtr, ByteCount);
	/*
	 * The receive is complete, disable the IIC device and return the number of
	 * bytes that was received
	 */
	XIo_Out8 (BaseAddress + XIIC_CR_REG_OFFSET, 0);

	/* Return the number of bytes that was received */

	return ByteCount - RemainingByteCount;
}

/******************************************************************************
*
* Receive the specified data from the device that has been previously addressed
* on the IIC bus.  This function assumes that the 7 bit address has been sent
* and it should wait for the transmit of the address to complete.
*
* @param    BaseAddress contains the base address of the IIC device.
* @param    BufferPtr points to the buffer to hold the data that is received.
* @param    ByteCount is the number of bytes to be received.
*
* @return
*
* The number of bytes remaining to be received.
*
* @note
*
* This function does not take advantage of the receive FIFO because it is
* designed for minimal code space and complexity.  It contains loops that
* that could cause the function not to return if the hardware is not working.
*
* This function assumes that the calling function will disable the IIC device
* after this function returns.
*
******************************************************************************/
static unsigned RecvData (u32 BaseAddress, u8 * BufferPtr, unsigned ByteCount)
{
	u8 CntlReg;
	u32 IntrStatusMask;
	u32 IntrStatus;

	/* Attempt to receive the specified number of bytes on the IIC bus */

	while (ByteCount > 0) {
		/* Setup the mask to use for checking errors because when receiving one
		 * byte OR the last byte of a multibyte message an error naturally
		 * occurs when the no ack is done to tell the slave the last byte
		 */
		if (ByteCount == 1) {
			IntrStatusMask =
				XIIC_INTR_ARB_LOST_MASK | XIIC_INTR_BNB_MASK;
		} else {
			IntrStatusMask =
				XIIC_INTR_ARB_LOST_MASK |
				XIIC_INTR_TX_ERROR_MASK | XIIC_INTR_BNB_MASK;
		}

		/* Wait for the previous transmit and the 1st receive to complete
		 * by checking the interrupt status register of the IPIF
		 */
		while (1) {
			IntrStatus = XIIF_V123B_READ_IISR (BaseAddress);
			if (IntrStatus & XIIC_INTR_RX_FULL_MASK) {
				break;
			}
			/* Check the transmit error after the receive full because when
			 * sending only one byte transmit error will occur because of the
			 * no ack to indicate the end of the data
			 */
			if (IntrStatus & IntrStatusMask) {
				return ByteCount;
			}
		}

		CntlReg = XIo_In8 (BaseAddress + XIIC_CR_REG_OFFSET);

		/* Special conditions exist for the last two bytes so check for them
		 * Note that the control register must be setup for these conditions
		 * before the data byte which was already received is read from the
		 * receive FIFO (while the bus is throttled
		 */
		if (ByteCount == 1) {
			/* For the last data byte, it has already been read and no ack
			 * has been done, so clear MSMS while leaving the device enabled
			 * so it can get off the IIC bus appropriately with a stop.
			 */
			XIo_Out8 (BaseAddress + XIIC_CR_REG_OFFSET,
				  XIIC_CR_ENABLE_DEVICE_MASK);
		}

		/* Before the last byte is received, set NOACK to tell the slave IIC
		 * device that it is the end, this must be done before reading the byte
		 * from the FIFO
		 */
		if (ByteCount == 2) {
			/* Write control reg with NO ACK allowing last byte to
			 * have the No ack set to indicate to slave last byte read.
			 */
			XIo_Out8 (BaseAddress + XIIC_CR_REG_OFFSET,
				  CntlReg | XIIC_CR_NO_ACK_MASK);
		}

		/* Read in data from the FIFO and unthrottle the bus such that the
		 * next byte is read from the IIC bus
		 */
		*BufferPtr++ = XIo_In8 (BaseAddress + XIIC_DRR_REG_OFFSET);

		/* Clear the latched interrupt status so that it will be updated with
		 * the new state when it changes, this must be done after the receive
		 * register is read
		 */
		XIic_mClearIisr (BaseAddress, XIIC_INTR_RX_FULL_MASK |
				 XIIC_INTR_TX_ERROR_MASK |
				 XIIC_INTR_ARB_LOST_MASK);
		ByteCount--;
	}

	/* Wait for the bus to transition to not busy before returning, the IIC
	 * device cannot be disabled until this occurs.	 It should transition as
	 * the MSMS bit of the control register was cleared before the last byte
	 * was read from the FIFO.
	 */
	while (1) {
		if (XIIF_V123B_READ_IISR (BaseAddress) & XIIC_INTR_BNB_MASK) {
			break;
		}
	}

	return ByteCount;
}

/****************************************************************************/
/**
* Send data as a master on the IIC bus.	 This function sends the data
* using polled I/O and blocks until the data has been sent.  It only supports
* 7 bit addressing and non-repeated start modes of operation.  The user is
* responsible for ensuring the bus is not busy if multiple masters are present
* on the bus.
*
* @param    BaseAddress contains the base address of the IIC device.
* @param    Address contains the 7 bit IIC address of the device to send the
*	    specified data to.
* @param    BufferPtr points to the data to be sent.
* @param    ByteCount is the number of bytes to be sent.
*
* @return
*
* The number of bytes sent.
*
* @note
*
* None
*
******************************************************************************/
unsigned XIic_Send (u32 BaseAddress, u8 Address,
		    u8 * BufferPtr, unsigned ByteCount)
{
	unsigned RemainingByteCount;

	/* Put the address into the FIFO to be sent and indicate that the operation
	 * to be performed on the bus is a write operation
	 */
	XIic_mSend7BitAddress (BaseAddress, Address, XIIC_WRITE_OPERATION);

	/* Clear the latched interrupt status so that it will be updated with the
	 * new state when it changes, this must be done after the address is put
	 * in the FIFO
	 */
	XIic_mClearIisr (BaseAddress, XIIC_INTR_TX_EMPTY_MASK |
			 XIIC_INTR_TX_ERROR_MASK | XIIC_INTR_ARB_LOST_MASK);

	/* MSMS must be set after putting data into transmit FIFO, indicate the
	 * direction is transmit, this device is master and enable the IIC device
	 */
	XIo_Out8 (BaseAddress + XIIC_CR_REG_OFFSET,
		  XIIC_CR_MSMS_MASK | XIIC_CR_DIR_IS_TX_MASK |
		  XIIC_CR_ENABLE_DEVICE_MASK);

	/* Clear the latched interrupt
	 * status for the bus not busy bit which must be done while the bus is busy
	 */
	XIic_mClearIisr (BaseAddress, XIIC_INTR_BNB_MASK);

	/* Send the specified data to the device on the IIC bus specified by the
	 * the address
	 */
	RemainingByteCount = SendData (BaseAddress, BufferPtr, ByteCount);

	/*
	 * The send is complete, disable the IIC device and return the number of
	 * bytes that was sent
	 */
	XIo_Out8 (BaseAddress + XIIC_CR_REG_OFFSET, 0);

	return ByteCount - RemainingByteCount;
}

/******************************************************************************
*
* Send the specified buffer to the device that has been previously addressed
* on the IIC bus.  This function assumes that the 7 bit address has been sent
* and it should wait for the transmit of the address to complete.
*
* @param    BaseAddress contains the base address of the IIC device.
* @param    BufferPtr points to the data to be sent.
* @param    ByteCount is the number of bytes to be sent.
*
* @return
*
* The number of bytes remaining to be sent.
*
* @note
*
* This function does not take advantage of the transmit FIFO because it is
* designed for minimal code space and complexity.  It contains loops that
* that could cause the function not to return if the hardware is not working.
*
******************************************************************************/
static unsigned SendData (u32 BaseAddress, u8 * BufferPtr, unsigned ByteCount)
{
	u32 IntrStatus;

	/* Send the specified number of bytes in the specified buffer by polling
	 * the device registers and blocking until complete
	 */
	while (ByteCount > 0) {
		/* Wait for the transmit to be empty before sending any more data
		 * by polling the interrupt status register
		 */
		while (1) {
			IntrStatus = XIIF_V123B_READ_IISR (BaseAddress);

			if (IntrStatus & (XIIC_INTR_TX_ERROR_MASK |
					  XIIC_INTR_ARB_LOST_MASK |
					  XIIC_INTR_BNB_MASK)) {
				return ByteCount;
			}

			if (IntrStatus & XIIC_INTR_TX_EMPTY_MASK) {
				break;
			}
		}
		/* If there is more than one byte to send then put the next byte to send
		 * into the transmit FIFO
		 */
		if (ByteCount > 1) {
			XIo_Out8 (BaseAddress + XIIC_DTR_REG_OFFSET,
				  *BufferPtr++);
		} else {
			/* Set the stop condition before sending the last byte of data so that
			 * the stop condition will be generated immediately following the data
			 * This is done by clearing the MSMS bit in the control register.
			 */
			XIo_Out8 (BaseAddress + XIIC_CR_REG_OFFSET,
				  XIIC_CR_ENABLE_DEVICE_MASK |
				  XIIC_CR_DIR_IS_TX_MASK);

			/* Put the last byte to send in the transmit FIFO */

			XIo_Out8 (BaseAddress + XIIC_DTR_REG_OFFSET,
				  *BufferPtr++);
		}

		/* Clear the latched interrupt status register and this must be done after
		 * the transmit FIFO has been written to or it won't clear
		 */
		XIic_mClearIisr (BaseAddress, XIIC_INTR_TX_EMPTY_MASK);

		/* Update the byte count to reflect the byte sent and clear the latched
		 * interrupt status so it will be updated for the new state
		 */
		ByteCount--;
	}

	/* Wait for the bus to transition to not busy before returning, the IIC
	 * device cannot be disabled until this occurs.
	 * Note that this is different from a receive operation because the stop
	 * condition causes the bus to go not busy.
	 */
	while (1) {
		if (XIIF_V123B_READ_IISR (BaseAddress) & XIIC_INTR_BNB_MASK) {
			break;
		}
	}

	return ByteCount;
}
