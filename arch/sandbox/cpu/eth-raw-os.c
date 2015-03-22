/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <asm/eth-raw-os.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <linux/if_ether.h>
#include <linux/if_packet.h>

int sandbox_eth_raw_os_start(const char *ifname, unsigned char *ethmac,
			    struct eth_sandbox_raw_priv *priv)
{
	struct sockaddr_ll *device;
	struct packet_mreq mr;
	int ret;
	int flags;

	/* Prepare device struct */
	priv->device = malloc(sizeof(struct sockaddr_ll));
	if (priv->device == NULL)
		return -ENOMEM;
	device = priv->device;
	memset(device, 0, sizeof(struct sockaddr_ll));
	device->sll_ifindex = if_nametoindex(ifname);
	device->sll_family = AF_PACKET;
	memcpy(device->sll_addr, ethmac, 6);
	device->sll_halen = htons(6);

	/* Open socket */
	priv->sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (priv->sd < 0) {
		printf("Failed to open socket: %d %s\n", errno,
		       strerror(errno));
		return -errno;
	}
	/* Bind to the specified interface */
	ret = setsockopt(priv->sd, SOL_SOCKET, SO_BINDTODEVICE, ifname,
		   strlen(ifname) + 1);
	if (ret < 0) {
		printf("Failed to bind to '%s': %d %s\n", ifname, errno,
		       strerror(errno));
		return -errno;
	}

	/* Make the socket non-blocking */
	flags = fcntl(priv->sd, F_GETFL, 0);
	fcntl(priv->sd, F_SETFL, flags | O_NONBLOCK);

	/* Enable promiscuous mode to receive responses meant for us */
	mr.mr_ifindex = device->sll_ifindex;
	mr.mr_type = PACKET_MR_PROMISC;
	ret = setsockopt(priv->sd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
		   &mr, sizeof(mr));
	if (ret < 0) {
		struct ifreq ifr;

		printf("Failed to set promiscuous mode: %d %s\n"
		       "Falling back to the old \"flags\" way...\n",
			errno, strerror(errno));
		strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
		if (ioctl(priv->sd, SIOCGIFFLAGS, &ifr) < 0) {
			printf("Failed to read flags: %d %s\n", errno,
			       strerror(errno));
			return -errno;
		}
		ifr.ifr_flags |= IFF_PROMISC;
		if (ioctl(priv->sd, SIOCSIFFLAGS, &ifr) < 0) {
			printf("Failed to write flags: %d %s\n", errno,
			       strerror(errno));
			return -errno;
		}
	}
	return 0;
}

int sandbox_eth_raw_os_send(void *packet, int length,
			    const struct eth_sandbox_raw_priv *priv)
{
	int retval;

	if (!priv->sd || !priv->device)
		return -EINVAL;

	retval = sendto(priv->sd, packet, length, 0,
			(struct sockaddr *)priv->device,
			sizeof(struct sockaddr_ll));
	if (retval < 0) {
		printf("Failed to send packet: %d %s\n", errno,
		       strerror(errno));
		return -errno;
	}
	return retval;
}

int sandbox_eth_raw_os_recv(void *packet, int *length,
			    const struct eth_sandbox_raw_priv *priv)
{
	int retval;
	int saddr_size;

	if (!priv->sd || !priv->device)
		return -EINVAL;
	saddr_size = sizeof(struct sockaddr);
	retval = recvfrom(priv->sd, packet, 1536, 0,
			  (struct sockaddr *)priv->device,
			  (socklen_t *)&saddr_size);
	*length = 0;
	if (retval >= 0) {
		*length = retval;
		return 0;
	}
	/* The socket is non-blocking, so expect EAGAIN when there is no data */
	if (errno == EAGAIN)
		return 0;
	return -errno;
}

void sandbox_eth_raw_os_stop(struct eth_sandbox_raw_priv *priv)
{
	free(priv->device);
	priv->device = NULL;
	close(priv->sd);
	priv->sd = -1;
}
