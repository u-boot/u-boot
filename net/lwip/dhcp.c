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
#include <u-boot/uuid.h>

#define DHCP_TIMEOUT_MS ((3ULL + 5ULL * CONFIG_NET_RETRY_COUNT) * 1000)
#define DHCP_RETRANSMIT_MAX_MS (U16_MAX * DHCP_FINE_TIMER_MSECS)

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

/* The address-less runtime attachment excludes a second DHCP command. */
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

/* The core rounds the result up to its fine timer interval. */
u32_t net_lwip_dhcp_timeout(u8_t tries)
{
	struct dhcp_options *options = active_options;
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
			log_err("Invalid DHCP timing setting: %s\n", names[i]);
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
			 * For the NTP list retain only the first address, but validate
			 * the length of the entire list below.
			 */
			if (code != DHCP_OPTION_NTP && code != DHCP_OPTION_DNS_SERVER &&
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

static int dhcp_server_env(struct dhcp *dhcp, struct dhcp_options *options)
{
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

	return env_set("serverip", ip4addr_ntoa(&dhcp->server_ip_addr)) ||
		env_set("tftpserverip", ip4_addr_isany(&dhcp->offered_si_addr) ?
			NULL : ip4addr_ntoa(&dhcp->offered_si_addr));
}

static int dhcp_loop(struct net_lwip_ctx *net, bool explicit_file,
		     struct dhcp_options *options)
{
	char ipstr[] = "ipaddr\0\0\0";
	char maskstr[] = "netmask\0\0\0";
	char gwstr[] = "gatewayip\0\0\0";
	unsigned long start;
	unsigned long proxy_start = 0;
	struct dhcp *dhcp;
	bool bound = false;
	int idx;

	idx = dev_seq(net->dev);
	if (idx < 0 || idx > 99) {
		log_err("unexpected idx %d\n", idx);
		return CMD_RET_FAILURE;
	}

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
			return CMD_RET_FAILURE;
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
		return CMD_RET_FAILURE;

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

	/* The core's file field also reflects replies it subsequently ignores. */
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
		net_ip.s_addr = ip_addr_get_ip4_u32(&dhcp->offered_ip_addr);
	}

	if (env_set(ipstr, ip4addr_ntoa(&dhcp->offered_ip_addr)) ||
	    env_set(maskstr, ip4addr_ntoa(&dhcp->offered_sn_mask)) ||
	    env_set(gwstr, ip4addr_ntoa(&dhcp->offered_gw_addr)) ||
	    dhcp_server_env(dhcp, options))
		return CMD_RET_FAILURE;

	if (dhcp_boot_env(&options->reply))
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

int do_dhcp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
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
		log_err("DHCP hostname/vendor class exceeds 255 bytes\n");
		return CMD_RET_FAILURE;
	}
	options = calloc(1, sizeof(*options) +
			 CONFIG_IS_ENABLED(EFI_LOADER) * sizeof(options->efi[0]));
	if (!options)
		return CMD_RET_FAILURE;
	strlcpy(options->hostname, hostname ?: "", sizeof(options->hostname));
	strlcpy(options->vendor, vendor, sizeof(options->vendor));
	if (dhcp_pxe_options(options) || dhcp_timing_options(options)) {
		free(options);
		return CMD_RET_FAILURE;
	}
	if (net_lwip_start(&net, NET_LWIP_ADDR_NONE)) {
		free(options);
		return CMD_RET_FAILURE;
	}
	options->netif = net.netif;
	active_options = options;

	ret = dhcp_loop(&net, !!filename, options);
	/*
	 * A download needs strict addressing, incompatible with DHCP's
	 * address-less runtime attachment. Retire DHCP before handing off.
	 */
	if (net.netif) {
		if (dhcp_supplied_address(net.netif))
			dhcp_stop_without_release(net.netif);
		else
			dhcp_release_and_stop(net.netif);
		dhcp_cleanup(net.netif);
	}
	active_options = NULL;
	free(options);
	net_lwip_stop(&net);

	if (ret)
		return ret;
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
