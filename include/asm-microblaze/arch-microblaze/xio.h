/*
 * xio.h
 *
 * Defines XIo functions for Xilinx OCP in terms of Linux primitives
 *
 * Author: MontaVista Software, Inc.
 *         source@mvista.com
 *
 * 2002 (c) MontaVista, Software, Inc.  This file is licensed under the terms
 * of the GNU General Public License version 2.  This program is licensed
 * "as is" without any warranty of any kind, whether express or implied.
 */

#ifndef XIO_H
#define XIO_H

#include "xbasic_types.h"
#include <asm/io.h>

typedef u32 XIo_Address;

extern inline u8
XIo_In8(XIo_Address InAddress)
{
	return (u8) in_8((volatile unsigned char *) InAddress);
}
extern inline u16
XIo_In16(XIo_Address InAddress)
{
	return (u16) in_be16((volatile unsigned short *) InAddress);
}
extern inline u32
XIo_In32(XIo_Address InAddress)
{
	return (u32) in_be32((volatile unsigned *) InAddress);
}
extern inline void
XIo_Out8(XIo_Address OutAddress, u8 Value)
{
	out_8((volatile unsigned char *) OutAddress, Value);
}
extern inline void
XIo_Out16(XIo_Address OutAddress, u16 Value)
{
	out_be16((volatile unsigned short *) OutAddress, Value);
}
extern inline void
XIo_Out32(XIo_Address OutAddress, u32 Value)
{
	out_be32((volatile unsigned *) OutAddress, Value);
}

#define XIo_ToLittleEndian16(s,d) (*(u16*)(d) = cpu_to_le16((u16)(s)))
#define XIo_ToLittleEndian32(s,d) (*(u32*)(d) = cpu_to_le32((u32)(s)))
#define XIo_ToBigEndian16(s,d) (*(u16*)(d) = cpu_to_be16((u16)(s)))
#define XIo_ToBigEndian32(s,d) (*(u32*)(d) = cpu_to_be32((u32)(s)))

#define XIo_FromLittleEndian16(s,d) (*(u16*)(d) = le16_to_cpu((u16)(s)))
#define XIo_FromLittleEndian32(s,d) (*(u32*)(d) = le32_to_cpu((u32)(s)))
#define XIo_FromBigEndian16(s,d) (*(u16*)(d) = be16_to_cpu((u16)(s)))
#define XIo_FromBigEndian32(s,d) (*(u32*)(d) = be32_to_cpu((u32)(s)))

#endif				/* XIO_H */
