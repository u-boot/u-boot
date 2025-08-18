// SPDX-License-Identifier: GPL-2.0
/*
 * WGET/HTTP support driver based on U-BOOT's nfs.c
 * Copyright Duncan Hare <dh@synoia.com> 2017
 */

#include <asm/global_data.h>
#include <command.h>
#include <display_options.h>
#include <env.h>
#include <efi_loader.h>
#include <image.h>
#include <lmb.h>
#include <mapmem.h>
#include <net.h>
#include <net/tcp.h>
#include <net/wget.h>
#include <stdlib.h>

DECLARE_GLOBAL_DATA_PTR;

/* The default, change with environment variable 'httpdstp' */
#define SERVER_PORT		80

#define HASHES_PER_LINE		65

#define HTTP_MAX_HDR_LEN	2048

#define HTTP_STATUS_BAD		0
#define HTTP_STATUS_OK		200

static const char http_proto[] = "HTTP/1.0";
static const char http_eom[] = "\r\n\r\n";
static const char content_len[] = "Content-Length:";
static const char linefeed[] = "\r\n";
static struct in_addr web_server_ip;
static unsigned int server_port;
static unsigned long content_length;
static u32 http_hdr_size, max_rx_pos;
static int wget_tsize_num_hash;

static char *image_url;
static enum net_loop_state wget_loop_state;

/**
 * store_block() - store block in memory
 * @src: source of data
 * @offset: offset
 * @len: length
 */
static inline int store_block(uchar *src, unsigned int offset, unsigned int len)
{
	ulong store_addr = image_load_addr + offset;
	uchar *ptr;

	// Avoid overflow
	if (wget_info->buffer_size && wget_info->buffer_size < offset + len)
		return -1;
	if (CONFIG_IS_ENABLED(LMB) && wget_info->set_bootdev) {
		if (store_addr < image_load_addr ||
		    lmb_read_check(store_addr, len)) {
			if (!wget_info->silent) {
				printf("\nwget error: ");
				printf("trying to overwrite reserved memory\n");
			}
			return -1;
		}
	}

	ptr = map_sysmem(store_addr, len);
	memcpy(ptr, src, len);
	unmap_sysmem(ptr);

	return 0;
}

static void show_block_marker(u32 packets)
{
	int cnt;

	if (wget_info->silent)
		return;

	if (content_length != -1) {
		if (net_boot_file_size > content_length)
			content_length = net_boot_file_size;

		cnt = net_boot_file_size * 50 / content_length;
		while (wget_tsize_num_hash < cnt) {
			putc('#');
			wget_tsize_num_hash++;
		}
	} else {
		if ((packets % 10) == 0)
			putc('#');
		else if (((packets + 1) % (10 * HASHES_PER_LINE)) == 0)
			puts("\n");
	}
}

static void tcp_stream_on_closed(struct tcp_stream *tcp)
{
	if (tcp->status != TCP_ERR_OK)
		wget_loop_state = NETLOOP_FAIL;

	net_set_state(wget_loop_state);
	if (wget_loop_state != NETLOOP_SUCCESS) {
		net_boot_file_size = 0;
		if (!wget_info->silent)
			printf("\nwget: Transfer Fail, TCP status - %d\n",
			       tcp->status);
		return;
	}

	if (!wget_info->silent)
		printf("\nPackets received %d, Transfer Successful\n",
		       tcp->rx_packets);
	wget_info->file_size = net_boot_file_size;
	if (wget_info->method == WGET_HTTP_METHOD_GET && wget_info->set_bootdev) {
		efi_set_bootdev("Http", NULL, image_url,
				map_sysmem(image_load_addr, 0),
				net_boot_file_size);
		env_set_hex("filesize", net_boot_file_size);
	}
}

static void tcp_stream_on_rcv_nxt_update(struct tcp_stream *tcp, u32 rx_bytes)
{
	char	*pos, *tail;
	uchar	saved, *ptr;
	int	reply_len;

	if (http_hdr_size) {
		net_boot_file_size = rx_bytes - http_hdr_size;
		show_block_marker(tcp->rx_packets);
		return;
	}

	ptr = map_sysmem(image_load_addr, rx_bytes + 1);

	saved = ptr[rx_bytes];
	ptr[rx_bytes] = '\0';
	pos = strstr((char *)ptr, http_eom);
	ptr[rx_bytes] = saved;

	if (!pos) {
		if (rx_bytes < HTTP_MAX_HDR_LEN &&
		    tcp->state == TCP_ESTABLISHED)
			goto end;

		if (!wget_info->silent)
			printf("ERROR: misssed HTTP header\n");
		tcp_stream_close(tcp);
		goto end;
	}

	http_hdr_size = pos - (char *)ptr + strlen(http_eom);
	*pos = '\0';

	if (wget_info->headers && http_hdr_size < MAX_HTTP_HEADERS_SIZE)
		strcpy(wget_info->headers, ptr);

	/* check for HTTP proto */
	if (strncasecmp((char *)ptr, "HTTP/", 5)) {
		debug_cond(DEBUG_WGET, "wget: Connected Bad Xfer "
				       "(no HTTP Status Line found)\n");
		tcp_stream_close(tcp);
		goto end;
	}

	/* get HTTP reply len */
	pos = strstr((char *)ptr, linefeed);
	if (pos)
		reply_len = pos - (char *)ptr;
	else
		reply_len = http_hdr_size - strlen(http_eom);

	pos = strchr((char *)ptr, ' ');
	if (!pos || pos - (char *)ptr > reply_len) {
		debug_cond(DEBUG_WGET, "wget: Connected Bad Xfer "
				       "(no HTTP Status Code found)\n");
		tcp_stream_close(tcp);
		goto end;
	}

	wget_info->status_code = (u32)simple_strtoul(pos + 1, &tail, 10);
	if (tail == pos + 1 || *tail != ' ') {
		debug_cond(DEBUG_WGET, "wget: Connected Bad Xfer "
				       "(bad HTTP Status Code)\n");
		tcp_stream_close(tcp);
		goto end;
	}

	debug_cond(DEBUG_WGET,
		   "wget: HTTP Status Code %d\n", wget_info->status_code);

	if (wget_info->status_code != HTTP_STATUS_OK) {
		debug_cond(DEBUG_WGET, "wget: Connected Bad Xfer\n");
		tcp_stream_close(tcp);
		goto end;
	}

	debug_cond(DEBUG_WGET, "wget: Connctd pkt %p  hlen %x\n",
		   ptr, http_hdr_size);

	content_length = -1;
	pos = strstr((char *)ptr, content_len);
	if (pos) {
		pos += strlen(content_len) + 1;
		while (*pos == ' ')
			pos++;
		content_length = simple_strtoul(pos, &tail, 10);
		if (*tail != '\r' && *tail != '\n' && *tail != '\0')
			content_length = -1;
	}

	if (content_length != -1) {
		debug_cond(DEBUG_WGET,
			   "wget: Connected Len %lu\n",
			   content_length);
		wget_info->hdr_cont_len = content_length;
		if (wget_info->buffer_size && wget_info->buffer_size < wget_info->hdr_cont_len){
			tcp_stream_reset(tcp);
			goto end;
		}

	}

	net_boot_file_size = rx_bytes - http_hdr_size;
	memmove(ptr, ptr + http_hdr_size, max_rx_pos + 1 - http_hdr_size);
	wget_loop_state = NETLOOP_SUCCESS;

end:
	unmap_sysmem(ptr);
}

static int tcp_stream_rx(struct tcp_stream *tcp, u32 rx_offs, void *buf, int len)
{
	if ((max_rx_pos == (u32)(-1)) || (max_rx_pos < rx_offs + len - 1))
		max_rx_pos = rx_offs + len - 1;

	// Avoid overflow
	if (store_block(buf, rx_offs - http_hdr_size, len) < 0)
		return -1;

	return len;
}

static int tcp_stream_tx(struct tcp_stream *tcp, u32 tx_offs, void *buf, int maxlen)
{
	int ret;
	const char *method;

	if (tx_offs)
		return 0;

	switch (wget_info->method) {
	case WGET_HTTP_METHOD_HEAD:
		method = "HEAD";
		break;
	case WGET_HTTP_METHOD_GET:
	default:
		method = "GET";
		break;
	}

	ret = snprintf(buf, maxlen, "%s %s %s\r\n\r\n",
		       method, image_url, http_proto);

	return ret;
}

static int tcp_stream_on_create(struct tcp_stream *tcp)
{
	if (tcp->rhost.s_addr != web_server_ip.s_addr ||
	    tcp->rport != server_port)
		return 0;

	tcp->max_retry_count = WGET_RETRY_COUNT;
	tcp->initial_timeout = WGET_TIMEOUT;
	tcp->on_closed = tcp_stream_on_closed;
	tcp->on_rcv_nxt_update = tcp_stream_on_rcv_nxt_update;
	tcp->rx = tcp_stream_rx;
	tcp->tx = tcp_stream_tx;

	return 1;
}

#define BLOCKSIZE 512

void wget_start(void)
{
	struct tcp_stream *tcp;

	if (!wget_info)
		wget_info = &default_wget_info;

	image_url = strchr(net_boot_file_name, ':');
	if (image_url > 0) {
		web_server_ip = string_to_ip(net_boot_file_name);
		++image_url;
		net_server_ip = web_server_ip;
	} else {
		web_server_ip = net_server_ip;
		image_url = net_boot_file_name;
	}

	debug_cond(DEBUG_WGET,
		   "wget: Transfer HTTP Server %pI4; our IP %pI4\n",
		   &web_server_ip, &net_ip);

	/* Check if we need to send across this subnet */
	if (net_gateway.s_addr && net_netmask.s_addr) {
		struct in_addr our_net;
		struct in_addr server_net;

		our_net.s_addr = net_ip.s_addr & net_netmask.s_addr;
		server_net.s_addr = net_server_ip.s_addr & net_netmask.s_addr;
		if (our_net.s_addr != server_net.s_addr)
			debug_cond(DEBUG_WGET,
				   "wget: sending through gateway %pI4",
				   &net_gateway);
	}
	debug_cond(DEBUG_WGET, "URL '%s'\n", image_url);

	if (net_boot_file_expected_size_in_blocks) {
		debug_cond(DEBUG_WGET, "wget: Size is 0x%x Bytes = ",
			   net_boot_file_expected_size_in_blocks * BLOCKSIZE);
		print_size(net_boot_file_expected_size_in_blocks * BLOCKSIZE,
			   "");
	}
	debug_cond(DEBUG_WGET,
		   "\nwget:Load address: 0x%lx\nLoading: *\b", image_load_addr);

	/*
	 * Zero out server ether to force arp resolution in case
	 * the server ip for the previous u-boot command, for example dns
	 * is not the same as the web server ip.
	 */

	memset(net_server_ethaddr, 0, 6);

	max_rx_pos = (u32)(-1);
	net_boot_file_size = 0;
	http_hdr_size = 0;
	wget_tsize_num_hash = 0;
	wget_loop_state = NETLOOP_FAIL;

	wget_info->status_code = HTTP_STATUS_BAD;
	wget_info->file_size = 0;
	wget_info->hdr_cont_len = 0;
	if (wget_info->headers)
		wget_info->headers[0] = 0;

	server_port = env_get_ulong("httpdstp", 10, SERVER_PORT) & 0xffff;
	tcp_stream_set_on_create_handler(tcp_stream_on_create);
	tcp = tcp_stream_connect(web_server_ip, server_port);
	if (!tcp) {
		if (!wget_info->silent)
			printf("No free tcp streams\n");
		net_set_state(NETLOOP_FAIL);
		return;
	}
	tcp_stream_put(tcp);
}

int wget_do_request(ulong dst_addr, char *uri)
{
	int ret;
	char *s, *host_name, *file_name, *str_copy;

	/*
	 * Download file using wget.
	 *
	 * U-Boot wget takes the target uri in this format.
	 *  "<http server ip>:<file path>"  e.g.) 192.168.1.1:/sample/test.iso
	 * Need to resolve the http server ip address before starting wget.
	 */
	str_copy = strdup(uri);
	if (!str_copy)
		return -ENOMEM;

	s = str_copy + strlen("http://");
	host_name = strsep(&s, "/");
	if (!s) {
		ret = -EINVAL;
		goto out;
	}
	file_name = s;

	host_name = strsep(&host_name, ":");

	if (string_to_ip(host_name).s_addr) {
		s = host_name;
	} else {
#if IS_ENABLED(CONFIG_DNS)
		net_dns_resolve = host_name;
		net_dns_env_var = "httpserverip";
		if (net_loop(DNS) < 0) {
			ret = -EINVAL;
			goto out;
		}
		s = env_get("httpserverip");
		if (!s) {
			ret = -EINVAL;
			goto out;
		}
#else
		ret = -EINVAL;
		goto out;
#endif
	}

	strlcpy(net_boot_file_name, s, sizeof(net_boot_file_name));
	strlcat(net_boot_file_name, ":/", sizeof(net_boot_file_name)); /* append '/' which is removed by strsep() */
	strlcat(net_boot_file_name, file_name, sizeof(net_boot_file_name));
	image_load_addr = dst_addr;
	ret = net_loop(WGET);

out:
	free(str_copy);

	return ret < 0 ? ret : 0;
}

/**
 * wget_validate_uri() - validate the uri for wget
 *
 * @uri:	uri string
 *
 * This function follows the current U-Boot wget implementation.
 * scheme: only "http:" is supported
 * authority:
 *   - user information: not supported
 *   - host: supported
 *   - port: not supported(always use the default port)
 *
 * Uri is expected to be correctly percent encoded.
 * This is the minimum check, control codes(0x1-0x19, 0x7F, except '\0')
 * and space character(0x20) are not allowed.
 *
 * TODO: stricter uri conformance check
 *
 * Return:	true on success, false on failure
 */
bool wget_validate_uri(char *uri)
{
	char c;
	bool ret = true;
	char *str_copy, *s, *authority;

	for (c = 0x1; c < 0x21; c++) {
		if (strchr(uri, c)) {
			log_err("invalid character is used\n");
			return false;
		}
	}
	if (strchr(uri, 0x7f)) {
		log_err("invalid character is used\n");
		return false;
	}

	if (strncmp(uri, "http://", 7)) {
		log_err("only http:// is supported\n");
		return false;
	}
	str_copy = strdup(uri);
	if (!str_copy)
		return false;

	s = str_copy + strlen("http://");
	authority = strsep(&s, "/");
	if (!s) {
		log_err("invalid uri, no file path\n");
		ret = false;
		goto out;
	}
	s = strchr(authority, '@');
	if (s) {
		log_err("user information is not supported\n");
		ret = false;
		goto out;
	}
	s = strchr(authority, ':');
	if (s) {
		log_err("user defined port is not supported\n");
		ret = false;
		goto out;
	}

out:
	free(str_copy);

	return ret;
}
