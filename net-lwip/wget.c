// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <display_options.h>
#include <image.h>
#include <lwip/apps/http_client.h>
#include <lwip/timeouts.h>
#include <net-lwip.h>
#include <time.h>

#define SERVER_NAME_SIZE 200
#define HTTP_PORT_DEFAULT 80

static ulong daddr;
static ulong saved_daddr;
static ulong size;
static ulong prevsize;
#define PROGRESS_PRINT_STEP_BYTES (100 * 1024)
static ulong start_time;
static enum done_state {
        NOT_DONE = 0,
        SUCCESS = 1,
        FAILURE = 2
} done;

static int parse_url(char *url, char *host, u16 *port, char **path)
{
	char *p, *pp;
	long lport;

	p = strstr(url, "http://");
	if (!p)
		return -EINVAL;

	p += strlen("http://");

	/* Parse hostname */
	pp = strchr(p, ':');
	if (!pp)
		pp = strchr(p, '/');
	if (!pp)
		return -EINVAL;

	if (p + SERVER_NAME_SIZE <= pp)
		return -EINVAL;

	memcpy(host, p, pp - p);
	host[pp - p + 1] = '\0';

	if (*pp == ':') {
		/* Parse port number */
		p = pp + 1;
		lport = simple_strtol(p, &pp, 10);
		if (pp && *pp != '/')
			return -EINVAL;
		if (lport > 65535)
			return -EINVAL;
		*port = (u16)lport;
	} else {
		*port = HTTP_PORT_DEFAULT;
	}
	if (*pp != '/')
		return -EINVAL;
	*path = pp;

	return 0;
}

static err_t httpc_recv_cb(void *arg, struct altcp_pcb *pcb, struct pbuf *pbuf,
			   err_t err)
{
	struct pbuf *buf;

	if (!pbuf)
		return ERR_BUF;

	for (buf = pbuf; buf; buf = buf->next) {
		memcpy((void *)daddr, buf->payload, buf->len);
		daddr += buf->len;
		size += buf->len;
		if (size - prevsize > PROGRESS_PRINT_STEP_BYTES) {
			printf("#");
			prevsize = size;
		}
	}

	altcp_recved(pcb, pbuf->tot_len);
	pbuf_free(pbuf);
	return ERR_OK;
}

static void httpc_result_cb(void *arg, httpc_result_t httpc_result,
			    u32_t rx_content_len, u32_t srv_res, err_t err)
{
	ulong elapsed;

	if (httpc_result != HTTPC_RESULT_OK) {
		log_err("\nHTTP client error %d\n", httpc_result);
		done = FAILURE;
		return;
	}

	elapsed = get_timer(start_time);
        log_info("\n%u bytes transferred in %lu ms (", rx_content_len,
                 get_timer(start_time));
        print_size(rx_content_len / elapsed * 1000, "/s)\n");

	if (env_set_hex("filesize", rx_content_len) ||
	    env_set_hex("fileaddr", saved_daddr)) {
		log_err("Could not set filesize or fileaddr\n");
		done = FAILURE;
		return;
	}

	done = SUCCESS;
}

int wget_with_dns(ulong dst_addr, char *uri)
{
	char server_name[SERVER_NAME_SIZE];
	httpc_connection_t conn;
	httpc_state_t *state;
	char *path;
	u16 port;

	daddr = dst_addr;
	saved_daddr = dst_addr;
	done = NOT_DONE;
	size = 0;
	prevsize = 0;

	if (parse_url(uri, server_name, &port, &path))
		return CMD_RET_USAGE;

	memset(&conn, 0, sizeof(conn));
	conn.result_fn = httpc_result_cb;
	start_time = get_timer(0);
	if (httpc_get_file_dns(server_name, port, path, &conn, httpc_recv_cb,
			       NULL, &state))
		return CMD_RET_FAILURE;

	while (!done) {
		eth_rx();
		sys_check_timeouts();
		if (ctrlc())
			break;
	}

	if (done == SUCCESS)
		return 0;

	return -1;
}

int do_wget(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	char *end;
	char *url;
	ulong dst_addr;

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	dst_addr = hextoul(argv[1], &end);
        if (end == (argv[1] + strlen(argv[1]))) {
		if (argc < 3)
			return CMD_RET_USAGE;
		url = argv[2];
	} else {
		dst_addr = image_load_addr;
		url = argv[1];
	}

	if (wget_with_dns(dst_addr, url))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
