/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _UBOOT_CRC_H
#define _UBOOT_CRC_H

/* lib/crc8.c */
unsigned int crc8(unsigned int crc_start, const unsigned char *vptr, int len);

/* lib/crc32.c */
uint32_t crc32 (uint32_t, const unsigned char *, uint);
uint32_t crc32_wd (uint32_t, const unsigned char *, uint, uint);
uint32_t crc32_no_comp (uint32_t, const unsigned char *, uint);

/**
 * crc32_wd_buf - Perform CRC32 on a buffer and return result in buffer
 *
 * @input:	Input buffer
 * @ilen:	Input buffer length
 * @output:	Place to put checksum result (4 bytes)
 * @chunk_sz:	Trigger watchdog after processing this many bytes
 */
void crc32_wd_buf(const unsigned char *input, uint ilen,
		    unsigned char *output, uint chunk_sz);

#endif /* _UBOOT_CRC_H */
