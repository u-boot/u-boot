/* Gaisler.com GRETH 10/100/1000 Ethernet MAC driver
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
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

#define GRETH_FD 0x10
#define GRETH_RESET 0x40
#define GRETH_MII_BUSY 0x8
#define GRETH_MII_NVALID 0x10

/* MII registers */
#define GRETH_MII_EXTADV_1000FD 0x00000200
#define GRETH_MII_EXTADV_1000HD 0x00000100
#define GRETH_MII_EXTPRT_1000FD 0x00000800
#define GRETH_MII_EXTPRT_1000HD 0x00000400

#define GRETH_MII_100T4 0x00000200
#define GRETH_MII_100TXFD 0x00000100
#define GRETH_MII_100TXHD 0x00000080
#define GRETH_MII_10FD 0x00000040
#define GRETH_MII_10HD 0x00000020

#define GRETH_BD_EN 0x800
#define GRETH_BD_WR 0x1000
#define GRETH_BD_IE 0x2000
#define GRETH_BD_LEN 0x7FF

#define GRETH_TXEN 0x1
#define GRETH_INT_TX 0x8
#define GRETH_TXI 0x4
#define GRETH_TXBD_STATUS 0x0001C000
#define GRETH_TXBD_MORE 0x20000
#define GRETH_TXBD_IPCS 0x40000
#define GRETH_TXBD_TCPCS 0x80000
#define GRETH_TXBD_UDPCS 0x100000
#define GRETH_TXBD_ERR_LC 0x10000
#define GRETH_TXBD_ERR_UE 0x4000
#define GRETH_TXBD_ERR_AL 0x8000
#define GRETH_TXBD_NUM 128
#define GRETH_TXBD_NUM_MASK (GRETH_TXBD_NUM-1)
#define GRETH_TX_BUF_SIZE 2048

#define GRETH_INT_RX         0x4
#define GRETH_RXEN           0x2
#define GRETH_RXI            0x8
#define GRETH_RXBD_STATUS    0xFFFFC000
#define GRETH_RXBD_ERR_AE    0x4000
#define GRETH_RXBD_ERR_FT    0x8000
#define GRETH_RXBD_ERR_CRC   0x10000
#define GRETH_RXBD_ERR_OE    0x20000
#define GRETH_RXBD_ERR_LE    0x40000
#define GRETH_RXBD_IP_DEC    0x80000
#define GRETH_RXBD_IP_CSERR  0x100000
#define GRETH_RXBD_UDP_DEC   0x200000
#define GRETH_RXBD_UDP_CSERR 0x400000
#define GRETH_RXBD_TCP_DEC   0x800000
#define GRETH_RXBD_TCP_CSERR 0x1000000

#define GRETH_RXBD_NUM 128
#define GRETH_RXBD_NUM_MASK (GRETH_RXBD_NUM-1)
#define GRETH_RX_BUF_SIZE 2048

/* Ethernet configuration registers */
typedef struct _greth_regs {
	volatile unsigned int control;
	volatile unsigned int status;
	volatile unsigned int esa_msb;
	volatile unsigned int esa_lsb;
	volatile unsigned int mdio;
	volatile unsigned int tx_desc_p;
	volatile unsigned int rx_desc_p;
} greth_regs;

/* Ethernet buffer descriptor */
typedef struct _greth_bd {
	volatile unsigned int stat;
	unsigned int addr;	/* Buffer address not changed by HW */
} greth_bd;
