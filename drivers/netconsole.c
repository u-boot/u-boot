/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <common.h>

#ifdef CONFIG_NETCONSOLE

#include <command.h>
#include <devices.h>
#include <net.h>

#ifndef CONFIG_NET_MULTI
#error define CONFIG_NET_MULTI to use netconsole
#endif

static uchar nc_buf = 0;	/* input buffer */
static int input_recursion = 0;
static int output_recursion = 0;
static int net_timeout;

static void nc_wait_arp_handler (uchar * pkt, unsigned dest, unsigned src,
				 unsigned len)
{
	NetState = NETLOOP_SUCCESS;	/* got arp reply - quit net loop */
}

static void nc_handler (uchar * pkt, unsigned dest, unsigned src,
			unsigned len)
{
	if (nc_buf)
		NetState = NETLOOP_SUCCESS;	/* got input - quit net loop */
}

static void nc_timeout (void)
{
	NetState = NETLOOP_SUCCESS;
}

void NcStart (void)
{
	if (memcmp (NetServerEther, NetEtherNullAddr, 6)) {
		/* going to check for input packet */
		NetSetHandler (nc_handler);
		NetSetTimeout (net_timeout, nc_timeout);
	} else {
		/* send arp request */
		NetSetHandler (nc_wait_arp_handler);
		NetSendUDPPacket (NetServerEther, NetServerIP, 6665, 6666, 0);
	}
}

int nc_input_packet (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	if (dest != 6666 || !len)
		return 0;		/* not for us */

	nc_buf = *pkt;
	return 1;
}

static void nc_send_packet (const char *buf, int len)
{
	DECLARE_GLOBAL_DATA_PTR;

	struct eth_device *eth;
	int inited = 0;
	uchar *pkt;

	if (!memcmp (NetServerEther, NetEtherNullAddr, 6))
		return;

	if ((eth = eth_get_dev ()) == NULL) {
		return;
	}

	if (eth->state != ETH_STATE_ACTIVE) {
		if (eth_init (gd->bd) < 0)
			return;
		inited = 1;
	}
	pkt = (uchar *) NetTxPacket + NetEthHdrSize () + IP_HDR_SIZE;
	memcpy (pkt, buf, len);
	NetSendUDPPacket (NetServerEther, NetServerIP, 6666, 6665, len);

	if (inited)
		eth_halt ();
}

int nc_start (void)
{
	if (memcmp (NetServerEther, NetEtherNullAddr, 6))
		return 0;

	return NetLoop (NETCONS);	/* wait for arp reply */
}

void nc_putc (char c)
{
	if (output_recursion)
		return;
	output_recursion = 1;

	nc_send_packet (&c, 1);

	output_recursion = 0;
}

void nc_puts (const char *s)
{
	if (output_recursion)
		return;
	output_recursion = 1;

	int len = strlen (s);

	if (len > 512)
		len = 512;

	nc_send_packet (s, len);

	output_recursion = 0;
}

int nc_getc (void)
{
	input_recursion = 1;

	net_timeout = 0;	/* no timeout */
	while (!nc_buf)
		NetLoop (NETCONS);

	input_recursion = 0;

	uchar tmp = nc_buf;

	nc_buf = 0;
	return tmp;
}

int nc_tstc (void)
{
	struct eth_device *eth;

	if (input_recursion)
		return 0;

	if (nc_buf)
		return 1;

	eth = eth_get_dev ();
	if (eth && eth->state == ETH_STATE_ACTIVE)
		return 0;	/* inside net loop */

	input_recursion = 1;

	net_timeout = 1;
	NetLoop (NETCONS);		/* kind of poll */

	input_recursion = 0;

	return nc_buf != 0;
}

int drv_nc_init (void)
{
	device_t dev;
	int rc;

	memset (&dev, 0, sizeof (dev));

	strcpy (dev.name, "nc");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	dev.start = nc_start;
	dev.putc = nc_putc;
	dev.puts = nc_puts;
	dev.getc = nc_getc;
	dev.tstc = nc_tstc;

	rc = device_register (&dev);

	return (rc == 0) ? 1 : rc;
}

#endif	/* CONFIG_NETCONSOLE */
