/******************************************************************************
*
*     Author: Xilinx, Inc.
*
*
*     This program is free software; you can redistribute it and/or modify it
*     under the terms of the GNU General Public License as published by the
*     Free Software Foundation; either version 2 of the License, or (at your
*     option) any later version.
*
*
*     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
*     COURTESY TO YOU. BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
*     ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD,
*     XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
*     FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING
*     ANY THIRD PARTY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
*     XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
*     THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY
*     WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM
*     CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND
*     FITNESS FOR A PARTICULAR PURPOSE.
*
*
*     Xilinx hardware products are not intended for use in life support
*     appliances, devices, or systems. Use in such applications is
*     expressly prohibited.
*
*
*     (c) Copyright 2002-2004 Xilinx Inc.
*     All rights reserved.
*
*
*     You should have received a copy of the GNU General Public License along
*     with this program; if not, write to the Free Software Foundation, Inc.,
*     675 Mass Ave, Cambridge, MA 02139, USA.
*
******************************************************************************/
/*****************************************************************************/
/*
*
* @file xpacket_fifo_v1_00_b.c
*
* Contains functions for the XPacketFifoV100b component. See xpacket_fifo_v1_00_b.h
* for more information about the component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b rpm 03/26/02  First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xio.h"
#include "xstatus.h"
#include "xpacket_fifo_v1_00_b.h"

/************************** Constant Definitions *****************************/

/* width of a FIFO word */

#define XPF_FIFO_WIDTH_BYTE_COUNT       4UL

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************* Variable Definitions ******************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/*
*
* This function initializes a packet FIFO.  Initialization resets the
* FIFO such that it's empty and ready to use.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
* @param RegBaseAddress contains the base address of the registers for
*        the packet FIFO.
* @param DataBaseAddress contains the base address of the data for
*        the packet FIFO.
*
* @return
*
* Always returns XST_SUCCESS.
*
* @note
*
* None.
*
******************************************************************************/
XStatus
XPacketFifoV100b_Initialize(XPacketFifoV100b * InstancePtr,
			    u32 RegBaseAddress, u32 DataBaseAddress)
{
	/* assert to verify input argument are valid */

	XASSERT_NONVOID(InstancePtr != NULL);

	/* initialize the component variables to the specified state */

	InstancePtr->RegBaseAddress = RegBaseAddress;
	InstancePtr->DataBaseAddress = DataBaseAddress;
	InstancePtr->IsReady = XCOMPONENT_IS_READY;

	/* reset the FIFO such that it's empty and ready to use and indicate the
	 * initialization was successful, note that the is ready variable must be
	 * set prior to calling the reset function to prevent an assert
	 */
	XPF_V100B_RESET(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function performs a self-test on the specified packet FIFO.  The self
* test resets the FIFO and reads a register to determine if it is the correct
* reset value.  This test is destructive in that any data in the FIFO will
* be lost.
*
* @param InstancePtr is a pointer to the packet FIFO to be operated on.
*
* @param FifoType specifies the type of FIFO, read or write, for the self test.
*        The FIFO type is specified by the values XPF_READ_FIFO_TYPE or
*        XPF_WRITE_FIFO_TYPE.
*
* @return
*
* XST_SUCCESS is returned if the selftest is successful, or
* XST_PFIFO_BAD_REG_VALUE indicating that the value readback from the
* occupancy/vacancy count register after a reset does not match the
* specified reset value.
*
* @note
*
* None.
*
******************************************************************************/
XStatus
XPacketFifoV100b_SelfTest(XPacketFifoV100b * InstancePtr, u32 FifoType)
{
	u32 Register;

	/* assert to verify valid input arguments */

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID((FifoType == XPF_READ_FIFO_TYPE) ||
			(FifoType == XPF_WRITE_FIFO_TYPE));
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* reset the fifo and then check to make sure the occupancy/vacancy
	 * register contents are correct for a reset condition
	 */
	XPF_V100B_RESET(InstancePtr);

	Register = XIo_In32(InstancePtr->RegBaseAddress +
			    XPF_COUNT_STATUS_REG_OFFSET);

	/* check the value of the register to ensure that it's correct for the
	 * specified FIFO type since both FIFO types reset to empty, but a bit
	 * in the register changes definition based upon FIFO type
	 */

	if (FifoType == XPF_READ_FIFO_TYPE) {
		/* check the regiser value for a read FIFO which should be empty */

		if (Register != XPF_EMPTY_FULL_MASK) {
			return XST_PFIFO_BAD_REG_VALUE;
		}
	} else {
		/* check the register value for a write FIFO which should not be full
		 * on reset
		 */
		if ((Register & XPF_EMPTY_FULL_MASK) != 0) {
			return XST_PFIFO_BAD_REG_VALUE;
		}
	}

	/* the test was successful */

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Read data from a FIFO and puts it into a specified buffer. The packet FIFO is
* currently 32 bits wide such that an input buffer which is a series of bytes
* is filled from the FIFO a word at a time. If the requested byte count is not
* a multiple of 32 bit words, it is necessary for this function to format the
* remaining 32 bit word from the FIFO into a series of bytes in the buffer.
* There may be up to 3 extra bytes which must be extracted from the last word
* of the FIFO and put into the buffer.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
* @param BufferPtr points to the memory buffer to write the data into. This
*        buffer must be 32 bit aligned or an alignment exception could be
*        generated. Since this buffer is a byte buffer, the data is assumed to
*        be endian independent.
* @param ByteCount contains the number of bytes to read from the FIFO. This
*        number of bytes must be present in the FIFO or an error will be
*        returned.
*
* @return
*
* XST_SUCCESS indicates the operation was successful.  If the number of
* bytes specified by the byte count is not present in the FIFO
* XST_PFIFO_LACK_OF_DATA is returned.
*
* If the function was successful, the specified buffer is modified to contain
* the bytes which were removed from the FIFO.
*
* @note
*
* Note that the exact number of bytes which are present in the FIFO is
* not known by this function.  It can only check for a number of 32 bit
* words such that if the byte count specified is incorrect, but is still
* possible based on the number of words in the FIFO, up to 3 garbage bytes
* may be present at the end of the buffer.
* <br><br>
* This function assumes that if the device consuming data from the FIFO is
* a byte device, the order of the bytes to be consumed is from the most
* significant byte to the least significant byte of a 32 bit word removed
* from the FIFO.
*
******************************************************************************/
XStatus
XPacketFifoV100b_Read(XPacketFifoV100b * InstancePtr,
		      u8 * BufferPtr, u32 ByteCount)
{
	u32 FifoCount;
	u32 WordCount;
	u32 ExtraByteCount;
	u32 *WordBuffer = (u32 *) BufferPtr;

	/* assert to verify valid input arguments including 32 bit alignment of
	 * the buffer pointer
	 */
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(BufferPtr != NULL);
	XASSERT_NONVOID(((u32) BufferPtr &
			 (XPF_FIFO_WIDTH_BYTE_COUNT - 1)) == 0);
	XASSERT_NONVOID(ByteCount != 0);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* get the count of how many 32 bit words are in the FIFO, if there aren't
	 * enought words to satisfy the request, return an error
	 */

	FifoCount = XIo_In32(InstancePtr->RegBaseAddress +
			     XPF_COUNT_STATUS_REG_OFFSET) & XPF_COUNT_MASK;

	if ((FifoCount * XPF_FIFO_WIDTH_BYTE_COUNT) < ByteCount) {
		return XST_PFIFO_LACK_OF_DATA;
	}

	/* calculate the number of words to read from the FIFO before the word
	 * containing the extra bytes, and calculate the number of extra bytes
	 * the extra bytes are defined as those at the end of the buffer when
	 * the buffer does not end on a 32 bit boundary
	 */
	WordCount = ByteCount / XPF_FIFO_WIDTH_BYTE_COUNT;
	ExtraByteCount = ByteCount % XPF_FIFO_WIDTH_BYTE_COUNT;

	/* Read the 32 bit words from the FIFO for all the buffer except the
	 * last word which contains the extra bytes, the following code assumes
	 * that the buffer is 32 bit aligned, otherwise an alignment exception could
	 * be generated
	 */
	for (FifoCount = 0; FifoCount < WordCount; FifoCount++) {
		WordBuffer[FifoCount] = XIo_In32(InstancePtr->DataBaseAddress);
	}

	/* if there are extra bytes to handle, read the last word from the FIFO
	 * and insert the extra bytes into the buffer
	 */
	if (ExtraByteCount > 0) {
		u32 LastWord;
		u8 *ExtraBytesBuffer = (u8 *) (WordBuffer + WordCount);

		/* get the last word from the FIFO for the extra bytes */

		LastWord = XIo_In32(InstancePtr->DataBaseAddress);

		/* one extra byte in the last word, put the byte into the next location
		 * of the buffer, bytes in a word of the FIFO are ordered from most
		 * significant byte to least
		 */
		if (ExtraByteCount == 1) {
			ExtraBytesBuffer[0] = (u8) (LastWord >> 24);
		}

		/* two extra bytes in the last word, put each byte into the next two
		 * locations of the buffer
		 */
		else if (ExtraByteCount == 2) {
			ExtraBytesBuffer[0] = (u8) (LastWord >> 24);
			ExtraBytesBuffer[1] = (u8) (LastWord >> 16);
		}
		/* three extra bytes in the last word, put each byte into the next three
		 * locations of the buffer
		 */
		else if (ExtraByteCount == 3) {
			ExtraBytesBuffer[0] = (u8) (LastWord >> 24);
			ExtraBytesBuffer[1] = (u8) (LastWord >> 16);
			ExtraBytesBuffer[2] = (u8) (LastWord >> 8);
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* Write data into a packet FIFO. The packet FIFO is currently 32 bits wide
* such that an input buffer which is a series of bytes must be written into the
* FIFO a word at a time. If the buffer is not a multiple of 32 bit words, it is
* necessary for this function to format the remaining bytes into a single 32
* bit word to be inserted into the FIFO. This is necessary to avoid any
* accesses past the end of the buffer.
*
* @param InstancePtr contains a pointer to the FIFO to operate on.
* @param BufferPtr points to the memory buffer that data is to be read from
*        and written into the FIFO. Since this buffer is a byte buffer, the data
*        is assumed to be endian independent. This buffer must be 32 bit aligned
*        or an alignment exception could be generated.
* @param ByteCount contains the number of bytes to read from the buffer and to
*        write to the FIFO.
*
* @return
*
* XST_SUCCESS is returned if the operation succeeded.  If there is not enough
* room in the FIFO to hold the specified bytes, XST_PFIFO_NO_ROOM is
* returned.
*
* @note
*
* This function assumes that if the device inserting data into the FIFO is
* a byte device, the order of the bytes in each 32 bit word is from the most
* significant byte to the least significant byte.
*
******************************************************************************/
XStatus
XPacketFifoV100b_Write(XPacketFifoV100b * InstancePtr,
		       u8 * BufferPtr, u32 ByteCount)
{
	u32 FifoCount;
	u32 WordCount;
	u32 ExtraByteCount;
	u32 *WordBuffer = (u32 *) BufferPtr;

	/* assert to verify valid input arguments including 32 bit alignment of
	 * the buffer pointer
	 */
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(BufferPtr != NULL);
	XASSERT_NONVOID(((u32) BufferPtr &
			 (XPF_FIFO_WIDTH_BYTE_COUNT - 1)) == 0);
	XASSERT_NONVOID(ByteCount != 0);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* get the count of how many words may be inserted into the FIFO */

	FifoCount = XIo_In32(InstancePtr->RegBaseAddress +
			     XPF_COUNT_STATUS_REG_OFFSET) & XPF_COUNT_MASK;

	/* Calculate the number of 32 bit words required to insert the specified
	 * number of bytes in the FIFO and determine the number of extra bytes
	 * if the buffer length is not a multiple of 32 bit words
	 */

	WordCount = ByteCount / XPF_FIFO_WIDTH_BYTE_COUNT;
	ExtraByteCount = ByteCount % XPF_FIFO_WIDTH_BYTE_COUNT;

	/* take into account the extra bytes in the total word count */

	if (ExtraByteCount > 0) {
		WordCount++;
	}

	/* if there's not enough room in the FIFO to hold the specified
	 * number of bytes, then indicate an error,
	 */
	if (FifoCount < WordCount) {
		return XST_PFIFO_NO_ROOM;
	}

	/* readjust the word count to not take into account the extra bytes */

	if (ExtraByteCount > 0) {
		WordCount--;
	}

	/* Write all the bytes of the buffer which can be written as 32 bit
	 * words into the FIFO, waiting to handle the extra bytes seperately
	 */
	for (FifoCount = 0; FifoCount < WordCount; FifoCount++) {
		XIo_Out32(InstancePtr->DataBaseAddress, WordBuffer[FifoCount]);
	}

	/* if there are extra bytes to handle, extract them from the buffer
	 * and create a 32 bit word and write it to the FIFO
	 */
	if (ExtraByteCount > 0) {
		u32 LastWord = 0;
		u8 *ExtraBytesBuffer = (u8 *) (WordBuffer + WordCount);

		/* one extra byte in the buffer, put the byte into the last word
		 * to be inserted into the FIFO, perform this processing inline rather
		 * than in a loop to help performance
		 */
		if (ExtraByteCount == 1) {
			LastWord = ExtraBytesBuffer[0] << 24;
		}

		/* two extra bytes in the buffer, put each byte into the last word
		 * to be inserted into the FIFO
		 */
		else if (ExtraByteCount == 2) {
			LastWord = ExtraBytesBuffer[0] << 24 |
			    ExtraBytesBuffer[1] << 16;
		}

		/* three extra bytes in the buffer, put each byte into the last word
		 * to be inserted into the FIFO
		 */
		else if (ExtraByteCount == 3) {
			LastWord = ExtraBytesBuffer[0] << 24 |
			    ExtraBytesBuffer[1] << 16 |
			    ExtraBytesBuffer[2] << 8;
		}

		/* write the last 32 bit word to the FIFO and return with no errors */

		XIo_Out32(InstancePtr->DataBaseAddress, LastWord);
	}

	return XST_SUCCESS;
}
