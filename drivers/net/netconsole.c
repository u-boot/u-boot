// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <env.h>
#include <log.h>
#include <stdio_dev.h>
#include <net.h>
#include <vsprintf.h>

#include "netconsole.h"

#ifndef CFG_NETCONSOLE_BUFFER_SIZE
#define CFG_NETCONSOLE_BUFFER_SIZE 512
#endif

#define NC_DEFAULT_PORT 6666

static char input_buffer[CFG_NETCONSOLE_BUFFER_SIZE];
static int input_size; /* char count in input buffer */
static int input_offset; /* offset to valid chars in input buffer */
static int input_recursion;
static int output_recursion;
struct nc_settings nc_settings = {
	.ip.s_addr = ~0,
	.out_port = NC_DEFAULT_PORT,
	.in_port = NC_DEFAULT_PORT,
};

bool nc_is_broadcast(struct in_addr ip)
{
	static struct in_addr netmask;
	static struct in_addr our_ip;
	static int env_changed_id;
	int env_id = env_get_id();

	/* update only when the environment has changed */
	if (env_changed_id != env_id) {
		netmask = string_to_ip(env_get("netmask"));
		our_ip = string_to_ip(env_get("ipaddr"));

		env_changed_id = env_id;
	}

	return (ip.s_addr == ~0 || /* 255.255.255.255 (global bcast) */
		((netmask.s_addr & our_ip.s_addr) ==
		 (netmask.s_addr & ip.s_addr) && /* on the same net and */
		 (netmask.s_addr | ip.s_addr) == ~0)); /* bcast to our net */
}

int nc_refresh_settings_from_env(void)
{
	struct nc_settings settings = {
		.ip.s_addr = ~0,
		.out_port = NC_DEFAULT_PORT,
		.in_port = NC_DEFAULT_PORT,
	};
	const char *p;
	static int env_changed_id = -1;
	int env_id = env_get_id();

	/* update only when the environment has changed */
	if (env_changed_id != env_id) {
		char *tmp = env_get("ncip");

		if (tmp) {
			settings.ip = string_to_ip(tmp);
			if (!settings.ip.s_addr)
				return -1;	/* ncip is 0.0.0.0 */
			p = strchr(tmp, ':');
			if (p != NULL) {
				settings.out_port = dectoul(p + 1, NULL);
				settings.in_port = settings.out_port;
			}
		}

		p = env_get("ncoutport");
		if (p != NULL)
			settings.out_port = dectoul(p, NULL);
		p = env_get("ncinport");
		if (p != NULL)
			settings.in_port = dectoul(p, NULL);

		nc_settings = settings;
		env_changed_id = env_id;
		return 1;
	}

	return 0;
}

bool nc_has_input(void)
{
	return input_size != 0;
}

void nc_set_input_recursion(bool enable)
{
	input_recursion = enable;
}

int nc_input_packet(uchar *pkt, struct in_addr src_ip, unsigned dest_port,
	unsigned src_port, unsigned len)
{
	int end, chunk;

	if (dest_port != nc_settings.in_port || !len)
		return 0; /* not for us */

	if (src_ip.s_addr != nc_settings.ip.s_addr &&
	    !nc_is_broadcast(nc_settings.ip))
		return 0; /* not from our client */

	debug_cond(DEBUG_NET_PKT_TRACE, "input: \"%*.*s\"\n", len, len,
		   pkt);

	if (input_size == sizeof(input_buffer))
		return 1; /* no space */
	if (len > sizeof(input_buffer) - input_size)
		len = sizeof(input_buffer) - input_size;

	end = input_offset + input_size;
	if (end >= sizeof(input_buffer))
		end -= sizeof(input_buffer);

	chunk = len;
	/* Check if packet will wrap in input_buffer */
	if (end + len >= sizeof(input_buffer)) {
		chunk = sizeof(input_buffer) - end;
		/* Copy the second part of the pkt to start of input_buffer */
		memcpy(input_buffer, pkt + chunk, len - chunk);
	}
	/* Copy first (or only) part of pkt after end of current valid input*/
	memcpy(input_buffer + end, pkt, chunk);

	input_size += len;

	return 1;
}

static int nc_stdio_start(struct stdio_dev *dev)
{
	return nc_transport_start();
}

static int nc_stdio_stop(struct stdio_dev *dev)
{
	return nc_transport_stop();
}

static void nc_stdio_putc(struct stdio_dev *dev, char c)
{
	if (output_recursion)
		return;
	output_recursion = 1;

	nc_transport_send(&c, 1);

	output_recursion = 0;
}

static void nc_stdio_puts(struct stdio_dev *dev, const char *s)
{
	int len;

	if (output_recursion)
		return;
	output_recursion = 1;

	len = strlen(s);
	while (len) {
		int send_len = min(len, (int)sizeof(input_buffer));
		nc_transport_send(s, send_len);
		len -= send_len;
		s += send_len;
	}

	output_recursion = 0;
}

static int nc_stdio_getc(struct stdio_dev *dev)
{
	uchar c;

	input_recursion = 1;

	while (!input_size)
		nc_transport_poll(true);

	input_recursion = 0;

	c = input_buffer[input_offset++];

	if (input_offset >= sizeof(input_buffer))
		input_offset -= sizeof(input_buffer);
	input_size--;

	return c;
}

static int nc_stdio_tstc(struct stdio_dev *dev)
{
	if (input_recursion)
		return 0;

	if (input_size)
		return 1;

	input_recursion = 1;
	nc_transport_poll(false);
	input_recursion = 0;

	return input_size != 0;
}

int drv_nc_init(void)
{
	struct stdio_dev dev;
	int rc;

	memset(&dev, 0, sizeof(dev));

	strcpy(dev.name, "nc");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;
	dev.start = nc_stdio_start;
	dev.stop = nc_stdio_stop;
	dev.putc = nc_stdio_putc;
	dev.puts = nc_stdio_puts;
	dev.getc = nc_stdio_getc;
	dev.tstc = nc_stdio_tstc;

	rc = stdio_register(&dev);

	return (rc == 0) ? 1 : rc;
}
