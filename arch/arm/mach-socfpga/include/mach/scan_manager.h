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

/*
 * Maximum polling loop to wait for IO scan chain engine
 * becomes idle to prevent infinite loop
 */
#define SCAN_MAX_DELAY				100

#define SCANMGR_STAT_ACTIVE_GET(x) (((x) & 0x80000000) >> 31)
#define SCANMGR_STAT_WFIFOCNT_GET(x) (((x) & 0x70000000) >> 28)

/*
 * Program HPS IO Scan Chain
 * io_scan_chain_id - IO scan chain ID
 * io_scan_chain_len_in_bits - IO scan chain length in bits
 * iocsr_scan_chain - IO scan chain table
 */
uint32_t scan_mgr_io_scan_chain_prg(
	uint32_t io_scan_chain_id,
	uint32_t io_scan_chain_len_in_bits,
	const uint32_t *iocsr_scan_chain);

extern const uint32_t iocsr_scan_chain0_table[
	((CONFIG_HPS_IOCSR_SCANCHAIN0_LENGTH / 32) + 1)];
extern const uint32_t iocsr_scan_chain1_table[
	((CONFIG_HPS_IOCSR_SCANCHAIN1_LENGTH / 32) + 1)];
extern const uint32_t iocsr_scan_chain2_table[
	((CONFIG_HPS_IOCSR_SCANCHAIN2_LENGTH / 32) + 1)];
extern const uint32_t iocsr_scan_chain3_table[
	((CONFIG_HPS_IOCSR_SCANCHAIN3_LENGTH / 32) + 1)];

int scan_mgr_configure_iocsr(void);

#endif /* _SCAN_MANAGER_H_ */
