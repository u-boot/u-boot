/*
 * Header file for SHA hardware acceleration
 *
 * Copyright (c) 2012  Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __HW_SHA_H
#define __HW_SHA_H


/**
 * Computes hash value of input pbuf using h/w acceleration
 *
 * @param in_addr	A pointer to the input buffer
 * @param bufleni	Byte length of input buffer
 * @param out_addr	A pointer to the output buffer. When complete
 *			32 bytes are copied to pout[0]...pout[31]. Thus, a user
 *			should allocate at least 32 bytes at pOut in advance.
 * @param chunk_size	chunk size for sha256
 */
void hw_sha256(const uchar * in_addr, uint buflen,
			uchar * out_addr, uint chunk_size);

/**
 * Computes hash value of input pbuf using h/w acceleration
 *
 * @param in_addr	A pointer to the input buffer
 * @param bufleni	Byte length of input buffer
 * @param out_addr	A pointer to the output buffer. When complete
 *			32 bytes are copied to pout[0]...pout[31]. Thus, a user
 *			should allocate at least 32 bytes at pOut in advance.
 * @param chunk_size	chunk_size for sha1
 */
void hw_sha1(const uchar * in_addr, uint buflen,
			uchar * out_addr, uint chunk_size);
#endif
