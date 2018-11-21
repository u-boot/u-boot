// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach, Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>
#include <command.h>
#include <console.h>

#include <gdsys_fpga.h>

enum {
	STATE_TX_PACKET_BUILDING = 1<<0,
	STATE_TX_TRANSMITTING = 1<<1,
	STATE_TX_BUFFER_FULL = 1<<2,
	STATE_TX_ERR = 1<<3,
	STATE_RECEIVE_TIMEOUT = 1<<4,
	STATE_PROC_RX_STORE_TIMEOUT = 1<<5,
	STATE_PROC_RX_RECEIVE_TIMEOUT = 1<<6,
	STATE_RX_DIST_ERR = 1<<7,
	STATE_RX_LENGTH_ERR = 1<<8,
	STATE_RX_FRAME_CTR_ERR = 1<<9,
	STATE_RX_FCS_ERR = 1<<10,
	STATE_RX_PACKET_DROPPED = 1<<11,
	STATE_RX_DATA_LAST = 1<<12,
	STATE_RX_DATA_FIRST = 1<<13,
	STATE_RX_DATA_AVAILABLE = 1<<15,
};

enum {
	CTRL_PROC_RECEIVE_ENABLE = 1<<12,
	CTRL_FLUSH_TRANSMIT_BUFFER = 1<<15,
};

enum {
	IRQ_CPU_TRANSMITBUFFER_FREE_STATUS = 1<<5,
	IRQ_CPU_PACKET_TRANSMITTED_EVENT = 1<<6,
	IRQ_NEW_CPU_PACKET_RECEIVED_EVENT = 1<<7,
	IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS = 1<<8,
};

struct io_generic_packet {
	u16 target_address;
	u16 source_address;
	u8 packet_type;
	u8 bc;
	u16 packet_length;
} __attribute__((__packed__));

unsigned long long rx_ctr;
unsigned long long tx_ctr;
unsigned long long err_ctr;

static void io_check_status(unsigned int fpga, u16 status, bool silent)
{
	u16 mask = STATE_RX_DIST_ERR | STATE_RX_LENGTH_ERR |
		   STATE_RX_FRAME_CTR_ERR | STATE_RX_FCS_ERR |
		   STATE_RX_PACKET_DROPPED | STATE_TX_ERR;

	if (!(status & mask)) {
		FPGA_SET_REG(fpga, ep.rx_tx_status, status);
		return;
	}

	err_ctr++;
	FPGA_SET_REG(fpga, ep.rx_tx_status, status);

	if (silent)
		return;

	if (status & STATE_RX_PACKET_DROPPED)
		printf("RX_PACKET_DROPPED, status %04x\n", status);

	if (status & STATE_RX_DIST_ERR)
		printf("RX_DIST_ERR\n");
	if (status & STATE_RX_LENGTH_ERR)
		printf("RX_LENGTH_ERR\n");
	if (status & STATE_RX_FRAME_CTR_ERR)
		printf("RX_FRAME_CTR_ERR\n");
	if (status & STATE_RX_FCS_ERR)
		printf("RX_FCS_ERR\n");

	if (status & STATE_TX_ERR)
		printf("TX_ERR\n");
}

static void io_send(unsigned int fpga, unsigned int size)
{
	unsigned int k;
	struct io_generic_packet packet = {
		.source_address = 1,
		.packet_type = 1,
		.packet_length = size,
	};
	u16 *p = (u16 *)&packet;

	for (k = 0; k < sizeof(packet) / 2; ++k)
		FPGA_SET_REG(fpga, ep.transmit_data, *p++);

	for (k = 0; k < (size + 1) / 2; ++k)
		FPGA_SET_REG(fpga, ep.transmit_data, k);

	FPGA_SET_REG(fpga, ep.rx_tx_control,
		     CTRL_PROC_RECEIVE_ENABLE | CTRL_FLUSH_TRANSMIT_BUFFER);

	tx_ctr++;
}

static void io_receive(unsigned int fpga)
{
	unsigned int k = 0;
	u16 rx_tx_status;

	FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);

	while (rx_tx_status & STATE_RX_DATA_AVAILABLE) {
		u16 rx;

		if (rx_tx_status & STATE_RX_DATA_LAST)
			rx_ctr++;

		FPGA_GET_REG(fpga, ep.receive_data, &rx);

		FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);

		++k;
	}
}

static void io_reflect(unsigned int fpga)
{
	u16 buffer[128];

	unsigned int k = 0;
	unsigned int n;
	u16 rx_tx_status;

	FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);

	while (rx_tx_status & STATE_RX_DATA_AVAILABLE) {
		FPGA_GET_REG(fpga, ep.receive_data, &buffer[k++]);
		if (rx_tx_status & STATE_RX_DATA_LAST)
			break;

		FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);
	}

	if (!k)
		return;

	for (n = 0; n < k; ++n)
		FPGA_SET_REG(fpga, ep.transmit_data, buffer[n]);

	FPGA_SET_REG(fpga, ep.rx_tx_control,
		     CTRL_PROC_RECEIVE_ENABLE | CTRL_FLUSH_TRANSMIT_BUFFER);

	tx_ctr++;
}

/*
 * FPGA io-endpoint reflector
 *
 * Syntax:
 *	ioreflect {fpga} {reportrate}
 */
int do_ioreflect(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int fpga;
	unsigned int rate = 0;
	unsigned long long last_seen = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	fpga = simple_strtoul(argv[1], NULL, 10);

	/*
	 * If another parameter, it is the report rate in packets.
	 */
	if (argc > 2)
		rate = simple_strtoul(argv[2], NULL, 10);

	/* enable receive path */
	FPGA_SET_REG(fpga, ep.rx_tx_control, CTRL_PROC_RECEIVE_ENABLE);

	/* set device address to dummy 1*/
	FPGA_SET_REG(fpga, ep.device_address, 1);

	rx_ctr = 0; tx_ctr = 0; err_ctr = 0;

	while (1) {
		u16 top_int;
		u16 rx_tx_status;

		FPGA_GET_REG(fpga, top_interrupt, &top_int);
		FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);

		io_check_status(fpga, rx_tx_status, true);
		if ((top_int & IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS) &&
		    (top_int & IRQ_CPU_TRANSMITBUFFER_FREE_STATUS))
			io_reflect(fpga);

		if (rate) {
			if (!(tx_ctr % rate) && (tx_ctr != last_seen))
				printf("refl %llu, err %llu\n", tx_ctr,
				       err_ctr);
			last_seen = tx_ctr;
		}

		if (ctrlc())
			break;
	}

	return 0;
}

/*
 * FPGA io-endpoint looptest
 *
 * Syntax:
 *	ioloop {fpga} {size} {rate}
 */
#define DISP_LINE_LEN	16
int do_ioloop(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int fpga;
	unsigned int size;
	unsigned int rate = 0;

	if (argc < 3)
		return CMD_RET_USAGE;

	/*
	 * FPGA is specified since argc > 2
	 */
	fpga = simple_strtoul(argv[1], NULL, 10);

	/*
	 * packet size is specified since argc > 2
	 */
	size = simple_strtoul(argv[2], NULL, 10);

	/*
	 * If another parameter, it is the test rate in packets per second.
	 */
	if (argc > 3)
		rate = simple_strtoul(argv[3], NULL, 10);

	/* enable receive path */
	FPGA_SET_REG(fpga, ep.rx_tx_control, CTRL_PROC_RECEIVE_ENABLE);

	/* set device address to dummy 1*/
	FPGA_SET_REG(fpga, ep.device_address, 1);

	rx_ctr = 0; tx_ctr = 0; err_ctr = 0;

	while (1) {
		u16 top_int;
		u16 rx_tx_status;

		FPGA_GET_REG(fpga, top_interrupt, &top_int);
		FPGA_GET_REG(fpga, ep.rx_tx_status, &rx_tx_status);

		io_check_status(fpga, rx_tx_status, false);
		if (top_int & IRQ_CPU_TRANSMITBUFFER_FREE_STATUS)
			io_send(fpga, size);
		if (top_int & IRQ_CPU_RECEIVE_DATA_AVAILABLE_STATUS)
			io_receive(fpga);

		if (rate) {
			if (ctrlc())
				break;
			udelay(1000000 / rate);
			if (!(tx_ctr % rate))
				printf("d %lld, tx %llu, rx %llu, err %llu\n",
				       tx_ctr - rx_ctr, tx_ctr, rx_ctr,
				       err_ctr);
		}
	}

	return 0;
}

U_BOOT_CMD(
	ioloop,	4,	0,	do_ioloop,
	"fpga io-endpoint looptest",
	"fpga packetsize [packets/sec]"
);

U_BOOT_CMD(
	ioreflect, 3,	0,	do_ioreflect,
	"fpga io-endpoint reflector",
	"fpga reportrate"
);
