// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <command.h>
#include <console.h>
#include <env.h>
#include <image.h>
#include <malloc.h>
#include <net.h>
#include <rand.h>
#include <time.h>
#include <asm/unaligned.h>
#include <lwip/udp.h>
#include <lwip/prot/dhcp6.h>

#define DHCP6_DUID_MAX_LEN 130
#define DHCP6_HOST_MAX_LEN 256
#define DHCP6_OPTION_OPT_BOOTFILE_URL 59
#define DHCP6_OPTION_OPT_BOOTFILE_PARAM 60
#define DHCP6_OPTION_CLIENT_ARCH_TYPE 61
#define DHCP6_OPTION_NII 62
#define DHCP6_OPTION_SOL_MAX_RT 82
#define DHCP6_SOL_MAX_RT 3600
#define DHCP6_TIMEOUT_MS ((3ULL + 5ULL * CONFIG_NET_RETRY_COUNT) * 1000)
#define DHCP6_REQ_TIMEOUT_MS 1000
#define DHCP6_REQ_MAX_RT_MS 30000
#define DHCP6_REQ_MAX_TRIES 10

struct dhcp6_offer {
	u8 server[DHCP6_DUID_MAX_LEN];
	u16 server_len;
	ip6_addr_t addr;
	u32 preferred;
	u32 valid;
	u32 t1;
	u32 t2;
	bool address;
	bool no_binding;
	bool failed;
	u32 sol_max_rt;
	u8 preference;
	ip6_addr_t dns[2];
	u8 dns_count;
	char url[1024];
	char pxe[256];
};

struct dhcp6_client {
	struct net_lwip_ctx net;
	struct udp_pcb *pcb;
	u8 duid[10];
	u32 iaid;
	u32 xid;
	u8 message;
	ulong exchange_start;
	struct dhcp6_offer offer;
	/* The published address can differ from a replacement undergoing DAD. */
	ip6_addr_t env_addr;
	bool done;
	bool bound;
	bool dad_pending;
	ulong dad_start;
	u64 acquired;
	u64 next_send;
	u32 retry_ms;
	u32 sol_max_rt;
	u32 advertised_sol_max_rt;
	unsigned int tries;
};

/* Lease metadata survives commands; PCBs never outlive their netif. */
static struct dhcp6_client *dhcp6_lease;

static void dhcp6_begin(struct dhcp6_client *client, u8 message)
{
	client->message = message;
	client->xid = rand() & 0xffffff;
	client->exchange_start = get_timer(0);
	client->next_send = get_timer_us(0);
	client->retry_ms = 0;
	client->tries = 0;
	client->done = false;
	if (message == DHCP6_SOLICIT)
		client->advertised_sol_max_rt = 0;
}

/* Advance only after checking the complete TLV against its enclosing option. */
static int dhcp6_option(const u8 **data, size_t *left, u16 *code,
			const u8 **value, u16 *len)
{
	if (!*left)
		return 0;
	if (*left < 4)
		return -EINVAL;
	*code = get_unaligned_be16(*data);
	*len = get_unaligned_be16(*data + 2);
	if (*len > *left - 4)
		return -EINVAL;
	*value = *data + 4;
	*data += 4 + *len;
	*left -= 4 + *len;
	return 1;
}

static int dhcp6_status(const u8 *data, size_t left, bool *failed)
{
	const u8 *value;
	u16 code, len;
	int ret;

	while ((ret = dhcp6_option(&data, &left, &code, &value, &len)) > 0) {
		if (code == DHCP6_OPTION_STATUS_CODE) {
			if (len < 2)
				return -EINVAL;
			*failed |= get_unaligned_be16(value) != 0;
		}
	}
	return ret;
}

static int dhcp6_ia(struct dhcp6_client *client, const u8 *data, size_t left,
		    struct dhcp6_offer *offer)
{
	const u8 *value;
	ip6_addr_t addr;
	u16 code, len;
	u32 preferred, valid;
	int ret;

	while ((ret = dhcp6_option(&data, &left, &code, &value, &len)) > 0) {
		if (code == DHCP6_OPTION_STATUS_CODE) {
			if (len < 2)
				return -EINVAL;
			if (client->bound && get_unaligned_be16(value) == DHCP6_STATUS_NOBINDING)
				offer->no_binding = true;
			else if (get_unaligned_be16(value))
				offer->failed = true;
		}
		if (code != DHCP6_OPTION_IAADDR)
			continue;
		if (len < 24 || dhcp6_status(value + 24, len - 24, &offer->failed))
			return -EINVAL;
		preferred = get_unaligned_be32(value + 16);
		valid = get_unaligned_be32(value + 20);
		if (preferred > valid)
			continue;
		/* A withdrawal only applies to our current address. */
		if (!valid && (!client->bound || memcmp(value, client->offer.addr.addr, 16)))
			continue;
		/* Prefer a usable address to a withdrawal, regardless of option order. */
		if (offer->address && (offer->valid || !valid))
			continue;
		memset(&addr, 0, sizeof(addr));
		memcpy(addr.addr, value, 16);
		if (ip6_addr_isany(&addr) || ip6_addr_ismulticast(&addr) ||
		    ip6_addr_islinklocal(&addr) || ip6_addr_isloopback(&addr) ||
		    ip6_addr_isipv4mappedipv6(&addr) || ip6_addr_isipv4compat(&addr))
			continue;
		ip6_addr_copy(offer->addr, addr);
		offer->preferred = preferred;
		offer->valid = valid;
		offer->address = true;
	}
	return ret;
}

static int dhcp6_text(char *dest, size_t size, const u8 *value, u16 len)
{
	if (!len || len >= size || memchr(value, 0, len))
		return -EINVAL;
	memcpy(dest, value, len);
	dest[len] = 0;
	return 0;
}

static int dhcp6_parse(struct dhcp6_client *client, const u8 *data, size_t left,
		       struct dhcp6_offer *offer, bool reply)
{
	bool client_id = false, ia = false, rapid = false;
	const u8 *value;
	u16 code, len;
	int ret;

	while ((ret = dhcp6_option(&data, &left, &code, &value, &len)) > 0) {
		switch (code) {
		case DHCP6_OPTION_CLIENTID:
			if (client_id || len != sizeof(client->duid) ||
			    memcmp(value, client->duid, len))
				return -EINVAL;
			client_id = true;
			break;
		case DHCP6_OPTION_SERVERID:
			if (offer->server_len || len < 2 || len > sizeof(offer->server))
				return -EINVAL;
			memcpy(offer->server, value, len);
			offer->server_len = len;
			break;
		case DHCP6_OPTION_IA_NA:
			if (len < 12)
				return -EINVAL;
			if (get_unaligned_be32(value) != client->iaid)
				break;
			if (ia || (get_unaligned_be32(value + 8) &&
				   get_unaligned_be32(value + 4) > get_unaligned_be32(value + 8)))
				return -EINVAL;
			ia = true;
			offer->t1 = get_unaligned_be32(value + 4);
			offer->t2 = get_unaligned_be32(value + 8);
			if (dhcp6_ia(client, value + 12, len - 12, offer))
				return -EINVAL;
			break;
		case DHCP6_OPTION_STATUS_CODE:
			if (len < 2)
				return -EINVAL;
			offer->failed |= get_unaligned_be16(value) != 0;
			break;
		case DHCP6_OPTION_PREFERENCE:
			if (len != 1)
				return -EINVAL;
			offer->preference = *value;
			break;
		case DHCP6_OPTION_RAPID_COMMIT:
			if (len || rapid)
				return -EINVAL;
			rapid = true;
			break;
		case DHCP6_OPTION_SOL_MAX_RT:
			if (len != 4)
				return -EINVAL;
			offer->sol_max_rt = get_unaligned_be32(value);
			if (offer->sol_max_rt < 60 || offer->sol_max_rt > 86400)
				offer->sol_max_rt = 0;
			break;
		case DHCP6_OPTION_DNS_SERVERS:
			if (!len || len % 16)
				return -EINVAL;
			offer->dns_count = min_t(unsigned int, len / 16, ARRAY_SIZE(offer->dns));
			for (unsigned int i = 0; i < offer->dns_count; i++) {
				memcpy(offer->dns[i].addr, value + 16 * i, 16);
				if (ip6_addr_isany(&offer->dns[i]) ||
				    ip6_addr_ismulticast(&offer->dns[i]))
					return -EINVAL;
			}
			break;
		case DHCP6_OPTION_OPT_BOOTFILE_URL:
			if (offer->url[0] || dhcp6_text(offer->url, sizeof(offer->url), value, len))
				return -EINVAL;
			break;
		case DHCP6_OPTION_OPT_BOOTFILE_PARAM:
			if (IS_ENABLED(CONFIG_DHCP6_PXE_DHCP_OPTION)) {
				bool first = true;

				while (len) {
					u16 size;

					if (len < 2)
						return -EINVAL;
					size = get_unaligned_be16(value);
					if (size > len - 2)
						return -EINVAL;
					if (first && dhcp6_text(offer->pxe, sizeof(offer->pxe),
							       value + 2, size))
						return -EINVAL;
					first = false;
					value += size + 2;
					len -= size + 2;
				}
			}
			break;
		}
	}
	if (ret || !client_id || !offer->server_len)
		return -EINVAL;
	if (reply && client->message == DHCP6_SOLICIT)
		return IS_ENABLED(CONFIG_DHCP6_RAPID_COMMIT) && rapid ? 0 : -EINVAL;
	if (reply && client->message != DHCP6_REBIND &&
	    (offer->server_len != client->offer.server_len ||
		      memcmp(offer->server, client->offer.server, offer->server_len)))
		return -EINVAL;
	return 0;
}

static void dhcp6_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
		       const ip_addr_t *addr, u16 port)
{
	struct dhcp6_client *client = arg;
	struct dhcp6_offer *offer;
	u8 *data;
	bool reply;

	if (client->done || !client->message || port != DHCP6_SERVER_PORT || !IP_IS_V6(addr) ||
	    p->tot_len < 4 || p->tot_len > 4096)
		goto out;
	data = malloc(p->tot_len);
	offer = calloc(1, sizeof(*offer));
	if (!data || !offer)
		goto free;
	pbuf_copy_partial(p, data, p->tot_len, 0);
	if ((((u32)data[1] << 16) | (data[2] << 8) | data[3]) != client->xid)
		goto free;
	reply = data[0] == DHCP6_REPLY;
	if (!reply && (client->message != DHCP6_SOLICIT || data[0] != DHCP6_ADVERTISE))
		goto free;
	if (dhcp6_parse(client, data + 4, p->tot_len - 4, offer, reply))
		goto free;
	/* Retry parameters also apply to valid replies that cannot offer an address. */
	if (offer->sol_max_rt) {
		if (!reply) {
			if (!client->advertised_sol_max_rt)
				client->advertised_sol_max_rt = offer->sol_max_rt;
			else if (client->advertised_sol_max_rt != offer->sol_max_rt)
				client->advertised_sol_max_rt = U32_MAX;
			/* Conflicting Advertises keep the default for this exchange. */
			client->sol_max_rt = client->advertised_sol_max_rt == U32_MAX ?
					     DHCP6_SOL_MAX_RT : client->advertised_sol_max_rt;
		} else {
			client->sol_max_rt = offer->sol_max_rt;
		}
	}
	if (offer->failed || (!offer->address && !offer->no_binding))
		goto free;
	if (reply && offer->no_binding) {
		if (client->message != DHCP6_RENEW && client->message != DHCP6_REBIND)
			goto free;
		dhcp6_begin(client, DHCP6_REQUEST);
		client->next_send += 1000000;
		memcpy(client->offer.server, offer->server, offer->server_len);
		client->offer.server_len = offer->server_len;
		goto free;
	}
	if (reply || !client->offer.server_len || offer->preference > client->offer.preference)
		client->offer = *offer;
	if (reply) {
		client->done = true;
		client->acquired = get_timer_us(0);
	}
free:
	free(offer);
	free(data);
out:
	pbuf_free(p);
}

static u8 *dhcp6_add(u8 *p, u16 code, const void *value, u16 len)
{
	put_unaligned_be16(code, p);
	put_unaligned_be16(len, p + 2);
	if (len)
		memcpy(p + 4, value, len);
	return p + 4 + len;
}

static int dhcp6_send(struct dhcp6_client *client)
{
	u8 packet[512], ia[40] = {}, elapsed[2], vendor[12], arch[2];
	const u8 nii[] = { 1, 0, 0 };
	u8 oro[] = { 0, DHCP6_OPTION_OPT_BOOTFILE_URL, 0, DHCP6_OPTION_DNS_SERVERS,
		     0, DHCP6_OPTION_SOL_MAX_RT, 0, DHCP6_OPTION_OPT_BOOTFILE_PARAM };
	ip_addr_t dest;
	struct pbuf *buf;
	u8 *p = packet;
	u16 ia_len = 12;
	err_t err;

	if (!ip6_addr_isvalid(netif_ip6_addr_state(client->net.netif, 0)))
		return -EADDRNOTAVAIL;

	*p++ = client->message;
	*p++ = client->xid >> 16;
	*p++ = client->xid >> 8;
	*p++ = client->xid;
	p = dhcp6_add(p, DHCP6_OPTION_CLIENTID, client->duid, sizeof(client->duid));
	put_unaligned_be16(min_t(ulong, get_timer(client->exchange_start) / 10, U16_MAX), elapsed);
	p = dhcp6_add(p, DHCP6_OPTION_ELAPSED_TIME, elapsed, sizeof(elapsed));
	put_unaligned_be32(client->iaid, ia);
	if (client->message != DHCP6_SOLICIT) {
		if (client->message != DHCP6_REBIND)
			p = dhcp6_add(p, DHCP6_OPTION_SERVERID, client->offer.server,
				      client->offer.server_len);
		put_unaligned_be16(DHCP6_OPTION_IAADDR, ia + 12);
		put_unaligned_be16(24, ia + 14);
		memcpy(ia + 16, client->offer.addr.addr, 16);
		ia_len = sizeof(ia);
	}
	p = dhcp6_add(p, DHCP6_OPTION_IA_NA, ia, ia_len);
	p = dhcp6_add(p, DHCP6_OPTION_ORO, oro, IS_ENABLED(CONFIG_DHCP6_PXE_DHCP_OPTION) ? 8 : 6);
	if (IS_ENABLED(CONFIG_DHCP6_RAPID_COMMIT) && client->message == DHCP6_SOLICIT)
		p = dhcp6_add(p, DHCP6_OPTION_RAPID_COMMIT, NULL, 0);
	if (CONFIG_DHCP_PXE_CLIENTARCH != 0xff) {
		put_unaligned_be16(CONFIG_DHCP_PXE_CLIENTARCH, arch);
		p = dhcp6_add(p, DHCP6_OPTION_CLIENT_ARCH_TYPE, arch, sizeof(arch));
	}
	put_unaligned_be32(CONFIG_DHCP6_ENTERPRISE_ID, vendor);
	put_unaligned_be16(6, vendor + 4);
	memcpy(vendor + 6, "U-Boot", 6);
	p = dhcp6_add(p, DHCP6_OPTION_VENDOR_CLASS, vendor, sizeof(vendor));
	p = dhcp6_add(p, DHCP6_OPTION_NII, nii, sizeof(nii));
	buf = pbuf_alloc(PBUF_TRANSPORT, p - packet, PBUF_RAM);
	if (!buf)
		return -ENOMEM;
	pbuf_take(buf, packet, p - packet);
	ipaddr_aton("ff02::1:2", &dest);
	ip6_addr_assign_zone(ip_2_ip6(&dest), IP6_MULTICAST, client->net.netif);
	err = udp_sendto_if_src(client->pcb, buf, &dest, DHCP6_SERVER_PORT, client->net.netif,
				netif_ip_addr6(client->net.netif, 0));
	pbuf_free(buf);
	return err == ERR_OK ? 0 : -EIO;
}

static int dhcp6_transmit(struct dhcp6_client *client)
{
	u32 initial, max_rt, base, rt;
	int ret;

	switch (client->message) {
	case DHCP6_REQUEST:
		if (client->tries >= DHCP6_REQ_MAX_TRIES)
			return -ETIMEDOUT;
		initial = DHCP6_REQ_TIMEOUT_MS;
		max_rt = DHCP6_REQ_MAX_RT_MS;
		break;
	case DHCP6_SOLICIT:
		initial = 1000;
		max_rt = (client->sol_max_rt ?: DHCP6_SOL_MAX_RT) * 1000;
		break;
	default:
		initial = 10000;
		max_rt = 600000;
		break;
	}
	ret = dhcp6_send(client);
	base = client->tries ? client->retry_ms : initial;
	rt = client->tries ? 2 * base : base;
	/* The first Solicit RT must exceed SOL_TIMEOUT (RFC 8415, 18.2.1). */
	if (!client->tries && client->message == DHCP6_SOLICIT)
		rt += 1 + rand() % (base / 10);
	else
		rt = rt - base / 10 + rand() % (base / 5 + 1);
	if (rt > max_rt)
		rt = max_rt - max_rt / 10 + rand() % (max_rt / 5 + 1);
	client->retry_ms = rt;
	client->next_send = get_timer_us(0) + (u64)rt * 1000;
	client->tries++;
	return ret;
}

static int dhcp6_attach(struct dhcp6_client *client)
{
	client->pcb = udp_new_ip_type(IPADDR_TYPE_V6);
	if (!client->pcb)
		return -ENOMEM;
	if (udp_bind(client->pcb, IP6_ADDR_ANY, DHCP6_CLIENT_PORT) != ERR_OK) {
		udp_remove(client->pcb);
		client->pcb = NULL;
		return -EADDRINUSE;
	}
	udp_bind_netif(client->pcb, client->net.netif);
	udp_recv(client->pcb, dhcp6_recv, client);
	return 0;
}

void net_lwip_dhcp6_detach(struct netif *netif)
{
	if (!dhcp6_lease || dhcp6_lease->net.netif != netif)
		return;
	if (dhcp6_lease->pcb)
		udp_remove(dhcp6_lease->pcb);
	dhcp6_lease->pcb = NULL;
	memset(&dhcp6_lease->net, 0, sizeof(dhcp6_lease->net));
}

static void dhcp6_forget(void)
{
	if (!dhcp6_lease)
		return;
	net_lwip_dhcp6_detach(dhcp6_lease->net.netif);
	free(dhcp6_lease);
	dhcp6_lease = NULL;
}

static int dhcp6_discard(void)
{
	const char *value = env_get("ip6addr");
	char text[IP6ADDR_STRLEN_MAX];
	ip6_addr_t addr;
	size_t len;
	int ret;

	/* Do not turn the old lease into a static address if acquisition fails. */
	if (dhcp6_lease && value) {
		len = strcspn(value, "/");
		if (len < sizeof(text)) {
			memcpy(text, value, len);
			text[len] = 0;
			if (!net_lwip_ip6_aton(text, &addr) &&
			    ip6_addr_zoneless_eq(&addr, &dhcp6_lease->env_addr)) {
				ret = env_set("ip6addr", NULL);
				if (ret)
					return ret;
			}
		}
	}
	dhcp6_forget();
	return 0;
}

static int dhcp6_publish_options(struct dhcp6_client *client)
{
	int ret;

	for (unsigned int i = 0; i < ARRAY_SIZE(client->offer.dns); i++) {
		ret = env_set(i ? "dnsip6_2" : "dnsip6", i < client->offer.dns_count ?
			      ip6addr_ntoa(&client->offer.dns[i]) : NULL);
		if (ret)
			return ret;
	}
#if CONFIG_IS_ENABLED(DNS)
	/* Updating the resolver must not recursively poll from lease maintenance. */
	net_lwip_dns_refresh(&client->net);
#endif
	if (IS_ENABLED(CONFIG_DHCP6_PXE_DHCP_OPTION))
		return env_set("pxe_configfile", client->offer.pxe[0] ? client->offer.pxe : NULL);
	return 0;
}

static u32 dhcp6_remaining(struct dhcp6_client *client, u32 lifetime)
{
	u64 elapsed = (get_timer_us(0) - client->acquired) / 1000000;

	if (lifetime == U32_MAX)
		return lifetime;
	return elapsed >= lifetime ? 0 : lifetime - elapsed;
}

static int dhcp6_lifetimes(struct dhcp6_client *client)
{
	struct netif *netif = client->net.netif;
	u32 valid = dhcp6_remaining(client, client->offer.valid);
	u32 preferred = dhcp6_remaining(client, client->offer.preferred);

	/* Zero is lwIP's static-address sentinel, not a dynamic expiry. */
	if (!valid) {
		netif_ip6_addr_set_state(netif, 1, IP6_ADDR_INVALID);
		return -EADDRNOTAVAIL;
	}
	netif_ip6_addr_set_valid_life(netif, 1, valid);
	netif_ip6_addr_set_pref_life(netif, 1, preferred);
	if (ip6_addr_isvalid(netif_ip6_addr_state(netif, 1)))
		netif_ip6_addr_set_state(netif, 1,
					preferred ? IP6_ADDR_PREFERRED : IP6_ADDR_DEPRECATED);
	return 0;
}

static int dhcp6_start_dad(struct dhcp6_client *client)
{
	u32 valid = dhcp6_remaining(client, client->offer.valid);
	u32 preferred = dhcp6_remaining(client, client->offer.preferred);
	int ret;

	ret = net_lwip_ip6_lease(&client->net, &client->offer.addr, preferred, valid);
	if (ret)
		return ret;
	client->dad_start = get_timer(0);
	client->dad_pending = true;
	return 0;
}

static int dhcp6_finish_dad(struct dhcp6_client *client)
{
	u8 state = netif_ip6_addr_state(client->net.netif, 1);
	int ret;

	ret = dhcp6_lifetimes(client);
	if (ret)
		return ret;
	if (ip6_addr_isduplicated(state))
		return -EADDRINUSE;
	if (!ip6_addr_isvalid(state)) {
		if (ip6_addr_isinvalid(state) || get_timer(client->dad_start) >= 15000)
			return -ETIMEDOUT;
		return -EAGAIN;
	}
	ret = net_lwip_ip6_lease_commit(&client->net);
	if (ret)
		return ret;
	ip6_addr_copy(client->env_addr, client->offer.addr);
	client->dad_pending = false;
	return 0;
}

static void dhcp6_invalidate(struct dhcp6_client *client)
{
	client->offer.valid = 0;
	client->message = 0;
	client->done = false;
	client->dad_pending = false;
	netif_ip6_addr_set_state(client->net.netif, 1, IP6_ADDR_INVALID);
}

int net_lwip_dhcp6_restore(struct net_lwip_ctx *ctx, const ip6_addr_t *addr)
{
	struct dhcp6_client *client = dhcp6_lease;
	int ret;

	if (!client)
		return 0;
	if (!ip6_addr_zoneless_eq(addr, &client->env_addr)) {
		dhcp6_forget();
		return 0;
	}
	if (memcmp(ctx->netif->hwaddr, client->duid + 4, 6)) {
		netif_ip6_addr_set_state(ctx->netif, 1, IP6_ADDR_INVALID);
		return 0;
	}
	client->net = *ctx;
	netif_set_ip6_autoconfig_enabled(ctx->netif, 0);
	ret = dhcp6_lifetimes(client);
	if (ret)
		return 0;
	/* Restore the accepted lease, not the old address pending publication. */
	ret = dhcp6_start_dad(client);
	if (!ret)
		ret = dhcp6_attach(client);
	if (ret) {
		netif_ip6_addr_set_state(ctx->netif, 1, IP6_ADDR_INVALID);
		client->dad_pending = false;
		net_lwip_dhcp6_detach(ctx->netif);
	}
	return ret;
}

void net_lwip_dhcp6_poll(void)
{
	struct dhcp6_client *client = dhcp6_lease;
	u8 message;
	int ret;

	if (!client || !client->net.netif)
		return;
	if (!dhcp6_remaining(client, client->offer.valid)) {
		dhcp6_invalidate(client);
		return;
	}
	if (client->done &&
	    (!ip6_addr_zoneless_eq(netif_ip6_addr(client->net.netif, 1), &client->offer.addr) ||
	     ip6_addr_isinvalid(netif_ip6_addr_state(client->net.netif, 1)))) {
		ret = dhcp6_start_dad(client);
		if (ret)
			goto failed;
	}
	if (client->dad_pending) {
		ret = dhcp6_finish_dad(client);
		if (ret == -EAGAIN)
			return;
		if (ret)
			goto failed;
	}
	if (client->done) {
		u32 lifetime = client->offer.preferred ?: client->offer.valid;

		ret = dhcp6_lifetimes(client);
		if (ret)
			return;
		ret = dhcp6_publish_options(client);
		if (ret)
			printf("DHCPv6 option update failed: %d\n", ret);
		if (!client->offer.t2)
			client->offer.t2 = max(client->offer.t1, lifetime == U32_MAX ?
					       U32_MAX : max(1U, lifetime / 5 * 4));
		if (!client->offer.t1)
			client->offer.t1 = min(client->offer.t2, lifetime == U32_MAX ?
					       U32_MAX : max(1U, lifetime / 2));
		client->message = 0;
		client->done = false;
	}
	message = !dhcp6_remaining(client, client->offer.t2) ? DHCP6_REBIND :
		  !dhcp6_remaining(client, client->offer.t1) ? DHCP6_RENEW : 0;
	if (!message || !ip6_addr_isvalid(netif_ip6_addr_state(client->net.netif, 0)))
		return;
	if (!client->message || (message == DHCP6_REBIND && client->message == DHCP6_RENEW))
		dhcp6_begin(client, message);
	if (get_timer_us(0) < client->next_send)
		return;
	ret = dhcp6_transmit(client);
	if (ret == -ETIMEDOUT) {
		printf("DHCPv6 Request failed; run dhcp6 to reacquire an address\n");
		dhcp6_invalidate(client);
		return;
	}
	if (ret)
		debug("DHCPv6 renewal send failed: %d\n", ret);
	return;

failed:
	if (ret == -EADDRINUSE) {
		dhcp6_begin(client, DHCP6_DECLINE);
		dhcp6_send(client);
	}
	printf("DHCPv6 address update failed: %d\n", ret);
	dhcp6_invalidate(client);
}

static int dhcp6_acquire(struct dhcp6_client *client)
{
	ulong start;
	int ret;

	ret = net_lwip_ip6_start(&client->net);
	if (ret)
		return ret;
	ret = dhcp6_attach(client);
	if (ret)
		return ret;
	if (memcmp(client->duid + 4, client->net.netif->hwaddr, 6))
		client->sol_max_rt = 0;
	put_unaligned_be16(DHCP6_DUID_LL, client->duid);
	put_unaligned_be16(1, client->duid + 2);
	memcpy(client->duid + 4, client->net.netif->hwaddr, 6);
	client->iaid = get_unaligned_be32(client->duid + 6);
	dhcp6_begin(client, DHCP6_SOLICIT);
	start = get_timer(0);
	/* Randomize the first solicitation to avoid synchronized boot storms. */
	client->next_send += (rand() % 1000) * 1000;
	while (!client->done) {
		if (ctrlc())
			return -EINTR;
		ret = net_lwip_poll();
		if (ret < 0)
			return ret;
		if (client->done)
			break;
		if (get_timer(start) >= DHCP6_TIMEOUT_MS)
			return -ETIMEDOUT;
		if (client->message == DHCP6_SOLICIT && client->offer.server_len &&
		    (client->offer.preference == 255 || get_timer_us(0) >= client->next_send))
			dhcp6_begin(client, DHCP6_REQUEST);
		if (get_timer_us(0) < client->next_send)
			continue;
		ret = dhcp6_transmit(client);
		if (ret)
			return ret;
	}
	ret = dhcp6_start_dad(client);
	if (ret)
		return ret;
	while ((ret = dhcp6_finish_dad(client)) == -EAGAIN) {
		if (ctrlc())
			return -EINTR;
		ret = net_lwip_poll();
		if (ret < 0)
			return ret;
	}
	if (ret == -EADDRINUSE) {
		dhcp6_begin(client, DHCP6_DECLINE);
		dhcp6_send(client);
	}
	return ret;
}

/* Translate RFC 5970 TFTP URLs into the existing TFTP argument syntax. */
static int dhcp6_bootfile(struct dhcp6_client *client, char *file, size_t size)
{
	char host[DHCP6_HOST_MAX_LEN], *url = client->offer.url, *end, *path, *port = NULL;
	u16 dstport = 69;
	unsigned int value;
	int ret;

	if (strncmp(url, "tftp://", 7))
		return strlcpy(file, url, size) >= size ? -ENAMETOOLONG : 0;
	url += 7;
	path = strchr(url, '/');
	if (!path || path == url)
		return -EINVAL;
	if (*url == '[') {
		url++;
		end = memchr(url, ']', path - url);
		if (!end || (end + 1 != path && end[1] != ':'))
			return -EINVAL;
		if (end + 1 != path)
			port = end + 2;
	} else {
		end = memchr(url, ':', path - url);
		if (end)
			port = end + 1;
		else
			end = path;
	}
	if (end == url || end - url >= sizeof(host))
		return -EINVAL;
	memcpy(host, url, end - url);
	host[end - url] = 0;
	if (port) {
		if (net_lwip_parse_decimal(port, path - port, U16_MAX, &value) || !value)
			return -EINVAL;
		dstport = value;
	}
	/* An escaped colon stays in the filename, not in the server prefix. */
	path++;
	if (!*path || strlen(path) >= size)
		return -EINVAL;
	for (end = file; *path; path++) {
		int byte = (u8)*path;

		if (*path == '%') {
			int hi, lo;

			if (!path[1] || !path[2])
				return -EINVAL;
			hi = hex_to_bin(path[1]);
			lo = hex_to_bin(path[2]);
			if (hi < 0 || lo < 0)
				return -EINVAL;
			byte = hi * 16 + lo;
			path += 2;
		}
		if (byte < 0x20 || byte == 0x7f)
			return -EINVAL;
		*end++ = byte;
	}
	*end = 0;
	ret = env_set("serverip6", host);
	if (!ret)
		ret = env_set_ulong("tftpdstp", dstport);
	return ret;
}

static int do_dhcp6(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct net_lwip_ctx net = {};
	struct dhcp6_client *client;
	char file[1024], arg[sizeof(file) + DHCP6_HOST_MAX_LEN + 8];
	char addr[2 * sizeof(ulong) + 1];
	char *tftp_argv[] = { "tftpboot", addr, file, "-ipv6" };
	const char *filename = NULL;
	ulong load = env_get_ulong("loadaddr", 16, image_load_addr);
	bool downloaded = false, offered, tftp_url;
	int ret;

	if (argc > 3 || (argc > 1 && strict_strtoul(argv[1], 16, &load)))
		return CMD_RET_USAGE;
	if (argc == 3)
		filename = argv[2];
	client = calloc(1, sizeof(*client));
	if (!client)
		return CMD_RET_FAILURE;
	ret = net_lwip_start(&net, NET_LWIP_ADDR_ENV_STRICT);
	if (ret)
		goto out;
	if (!net_lwip_is_exclusive(&net)) {
		ret = -EBUSY;
		goto out;
	}
	ret = dhcp6_discard();
	if (ret)
		goto out;
	client->net = net;
	net_try_count = 1;
	for (;;) {
		memset(&client->offer, 0, sizeof(client->offer));
		client->done = false;
		ret = dhcp6_acquire(client);
		if (ret && client->pcb) {
			udp_remove(client->pcb);
			client->pcb = NULL;
		}
		if (ret != -ETIMEDOUT)
			break;
		ret = net_lwip_restart(&net);
		client->net = net;
		if (ret)
			break;
	}
	if (ret)
		goto out;
	client->bound = true;
	dhcp6_lease = client;
	printf("DHCPv6 address: %s\n", ip6addr_ntoa(&client->offer.addr));
	ret = dhcp6_publish_options(client);
	if (ret)
		goto out;
	/* Retain the advertised server even with a caller-supplied filename. */
	offered = client->offer.url[0];
	tftp_url = offered && !strncmp(client->offer.url, "tftp://", 7);
	if (offered) {
		ret = dhcp6_bootfile(client, file, sizeof(file));
		if (ret)
			goto out;
	}
	if (filename) {
		if (strlcpy(file, filename, sizeof(file)) >= sizeof(file)) {
			ret = -ENAMETOOLONG;
			goto out;
		}
	} else if (!offered) {
		filename = env_get("bootfile");
		if (strlcpy(file, filename ?: "", sizeof(file)) >= sizeof(file)) {
			ret = -ENAMETOOLONG;
			goto out;
		}
	}
	if (*file) {
		ret = env_set("bootfile", file);
		if (ret)
			goto out;
	}
	if (env_get_yesno("autoload") == 0)
		goto out;
	if (!*file) {
		ret = -ENOENT;
		goto out;
	}
	if ((offered && !tftp_url) ||
	    ((!tftp_url || filename) && strstr(file, "://"))) {
		ret = -EPROTONOSUPPORT;
		goto out;
	}
	snprintf(addr, sizeof(addr), "%lx", load);
	if (!filename && offered) {
		/* Disambiguate colons in the decoded URL path from TFTP prefixes. */
		snprintf(arg, sizeof(arg), "[%s]:%s:%s", env_get("serverip6"),
			 env_get("tftpdstp"), file);
		tftp_argv[2] = arg;
	}
	if (IS_ENABLED(CONFIG_CMD_TFTPBOOT)) {
		ret = net_lwip_tftp_download(&client->net, ARRAY_SIZE(tftp_argv), tftp_argv);
		downloaded = !ret;
	} else {
		ret = -ENOSYS;
	}
out:
	if (client != dhcp6_lease) {
		if (client->pcb)
			udp_remove(client->pcb);
		free(client);
	}
	net_lwip_stop(&net);
	if (ret) {
		printf("DHCPv6 failed: %d\n", ret);
		return CMD_RET_FAILURE;
	}
	if (downloaded && net_boot_file_size)
		return bootm_maybe_autostart(cmdtp, argv[0]);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(dhcp6, 3, 1, do_dhcp6, "boot using DHCPv6",
	   "[loadAddress] [bootfilename]");
