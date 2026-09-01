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

static inline int eth_is_on_demand_init(void)
{
	return 1;
}

int eth_init_state_only(void); /* Set active state */

int net_lwip_dns_init(void);
int net_lwip_start(struct net_lwip_ctx *ctx,
		   enum net_lwip_addr_mode addr_mode);
void net_lwip_stop(struct net_lwip_ctx *ctx);
int net_lwip_restart(struct net_lwip_ctx *ctx);
int net_lwip_refresh(struct net_lwip_ctx *ctx);
int net_lwip_poll(void);
int net_lwip_dns_resolve(char *name_or_ip, ip_addr_t *ip);

/**
 * wget_validate_uri() - varidate the uri
 *
 * @uri:	uri string of target file of wget
 * Return:	true if uri is valid, false if uri is invalid
 */
bool wget_validate_uri(char *uri);

int do_dns(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_nfs(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_tftpsrv(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_wget(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[]);

#endif /* __NET_LWIP_H__ */
