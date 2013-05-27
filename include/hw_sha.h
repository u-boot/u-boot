/*
 * Header file for SHA hardware acceleration
 *
 * Copyright (c) 2012  Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
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
