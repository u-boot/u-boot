/*
 * Copyright (C) ST-Ericsson SA 2009
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

#ifndef _U8500_I2C_H_
#define _U8500_I2C_H_

#include <asm/types.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/u8500.h>

struct u8500_i2c_regs {
	u32 cr;			/* Control Register                      0x00 */
	u32 scr;		/* Slave Address Register                0x04 */
	u32 hsmcr;		/* HS Master code Register               0x08 */
	u32 mcr;		/* Master Control Register               0x0C */
	u32 tfr;		/* Transmit Fifo Register                0x10 */
	u32 sr;			/* Status Register                       0x14 */
	u32 rfr;		/* Receiver Fifo Register                0x18 */
	u32 tftr;		/* Transmit Fifo Threshold Register      0x1C */
	u32 rftr;		/* Receiver Fifo Threshold Register      0x20 */
	u32 dmar;		/* DMA register                          0x24 */
	u32 brcr;		/* Baud Rate Counter Register            0x28 */
	u32 imscr;		/* Interrupt Mask Set and Clear Register 0x2C */
	u32 risr;		/* Raw interrupt status register         0x30 */
	u32 misr;		/* Masked interrupt status register      0x34 */
	u32 icr;		/* Interrupt Set and Clear Register      0x38 */
	u32 reserved_1[(0xFE0 - 0x3c) >> 2];	/* Reserved 0x03C to 0xFE0 */
	u32 periph_id_0;	/* peripheral ID 0			0xFE0 */
	u32 periph_id_1;	/* peripheral ID 1			0xFE4 */
	u32 periph_id_2;	/* peripheral ID 2			0xFE8 */
	u32 periph_id_3;	/* peripheral ID 3			0xFEC */
	u32 cell_id_0;		/* I2C cell   ID 0			0xFF0 */
	u32 cell_id_1;		/* I2C cell   ID 1			0xFF4 */
	u32 cell_id_2;		/* I2C cell   ID 2			0xFF8 */
	u32 cell_id_3;		/* I2C cell   ID 3			0xFFC */
};


/* Control Register */

/* Mask values for control register mask */
#define U8500_I2C_CR_PE          0x0001	/* Peripheral enable */
#define U8500_I2C_CR_OM          0x0006	/* Operation mode */
#define U8500_I2C_CR_SAM         0x0008	/* Slave Addressing mode */
#define U8500_I2C_CR_SM          0x0030	/* Speed mode */
#define U8500_I2C_CR_SGCM        0x0040	/* Slave General call mode */
#define U8500_I2C_CR_FTX         0x0080	/* Flush Transmit */
#define U8500_I2C_CR_FRX         0x0100	/* Flush Receive */
#define U8500_I2C_CR_DMA_TX_EN   0x0200	/* DMA TX Enable */
#define U8500_I2C_CR_DMA_RX_EN   0x0400	/* DMA Rx Enable */
#define U8500_I2C_CR_DMA_SLE     0x0800	/* DMA Synchronization Logic enable */
#define U8500_I2C_CR_LM          0x1000	/* Loop back mode */
#define U8500_I2C_CR_FON         0x6000	/* Filtering On */

/* shift valus for control register bit fields */
#define U8500_I2C_CR_SHIFT_PE		0	/* Peripheral enable */
#define U8500_I2C_CR_SHIFT_OM		1	/* Operation mode */
#define U8500_I2C_CR_SHIFT_SAM		3	/* Slave Addressing mode */
#define U8500_I2C_CR_SHIFT_SM		4	/* Speed mode */
#define U8500_I2C_CR_SHIFT_SGCM		6	/* Slave General call mode */
#define U8500_I2C_CR_SHIFT_FTX		7	/* Flush Transmit */
#define U8500_I2C_CR_SHIFT_FRX		8	/* Flush Receive */
#define U8500_I2C_CR_SHIFT_DMA_TX_EN	9	/* DMA TX Enable */
#define U8500_I2C_CR_SHIFT_DMA_RX_EN	10	/* DMA Rx Enable */
#define U8500_I2C_CR_SHIFT_DMA_SLE	11	/* DMA Synch Logic enable */
#define U8500_I2C_CR_SHIFT_LM		12	/* Loop back mode */
#define U8500_I2C_CR_SHIFT_FON		13	/* Filtering On */

/* bus operation modes */
#define U8500_I2C_BUS_SLAVE_MODE		0
#define U8500_I2C_BUS_MASTER_MODE		1
#define U8500_I2C_BUS_MASTER_SLAVE_MODE	2


/* Slave control register*/

/* Mask values slave control register */
#define U8500_I2C_SCR_ADDR                   0x3FF
#define U8500_I2C_SCR_DATA_SETUP_TIME        0xFFFF0000

/* Shift values for Slave control register */
#define U8500_I2C_SCR_SHIFT_ADDR               0
#define U8500_I2C_SCR_SHIFT_DATA_SETUP_TIME    16


/* Master Control Register */

/* Mask values for Master control register */
#define U8500_I2C_MCR_OP	0x00000001	/* Operation */
#define U8500_I2C_MCR_A7	0x000000FE	/* LSB bits of Address */
#define U8500_I2C_MCR_EA10	0x00000700	/* Extended Address */
#define U8500_I2C_MCR_SB	0x00000800	/* Start byte procedure */
#define U8500_I2C_MCR_AM	0x00003000	/* Address type */
#define U8500_I2C_MCR_STOP	0x00004000	/* stop condition */
#define U8500_I2C_MCR_LENGTH	0x03FF8000	/* Frame length */
#define U8500_I2C_MCR_A10	0x000007FE	/* Enable 10 bit address */
/* mask for length field,stop and operation  */
#define U8500_I2C_MCR_LENGTH_STOP_OP	0x3FFC001

/* Shift values for Master control values */
#define U8500_I2C_MCR_SHIFT_OP		0	/* Operation */
#define U8500_I2C_MCR_SHIFT_A7		1	/* LSB bits of Address */
#define U8500_I2C_MCR_SHIFT_EA10	8	/* Extended Address */
#define U8500_I2C_MCR_SHIFT_SB		11	/* Start byte procedure */
#define U8500_I2C_MCR_SHIFT_AM		12	/* Address type */
#define U8500_I2C_MCR_SHIFT_STOP	14	/* stop condition */
#define U8500_I2C_MCR_SHIFT_LENGTH	15	/* Frame length */
#define U8500_I2C_MCR_SHIFT_A10		1	/* Enable 10 bit address */

#define U8500_I2C_MCR_SHIFT_LENGTH_STOP_OP	0


/* Status Register */

/* Mask values for Status register */
#define U8500_I2C_SR_OP	0x00000003	/* Operation */
#define U8500_I2C_SR_STATUS	0x0000000C	/* Controller Status */
#define U8500_I2C_SR_CAUSE	0x00000070	/* Abort Cause */
#define U8500_I2C_SR_TYPE	0x00000180	/* Receive Type */
#define U8500_I2C_SR_LENGTH	0x000FF700	/* Transfer length */

/* Shift values for Status register */
#define U8500_I2C_SR_SHIFT_OP		0	/* Operation */
#define U8500_I2C_SR_SHIFT_STATUS	2	/* Controller Status */
#define U8500_I2C_SR_SHIFT_CAUSE	4	/* Abort Cause */
#define U8500_I2C_SR_SHIFT_TYPE	7	/* Receive Type */
#define U8500_I2C_SR_SHIFT_LENGTH	9	/* Transfer length */

/* abort cause */
#define	U8500_I2C_NACK_ADDR	0
#define	U8500_I2C_NACK_DATA	1
#define	U8500_I2C_ACK_MCODE	2
#define	U8500_I2C_ARB_LOST	3
#define	U8500_I2C_BERR_START	4
#define	U8500_I2C_BERR_STOP	5
#define	U8500_I2C_OVFL	6


/* Baud rate counter registers */

/* Mask values for Baud rate counter register */
#define U8500_I2C_BRCR_BRCNT2	0xFFFF		/* Baud Rate Cntr BRCR for HS */
#define U8500_I2C_BRCR_BRCNT1	0xFFFF0000	/* BRCR for Standard and Fast */

/* Shift values for the Baud rate counter register */
#define U8500_I2C_BRCR_SHIFT_BRCNT2	0
#define U8500_I2C_BRCR_SHIFT_BRCNT1	16


/* Interrupt Register  */

/* Mask values for Interrupt registers */
#define U8500_I2C_INT_TXFE	0x00000001	/* Tx fifo empty */
#define U8500_I2C_INT_TXFNE	0x00000002	/* Tx Fifo nearly empty */
#define U8500_I2C_INT_TXFF	0x00000004	/* Tx Fifo Full */
#define U8500_I2C_INT_TXFOVR	0x00000008	/* Tx Fifo over run */
#define U8500_I2C_INT_RXFE	0x00000010	/* Rx Fifo Empty */
#define U8500_I2C_INT_RXFNF	0x00000020	/* Rx Fifo nearly empty */
#define U8500_I2C_INT_RXFF	0x00000040	/* Rx Fifo Full  */
#define U8500_I2C_INT_RFSR	0x00010000	/* Read From slave request */
#define U8500_I2C_INT_RFSE	0x00020000	/* Read from slave empty */
#define U8500_I2C_INT_WTSR	0x00040000	/* Write to Slave request */
#define U8500_I2C_INT_MTD	0x00080000	/* Master Transcation Done*/
#define U8500_I2C_INT_STD	0x00100000	/* Slave Transaction Done */
#define U8500_I2C_INT_MAL	0x01000000	/* Master Arbitation Lost */
#define U8500_I2C_INT_BERR	0x02000000	/* Bus Error */
#define U8500_I2C_INT_MTDWS	0x10000000	/* Master Tran Done wo/ Stop */

/* Max clocks (Hz) */
#define U8500_I2C_MAX_STANDARD_SCL	100000
#define U8500_I2C_MAX_FAST_SCL		400000
#define U8500_I2C_MAX_HIGH_SPEED_SCL	3400000

#endif	/* _U8500_I2C_H_ */
