/*
 * Altera 10/100/1000 triple speed ethernet mac
 *
 * Copyright (C) 2008 Altera Corporation.
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _ALTERA_TSE_H_
#define _ALTERA_TSE_H_

#define __packed_1_    __attribute__ ((packed, aligned(1)))

/* SGDMA Stuff */
#define ALT_SGDMA_STATUS_BUSY_MSK			(0x00000010)

#define ALT_SGDMA_CONTROL_RUN_MSK			(0x00000020)
#define ALT_SGDMA_CONTROL_STOP_DMA_ER_MSK		(0x00000040)
#define ALT_SGDMA_CONTROL_SOFTWARERESET_MSK		(0x00010000)

/*
 * Descriptor control bit masks & offsets
 *
 * Note: The control byte physically occupies bits [31:24] in memory.
 *	 The following bit-offsets are expressed relative to the LSB of
 *	 the control register bitfield.
 */
#define ALT_SGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MSK		(0x00000001)
#define ALT_SGDMA_DESCRIPTOR_CONTROL_READ_FIXED_ADDRESS_MSK	(0x00000002)
#define ALT_SGDMA_DESCRIPTOR_CONTROL_WRITE_FIXED_ADDRESS_MSK	(0x00000004)
#define ALT_SGDMA_DESCRIPTOR_CONTROL_OWNED_BY_HW_MSK		(0x00000080)

/*
 * Descriptor status bit masks & offsets
 *
 * Note: The status byte physically occupies bits [23:16] in memory.
 *	 The following bit-offsets are expressed relative to the LSB of
 *	 the status register bitfield.
 */
#define ALT_SGDMA_DESCRIPTOR_STATUS_TERMINATED_BY_EOP_MSK	(0x00000080)

/*
 * The SGDMA controller buffer descriptor allocates
 * 64 bits for each address. To support ANSI C, the
 * struct implementing a descriptor places 32-bits
 * of padding directly above each address; each pad must
 * be cleared when initializing a descriptor.
 */

/*
 * Buffer Descriptor data structure
 *
 */
struct alt_sgdma_descriptor {
	unsigned int source;	/* the address of data to be read. */
	unsigned int source_pad;

	unsigned int destination;	/* the address to write data */
	unsigned int destination_pad;

	unsigned int next;	/* the next descriptor in the list. */
	unsigned int next_pad;

	unsigned short bytes_to_transfer; /* the number of bytes to transfer */
	unsigned char read_burst;
	unsigned char write_burst;

	unsigned short actual_bytes_transferred;/* bytes transferred by DMA */
	unsigned char descriptor_status;
	unsigned char descriptor_control;

} __packed_1_;

/* SG-DMA Control/Status Slave registers map */

struct alt_sgdma_registers {
	unsigned int status;
	unsigned int status_pad[3];
	unsigned int control;
	unsigned int control_pad[3];
	unsigned int next_descriptor_pointer;
	unsigned int descriptor_pad[3];
};

/* TSE Stuff */
#define ALTERA_TSE_CMD_TX_ENA_MSK		(0x00000001)
#define ALTERA_TSE_CMD_RX_ENA_MSK		(0x00000002)
#define ALTERA_TSE_CMD_ETH_SPEED_MSK		(0x00000008)
#define ALTERA_TSE_CMD_HD_ENA_MSK		(0x00000400)
#define ALTERA_TSE_CMD_SW_RESET_MSK		(0x00002000)
#define ALTERA_TSE_CMD_ENA_10_MSK		(0x02000000)

#define ALT_TSE_SW_RESET_TIMEOUT		(3 * CONFIG_SYS_HZ)
#define ALT_TSE_SGDMA_BUSY_TIMEOUT		(3 * CONFIG_SYS_HZ)

/* MAC register Space */

struct alt_tse_mac {
	unsigned int megacore_revision;
	unsigned int scratch_pad;
	unsigned int command_config;
	unsigned int mac_addr_0;
	unsigned int mac_addr_1;
	unsigned int max_frame_length;
	unsigned int pause_quanta;
	unsigned int rx_sel_empty_threshold;
	unsigned int rx_sel_full_threshold;
	unsigned int tx_sel_empty_threshold;
	unsigned int tx_sel_full_threshold;
	unsigned int rx_almost_empty_threshold;
	unsigned int rx_almost_full_threshold;
	unsigned int tx_almost_empty_threshold;
	unsigned int tx_almost_full_threshold;
	unsigned int mdio_phy0_addr;
	unsigned int mdio_phy1_addr;

	unsigned int reserved1[0x29];

	/*FIFO control register. */
	unsigned int tx_cmd_stat;
	unsigned int rx_cmd_stat;

	unsigned int reserved2[0x44];

	/*Registers 0 to 31 within PHY device 0/1 */
	unsigned int mdio_phy0[0x20];
	unsigned int mdio_phy1[0x20];

	/*4 Supplemental MAC Addresses */
	unsigned int supp_mac_addr_0_0;
	unsigned int supp_mac_addr_0_1;
	unsigned int supp_mac_addr_1_0;
	unsigned int supp_mac_addr_1_1;
	unsigned int supp_mac_addr_2_0;
	unsigned int supp_mac_addr_2_1;
	unsigned int supp_mac_addr_3_0;
	unsigned int supp_mac_addr_3_1;

	unsigned int reserved3[0x38];
};

struct altera_tse_priv {
	struct alt_tse_mac *mac_dev;
	struct alt_sgdma_registers *sgdma_rx;
	struct alt_sgdma_registers *sgdma_tx;
	unsigned int rx_fifo_depth;
	unsigned int tx_fifo_depth;
	struct alt_sgdma_descriptor *rx_desc;
	struct alt_sgdma_descriptor *tx_desc;
	unsigned char *rx_buf;
	unsigned int phyaddr;
	unsigned int interface;
	struct phy_device *phydev;
	struct mii_dev *bus;
};

#endif /* _ALTERA_TSE_H_ */
