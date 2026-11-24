/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __NET_LWIP_H__
#define __NET_LWIP_H__

#include <lwip/ip4.h>
#include <lwip/netif.h>

struct udevice;

/**
 * enum net_lwip_addr_mode - IPv4 requirements of a runtime client
 * @NET_LWIP_ADDR_ENV_STRICT: Use environment addressing and require it to
 *	remain stable while the client is attached
 * @NET_LWIP_ADDR_ENV_FLEXIBLE: Use environment addressing, but permit another
 *	client to temporarily change or clear the shared interface configuration
 * @NET_LWIP_ADDR_NONE: Temporarily own the shared interface configuration and
 *	start without an IPv4 address. This mode can coexist only with flexible
 *	clients
 */
enum net_lwip_addr_mode {
	NET_LWIP_ADDR_ENV_STRICT,
	NET_LWIP_ADDR_ENV_FLEXIBLE,
	NET_LWIP_ADDR_NONE,
};

/**
 * struct net_lwip_ctx - Attachment to the shared lwIP runtime
 * @dev: Ethernet device used by the runtime
 * @netif: Shared lwIP network interface
 * @addr_mode: Address mode requested by this client
 *
 * Clients must zero-initialize this structure before passing it to
 * net_lwip_start(). Multiple active clients share @dev and @netif. A client
 * must remove its callbacks and protocol control blocks before calling
 * net_lwip_stop().
 */
struct net_lwip_ctx {
	struct udevice *dev;
	struct netif *netif;
	enum net_lwip_addr_mode addr_mode;
};

/* HTTPS authentication mode */
enum auth_mode {
	AUTH_NONE,
	AUTH_OPTIONAL,
	AUTH_REQUIRED,
};

extern char *cacert;
extern size_t cacert_size;
extern enum auth_mode cacert_auth_mode;
extern bool cacert_initialized;

extern int net_try_count;

int set_cacert_builtin(void);

enum proto_t {
	TFTPGET
};

static inline void net_set_server_ip(struct in_addr ip)
{
	/* lwIP reads serverip from the environment when starting a command. */
}

static inline int eth_is_on_demand_init(void)
{
	return 1;
}

int eth_init_state_only(void); /* Set active state */

int net_lwip_parse_decimal(const char *text, size_t len, unsigned int max,
			   unsigned int *value);
int net_lwip_dns_init(struct net_lwip_ctx *ctx);
int net_lwip_start(struct net_lwip_ctx *ctx,
		   enum net_lwip_addr_mode addr_mode);
void net_lwip_stop(struct net_lwip_ctx *ctx);
int net_lwip_restart(struct net_lwip_ctx *ctx);
int net_lwip_refresh(struct net_lwip_ctx *ctx);
int net_lwip_poll(void);
int net_lwip_resolve(struct net_lwip_ctx *ctx, const char *name,
		     ip_addr_t *addr, u8 type);
int net_lwip_prepare_addr(struct net_lwip_ctx *ctx, ip_addr_t *addr);
#if CONFIG_IS_ENABLED(DNS)
int net_lwip_dns_refresh(struct net_lwip_ctx *ctx);
int net_lwip_dns_lookup(struct net_lwip_ctx *ctx, const char *name,
			ip_addr_t *addr, u8 type);
#endif
#if CONFIG_IS_ENABLED(IPV6)
void net_lwip_ip6_update_lifetimes(void);
int net_lwip_ip6_aton(const char *text, ip6_addr_t *addr);
int net_lwip_ip6_init(struct net_lwip_ctx *ctx);
int net_lwip_ip6_ready(struct net_lwip_ctx *ctx, ip_addr_t *dest);
int net_lwip_ip6_start(struct net_lwip_ctx *ctx);
int net_lwip_ip6_lease(struct net_lwip_ctx *ctx, const ip6_addr_t *addr,
		       u32 preferred, u32 valid);
int net_lwip_ip6_lease_commit(struct net_lwip_ctx *ctx);
bool net_lwip_is_exclusive(struct net_lwip_ctx *ctx);
int net_lwip_dhcp6_restore(struct net_lwip_ctx *ctx, const ip6_addr_t *addr);
void net_lwip_dhcp6_detach(struct netif *netif);
void net_lwip_dhcp6_poll(void);
int net_lwip_ip6_prepare(struct net_lwip_ctx *ctx, ip_addr_t *dest);
#endif
int net_lwip_tftp_download(struct net_lwip_ctx *net, int argc, char *const argv[]);

/**
 * wget_validate_uri() - varidate the uri
 *
 * @uri:	uri string of target file of wget
 * Return:	true if uri is valid, false if uri is invalid
 */
bool wget_validate_uri(char *uri);

int do_bootp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_dns(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_nfs(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_tftpput(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_tftpsrv(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_wget(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[]);

#endif /* __NET_LWIP_H__ */
