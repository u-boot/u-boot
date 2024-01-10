/* SPDX-License-Identifier: GPL-2.0+ */

/* ARP hardware address length */
#define ARP_HLEN 6
/*
 * The size of a MAC address in string form, each digit requires two chars
 * and five separator characters to form '00:00:00:00:00:00'.
 */
#define ARP_HLEN_ASCII (ARP_HLEN * 2) + (ARP_HLEN - 1)
