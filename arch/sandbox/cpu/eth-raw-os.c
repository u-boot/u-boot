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
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

static int _raw_packet_start(const char *ifname, unsigned char *ethmac,
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

static int _local_inet_start(struct eth_sandbox_raw_priv *priv)
{
	struct sockaddr_in *device;
	int ret;
	int flags;
	int one = 1;

	/* Prepare device struct */
	priv->device = malloc(sizeof(struct sockaddr_in));
	if (priv->device == NULL)
		return -ENOMEM;
	device = priv->device;
	memset(device, 0, sizeof(struct sockaddr_in));
	device->sin_family = AF_INET;
	device->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	/**
	 * Open socket
	 *  Since we specify UDP here, any incoming ICMP packets will
	 *  not be received, so things like ping will not work on this
	 *  localhost interface.
	 */
	priv->sd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (priv->sd < 0) {
		printf("Failed to open socket: %d %s\n", errno,
		       strerror(errno));
		return -errno;
	}

	/* Make the socket non-blocking */
	flags = fcntl(priv->sd, F_GETFL, 0);
	fcntl(priv->sd, F_SETFL, flags | O_NONBLOCK);

	/* Include the UDP/IP headers on send and receive */
	ret = setsockopt(priv->sd, IPPROTO_IP, IP_HDRINCL, &one,
			 sizeof(one));
	if (ret < 0) {
		printf("Failed to set header include option: %d %s\n", errno,
		       strerror(errno));
		return -errno;
	}
	priv->local_bind_sd = -1;
	priv->local_bind_udp_port = 0;
	return 0;
}

int sandbox_eth_raw_os_start(const char *ifname, unsigned char *ethmac,
			    struct eth_sandbox_raw_priv *priv)
{
	if (priv->local)
		return _local_inet_start(priv);
	else
		return _raw_packet_start(ifname, ethmac, priv);
}

int sandbox_eth_raw_os_send(void *packet, int length,
			    struct eth_sandbox_raw_priv *priv)
{
	int retval;
	struct udphdr *udph = packet + sizeof(struct iphdr);

	if (!priv->sd || !priv->device)
		return -EINVAL;

	/*
	 * This block of code came about when testing tftp on the localhost
	 * interface. When using the RAW AF_INET API, the network stack is still
	 * in play responding to incoming traffic based on open "ports". Since
	 * it is raw (at the IP layer, no Ethernet) the network stack tells the
	 * TFTP server that the port it responded to is closed. This causes the
	 * TFTP transfer to be aborted. This block of code inspects the outgoing
	 * packet as formulated by the u-boot network stack to determine the
	 * source port (that the TFTP server will send packets back to) and
	 * opens a typical UDP socket on that port, thus preventing the network
	 * stack from sending that ICMP message claiming that the port has no
	 * bound socket.
	 */
	if (priv->local && (priv->local_bind_sd == -1 ||
			    priv->local_bind_udp_port != udph->source)) {
		struct iphdr *iph = packet;
		struct sockaddr_in addr;

		if (priv->local_bind_sd != -1)
			close(priv->local_bind_sd);

		/* A normal UDP socket is required to bind */
		priv->local_bind_sd = socket(AF_INET, SOCK_DGRAM, 0);
		if (priv->local_bind_sd < 0) {
			printf("Failed to open bind sd: %d %s\n", errno,
			       strerror(errno));
			return -errno;
		}
		priv->local_bind_udp_port = udph->source;

		/**
		 * Bind the UDP port that we intend to use as our source port
		 * so that the kernel will not send an ICMP port unreachable
		 * message to the server
		 */
		addr.sin_family = AF_INET;
		addr.sin_port = udph->source;
		addr.sin_addr.s_addr = iph->saddr;
		retval = bind(priv->local_bind_sd, &addr, sizeof(addr));
		if (retval < 0)
			printf("Failed to bind: %d %s\n", errno,
			       strerror(errno));
	}

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
	if (priv->local) {
		if (priv->local_bind_sd != -1)
			close(priv->local_bind_sd);
		priv->local_bind_sd = -1;
		priv->local_bind_udp_port = 0;
	}
}
