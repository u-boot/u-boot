/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	Intel
 */

#ifndef __FSP_TYPES_H__
#define __FSP_TYPES_H__

/* 128 bit buffer containing a unique identifier value */
struct efi_guid {
	u32	data1;
	u16	data2;
	u16	data3;
	u8	data4[8];
};

/**
 * Returns a 16-bit signature built from 2 ASCII characters.
 *
 * This macro returns a 16-bit value built from the two ASCII characters
 * specified by A and B.
 *
 * @A: The first ASCII character.
 * @B: The second ASCII character.
 *
 * @return: A 16-bit value built from the two ASCII characters specified by
 *          A and B.
 */
#define SIGNATURE_16(A, B)	((A) | (B << 8))

/**
 * Returns a 32-bit signature built from 4 ASCII characters.
 *
 * This macro returns a 32-bit value built from the four ASCII characters
 * specified by A, B, C, and D.
 *
 * @A: The first ASCII character.
 * @B: The second ASCII character.
 * @C: The third ASCII character.
 * @D: The fourth ASCII character.
 *
 * @return: A 32-bit value built from the two ASCII characters specified by
 *          A, B, C and D.
 */
#define SIGNATURE_32(A, B, C, D)	\
	(SIGNATURE_16(A, B) | (SIGNATURE_16(C, D) << 16))

/**
 * Returns a 64-bit signature built from 8 ASCII characters.
 *
 * This macro returns a 64-bit value built from the eight ASCII characters
 * specified by A, B, C, D, E, F, G,and H.
 *
 * @A: The first ASCII character.
 * @B: The second ASCII character.
 * @C: The third ASCII character.
 * @D: The fourth ASCII character.
 * @E: The fifth ASCII character.
 * @F: The sixth ASCII character.
 * @G: The seventh ASCII character.
 * @H: The eighth ASCII character.
 *
 * @return: A 64-bit value built from the two ASCII characters specified by
 *          A, B, C, D, E, F, G and H.
 */
#define SIGNATURE_64(A, B, C, D, E, F, G, H)	\
	(SIGNATURE_32(A, B, C, D) | ((u64)(SIGNATURE_32(E, F, G, H)) << 32))

/*
 * Define FSP API return status code.
 * Compatiable with EFI_STATUS defined in PI Spec.
 */
#define FSP_SUCCESS		0
#define FSP_INVALID_PARAM	0x80000002
#define FSP_UNSUPPORTED		0x80000003
#define FSP_DEVICE_ERROR	0x80000007
#define FSP_NOT_FOUND		0x8000000E
#define FSP_ALREADY_STARTED	0x80000014

#endif
