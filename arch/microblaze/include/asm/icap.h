#include <asm/io.h>

#define XHwIcap_In32 readl

#define XHwIcap_Out32(a,b) writel(b,a) /* switch address & data */

/* Packet Types */
#define XHI_SYNC_PACKET			0xAA995566
#define XHI_DUMMY_PACKET		0xFFFFFFFF
#define XHI_DEVICE_ID_READ		0x28018001
#define XHI_NOOP_PACKET			0x20000000

/* Command types */
#define XHI_TYPE_1			1

/* Command Direction */
#define XHI_OP_READ			1

/* Register Programming Offsets */
#define XHI_TYPE_SHIFT			29
#define XHI_REGISTER_SHIFT		13
#define XHI_OP_SHIFT			27

/* Register Offsets */
#define XHI_WBSTAR			16

/* Register offsets for the XHwIcap device. */
#define XHI_GIER_OFFSET		0x1C  /**< Device Global Interrupt Enable Reg */
#define XHI_IPISR_OFFSET	0x20  /**< Interrupt Status Register */
#define XHI_IPIER_OFFSET	0x28  /**< Interrupt Enable Register */
#define XHI_WF_OFFSET		0x100 /**< Write FIFO */
#define XHI_RF_OFFSET		0x104 /**< Read FIFO */
#define XHI_SZ_OFFSET		0x108 /**< Size Register */
#define XHI_CR_OFFSET		0x10C /**< Control Register */
#define XHI_SR_OFFSET		0x110 /**< Status Register */
#define XHI_WFV_OFFSET		0x114 /**< Write FIFO Vacancy Register */
#define XHI_RFO_OFFSET		0x118 /**< Read FIFO Occupancy Register */

/* Control Register Contents */
#define XHI_CR_READ_MASK	0x00000002 /**< Read from ICAP to FIFO */
#define XHI_CR_WRITE_MASK	0x00000001 /**< Write from FIFO to ICAP */

/* Status Register Contents */
#define XHI_SR_DONE_MASK	0x00000001 /**< Done bit Mask  */

/* Number of times to poll the Status Register */
#define XHI_MAX_RETRIES			1000

/* Program Command */
#define XHI_CMD_IPROG			15


/****************************************************************************/
/**
*
* Read from the specified HwIcap device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XHwIcap_ReadReg(u32 BaseAddress, u32 RegOffset);
*
******************************************************************************/
#define XHwIcap_ReadReg(BaseAddress, RegOffset) \
	XHwIcap_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
*
* Write to the specified HwIcap device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XHwIcap_WriteReg(u32 BaseAddress, u32 RegOffset,
*					u32 RegisterValue);
******************************************************************************/
#define XHwIcap_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	XHwIcap_Out32((BaseAddress) + (RegOffset), (RegisterValue))

/****************************************************************************/
/**
*
* Write data to the Write FIFO.
*
* @param	BaseAddress contains the base address of the device.
* @param	Data is the 32-bit value to be written to the FIFO.
*
* @return	None.
*
* @note		C-style Signature:
* 		void XHwIcap_FifoWrite(u32 BaseAddress, u32 Data);
*
*****************************************************************************/
#define XHwIcap_FifoWrite(BaseAddress,Data) 				\
	(XHwIcap_WriteReg(BaseAddress,  XHI_WF_OFFSET, (Data)))

/****************************************************************************/
/**
*
* Read data from the Read FIFO.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	The 32-bit Data read from the FIFO.
*
* @note		C-style Signature:
* 		u32 XHwIcap_FifoRead(u32 BaseAddress);
*
*****************************************************************************/
#define XHwIcap_FifoRead(BaseAddress) 					\
(XHwIcap_ReadReg(BaseAddress, XHI_RF_OFFSET))

/****************************************************************************/
/**
*
* Get the contents of the Control register.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	A 32-bit value representing the contents of the Control
*		register.
*
* @note		u32 XHwIcap_GetControlReg(u32 BaseAddress);
*
*****************************************************************************/
#define XHwIcap_GetControlReg(BaseAddress) \
 (XHwIcap_ReadReg(BaseAddress, XHI_CR_OFFSET))


/****************************************************************************/
/**
*
* Set the Control Register to initiate a configuration (write) to the device.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	None.
*
* @note		C-style Signature:
*		void XHwIcap_StartConfig(u32 BaseAddress);
*
*****************************************************************************/
#define XHwIcap_StartConfig(BaseAddress) \
 (XHwIcap_WriteReg(BaseAddress, XHI_CR_OFFSET, (XHwIcap_GetControlReg(BaseAddress) & \
 	(~ XHI_CR_READ_MASK)) | XHI_CR_WRITE_MASK))

/******************************************************************************/
/**
*
* This macro returns the vacancy of the Write FIFO. This indicates the
* number of words that can be written to the Write FIFO before it becomes
* full.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	The contents read from the Write FIFO Vacancy Register.
*
* @note		C-Style signature:
*		u32 XHwIcap_GetWrFifoVacancy(u32 BaseAddress)
*
******************************************************************************/
#define XHwIcap_GetWrFifoVacancy(BaseAddress)				\
 XHwIcap_ReadReg(BaseAddress, XHI_WFV_OFFSET)

/******************************************************************************/
/**
*
* This macro returns the occupancy  of the Read FIFO.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	The contents read from the Read FIFO Occupancy Register.
*
* @note		C-Style signature:
*		u32 XHwIcap_GetRdFifoOccupancy(u32 BaseAddress)
*
******************************************************************************/
#define XHwIcap_GetRdFifoOccupancy(BaseAddress)		\
 XHwIcap_ReadReg(BaseAddress, XHI_RFO_OFFSET)

/****************************************************************************/
/**
*
* Get the contents of the status register.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	A 32-bit value representing the contents of the status register.
*
* @note		u32 XHwIcap_GetStatusReg(u32 BaseAddress);
*
*****************************************************************************/
#define XHwIcap_GetStatusReg(BaseAddress) \
(XHwIcap_ReadReg(BaseAddress, XHI_SR_OFFSET))

/****************************************************************************/
/**
*
* This macro checks if the last Read/Write to the ICAP device in the FPGA
* is completed.
*
* @param	BaseAddress contains the base address of the device.
*
* @return
*		- 1 if the last Read/Write(Config) to the ICAP is NOT
*		completed.
*		- 0 if the Read/Write(Config) to the ICAP is completed..
*
* @note		C-Style signature:
*		int XHwIcap_IsDeviceBusy(u32 BaseAddress);
*
*****************************************************************************/
#define XHwIcap_IsDeviceBusy(BaseAddress)			\
	((XHwIcap_GetStatusReg(BaseAddress) & XHI_SR_DONE_MASK) ? 0 : 1)

/****************************************************************************/
/**
*
* Set the number of words to be read from the Icap in the Size register.
*
* The Size Register holds the number of 32 bit words to transfer from the
* the Icap to the Read FIFO of the HwIcap device.
*
* @param	BaseAddress contains the base address of the device.
* @param	Data is the size in words.
*
* @return	None.
*
* @note		C-style Signature:
*		void XHwIcap_SetSizeReg(u32 BaseAddress, u32 Data);
*
*****************************************************************************/
#define XHwIcap_SetSizeReg(BaseAddress, Data) \
	(XHwIcap_WriteReg(BaseAddress, XHI_SZ_OFFSET, (Data)))
    
/****************************************************************************/
/**
*
* Set the Control Register to initiate a ReadBack from the device.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	None.
*
* @note		C-style Signature:
*		void XHwIcap_StartReadBack(u32 BaseAddress);
*
*****************************************************************************/
#define XHwIcap_StartReadBack(BaseAddress) \
 (XHwIcap_WriteReg(BaseAddress, XHI_CR_OFFSET, (XHwIcap_GetControlReg(BaseAddress) & \
 	(~ XHI_CR_WRITE_MASK)) | XHI_CR_READ_MASK))
    
/****************************************************************************/
/**
*
* Generates a Type 1 packet header that reads back the requested Configuration
* register.
*
* @param	Register is the address of the register to be read back.
*
* @return	Type 1 packet header to read the specified register
*
* @note		None.
*
*****************************************************************************/
#define XHwIcap_Type1Read(Register) \
	( (XHI_TYPE_1 << XHI_TYPE_SHIFT) | (Register << XHI_REGISTER_SHIFT) | \
	(XHI_OP_READ << XHI_OP_SHIFT) )

