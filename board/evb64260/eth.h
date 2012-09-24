/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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

/*
 * eth.h - header file for the polled mode GT ethernet driver
 */

#ifndef __GT6426x_ETH_H__
#define __GT6426x_ETH_H__

#include <asm/types.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <common.h>

typedef struct eth0_tx_desc_struct {
	volatile __u32 bytecount_reserved;
	volatile __u32 command_status;
	volatile struct eth0_tx_desc_struct * next_desc;
	/* Note - the following will not work for 64 bit addressing */
	volatile unsigned char * buff_pointer;
} __attribute__ ((packed)) eth0_tx_desc_single;

typedef struct eth0_rx_desc_struct {
  volatile __u32 buff_size_byte_count;
  volatile __u32 command_status;
  volatile struct eth0_rx_desc_struct * next_desc;
  volatile unsigned char * buff_pointer;
} __attribute__ ((packed)) eth0_rx_desc_single;

#define NT 20 /* Number of Transmit buffers */
#define NR 20 /* Number of Receive buffers */
#define MAX_BUFF_SIZE (1536+2*CACHE_LINE_SIZE) /* 1600 */
#define ETHERNET_PORTS_DIFFERENCE_OFFSETS 0x400

unsigned long TDN_ETH0 , RDN_ETH0; /* Rx/Tx current Descriptor Number*/
unsigned int EVB64260_ETH0_irq;

#define CLOSED 0
#define OPENED 1

#define PORT_ETH0 0

extern eth0_tx_desc_single *eth0_tx_desc;
extern eth0_rx_desc_single *eth0_rx_desc;
extern char *eth0_tx_buffer;
extern char *eth0_rx_buffer[NR];
extern char *eth_data;

extern int gt6426x_eth_poll(void *v);
extern int gt6426x_eth_transmit(void *v, char *p, unsigned int s);
extern void gt6426x_eth_disable(void *v);
extern int gt6426x_eth_probe(void *v, bd_t *bis);

#endif  /* __GT64260x_ETH_H__ */
