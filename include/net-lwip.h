/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __NET_LWIP_H__
#define __NET_LWIP_H__

#include <lwip/ip4.h>
#include <lwip/netif.h>

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
int net_lwip_eth_start(void);
struct netif *net_lwip_new_netif(struct udevice *udev);
struct netif *net_lwip_new_netif_noip(struct udevice *udev);
void net_lwip_remove_netif(struct netif *netif);
struct netif *net_lwip_get_netif(void);
int net_lwip_rx(struct udevice *udev, struct netif *netif);
int net_lwip_dns_resolve(char *name_or_ip, ip_addr_t *ip);

/**
 * wget_validate_uri() - varidate the uri
 *
 * @uri:	uri string of target file of wget
 * Return:	true if uri is valid, false if uri is invalid
 */
bool wget_validate_uri(char *uri);

int do_dhcp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_dns(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);
int do_wget(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[]);

#endif /* __NET_LWIP_H__ */
