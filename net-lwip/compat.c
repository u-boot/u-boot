// SPDX-License-Identifier: GPL-2.0

/* Copyright (C) 2024 Linaro Ltd. */

#include <linux/types.h>
#include <lwip/inet.h> /* For struct in_addr */
#include <net-lwip.h>

struct in_addr net_ip;
struct in_addr net_server_ip;

const u8 net_bcast_ethaddr[ARP_HLEN];

enum net_loop_state { NLS_UNUSED };
enum net_loop_state net_state;

enum proto_t { P_UNUSED };
int net_loop(enum proto_t proto)
{
	return -1;
}

int net_set_ether(uchar *xet, const uchar *dest_ethaddr, uint prot)
{
	return -1;
}

typedef void thand_f(void);
void net_set_timeout_handler(ulong iv, thand_f *f)
{
}

char net_boot_file_name[1024];
void copy_filename(char *dst, const char *src, int size)
{
}
