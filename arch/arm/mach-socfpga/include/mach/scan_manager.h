/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	_SCAN_MANAGER_H_
#define	_SCAN_MANAGER_H_

struct socfpga_scan_manager {
	u32	stat;
	u32	en;
	u32	padding[2];
	u32	fifo_single_byte;
	u32	fifo_double_byte;
	u32	fifo_triple_byte;
	u32	fifo_quad_byte;
};

/*
 * Shift count to get number of IO scan chain data in granularity
 * of 128-bit ( N / 128 )
 */
#define IO_SCAN_CHAIN_128BIT_SHIFT		7

/*
 * Mask to get residual IO scan chain data in
 * granularity of 128-bit ( N mod 128 )
 */
#define IO_SCAN_CHAIN_128BIT_MASK		0x7F

/*
 * Shift count to get number of IO scan chain
 * data in granularity of 32-bit ( N / 32 )
 */
#define IO_SCAN_CHAIN_32BIT_SHIFT		5

/*
 * Mask to get residual IO scan chain data in
 * granularity of 32-bit ( N mod 32 )
 */
#define IO_SCAN_CHAIN_32BIT_MASK		0x1F

/* Byte mask */
#define IO_SCAN_CHAIN_BYTE_MASK			0xFF

/* 24-bits (3 bytes) IO scan chain payload definition */
#define IO_SCAN_CHAIN_PAYLOAD_24BIT		24

/*
 * Maximum length of TDI_TDO packet payload is 128 bits,
 * represented by (length - 1) in TDI_TDO header
 */
#define TDI_TDO_MAX_PAYLOAD			127

/* TDI_TDO packet header for IO scan chain program */
#define TDI_TDO_HEADER_FIRST_BYTE		0x80

/* Position of second command byte for TDI_TDO packet */
#define TDI_TDO_HEADER_SECOND_BYTE_SHIFT	8

int scan_mgr_configure_iocsr(void);
int iocsr_get_config_table(const unsigned int chain_id,
			   const unsigned long **table,
			   unsigned int *table_len);

#endif /* _SCAN_MANAGER_H_ */
