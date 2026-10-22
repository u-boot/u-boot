// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <efi_loader.h>
#include <env.h>
#include <hexdump.h>
#include <log.h>
#include <lwip-dhcp.h>
#include <malloc.h>
#include <net.h>
#include <rand.h>
#include <time.h>
#include <asm/unaligned.h>
#include <dm/device.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <lwip/apps/sntp.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/prot/dhcp.h>
#include <lwip/udp.h>
#include <u-boot/uuid.h>

#define DHCP_TIMEOUT_MS ((3ULL + 5ULL * CONFIG_NET_RETRY_COUNT) * 1000)
/* Keep shared retry settings representable by DHCP's u16, 500 ms timer. */
#define DHCP_RETRANSMIT_MAX_MS (U16_MAX * 500U)
#define BOOTP_MIN_LEN (DHCP_MSG_LEN + 64)

struct bootp_config {
	ip4_addr_t ip;
	ip4_addr_t netmask;
	ip4_addr_t gateway;
	ip4_addr_t server;
	ip4_addr_t next_server;
	bool have_netmask;
	bool have_gateway;
};

struct dhcp_boot_data {
	char hostname[256];
	char rootpath[CONFIG_BOOTP_MAX_ROOT_PATH_LEN];
	char domain[256];
	char bootfile[sizeof(net_boot_file_name)];
	char pxe_config[256];
	char vendor[256];
	ip4_addr_t server;
	u8 type;
	u8 file_size[2];
	u8 time_offset[4];
	ip4_addr_t ntp;
	ip4_addr_t dns[2];
	u8 dns_count;
	bool have_size;
	bool have_offset;
	bool have_ntp;
};

struct dhcp_efi_cache {
	struct efi_pxe_packet ack;
	struct efi_pxe_packet proxy_offer;
	u16 ack_len;
	u16 proxy_len;
};

struct dhcp_options {
	struct netif *netif;
	struct bootp_config config;
	u32 bootp_xid;
	bool bootp_bound;
	ulong start;
	ulong timeout;
	u32 retransmit_init;
	u32 retransmit_max;
	u16 discover_secs;
	struct dhcp_boot_data reply;
	struct dhcp_boot_data candidate;
	struct dhcp_boot_data offer;
	bool have_offer;
	struct dhcp_boot_data proxy;
	ip4_addr_t proxy_server;
	u32 proxy_xid;
	bool have_proxy;
	char hostname[256];
	char vendor[256];
	u16 arch;
	u8 uuid[1 + UUID_BIN_LEN];
	bool have_uuid;
	bool append_failed;
	/* Allocate one packet cache only when EFI consumes the DHCP replies. */
	struct dhcp_efi_cache efi[];
};

/* The address-less runtime attachment excludes another BOOTP/DHCP command. */
static struct dhcp_options *active_options;

/*
 * Parse decimal or hexadecimal settings without allowing the accumulator to
 * wrap before checking the setting's limit. strict_strtoul() does not check
 * overflow in U-Boot.
 */
static int dhcp_parse_number(const char *str, uint base, ulong limit, ulong *result)
{
	ulong value = 0;
	uint digit;

	if (base == 16 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		str += 2;
	if (!*str)
		return -EINVAL;
	do {
		digit = hex_to_bin(*str++);
		if (digit >= base)
			return -EINVAL;
		if (digit > limit || value > (limit - digit) / base)
			return -ERANGE;
		value = value * base + digit;
	} while (*str);
	*result = value;

	return 0;
}

static u32 bootp_timeout(struct dhcp_options *options, u8 tries)
{
	u32 timeout;
	unsigned int i;

	if (!options)
		return (tries < 6 ? 1U << tries : 60) * 1000;
	timeout = options->retransmit_init;
	for (i = 1; i < tries && timeout < options->retransmit_max; i++)
		timeout = min(timeout * 2, options->retransmit_max);
	if (tries <= 1)
		return timeout;
	/* Match the legacy client's approximately ten percent randomization. */
	timeout = mult_frac(timeout, rand() % 200 + 900, 1000);

	return clamp(timeout, 1U, options->retransmit_max);
}

/* The core rounds the result up to its fine timer interval. */
u32_t net_lwip_dhcp_timeout(u8_t tries)
{
	return bootp_timeout(active_options, tries);
}

static int dhcp_timing_options(struct dhcp_options *options)
{
	static const char * const names[] = {
		"bootpretryperiod", "bootpretransmitperiodinit", "bootpretransmitperiodmax",
	};
	ulong values[] = { min_t(u64, DHCP_TIMEOUT_MS, ULONG_MAX), 250, 60000 };
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(names); i++) {
		const char *value = env_get(names[i]);

		if ((value && dhcp_parse_number(value, 10,
						i ? DHCP_RETRANSMIT_MAX_MS : ULONG_MAX,
						&values[i])) || (i && !values[i])) {
			log_err("Invalid BOOTP/DHCP timing setting: %s\n", names[i]);
			return -EINVAL;
		}
	}
	options->timeout = values[0];
	options->retransmit_max = values[2];
	options->retransmit_init = min(values[1], values[2]);

	return 0;
}

struct dhcp_boot_option {
	u8 code;
	bool enabled;
	bool string;
	void *data;
	size_t size;
	size_t len;
};

static int dhcp_boot_options(struct pbuf *p, unsigned int pos, unsigned int end,
			     struct dhcp_boot_option *opts, size_t count,
			     u8 *overload)
{
	while (pos < end) {
		u8 code = pbuf_get_at(p, pos++);
		unsigned int len;
		size_t i;

		if (code == DHCP_OPTION_END)
			return 0;
		if (code == DHCP_OPTION_PAD)
			continue;
		if (pos == end)
			return -EINVAL;
		len = pbuf_get_at(p, pos++);
		if (len > end - pos)
			return -EINVAL;
		if (code == DHCP_OPTION_OVERLOAD) {
			/* Overload is only meaningful in the main options field. */
			if (!overload || *overload || len != 1)
				return -EINVAL;
			*overload = pbuf_get_at(p, pos);
			if (!*overload || *overload > DHCP_OVERLOAD_SNAME_FILE)
				return -EINVAL;
		}
		for (i = 0; i < count; i++) {
			struct dhcp_boot_option *opt = &opts[i];
			size_t copy;

			if (code != opt->code || !opt->enabled)
				continue;
			if (!len)
				return -EINVAL;
			/*
			 * RFC 3396: concatenate fragments before interpreting them.
			 * Retain only the requested addresses from server/router lists,
			 * but validate the length of the entire list below.
			 */
			if (code != DHCP_OPTION_NTP && code != DHCP_OPTION_DNS_SERVER &&
			    code != DHCP_OPTION_ROUTER &&
			    len > opt->size - opt->len)
				return -E2BIG;
			copy = min_t(size_t, len, opt->size - min(opt->len, opt->size));
			if (copy && pbuf_copy_partial(p, (u8 *)opt->data + opt->len,
						      copy, pos) != copy)
				return -EINVAL;
			opt->len += len;
			break;
		}
		pos += len;
	}

	return 0;
}

static err_t dhcp_parse_boot_data(struct pbuf *p, struct dhcp_boot_data *data,
				  bool proxy)
{
	struct dhcp_boot_option opts[] = {
		{ 12, IS_ENABLED(CONFIG_BOOTP_HOSTNAME), true,
		  data->hostname, sizeof(data->hostname) - 1 },
		{ 17, IS_ENABLED(CONFIG_BOOTP_BOOTPATH), true,
		  data->rootpath, sizeof(data->rootpath) - 1 },
		{ 40, IS_ENABLED(CONFIG_BOOTP_NISDOMAIN), true,
		  data->domain, sizeof(data->domain) - 1 },
		{ 67, true, true, data->bootfile, sizeof(data->bootfile) - 1 },
		{ 209, IS_ENABLED(CONFIG_BOOTP_PXE_DHCP_OPTION), true,
		  data->pxe_config, sizeof(data->pxe_config) - 1 },
		{ 13, IS_ENABLED(CONFIG_BOOTP_BOOTFILESIZE), false,
		  data->file_size, sizeof(data->file_size) },
		{ 2, IS_ENABLED(CONFIG_BOOTP_TIMEOFFSET), false,
		  data->time_offset, sizeof(data->time_offset) },
		{ 42, IS_ENABLED(CONFIG_BOOTP_NTPSERVER), false,
		  &data->ntp, sizeof(data->ntp) },
		{ 53, true, false, &data->type, sizeof(data->type) },
		{ 54, true, false, &data->server, sizeof(data->server) },
		{ 60, proxy, true, data->vendor, sizeof(data->vendor) - 1 },
		{ 6, IS_ENABLED(CONFIG_BOOTP_DNS), false, data->dns, sizeof(data->dns) },
	};
	bool have_filename = false, have_file_size = false;
	u8 overload = 0;
	u32 cookie;
	size_t i;
	int ret;

	/* A fallback offer cannot supply the ACK's type or server identity. */
	data->type = 0;
	ip4_addr_set_zero(&data->server);
	if (pbuf_copy_partial(p, &cookie, sizeof(cookie), DHCP_MSG_LEN) != sizeof(cookie) ||
	    ntohl(cookie) != DHCP_MAGIC_COOKIE)
		return ERR_VAL;
	ret = dhcp_boot_options(p, DHCP_OPTIONS_OFS, p->tot_len, opts,
				ARRAY_SIZE(opts), &overload);
	if (!ret && (overload & DHCP_OVERLOAD_FILE))
		ret = dhcp_boot_options(p, DHCP_FILE_OFS, DHCP_FILE_OFS + DHCP_FILE_LEN,
					opts, ARRAY_SIZE(opts), NULL);
	if (!ret && (overload & DHCP_OVERLOAD_SNAME))
		ret = dhcp_boot_options(p, DHCP_SNAME_OFS, DHCP_SNAME_OFS + DHCP_SNAME_LEN,
					opts, ARRAY_SIZE(opts), NULL);
	if (ret)
		return ERR_VAL;
	for (i = 0; i < ARRAY_SIZE(opts); i++) {
		struct dhcp_boot_option *opt = &opts[i];
		char *str = opt->data;

		if (!opt->len)
			continue;
		if (opt->code == 67)
			have_filename = true;
		if (opt->string) {
			/* RFC 2132 allows trailing NULs, not an embedded terminator. */
			while (opt->len && !str[opt->len - 1])
				opt->len--;
			if (memchr(str, 0, opt->len))
				return ERR_VAL;
			str[opt->len] = 0;
		} else if (opt->code == DHCP_OPTION_NTP || opt->code == DHCP_OPTION_DNS_SERVER) {
			if (opt->len % sizeof(ip4_addr_t))
				return ERR_VAL;
			if (opt->code == DHCP_OPTION_NTP)
				data->have_ntp = !ip4_addr_isany(&data->ntp);
			else
				data->dns_count = min_t(size_t, opt->len / sizeof(ip4_addr_t),
							IS_ENABLED(CONFIG_BOOTP_DNS2) ? 2 : 1);
		} else {
			if (opt->len != opt->size)
				return ERR_VAL;
			if (opt->code == 13) {
				data->have_size = true;
				have_file_size = true;
			} else if (opt->code == 2) {
				data->have_offset = true;
			}
		}
	}
	if (!(overload & DHCP_OVERLOAD_FILE) && !have_filename &&
	    pbuf_get_at(p, DHCP_FILE_OFS)) {
		size_t len = min_t(size_t, DHCP_FILE_LEN, sizeof(data->bootfile) - 1);

		if (pbuf_copy_partial(p, data->bootfile, len, DHCP_FILE_OFS) != len)
			return ERR_VAL;
		data->bootfile[len] = 0;
		have_filename = true;
	}
	/* An offered size must not describe a replacement filename in the ACK. */
	if (!have_file_size && have_filename)
		data->have_size = false;

	return ERR_OK;
}

/* lwip/dhcp.h exposes the DHCP state and API only with LWIP_DHCP. */
#if LWIP_DHCP
err_t net_lwip_dhcp_offer(struct netif *netif, struct dhcp *dhcp, struct pbuf *p)
{
	struct dhcp_options *options = active_options;
	struct dhcp_boot_data *data;

	if (!options || options->netif != netif)
		return ERR_OK;
	options->have_offer = false;
	data = &options->offer;
	memset(data, 0, sizeof(*data));
	if (dhcp_parse_boot_data(p, data, false) || data->type != DHCP_OFFER ||
	    ip4_addr_isany(&data->server))
		return ERR_VAL;
	options->have_offer = true;

	return ERR_OK;
}

err_t net_lwip_dhcp_recv(struct netif *netif, struct dhcp *dhcp, struct pbuf *p)
{
	struct dhcp_options *options = active_options;
	struct dhcp_boot_data *data;
	ip4_addr_t offered, server;
	u32 cookie;
	u8 type = 0, overload = 0;
	struct dhcp_boot_option msg_type = { 53, true, false, &type, sizeof(type) };

	if (!options || options->netif != netif)
		return ERR_OK;
	if (pbuf_copy_partial(p, &cookie, sizeof(cookie), DHCP_MSG_LEN) != sizeof(cookie) ||
	    ntohl(cookie) != DHCP_MAGIC_COOKIE ||
	    pbuf_get_at(p, offsetof(struct dhcp_msg, htype)) != 1 ||
	    pbuf_get_at(p, offsetof(struct dhcp_msg, hlen)) != netif->hwaddr_len ||
	    pbuf_copy_partial(p, &offered, sizeof(offered),
			      offsetof(struct dhcp_msg, yiaddr)) != sizeof(offered))
		return ERR_VAL;
	if (!ip4_addr_isany(&offered))
		return ERR_OK;
	/*
	 * NAK also has a zero yiaddr and must still drive the lease state
	 * machine. Leave its full validation to the built-in DHCP parser.
	 */
	if (dhcp_boot_options(p, DHCP_OPTIONS_OFS, p->tot_len, &msg_type, 1, &overload) ||
	    msg_type.len != 1)
		return ERR_VAL;
	if (type == DHCP_NAK)
		return ERR_OK;

	/*
	 * Zero-address offers must never select a lease, even without proxy
	 * support. Consume them before lwIP clears the ACK's boot filename.
	 */
	if (!IS_ENABLED(CONFIG_SERVERIP_FROM_PROXYDHCP) || options->have_proxy ||
	    (dhcp->state != DHCP_STATE_SELECTING && dhcp->state != DHCP_STATE_REQUESTING &&
	     dhcp->state != DHCP_STATE_BOUND))
		return ERR_VAL;
	data = &options->proxy;
	memset(data, 0, sizeof(*data));
	if (dhcp_parse_boot_data(p, data, true) ||
	    (data->type != DHCP_OFFER && data->type != DHCP_ACK) ||
	    strncmp(data->vendor, "PXEClient", 9) || ip4_addr_isany(&data->server))
		return ERR_VAL;
	if (pbuf_copy_partial(p, &server, sizeof(server),
			      offsetof(struct dhcp_msg, siaddr)) != sizeof(server))
		return ERR_VAL;
	if (ip4_addr_isany(&server)) {
		/* Menu-only offers need PXE boot-service discovery, not TFTP. */
		if (!data->bootfile[0] && !data->pxe_config[0])
			return ERR_VAL;
		server = data->server;
	}
	if (ip4_addr_ismulticast(&server) || ip4_addr_isbroadcast(&server, netif))
		return ERR_VAL;
	options->proxy_server = server;
	options->have_proxy = true;
	if (CONFIG_IS_ENABLED(EFI_LOADER)) {
		struct dhcp_efi_cache *efi = options->efi;

		efi->proxy_len = min_t(size_t, p->tot_len, sizeof(efi->proxy_offer));
		pbuf_copy_partial(p, &efi->proxy_offer, efi->proxy_len, 0);
	}

	return ERR_ABRT;
}

err_t net_lwip_dhcp_ack(struct netif *netif, struct dhcp *dhcp, struct pbuf *p)
{
	struct dhcp_boot_data *data;
	err_t ret;

	if (!active_options || active_options->netif != netif)
		return ERR_OK;
	/* A rejected renewal must leave the accepted lease metadata intact. */
	data = &active_options->candidate;
	if (active_options->have_offer && dhcp->state == DHCP_STATE_REQUESTING &&
	    ip4_addr_cmp(&active_options->offer.server, ip_2_ip4(&dhcp->server_ip_addr)))
		*data = active_options->offer;
	else
		memset(data, 0, sizeof(*data));
	ret = dhcp_parse_boot_data(p, data, false);
	if (ret)
		return ret;
	/* The lease comes only from the selected DHCP server, not the proxy. */
	if (data->type != DHCP_ACK || ip4_addr_isany(&data->server) ||
	    (dhcp->state == DHCP_STATE_REQUESTING &&
	     !ip4_addr_cmp(&data->server, ip_2_ip4(&dhcp->server_ip_addr))))
		return ERR_VAL;
	active_options->reply = *data;
	if (CONFIG_IS_ENABLED(EFI_LOADER)) {
		struct dhcp_efi_cache *efi = active_options->efi;

		/* Copy while the pbuf is alive, publish only after binding. */
		efi->ack_len = min_t(size_t, p->tot_len, sizeof(efi->ack));
		pbuf_copy_partial(p, &efi->ack, efi->ack_len, 0);
	}

	return ERR_OK;
}

#endif /* LWIP_DHCP */

static void dhcp_append(struct dhcp_options *options, struct dhcp_msg *msg,
			u16_t *len, u8 code, const void *data, size_t size)
{
	if (!size)
		return;
	/* Leave room for END and the core's four-byte padding. */
	if (*len + size + 2 + 4 > DHCP_OPTIONS_LEN) {
		options->append_failed = true;
		return;
	}
	msg->options[(*len)++] = code;
	msg->options[(*len)++] = size;
	memcpy(&msg->options[*len], data, size);
	*len += size;
}

#if LWIP_DHCP
void net_lwip_dhcp_append(struct netif *netif, struct dhcp *dhcp, u8_t state,
			  struct dhcp_msg *msg, u8_t type, u16_t *len)
{
	/* Match the legacy client: no PXE ROM APIs are provided by U-Boot. */
	static const u8 undi[] = { 1, 0, 0 };
	struct dhcp_options *options = active_options;
	u8 arch[2];

	if (!active_options || active_options->netif != netif ||
	    (type != DHCP_DISCOVER && type != DHCP_REQUEST))
		return;
	if (type == DHCP_DISCOVER && (!dhcp->tries || options->proxy_xid != dhcp->xid)) {
		options->have_proxy = false;
		options->have_offer = false;
		options->proxy_xid = dhcp->xid;
	}
	if (type == DHCP_DISCOVER)
		options->discover_secs = min_t(ulong, get_timer(options->start) / 1000,
					       U16_MAX);
	/* RFC 2131 requires selecting REQUESTs to reuse DISCOVER's secs. */
	msg->secs = htons(options->discover_secs);
	copy_filename((char *)msg->file, net_boot_file_name, sizeof(msg->file));
	dhcp_append(options, msg, len, DHCP_OPTION_HOSTNAME,
		    options->hostname, strlen(options->hostname));
	dhcp_append(options, msg, len, DHCP_OPTION_US, options->vendor, strlen(options->vendor));
	if (options->arch != 0xff) {
		put_unaligned_be16(options->arch, arch);
		dhcp_append(options, msg, len, 93, arch, sizeof(arch));
	}
	dhcp_append(options, msg, len, 94, undi, sizeof(undi));
	if (options->have_uuid)
		dhcp_append(options, msg, len, 97, options->uuid, sizeof(options->uuid));
}
#endif /* LWIP_DHCP */

static int dhcp_pxe_options(struct dhcp_options *options)
{
	const char *arch = env_get("bootp_arch");
	const char *uuid = env_get("pxeuuid");
	ulong value = 0xff;

	if (IS_ENABLED(CONFIG_BOOTP_PXE))
		value = IF_ENABLED_INT(CONFIG_BOOTP_PXE, CONFIG_DHCP_PXE_CLIENTARCH);
	/* EFI boot methods override the native U-Boot architecture. */
	if ((arch && dhcp_parse_number(arch, 16, U16_MAX, &value)) || value > U16_MAX) {
		log_err("Invalid DHCP client architecture\n");
		return -EINVAL;
	}
	options->arch = value;
	if (IS_ENABLED(CONFIG_LIB_UUID) && uuid) {
		if (uuid_str_to_bin(uuid, options->uuid + 1, UUID_STR_FORMAT_STD)) {
			log_err("Invalid PXE UUID\n");
			return -EINVAL;
		}
		options->have_uuid = true;
	}

	return 0;
}

static int dhcp_boot_env(struct dhcp_boot_data *data)
{
	char offset[12];
	char *config = NULL;
	unsigned int i;
#if LWIP_DNS
	ip_addr_t addr;
#endif

	for (i = 0; i < data->dns_count; i++) {
		if (env_set(i ? "dnsip2" : "dnsip", ip4addr_ntoa(&data->dns[i])))
			return CMD_RET_FAILURE;
#if LWIP_DNS
		ip_addr_copy_from_ip4(addr, data->dns[i]);
		dns_setserver(i, &addr);
#endif
	}

	if ((data->hostname[0] && env_set("hostname", data->hostname)) ||
	    (data->rootpath[0] && env_set("rootpath", data->rootpath)) ||
	    (data->domain[0] && env_set("domain", data->domain)) ||
	    (data->have_ntp && env_set("ntpserverip", ip4addr_ntoa(&data->ntp))))
		return CMD_RET_FAILURE;
	if (data->have_offset) {
		snprintf(offset, sizeof(offset), "%d", (s32)get_unaligned_be32(data->time_offset));
		if (env_set("timeoffset", offset))
			return CMD_RET_FAILURE;
	}
	net_boot_file_expected_size_in_blocks = data->have_size ?
		get_unaligned_be16(data->file_size) : 0;
	if (CONFIG_IS_ENABLED(CMD_SNTP) && data->have_ntp) {
		ip_addr_t addr;

		ip_addr_copy_from_ip4(addr, data->ntp);
		sntp_setserver(0, &addr);
	}
	if (IS_ENABLED(CONFIG_BOOTP_PXE_DHCP_OPTION)) {
		if (data->pxe_config[0]) {
			config = strdup(data->pxe_config);
			if (!config)
				return CMD_RET_FAILURE;
		}
		/* Unlike environment defaults, this string belongs to DHCP. */
		free(pxelinux_configfile);
		pxelinux_configfile = config;
	}

	return CMD_RET_SUCCESS;
}

static int dhcp_server_env(struct dhcp_options *options)
{
	struct bootp_config *config = &options->config;
	const char *server = env_get("serverip");
	ip4_addr_t addr;

	/* tftpserverip otherwise overrides serverip in the lwIP TFTP client. */
	if (IS_ENABLED(CONFIG_BOOTP_SERVERIP) ||
	    (IS_ENABLED(CONFIG_BOOTP_PREFER_SERVERIP) && server &&
	     ip4addr_aton(server, &addr) && !ip4_addr_isany(&addr)))
		return env_set("tftpserverip", NULL);

	if (options->have_proxy)
		return env_set("serverip", ip4addr_ntoa(&options->proxy_server)) ||
			env_set("tftpserverip", ip4addr_ntoa(&options->proxy_server));

	/* A BOOTP reply may omit siaddr. Keep the configured server in that case. */
	return (!ip4_addr_isany(&config->server) &&
		env_set("serverip", ip4addr_ntoa(&config->server))) ||
		env_set("tftpserverip", ip4_addr_isany(&config->next_server) ?
			NULL : ip4addr_ntoa(&config->next_server));
}

static int bootp_store_config(struct net_lwip_ctx *net, bool explicit_file,
			      struct dhcp_options *options)
{
	struct bootp_config *config = &options->config;
	char ipstr[] = "ipaddr\0\0\0";
	char maskstr[] = "netmask\0\0\0";
	char gwstr[] = "gatewayip\0\0\0";
	int idx = dev_seq(net->dev);

	if (!explicit_file && options->reply.bootfile[0])
		copy_filename(net_boot_file_name, options->reply.bootfile,
			      sizeof(net_boot_file_name));
	if (*net_boot_file_name && env_set("bootfile", net_boot_file_name))
		return CMD_RET_FAILURE;

	if (idx > 0) {
		sprintf(ipstr, "ipaddr%d", idx);
		sprintf(maskstr, "netmask%d", idx);
		sprintf(gwstr, "gatewayip%d", idx);
	} else {
		net_ip.s_addr = ip4_addr_get_u32(&config->ip);
	}

	if (env_set(ipstr, ip4addr_ntoa(&config->ip)) ||
	    (config->have_netmask && env_set(maskstr, ip4addr_ntoa(&config->netmask))) ||
	    (config->have_gateway && env_set(gwstr, ip4addr_ntoa(&config->gateway))) ||
	    dhcp_server_env(options))
		return CMD_RET_FAILURE;

	return dhcp_boot_env(&options->reply);
}

#if LWIP_DHCP
static int dhcp_loop(struct net_lwip_ctx *net, bool explicit_file,
		     struct dhcp_options *options)
{
	struct bootp_config *config = &options->config;
	unsigned long start;
	unsigned long proxy_start = 0;
	struct dhcp *dhcp;
	bool bound = false;

	start = get_timer(0);
	options->start = start;

	if (dhcp_start(net->netif) || options->append_failed)
		return CMD_RET_FAILURE;

	/*
	 * Keep the lease and optional proxy reply separate. Continue receiving
	 * packets during the bounded proxy wait, without delaying DHCPREQUEST.
	 */
	for (;;) {
		if (net_lwip_poll() < 0 || options->append_failed)
			return CMD_RET_FAILURE;
		if (ctrlc()) {
			printf("Abort\n");
			return -EINTR;
		}
		if (IS_ENABLED(CONFIG_SERVERIP_FROM_PROXYDHCP) &&
		    !bound && dhcp_supplied_address(net->netif))
			proxy_start = get_timer(0);
		bound = dhcp_supplied_address(net->netif);
		if (bound) {
			if (IS_ENABLED(CONFIG_SERVERIP_FROM_PROXYDHCP) && !options->have_proxy &&
			    get_timer(proxy_start) <
			    IF_ENABLED_INT(CONFIG_SERVERIP_FROM_PROXYDHCP,
					   CONFIG_SERVERIP_FROM_PROXYDHCP_DELAY_MS)) {
				mdelay(1);
				continue;
			}
			break;
		}
		if (get_timer(start) >= options->timeout)
			break;
		mdelay(1);
	}

	if (!bound)
		return -ETIMEDOUT;

	dhcp = netif_dhcp_data(net->netif);
	if (options->have_proxy) {
		struct dhcp_boot_data *proxy = &options->proxy;
		struct dhcp_boot_data *reply = &options->reply;

		if (proxy->bootfile[0]) {
			strlcpy(reply->bootfile, proxy->bootfile, sizeof(reply->bootfile));
			/* An ACK's size describes its own file, not the proxy's file. */
			reply->have_size = proxy->have_size;
			memcpy(reply->file_size, proxy->file_size, sizeof(reply->file_size));
		}
		if (proxy->pxe_config[0])
			strlcpy(reply->pxe_config, proxy->pxe_config, sizeof(reply->pxe_config));
	}

	config->ip = dhcp->offered_ip_addr;
	config->netmask = dhcp->offered_sn_mask;
	config->gateway = dhcp->offered_gw_addr;
	config->server = *ip_2_ip4(&dhcp->server_ip_addr);
	config->next_server = dhcp->offered_si_addr;
	config->have_netmask = true;
	config->have_gateway = true;
	if (bootp_store_config(net, explicit_file, options))
		return CMD_RET_FAILURE;

	if (CONFIG_IS_ENABLED(EFI_LOADER)) {
		struct dhcp_efi_cache *efi = options->efi;

		efi_net_set_dhcp_ack(&efi->ack, efi->ack_len,
				     options->have_proxy ? &efi->proxy_offer : NULL,
				     options->have_proxy ? efi->proxy_len : 0);
	}

	printf("DHCP client bound to address %pI4 (%lu ms)\n",
	       &dhcp->offered_ip_addr, get_timer(start));

	return CMD_RET_SUCCESS;
}

static void dhcp_retire(struct net_lwip_ctx *net)
{
	if (!net->netif || !netif_dhcp_data(net->netif))
		return;
	if (dhcp_supplied_address(net->netif))
		dhcp_stop_without_release(net->netif);
	else
		dhcp_release_and_stop(net->netif);
	dhcp_cleanup(net->netif);
}
#else
static inline int dhcp_loop(struct net_lwip_ctx *net, bool explicit_file,
			    struct dhcp_options *options)
{
	return CMD_RET_FAILURE;
}

static inline void dhcp_retire(struct net_lwip_ctx *net)
{
}
#endif /* LWIP_DHCP */

static void bootp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
		       const ip_addr_t *addr, u16_t port)
{
	struct dhcp_options *options = arg;
	struct netif *netif = options->netif;
	struct dhcp_boot_data *data = &options->candidate;
	struct bootp_config config = {};
	struct dhcp_msg msg;
	u8 type, overload = 0;
	struct dhcp_boot_option opts[] = {
		{ 1, true, false, &config.netmask, sizeof(config.netmask) },
		{ 3, true, false, &config.gateway, sizeof(config.gateway) },
		{ 53, true, false, &type, sizeof(type) },
	};
	int ret;

	if (options->bootp_bound || port != 67 || !IP_IS_V4(addr) ||
	    p->tot_len < BOOTP_MIN_LEN ||
	    pbuf_copy_partial(p, &msg, DHCP_OPTIONS_OFS, 0) != DHCP_OPTIONS_OFS ||
	    msg.op != DHCP_BOOTREPLY || msg.htype != 1 ||
	    msg.hlen != netif->hwaddr_len || msg.hlen > sizeof(msg.chaddr) ||
	    memcmp(msg.chaddr, netif->hwaddr, msg.hlen) ||
	    ntohl(msg.xid) != options->bootp_xid)
		goto out;
	memcpy(&config.ip, &msg.yiaddr, sizeof(config.ip));
	if (ip4_addr_isany(&config.ip) || ip4_addr_ismulticast(&config.ip) ||
	    ip4_addr_get_u32(&config.ip) == IPADDR_BROADCAST)
		goto out;
	memcpy(&config.server, &msg.siaddr, sizeof(config.server));
	config.next_server = config.server;
	memset(data, 0, sizeof(*data));
	if (ntohl(msg.cookie) == DHCP_MAGIC_COOKIE) {
		if (dhcp_parse_boot_data(p, data, false))
			goto out;
		ret = dhcp_boot_options(p, DHCP_OPTIONS_OFS, p->tot_len,
					opts, ARRAY_SIZE(opts), &overload);
		if (!ret && (overload & DHCP_OVERLOAD_FILE))
			ret = dhcp_boot_options(p, DHCP_FILE_OFS, DHCP_FILE_OFS + DHCP_FILE_LEN,
						opts, ARRAY_SIZE(opts), NULL);
		if (!ret && (overload & DHCP_OVERLOAD_SNAME))
			ret = dhcp_boot_options(p, DHCP_SNAME_OFS, DHCP_SNAME_OFS + DHCP_SNAME_LEN,
						opts, ARRAY_SIZE(opts), NULL);
		/*
		 * A DHCP offer/ACK is not a BOOTP assignment. In particular, never
		 * use an offer without completing the DHCP request/ACK exchange.
		 */
		if (ret || opts[2].len ||
		    (opts[0].len && opts[0].len != sizeof(config.netmask)) ||
		    opts[1].len % sizeof(config.gateway))
			goto out;
		config.have_netmask = opts[0].len != 0;
		config.have_gateway = opts[1].len != 0;
	} else {
		/* RFC 951 permits a vendor area without RFC 1048 extensions. */
		memcpy(data->bootfile, msg.file, DHCP_FILE_LEN);
		data->bootfile[DHCP_FILE_LEN] = 0;
	}
	options->reply = *data;
	options->config = config;
	options->bootp_bound = true;
out:
	pbuf_free(p);
}

static int bootp_send(struct udp_pcb *pcb, struct dhcp_options *options)
{
	static const u8 zeros[32];
	const struct {
		u8 code;
		u8 len;
		bool enabled;
	} requests[] = {
		{ 1, 4, IS_ENABLED(CONFIG_BOOTP_SUBNETMASK) },
		{ 3, 4, IS_ENABLED(CONFIG_BOOTP_GATEWAY) },
		{ 6, 4, IS_ENABLED(CONFIG_BOOTP_DNS) },
		{ 12, 32, IS_ENABLED(CONFIG_BOOTP_HOSTNAME) && !options->hostname[0] },
		{ 13, 2, IS_ENABLED(CONFIG_BOOTP_BOOTFILESIZE) },
		{ 17, 32, IS_ENABLED(CONFIG_BOOTP_BOOTPATH) },
		{ 40, 32, IS_ENABLED(CONFIG_BOOTP_NISDOMAIN) },
		{ 42, 4, IS_ENABLED(CONFIG_BOOTP_NTPSERVER) },
		{ 2, 4, IS_ENABLED(CONFIG_BOOTP_TIMEOFFSET) },
	};
	struct dhcp_msg *msg;
	struct pbuf *p;
	u16 len = 0;
	unsigned int i;
	int ret = -EINVAL;

	p = pbuf_alloc(PBUF_TRANSPORT, sizeof(*msg), PBUF_RAM);
	if (!p)
		return -ENOMEM;
	msg = p->payload;
	memset(msg, 0, sizeof(*msg));
	msg->op = DHCP_BOOTREQUEST;
	msg->htype = 1;
	msg->hlen = options->netif->hwaddr_len;
	msg->xid = htonl(options->bootp_xid);
	msg->secs = htons(min_t(ulong, get_timer(options->start) / 1000, U16_MAX));
	memcpy(msg->chaddr, options->netif->hwaddr, msg->hlen);
	copy_filename((char *)msg->file, net_boot_file_name, sizeof(msg->file));
	msg->cookie = htonl(DHCP_MAGIC_COOKIE);
	dhcp_append(options, msg, &len, 60, options->vendor, strlen(options->vendor));
	dhcp_append(options, msg, &len, 12, options->hostname, strlen(options->hostname));
	/*
	 * BOOTP uses zero-filled RFC 1048 tags, not a DHCP parameter request
	 * list. There is deliberately no DHCP message type or lease request.
	 */
	for (i = 0; i < ARRAY_SIZE(requests); i++)
		if (requests[i].enabled)
			dhcp_append(options, msg, &len, requests[i].code, zeros, requests[i].len);
	if (options->append_failed)
		goto out;
	msg->options[len++] = DHCP_OPTION_END;
	pbuf_realloc(p, max_t(u16, BOOTP_MIN_LEN, DHCP_OPTIONS_OFS + len));
	ret = udp_sendto_if_src(pcb, p, IP_ADDR_BROADCAST, 67,
				options->netif, IP4_ADDR_ANY);
out:
	pbuf_free(p);

	return ret;
}

static int bootp_loop(struct net_lwip_ctx *net, bool explicit_file,
		      struct dhcp_options *options)
{
	struct udp_pcb *pcb;
	ulong sent, timeout;
	u8 tries = 1;
	int ret;

	pcb = udp_new_ip_type(IPADDR_TYPE_V4);
	if (!pcb)
		return -ENOMEM;
	ip_set_option(pcb, SOF_BROADCAST);
	udp_bind_netif(pcb, net->netif);
	ret = udp_bind(pcb, IP4_ADDR_ANY, 68);
	if (ret)
		goto out;
	udp_recv(pcb, bootp_recv, options);
	options->bootp_xid = rand();
	options->bootp_bound = false;
	options->start = get_timer(0);
	sent = options->start;
	timeout = bootp_timeout(options, tries);
	ret = bootp_send(pcb, options);
	while (!ret) {
		ret = net_lwip_poll();
		if (ret < 0)
			break;
		if (ctrlc()) {
			printf("Abort\n");
			ret = -EINTR;
			break;
		}
		if (options->bootp_bound) {
			ret = bootp_store_config(net, explicit_file, options);
			if (!ret)
				printf("BOOTP bound to address %pI4 (%lu ms)\n",
				       &options->config.ip, get_timer(options->start));
			break;
		}
		if (get_timer(options->start) >= options->timeout) {
			ret = -ETIMEDOUT;
			break;
		}
		ret = 0;
		if (get_timer(sent) >= timeout) {
			ret = bootp_send(pcb, options);
			sent = get_timer(0);
			if (tries < U8_MAX)
				tries++;
			timeout = bootp_timeout(options, tries);
		}
		mdelay(1);
	}
out:
	/* Remove the receive callback before retry, handoff or stack release. */
	udp_remove(pcb);

	return ret;
}

static int dhcp_nfs_autoload(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	const char *server = env_get("tftpserverip");
	int ret;

	/*
	 * DHCP's siaddr is a boot server, not necessarily the address server
	 * stored in serverip. Supply it only for this download, preserving an
	 * explicit nfsserverip and the NFS command's filename-prefix priority.
	 */
	if (!server || env_get("nfsserverip"))
		return do_nfs(cmdtp, flag, argc, argv);
	if (env_set("nfsserverip", server))
		return CMD_RET_FAILURE;
	ret = do_nfs(cmdtp, flag, argc, argv);
	if (env_set("nfsserverip", NULL))
		return CMD_RET_FAILURE;

	return ret;
}

static int bootp_run(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[], bool bootp)
{
	struct net_lwip_ctx net = {};
	struct dhcp_options *options;
	const char *autoload;
	const char *filename = NULL;
	ulong addr;
	char *end;
	const char *hostname, *vendor;
	int ret;

	switch (argc) {
	case 1:
		break;
	case 2:
		hextoul(argv[1], &end);
		if (end == argv[1] || *end)
			filename = argv[1];
		break;
	case 3:
		if (strict_strtoul(argv[1], 16, &addr))
			return CMD_RET_USAGE;
		filename = argv[2];
		break;
	default:
		return CMD_RET_USAGE;
	}
	copy_filename(net_boot_file_name, filename ?: env_get("bootfile"),
		      sizeof(net_boot_file_name));

	hostname = IS_ENABLED(CONFIG_BOOTP_SEND_HOSTNAME) ? env_get("hostname") : NULL;
	vendor = env_get("bootp_vci") ?: CONFIG_BOOTP_VCI_STRING;
	if ((hostname && strlen(hostname) > 255) || strlen(vendor) > 255) {
		log_err("BOOTP/DHCP hostname/vendor class exceeds 255 bytes\n");
		return CMD_RET_FAILURE;
	}
	options = calloc(1, sizeof(*options) +
			 CONFIG_IS_ENABLED(EFI_LOADER) * sizeof(options->efi[0]));
	if (!options)
		return CMD_RET_FAILURE;
	strlcpy(options->hostname, hostname ?: "", sizeof(options->hostname));
	strlcpy(options->vendor, vendor, sizeof(options->vendor));
	ret = dhcp_timing_options(options);
	if (IS_ENABLED(CONFIG_CMD_DHCP) && !ret && !bootp)
		ret = dhcp_pxe_options(options);
	if (ret) {
		free(options);
		return CMD_RET_FAILURE;
	}
	if (net_lwip_start(&net, NET_LWIP_ADDR_NONE)) {
		free(options);
		return CMD_RET_FAILURE;
	}
	net_try_count = 1;
	net_restart_wrap = 0;
	active_options = options;

	for (;;) {
		if (dev_seq(net.dev) < 0 || dev_seq(net.dev) > 99 || net.netif->hwaddr_len != 6) {
			ret = CMD_RET_FAILURE;
			break;
		}
		options->netif = net.netif;
		options->have_offer = false;
		options->have_proxy = false;
		memset(&options->reply, 0, sizeof(options->reply));
		if (CONFIG_IS_ENABLED(EFI_LOADER)) {
			options->efi->ack_len = 0;
			options->efi->proxy_len = 0;
		}
		ret = CMD_RET_FAILURE;
		if (IS_ENABLED(CONFIG_CMD_BOOTP) && bootp)
			ret = bootp_loop(&net, !!filename, options);
		if (IS_ENABLED(CONFIG_CMD_DHCP) && !bootp)
			ret = dhcp_loop(&net, !!filename, options);
		if (ret != -ETIMEDOUT)
			break;
		if (IS_ENABLED(CONFIG_CMD_DHCP) && !bootp)
			dhcp_retire(&net);
		if (IS_ENABLED(CONFIG_BOOTP_MAY_FAIL) &&
		    (env_get_yesno("ethrotate") == 0 || net_restart_wrap))
			break;
		/* Other runtime clients must not lose their interface on retry. */
		ret = net_lwip_restart(&net);
		if (ret || (IS_ENABLED(CONFIG_BOOTP_MAY_FAIL) && net_restart_wrap)) {
			ret = CMD_RET_FAILURE;
			break;
		}
	}
	/*
	 * A download needs strict addressing, incompatible with the
	 * address-less runtime attachment. Retire DHCP before handing off;
	 * the BOOTP loop has already removed its receive callback.
	 */
	if (IS_ENABLED(CONFIG_CMD_DHCP) && !bootp)
		dhcp_retire(&net);
	active_options = NULL;
	free(options);
	net_lwip_stop(&net);

	if (ret)
		return CMD_RET_FAILURE;
	autoload = env_get("autoload");
	if (autoload && !strcmp(autoload, "NFS")) {
		if (IS_ENABLED(CONFIG_CMD_NFS))
			return dhcp_nfs_autoload(cmdtp, flag, argc, argv);
		log_err("Cannot autoload with NFS: command is disabled\n");
		return CMD_RET_FAILURE;
	}
	if (env_get_yesno("autoload") == 0)
		return CMD_RET_SUCCESS;
	if (IS_ENABLED(CONFIG_CMD_TFTPBOOT))
		return do_tftpb(cmdtp, flag, argc, argv);
	log_err("Cannot autoload with TFTP: command is disabled\n");

	return CMD_RET_FAILURE;
}

int do_dhcp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return bootp_run(cmdtp, flag, argc, argv, false);
}

int do_bootp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return bootp_run(cmdtp, flag, argc, argv, true);
}
